/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include <Message.h>

#include "Bitmaps.h"
#include "RGB.h"

using std::min;
using std::max;
#if (WINDOWSPC>0)
#include <wingdi.h>
#endif


BOOL MapWindow::CLOSETHREAD = FALSE;
BOOL MapWindow::THREADRUNNING = TRUE;
BOOL MapWindow::THREADEXIT = FALSE;
BOOL MapWindow::Initialised = FALSE;

DWORD  MapWindow::dwDrawThreadID;
HANDLE MapWindow::hDrawThread;

#if FASTPAN
extern bool PanRefreshed;
#endif

#ifdef CPUSTATS
extern void DrawCpuStats(HDC hdc, RECT rc );
#endif

extern void Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);

DWORD MapWindow::DrawThread (LPVOID lpvoid)
{

#ifdef CPUSTATS
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;
#endif


  while ((!ProgramStarted) || (!Initialised)) {
    Sleep(100);
  }

  //  THREADRUNNING = FALSE;
  THREADEXIT = FALSE;

  // Reset for topology labels decluttering engine occurs also in another place here!
  nLabelBlocks = 0;
  #if TOPOFASTLABEL
  for (short nvi=0; nvi<SCREENVSLOTS; nvi++) nVLabelBlocks[nvi]=0;
  #endif

  GetClientRect(hWndMapWindow, &MapRect);

  UpdateTimeStats(true);

  
  SetBkMode(hdcDrawWindow,TRANSPARENT);
  SetBkMode(hDCTemp,OPAQUE);
  SetBkMode(hDCMask,OPAQUE);

  // paint draw window black to start
  SelectObject(hdcDrawWindow, GetStockObject(BLACK_PEN));
  Rectangle(hdcDrawWindow,MapRect.left,MapRect.top,
            MapRect.right,MapRect.bottom);

  BitBlt(hdcScreen, 0, 0, MapRect.right-MapRect.left,
         MapRect.bottom-MapRect.top, 
         hdcDrawWindow, 0, 0, SRCCOPY);

  // This is just here to give fully rendered start screen
  UpdateInfo(&GPS_INFO, &CALCULATED_INFO);
  MapDirty = true;
  UpdateTimeStats(true);
  //

  zoom.RequestedScale(zoom.Scale());
  zoom.ModifyMapScale();
  FillScaleListForEngineeringUnits();
  
  bool first = true;

  while (!CLOSETHREAD) 
    {
      WaitForSingleObject(drawTriggerEvent, 5000);
      ResetEvent(drawTriggerEvent);
      if (CLOSETHREAD) break; // drop out without drawing

      if ((!THREADRUNNING) || (!GlobalRunning)) {
	Sleep(100);
	continue;
      }

      // This is also occuring on resolution change
      if (LKSW_ReloadProfileBitmaps) {
	#if TESTBENCH
	StartupStore(_T(".... SWITCH: ReloadProfileBitmaps detected\n"));
	#endif
        // This is needed to update resolution change
        GetClientRect(hWndMapWindow, &MapRect);
	LKUnloadProfileBitmaps();
	LKLoadProfileBitmaps();
	LKSW_ReloadProfileBitmaps=false;
	first=true; // check it
      }

#ifdef CPUSTATS
	GetThreadTimes( hDrawThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
#endif

      // Until MapDirty is set true again, we shall only repaint the screen. No Render, no calculations, no updates.
      // This is intended for very fast immediate screen refresh.
      //
      // MapDirty is set true by:
      //   TriggerRedraws()  in calculations thread
      //   RefreshMap()      in drawthread generally
      //

#if FASTPAN

extern int XstartScreen, YstartScreen, XtargetScreen, YtargetScreen;
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
      if (!MapDirty && !first && OnFastPanning) {
	if (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN)) {

		int fromX=0, fromY=0;

		fromX=XstartScreen-XtargetScreen;
		fromY=YstartScreen-YtargetScreen;

		BitBlt(hdcScreen, 0, 0, MapRect.right-MapRect.left,
		       MapRect.bottom-MapRect.top, 
		       hdcDrawWindow, 0, 0, WHITENESS);


		BitBlt(hdcScreen, 
			0, 0,				// destination 
			MapRect.right-MapRect.left,
			MapRect.bottom-MapRect.top, 
			hdcDrawWindow, 
			fromX,fromY, 				// source
			SRCCOPY);

		POINT centerscreen;
		centerscreen.x=ScreenSizeX/2; centerscreen.y=ScreenSizeY/2;
		DrawCrossHairs(hdcScreen, centerscreen, MapRect);

	} else {
		//
		// The map was not dirty, and we are not in fastpanning mode.
		// FastRefresh!  We simply redraw old bitmap. 
		//
		BitBlt(hdcScreen, 0, 0, MapRect.right-MapRect.left,
		       MapRect.bottom-MapRect.top, 
		       hdcDrawWindow, 0, 0, SRCCOPY);
	}

	// Now we can clear the flag. If it was off already, no problems.
	OnFastPanning=false;
#else
      if (!MapDirty && !first) {
	// redraw old screen, must have been a request for fast refresh
	BitBlt(hdcScreen, 0, 0, MapRect.right-MapRect.left,
	       MapRect.bottom-MapRect.top, 
	       hdcDrawWindow, 0, 0, SRCCOPY);
#endif
	continue;

      } else {
	//
	// Else the map wasy dirty, and we must render it..
	// Notice: if we were fastpanning, than the map could not be dirty.
	//
	MapDirty = false;
#if FASTPAN
	PanRefreshed=true; // faster with no checks
#endif
      } // MapDirty

      MapWindow::UpdateInfo(&GPS_INFO, &CALCULATED_INFO);

      RenderMapWindow(MapRect);
    
      if (!first) {
	BitBlt(hdcScreen, 0, 0, 
	       MapRect.right-MapRect.left,
	       MapRect.bottom-MapRect.top, 
	       hdcDrawWindow, 0, 0, SRCCOPY);
	InvalidateRect(hWndMapWindow, &MapRect, false);
      }

      // Draw cross sight for pan mode, in the screen center, 
      // after a full repaint while not fastpanning
      if (mode.AnyPan() && !mode.Is(Mode::MODE_TARGET_PAN) && !OnFastPanning) {
		POINT centerscreen;
		centerscreen.x=ScreenSizeX/2; centerscreen.y=ScreenSizeY/2;
		DrawCrossHairs(hdcScreen, centerscreen, MapRect);
      }

      UpdateTimeStats(false);

      // we do caching after screen update, to minimise perceived delay
      UpdateCaches(first);
      first = false;
      if (ProgramStarted==psInitDone) {
	ProgramStarted = psFirstDrawDone;

      }
#ifdef CPUSTATS
	if ( (GetThreadTimes( hDrawThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
		Cpu_Draw=9999;
	} else {
		Cpustats(&Cpu_Draw,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
	}
#endif
    
    }
  THREADEXIT = TRUE;
  return 0;
}



void MapWindow::CreateDrawingThread(void)
{
  CLOSETHREAD = FALSE;
  THREADEXIT = FALSE;
  hDrawThread = CreateThread (NULL, 0,  
                              (LPTHREAD_START_ROUTINE )MapWindow::DrawThread, 
                              0, 0, &dwDrawThreadID);
  SetThreadPriority(hDrawThread,THREAD_PRIORITY_NORMAL);
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
  CLOSETHREAD = TRUE;
  SetEvent(drawTriggerEvent); // wake self up
  LockTerrainDataGraphics();
  SuspendDrawingThread();
  UnlockTerrainDataGraphics();
  while(!THREADEXIT) { Sleep(100); };
}


