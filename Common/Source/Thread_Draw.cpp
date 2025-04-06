/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "LKInterface.h"
#include "Bitmaps.h"
#include "RGB.h"
#include "Hardware/CPU.hpp"
#include "Draw/ScreenProjection.h"
#include "OS/Sleep.h"
#ifndef USE_GDI
#include "Screen/Canvas.hpp"
#endif

BOOL MapWindow::CLOSETHREAD = FALSE;
BOOL MapWindow::THREADEXIT = FALSE;
BOOL MapWindow::Initialised = FALSE;
atomic_shared_flag MapWindow::ThreadSuspended;


#ifndef ENABLE_OPENGL
Mutex MapWindow::Surface_Mutex;
Poco::Event MapWindow::drawTriggerEvent;

LKBitmapSurface MapWindow::BackBufferSurface;
Mutex MapWindow::BackBuffer_Mutex;
LKBitmapSurface MapWindow::DrawSurface;
#else
LKBitmapSurface MapWindow::BackBufferSurface;
#endif

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

extern bool PanRefreshed;
bool ForceRenderMap=true;
bool first_run=true;

void MapWindow::Initialize() {
#ifndef ENABLE_OPENGL
    ScopeLock Lock(Surface_Mutex);
#endif
    // Reset common topology and waypoint label declutter, first init. Done also in other places.
    ResetLabelDeclutter();

    // Default draw area is full screen, no opacity
    //MapRect = main_window->GetClientRect(); // not correct

    // This is the same we already do with RenderMapWindowBg when we select the drawing area.
    MapRect = {0, 0, ScreenSizeX, ScreenSizeY};
    DrawRect = MapRect;
    UpdateActiveScreenZone(MapRect);

    UpdateTimeStats();

#ifndef ENABLE_OPENGL
    // paint draw window black to start
    DrawSurface.SetBackgroundTransparent();
	DrawSurface.Blackness(MapRect.left, MapRect.top,MapRect.right-MapRect.left, MapRect.bottom-MapRect.top);

    hdcMask.SetBackgroundOpaque();
    BackBufferSurface.Blackness(MapRect.left, MapRect.top,MapRect.right-MapRect.left, MapRect.bottom-MapRect.top);
#endif

    // This is just here to give fully rendered start screen
    UpdateInfo(GPS_INFO, CALCULATED_INFO);
    MapDirty = true;

    FillScaleListForEngineeringUnits();
    zoom.RequestedScale(zoom.Scale());
    zoom.ModifyMapScale();

    LKUnloadFixedBitmaps();
    LKUnloadProfileBitmaps();

	LKLoadFixedBitmaps();
	LKLoadProfileBitmaps();

	// This will reset the function for the new ScreenScale
	PolygonRotateShift((POINT*)NULL,0,0,0,DisplayAngle+1);

	// These should be better checked. first_run is forcing also cache update for topo.
	ForceRenderMap=true;
	first_run=true;
    // Signal that draw thread can run now
    Initialised = TRUE;
#ifndef ENABLE_OPENGL
    drawTriggerEvent.set();
#endif
}

#ifndef ENABLE_OPENGL

void MapWindow::DrawThread ()
{
  while ((!ProgramStarted) || (!Initialised)) {
	Sleep(50);
  }

  TestLog(_T("... DrawThread START"));

  THREADEXIT = FALSE;

  bool lastdrawwasbitblitted=false;

  //
  // Big LOOP
  //

  while (!CLOSETHREAD)
  {
	if(drawTriggerEvent.tryWait(5000))
	if (CLOSETHREAD) break; // drop out without drawing

	if (ThreadSuspended || (!GlobalRunning)) {
        Sleep(50);
		continue;
	}
    drawTriggerEvent.reset();

#ifdef HAVE_CPU_FREQUENCY
    const ScopeLockCPU cpu;
#endif

    ScopeLock Lock(Surface_Mutex);

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

            PixelRect  clipSourceArea(MapRect); // Source Rectangle
            RasterPoint clipDestPoint(clipSourceArea.GetOrigin()); // destination origin position
            PixelRect  WhiteRectV(MapRect); // vertical White band (left or right)
            PixelRect  WhiteRectH(MapRect); // horizontal White band (top or bottom)

            if (fromX<0) {
                clipSourceArea.right += fromX; // negative fromX
                clipDestPoint.x -= fromX;
                WhiteRectV.right = WhiteRectV.left - fromX;
                WhiteRectH.left = WhiteRectV.right;
            } else {
                clipSourceArea.left += fromX;
                WhiteRectV.left = WhiteRectV.right - fromX;
                WhiteRectH.right = WhiteRectV.left;
            }

            if (fromY<0) {
                clipSourceArea.bottom += fromY; // negative fromX
                clipDestPoint.y -= fromY;
                WhiteRectH.bottom = WhiteRectH.top - fromY;
            } else {
                clipSourceArea.top += fromY;
                WhiteRectH.top = WhiteRectH.bottom - fromY;
            }

            ScopeLock Lock(BackBuffer_Mutex);

            BackBufferSurface.Whiteness(WhiteRectV.left, WhiteRectV.top, WhiteRectV.GetSize().cx, WhiteRectV.GetSize().cy);
            BackBufferSurface.Whiteness(WhiteRectH.left, WhiteRectH.top, WhiteRectH.GetSize().cx, WhiteRectH.GetSize().cy);
            BackBufferSurface.Copy(clipDestPoint.x,clipDestPoint.y,
                clipSourceArea.GetSize().cx,
                clipSourceArea.GetSize().cy,
                DrawSurface,
                clipSourceArea.left,clipSourceArea.top);


			const RasterPoint centerscreen = { ScreenSizeX/2, ScreenSizeY/2 };
            const ScreenProjection _Proj;
			DrawMapScale(BackBufferSurface,MapRect,_Proj);
			DrawCrossHairs(BackBufferSurface, centerscreen, MapRect);
			lastdrawwasbitblitted=true;
		} else {
			// THIS IS NOT GOING TO HAPPEN!
			//
			// The map was not dirty, and we are not in fastpanning mode.
			// FastRefresh!  We simply redraw old bitmap.
			//
            ScopeLock Lock(BackBuffer_Mutex);
            DrawSurface.CopyTo(BackBufferSurface);

			lastdrawwasbitblitted=true;
		}

		// Now we can clear the flag. If it was off already, no problems.
//		OnFastPanning=false;
        main_window->Redraw(MapRect);
		continue;

	} else {
		//
		// Else the map wasy dirty, and we must render it..
		// Notice: if we were fastpanning, than the map could not be dirty.
		//

		#if 1 // --------------------- EXPERIMENTAL, CHECK ZOOM IS WORKING IN PNA
		static unsigned lasthere=0;
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

            ScopeLock Lock(BackBuffer_Mutex);
            DrawSurface.CopyTo(BackBufferSurface);

			const RasterPoint centerscreen = { ScreenSizeX/2, ScreenSizeY/2 };
            const ScreenProjection _Proj;
			DrawMapScale(BackBufferSurface,MapRect,_Proj);
			DrawCrossHairs(BackBufferSurface, centerscreen, MapRect);
            main_window->Redraw(MapRect);
			continue;
		}
		#endif // --------------------------
_dontbitblt:
		MapDirty = false;
		PanRefreshed=true;
	} // MapDirty

	lastdrawwasbitblitted=false;
	MapWindow::UpdateInfo(GPS_INFO, CALCULATED_INFO);
	RenderMapWindow(DrawSurface, MapRect);

    {
        ScopeLock Lock(BackBuffer_Mutex);
        if (!ForceRenderMap && !first_run) {
            DrawSurface.CopyTo(BackBufferSurface);
        }

        // Draw cross sight for pan mode, in the screen center,
        // after a full repaint while not fastpanning
        if (mode.AnyPan() && !mode.Is(Mode::MODE_TARGET_PAN) && !OnFastPanning) {
            POINT centerscreen;
            centerscreen.x=ScreenSizeX/2; centerscreen.y=ScreenSizeY/2;
            const ScreenProjection _Proj;
            DrawMapScale(BackBufferSurface,MapRect,_Proj);
            DrawCompass(BackBufferSurface, MapRect, DisplayAngle);
            DrawCrossHairs(BackBufferSurface, centerscreen, MapRect);
        }
    }

	// we do caching after screen update, to minimise perceived delay
	// UpdateCaches is updating topology bounds when either forced (only here)
	// or because MapWindow::ForceVisibilityScan  is set true.
    const ScreenProjection _Proj;
	UpdateCaches(_Proj, first_run);
	first_run=false;

	ForceRenderMap = false;

	if (ProgramStarted==psInitDone) {
		ProgramStarted = psFirstDrawDone;
	}
    main_window->Redraw(MapRect);

  } // Big LOOP

  TestLog(_T("... Thread_Draw terminated"));
  THREADEXIT = TRUE;

}

class ThreadDraw : public Thread {
public:
    ThreadDraw() : Thread("MapWindow") {}

protected:
    void Run() override {
        MapWindow::DrawThread();
    }
};

ThreadDraw MapWindowThread;
#endif

void MapWindow::CreateDrawingThread(void)
{
  Initialize();

  CLOSETHREAD = FALSE;
  THREADEXIT = FALSE;

#ifndef ENABLE_OPENGL
  MapWindowThread.Start();
#endif
}

void MapWindow::SuspendDrawingThread() {
    ThreadSuspended = true;
}

void MapWindow::ResumeDrawingThread() {
    ThreadSuspended = false;
}

void MapWindow::CloseDrawingThread(void)
{
#ifndef ENABLE_OPENGL
  TestLog(_T("... CloseDrawingThread started"));
  CLOSETHREAD = TRUE;
  drawTriggerEvent.set(); // wake self up
  SuspendDrawingThread();

  TestLog(_T("... CloseDrawingThread waitforsingleobject"));
  #ifdef __linux__
  #else
  drawTriggerEvent.reset(); // on linux this is delaying 5000
  #endif
  MapWindowThread.Join();

  TestLog(_T("... CloseDrawingThread wait THREADEXIT"));
  while(!THREADEXIT) {
    Sleep(50);
  }
  TestLog(_T("... CloseDrawingThread finished"));
#else
  CLOSETHREAD = TRUE;
  THREADEXIT = TRUE;
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
