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
  
  bool first = true;

  for (int i=0; i<AIRSPACECLASSCOUNT; i++) {
    hAirspacePens[i] =
      CreatePen(PS_SOLID, NIBLSCALE(2), Colours[iAirspaceColour[i]]);
  }
  hAirspaceBorderPen = CreatePen(PS_SOLID, NIBLSCALE(10), RGB_WHITE);

  while (!CLOSETHREAD) 
    {
      WaitForSingleObject(drawTriggerEvent, 5000);
      ResetEvent(drawTriggerEvent);
      if (CLOSETHREAD) break; // drop out without drawing

      if ((!THREADRUNNING) || (!GlobalRunning)) {
	Sleep(100);
	continue;
      }

      if (LKSW_ReloadProfileBitmaps) {
	#if TESTBENCH
	StartupStore(_T(".... SWITCH: ReloadProfileBitmaps detected\n"));
	#endif
	LKUnloadProfileBitmaps();
	LKLoadProfileBitmaps();
	LKSW_ReloadProfileBitmaps=false;
      }

#ifdef CPUSTATS
	GetThreadTimes( hDrawThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
#endif

      // MapDirty was triggered mainly by gpsupdated
      if (!MapDirty && !first) {
	// redraw old screen, must have been a request for fast refresh
	BitBlt(hdcScreen, 0, 0, MapRect.right-MapRect.left,
	       MapRect.bottom-MapRect.top, 
	       hdcDrawWindow, 0, 0, SRCCOPY);
	continue;
      } else {
	MapDirty = false;
      }

      MapWindow::UpdateInfo(&GPS_INFO, &CALCULATED_INFO);

      RenderMapWindow(MapRect);
    
      if (!first) {
	BitBlt(hdcScreen, 0, 0, 
	       MapRect.right-MapRect.left,
	       MapRect.bottom-MapRect.top, 
	       hdcDrawWindow, 0, 0, SRCCOPY);
	InvalidateRect(hWndMapWindow, &MapRect, false);
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


