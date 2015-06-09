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
#include "Hardware/CPU.hpp"

#ifndef USE_GDI
#include "Screen/Canvas.hpp"
#endif

BOOL MapWindow::CLOSETHREAD = FALSE;
BOOL MapWindow::THREADRUNNING = TRUE;
BOOL MapWindow::THREADEXIT = FALSE;
BOOL MapWindow::Initialised = FALSE;

Poco::FastMutex MapWindow::Surface_Mutex;

// #define TESTMAPRECT 1
// Although we are capable of autoresizing, all fonts are tuned for the original screen geometry.
// It is unlikely that we shall do sliding windows by changing MapRect like we do here, because
// this would mean to force a ChangeScreen everytime.
// It is although possible to use only a portion of the screen and leave the rest for example for
// a realtime vario, or for menu buttons, or for text. In such cases, we can assume that the reserved
// portion of screen will be limited to a -say- NIBLSCALE(25) and geometry will not change much.
// Reducing MapRect like we do in the test is not useful, only for checking if we have pending problems.
//
#ifdef TESTMAPRECT
#define TM_T 45
#define TM_B 45
#define TM_L 30
#define TM_R 20
#endif

#ifdef UNGHOST
// When we reach this level we apply the unghosting method.
// Notice that each draw trigger has a default forward step of 10. 
// Every second we redraw something, and we add or subtract 0 to 10.
// So this is the number of "dirty" writes that require unghosting
#define MAX_UNGHOST 180*10
#endif

extern bool PanRefreshed;
bool ForceRenderMap=true;

void MapWindow::DrawThread ()
{
  #ifdef UNGHOST
  static long long int unghost_lasttime=0;
  static Poco::Timestamp StartTime;
  #endif

  while ((!ProgramStarted) || (!Initialised)) {
	Poco::Thread::sleep(50);
  }

  #if TRACETHREAD
  StartupStore(_T("##############  DRAW threadid=%d\n"),GetCurrentThreadId());
  #endif

  #if TESTBENCH
  StartupStore(_T("... DrawThread START%s"),NEWLINE);
  #endif

  // THREADRUNNING = FALSE;
  THREADEXIT = FALSE;

  // Reset common topology and waypoint label declutter, first init. Done also in other places.
  ResetLabelDeclutter();

  MapRect = MainWindow.GetClientRect();
  #ifdef TESTMAPRECT
  MapRect.top+=TM_T;
  MapRect.left+=TM_L;
  MapRect.right-=TM_R;
  MapRect.bottom-=TM_B;
  #endif
  // Default draw area is full screen, no opacity
  DrawRect=MapRect;
  UpdateActiveScreenZone(MapRect);

  UpdateTimeStats(true);

  { // // Begin Critical section
    Poco::FastMutex::ScopedLock Lock(Surface_Mutex);

    DrawSurface.SetBackgroundTransparent();
    hdcMask.SetBackgroundOpaque();

    // paint draw window black to start
    DrawSurface.SelectObject(LK_BLACK_PEN);
    DrawSurface.Rectangle(MapRect.left,MapRect.top, MapRect.right,MapRect.bottom);

    BackBufferSurface.Copy(MapRect.left, MapRect.top, MapRect.right-MapRect.left,
           MapRect.bottom-MapRect.top, 
           DrawSurface, MapRect.left, MapRect.top);
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
                // When we are in dialogs, we are coming here every 5s
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
                #ifdef TESTMAPRECT
                MapRect.top+=TM_T;
                MapRect.left+=TM_L;
                MapRect.right-=TM_R;
                MapRect.bottom-=TM_B;
                #endif
		DrawRect=MapRect;
                UpdateActiveScreenZone(MapRect);
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

			BackBufferSurface.Whiteness(MapRect.left, MapRect.top,MapRect.right-MapRect.left, MapRect.bottom-MapRect.top);

                        RECT  clipSourceArea;
                        POINT clipDestPoint;

                        if (fromX<0) {
                            clipSourceArea.left=MapRect.left;
                            clipSourceArea.right=MapRect.right+fromX; // negative fromX
                            clipDestPoint.x=MapRect.left-fromX;
                        } else {
                            clipSourceArea.left=MapRect.left+fromX;
                            clipSourceArea.right=MapRect.right;
                            clipDestPoint.x=MapRect.left;
                        }

                        if (fromY<0) {
                            clipSourceArea.top=MapRect.top;
                            clipSourceArea.bottom=MapRect.bottom+fromY; // negative fromX
                            clipDestPoint.y=MapRect.top-fromY;
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
			BUGSTOP_LKASSERT(0);
                       
			BackBufferSurface.Copy(MapRect.left, MapRect.top, MapRect.right-MapRect.left,
				MapRect.bottom-MapRect.top, 
				DrawSurface, MapRect.left, MapRect.top);

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
			BackBufferSurface.Copy(MapRect.left, MapRect.top, MapRect.right-MapRect.left,
				MapRect.bottom-MapRect.top, 
				DrawSurface, MapRect.left, MapRect.top);

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
        #ifdef UNGHOST
        // In info pages the ghosting effect is "10" times less important
        if (MapSpaceMode>MSM_MAP && MapSpaceMode<MSM_TRAFFIC)
            Unghost++;
        else
            // Otherwise, we add 10, the unit
            Unghost+=10;
        #endif
    
	if (!ForceRenderMap && !first_run) {

            #ifdef UNGHOST
            unsigned short elapsed= ((StartTime.elapsed() - unghost_lasttime)/1000000);
            bool doit=false;
            // Rules. If we have enough triggers requiring unghosting..
            if (Unghost>=(MAX_UNGHOST) && elapsed>60) 
                doit=true;  // max every minute
            else
                if (Unghost>=(5*10) && elapsed>180)
                    doit=true;  // min every 3 minutes

            if (doit) { 
                BackBufferSurface.InvertRect(MapRect);
                MainWindow.Redraw(MapRect);
                Poco::Thread::sleep(550); // eink framebuffer latency
                unghost_lasttime=StartTime.elapsed();
                Unghost=0; // not thread safe, only if draw thread can change Unghost.
            }

            #endif


            BackBufferSurface.Copy(MapRect.left, MapRect.top,
                MapRect.right-MapRect.left, MapRect.bottom-MapRect.top, 
                DrawSurface, MapRect.left, MapRect.top);

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
    //
    // We force framebuffer redraw, and immediately go back to the loop. We are assuming
    // that the time taken for new map draw will be enough to let framebuffer refresh the
    // screen. Otherwise, we should place after Redraw a delay.
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
  #ifdef __linux__
  #else
  drawTriggerEvent.reset(); // on linux this is delaying 5000
  #endif
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

