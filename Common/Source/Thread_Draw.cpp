/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "LKInterface.h"
#include "Bitmaps.h"
#include "RGB.h"
#include "TraceThread.h"

#ifndef USE_GDI
#include "Screen/Canvas.hpp"
#endif

BOOL MapWindow::CLOSETHREAD = FALSE;
BOOL MapWindow::THREADRUNNING = TRUE;
BOOL MapWindow::THREADEXIT = FALSE;
BOOL MapWindow::Initialised = FALSE;

Poco::FastMutex MapWindow::Surface_Mutex;

extern bool PanRefreshed;
bool ForceRenderMap=true;

void MapWindow::DrawThread ()
{
  while ((!ProgramStarted) || (!Initialised)) {
	Poco::Thread::sleep(50);
  }

  #if TRACETHREAD
  StartupStore(_T("##############  DRAW threadid=%d\n"),GetCurrentThreadId());
  #endif


  // THREADRUNNING = FALSE;
  THREADEXIT = FALSE;

  // Reset common topology and waypoint label declutter, first init. Done also in other places.
  ResetLabelDeclutter();

  MapRect = MainWindow.GetClientRect();
  // Default draw area is full screen, no opacity
  DrawRect=MapRect;

  UpdateTimeStats(true);

  { // // Begin Critical section
    Poco::FastMutex::ScopedLock Lock(Surface_Mutex);

    DrawSurface.SetBackgroundTransparent();
    hdcMask.SetBackgroundOpaque();

    // paint draw window black to start
    DrawSurface.SelectObject(LK_BLACK_PEN);
    DrawSurface.Rectangle(MapRect.left,MapRect.top, MapRect.right,MapRect.bottom);

    BackBufferSurface.Copy(0, 0, MapRect.right-MapRect.left,
           MapRect.bottom-MapRect.top, 
           DrawSurface, 0, 0);
  } // End Critical section
  
  // This is just here to give fully rendered start screen
  UpdateInfo(&GPS_INFO, &CALCULATED_INFO);
  MapDirty = true;
  UpdateTimeStats(true);

  zoom.RequestedScale(zoom.Scale());
  zoom.ModifyMapScale();
  FillScaleListForEngineeringUnits();
  
  bool lastdrawwasbitblitted=false;
  bool first_run=true;

  // 
  // Big LOOP
  //

  while (!CLOSETHREAD) 
  {
	if(drawTriggerEvent.tryWait(5000)) drawTriggerEvent.reset();
	if (CLOSETHREAD) break; // drop out without drawing

	if ((!THREADRUNNING) || (!GlobalRunning)) {
		Poco::Thread::sleep(50);
		continue;
	}
    
#ifdef HAVE_CPU_FREQUENCY
    const ScopeLockCPU cpu;
#endif
  
    Poco::FastMutex::ScopedLock Lock(Surface_Mutex);
	// This is also occuring on resolution change
	if (LKSW_ReloadProfileBitmaps) {
		#if TESTBENCH
		StartupStore(_T(".... SWITCH: ReloadProfileBitmaps detected\n"));
		#endif
		// This is needed to update resolution change
		MapRect = MainWindow.GetClientRect();
		DrawRect=MapRect;
		FillScaleListForEngineeringUnits();
		LKUnloadProfileBitmaps();
		LKLoadProfileBitmaps();
		LKUnloadFixedBitmaps();
		LKLoadFixedBitmaps();
		MapWindow::zoom.Reset();

		// This will reset the function for the new ScreenScale
		PolygonRotateShift((POINT*)NULL,0,0,0,DisplayAngle+1);

		// Restart from moving map
		if (MapSpaceMode!=MSM_WELCOME) SetModeType(LKMODE_MAP, MP_MOVING);

		LKSW_ReloadProfileBitmaps=false;
		// These should be better checked. first_run is forcing also cache update for topo.
		ForceRenderMap=true;
		first_run=true;
	}



	// Until MapDirty is set true again, we shall only repaint the screen. No Render, no calculations, no updates.
	// This is intended for very fast immediate screen refresh.
	//
	// MapDirty is set true by:
	//   - TriggerRedraws()  in calculations thread
	//   - RefreshMap()      in drawthread generally
	//


	extern POINT startScreen, targetScreen;
	extern bool OnFastPanning;
	// While we are moving in bitblt mode, ignore RefreshMap requests from LK
	// unless a timeout was triggered by MapWndProc itself.
	if (OnFastPanning) {
		MapDirty=false;
	} 

	// We must check if we are on FastPanning, because we may be in pan mode even while
	// the menu buttons are active and we are using them, accessing other functions.
	// In that case, without checking OnFastPanning, we would fall back here and repaint
	// with bitblt everytime, while instead we were asked a simple fastrefresh!
	//
	// Notice: we could be !MapDirty without OnFastPanning, of course!
	//
	if (!MapDirty && !ForceRenderMap && OnFastPanning && !first_run) {
		
		if (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN)) {

			const int fromX=startScreen.x-targetScreen.x;
			const int fromY=startScreen.y-targetScreen.y;

			BackBufferSurface.Whiteness(0, 0,MapRect.right-MapRect.left, MapRect.bottom-MapRect.top);

                        RECT  clipSourceArea;
                        POINT clipDestPoint;

                        if (fromX<0) {
                            clipSourceArea.left=MapRect.left;
                            clipSourceArea.right=MapRect.right+fromX; // negative fromX
                            clipDestPoint.x=-fromX;
                        } else {
                            clipSourceArea.left=MapRect.left+fromX;
                            clipSourceArea.right=MapRect.right;
                            clipDestPoint.x=MapRect.left;
                        }

                        if (fromY<0) {
                            clipSourceArea.top=MapRect.top;
                            clipSourceArea.bottom=MapRect.bottom+fromY; // negative fromX
                            clipDestPoint.y=-fromY;
                        } else {
                            clipSourceArea.top=MapRect.top+fromY;
                            clipSourceArea.bottom=MapRect.bottom;
                            clipDestPoint.y=MapRect.top;
                        }


                        BackBufferSurface.Copy(clipDestPoint.x,clipDestPoint.y,
                            clipSourceArea.right-clipSourceArea.left,
                            clipSourceArea.bottom-clipSourceArea.top,
                            DrawSurface, 
                            clipSourceArea.left,clipSourceArea.top);


			POINT centerscreen;
			centerscreen.x=ScreenSizeX/2; centerscreen.y=ScreenSizeY/2;
			DrawMapScale(BackBufferSurface,MapRect,false);
			DrawCrossHairs(BackBufferSurface, centerscreen, MapRect);
			lastdrawwasbitblitted=true;
		} else {
			// THIS IS NOT GOING TO HAPPEN!
			//
			// The map was not dirty, and we are not in fastpanning mode.
			// FastRefresh!  We simply redraw old bitmap. 
			//
			BackBufferSurface.Copy(0, 0, MapRect.right-MapRect.left,
				MapRect.bottom-MapRect.top, 
				DrawSurface, 0, 0);

			lastdrawwasbitblitted=true;
		}

		// Now we can clear the flag. If it was off already, no problems.
		OnFastPanning=false;
                MainWindow.Redraw(MapRect);
		continue;

	} else {
		//
		// Else the map wasy dirty, and we must render it..
		// Notice: if we were fastpanning, than the map could not be dirty.
		//

		#if 1 // --------------------- EXPERIMENTAL, CHECK ZOOM IS WORKING IN PNA
		static double lasthere=0;
		// Only for special case: PAN mode, map not dirty (including requests for zooms!)
		// not in the ForceRenderMap run and last time was a real rendering. THEN, at these conditions,
		// we simply redraw old bitmap, for the scope of accelerating touch response.
		// In fact, if we are panning the map while rendering, there would be an annoying delay.
		// This is using lastdrawwasbitblitted
		if (INPAN && !MapDirty && !lastdrawwasbitblitted && !ForceRenderMap && !first_run) {
			// In any case, after 5 seconds redraw all
			if ( (LKHearthBeats-8) >lasthere ) {
				lasthere=LKHearthBeats;
				goto _dontbitblt;
			}
			BackBufferSurface.Copy(0, 0, MapRect.right-MapRect.left,
				MapRect.bottom-MapRect.top, 
				DrawSurface, 0, 0);

			POINT centerscreen;
			centerscreen.x=ScreenSizeX/2; centerscreen.y=ScreenSizeY/2;
			DrawMapScale(BackBufferSurface,MapRect,false);
			DrawCrossHairs(BackBufferSurface, centerscreen, MapRect);
            MainWindow.Redraw(MapRect);
			continue;
		} 
		#endif // --------------------------
_dontbitblt:
		MapDirty = false;
		PanRefreshed=true;
	} // MapDirty

	lastdrawwasbitblitted=false;
	MapWindow::UpdateInfo(&GPS_INFO, &CALCULATED_INFO);

	RenderMapWindow(DrawSurface, MapRect);
    
	if (!ForceRenderMap && !first_run) {
		BackBufferSurface.Copy(0, 0,
			MapRect.right-MapRect.left,
			MapRect.bottom-MapRect.top, 
			DrawSurface, 0, 0);

	}

	// Draw cross sight for pan mode, in the screen center, 
	// after a full repaint while not fastpanning
	if (mode.AnyPan() && !mode.Is(Mode::MODE_TARGET_PAN) && !OnFastPanning) {
		POINT centerscreen;
		centerscreen.x=ScreenSizeX/2; centerscreen.y=ScreenSizeY/2;
		DrawMapScale(BackBufferSurface,MapRect,false);
		DrawCompass(BackBufferSurface, MapRect, DisplayAngle);
		DrawCrossHairs(BackBufferSurface, centerscreen, MapRect);
	}

	UpdateTimeStats(false);

	// we do caching after screen update, to minimise perceived delay
	// UpdateCaches is updating topology bounds when either forced (only here)
	// or because MapWindow::ForceVisibilityScan  is set true.
	UpdateCaches(first_run);
	first_run=false;

	ForceRenderMap = false;

	if (ProgramStarted==psInitDone) {
		ProgramStarted = psFirstDrawDone;
	}
    MainWindow.Redraw(MapRect);

  } // Big LOOP

  #if TESTBENCH
  StartupStore(_T("... Thread_Draw terminated\n"));
  #endif
  THREADEXIT = TRUE;

}

Poco::ThreadTarget MapWindow::MapWindowThreadRun(MapWindow::DrawThread);
Poco::Thread MapWindowThread;

void MapWindow::CreateDrawingThread(void)
{
  CLOSETHREAD = FALSE;
  THREADEXIT = FALSE;
  MapWindowThread.start(MapWindowThreadRun);
  MapWindowThread.setPriority(Poco::Thread::PRIO_NORMAL);
}

void MapWindow::SuspendDrawingThread(void)
{
  LockTerrainDataGraphics();
  THREADRUNNING = FALSE;
  UnlockTerrainDataGraphics();
  //  SuspendThread(hDrawThread);
}



void MapWindow::ResumeDrawingThread(void)
{
  LockTerrainDataGraphics();
  THREADRUNNING = TRUE;
  UnlockTerrainDataGraphics();
  //  ResumeThread(hDrawThread);
}



void MapWindow::CloseDrawingThread(void)
{
  #if TESTBENCH
  StartupStore(_T("... CloseDrawingThread started\n"));
  #endif
  CLOSETHREAD = TRUE;
  drawTriggerEvent.set(); // wake self up
  LockTerrainDataGraphics();
  SuspendDrawingThread();
  UnlockTerrainDataGraphics();
  
  #if TESTBENCH
  StartupStore(_T("... CloseDrawingThread waitforsingleobject\n"));
  #endif
  drawTriggerEvent.reset();
  MapWindowThread.join();
          
  #if TESTBENCH
  StartupStore(_T("... CloseDrawingThread wait THREADEXIT\n"));
  #endif
  while(!THREADEXIT) { Poco::Thread::sleep(50); };
  #if TESTBENCH
  StartupStore(_T("... CloseDrawingThread finished\n"));
  #endif
}

//
// Change resolution of terrain,topology,airspace
// 
bool MapWindow::ChangeDrawRect(const RECT rectarea)
{
  LKASSERT(rectarea.right>0);
  LKASSERT(rectarea.bottom>0);
  // Passing an invalid area will be checked also later, and managed.
  DrawRect=rectarea;
  return true;
}


#if 0
// This is for exhaustive testing of Renderterrain init/deinit.
void TestChangeRect(void) {

  static unsigned short flipper=2;
  static bool testflip=false;
  RECT testRect={0,0,460,200};
  if (--flipper==0) {
	if (testflip)
		MapWindow::ChangeDrawRect(MapWindow::MapRect);
  	else
		MapWindow::ChangeDrawRect(testRect);
	testflip=!testflip;
	flipper=2;
	MapWindow::RefreshMap();
  }
}
#endif

