/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgOracle.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "TraceThread.h"
#include "WindowControls.h"
#include "dlgTools.h"

static WndForm *wf=NULL;
extern void WhereAmI(void);

extern void ResetNearestTopology();

static CallBackTableEntry_t CallBackTable[]={
  EndCallBackEntry()
};

short WaitToCallForce=0;

//#define DEBUG_ORTIMER

// Remember that this function is called at 2hz
static bool OnTimerNotify()
{
  static short rezoom=0;
  static short limiter=0; // safe limiter
  #define ORCREZOOM 2

  // We wait for some time before forcing nearest search, because we need to be sure
  // that the visibility scan is updated
  if (--WaitToCallForce>0) {
	if (WaitToCallForce==1) {
		#ifdef DEBUG_ORTIMER
		StartupStore(_T("..... Force Calculation\n"));
		#endif
		MapWindow::RefreshMap(); // trigger rescan or we shall wait for 2 seconds more!
		LKSW_ForceNearestTopologyCalculation=true;
		// set wait times for zoom effective in the background and return
		rezoom=ORCREZOOM; 
		limiter=0;
	}
	#ifdef DEBUG_ORTIMER
	else {
		StartupStore(_T("..... WaitToCallForce\n"));
	}
	#endif
	return true;
  }

  // If the Nearest topology calculation is still running we wait
  if (LKSW_ForceNearestTopologyCalculation) {
	if (++limiter > 20) {
		// Something is wrong, lets get out of here because there is no exit button!
		StartupStore(_T("...... Oracle ForceNearestTopo limit exceeded, aborting search%s"),NEWLINE);
		LKSW_ForceNearestTopologyCalculation=false; // time exceeded, reset forced
		limiter=0;
		rezoom=0;
		goto _end;
	}
	#ifdef DEBUG_ORTIMER
	StartupStore(_T("..... Wait for ForceNearest done\n"));
	#endif
	return true;
  }

  limiter=0; // probably useless

  // Ok the ForceNearestTopo was cleared by Terrain search, so we have the nearesttopo items ready.
  // Do we need still to rezoom for letting whereami search for nearest waypoints in a visibility
  // screen range?
  if (rezoom>0) {
	// Extend a wide search of farvisible items 
	// We do it now that nearestTopology has been calculated,
	// because WhereAmI will issue some Nearest search on waypoints
	// and we need to include airports a bit more far away
	if (rezoom==ORCREZOOM) {
		#ifdef DEBUG_ORTIMER
		StartupStore(_T("..... REZOOM\n"));
		#endif
		MapWindow::zoom.EventSetZoom(6);
		MapWindow::ForceVisibilityScan=true;
		MapWindow::RefreshMap();
	} 
	#ifdef DEBUG_ORTIMER
	else StartupStore(_T("..... WAIT AFTER REZOOM\n"));
	#endif
	// rezoom once, then wait
	rezoom--;
	return true;
  }

_end:
  #ifdef DEBUG_ORTIMER
  StartupStore(_T("..... LETS GO\n"));
  #endif
  // Dont come back here anymore!
  wf->SetTimerNotify(0, NULL);

  // Bell, and print results
  LKSound(TEXT("LK_GREEN.WAV"));
  WhereAmI();

  // Remember to force exit from showmodal, because there is no Close button
  wf->SetModalResult(mrOK);
  return true;
}


void dlgOracleShowModal(void){

  SHOWTHREAD(_T("dlgOracleShowModal"));

  wf=NULL;
 
  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgOracle_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, filename, TEXT("IDR_XML_ORACLE_L"));
  } else  {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgOracle.xml"));
    wf = dlgLoadFromXML(CallBackTable, filename, TEXT("IDR_XML_ORACLE"));
  }

  if (!wf) return;

  // Since the topology search is made in the cache, and the cache has only items that
  // are ok to be printed for the current scale, and we want also items for high zoom,
  // we force high zoom and refresh. First we save current scale, of course.
  double oldzoom=MapWindow::zoom.Scale();

  // set a zoom level for topology visibility scan
  MapWindow::zoom.EventSetZoom(3);
  MapWindow::RefreshMap(); 

  // Make the current nearest invalid
  ResetNearestTopology();

  WaitToCallForce=2;

  // We must wait for data ready, so we shall do it  with timer notify.
  wf->SetTimerNotify(500, OnTimerNotify);
  wf->ShowModal();

  delete wf;
  wf = NULL;
  // Now we restore old zoom
  MapWindow::zoom.EventSetZoom(oldzoom);
  // And force rescan of topology in the cache
  MapWindow::ForceVisibilityScan=true;
  MapWindow::RefreshMap();

}


