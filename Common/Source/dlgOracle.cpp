/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include "options.h"
#include "lk8000.h"
#include "externs.h"
#include "dlgTools.h"

static WndForm *wf=NULL;
extern bool ForceNearestTopologyCalculation;
extern void WhereAmI(void);

extern void ResetNearestTopology();

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(NULL)
};


static int OnTimerNotify(WindowControl *Sender)
{
  
  // If the Nearest topology calculation is still running we wait
  if (ForceNearestTopologyCalculation) return 0;

  // Dont come back here anymore!
  wf->SetTimerNotify(NULL);

  // Extend a wide search of farvisible items 
  // We do it now that nearestTopology has been calculated,
  // because WhereAmI will issue some Nearest search on waypoints
  // and we need to include airports a bit more far away
  MapWindow::zoom.EventSetZoom(10);
  MapWindow::ForceVisibilityScan=true;
  MapWindow::RefreshMap();
  Sleep(1000); // We need a cycle to recalculate it

  // Bell, and print results
  if (EnableSoundModes) LKSound(TEXT("LK_BELL.WAV"));
  WhereAmI();

  // Remember to force exit from showmodal, because there is no Close button
  wf->SetModalResult(mrOK);
  return 0;
}


void dlgOracleShowModal(void){

  wf=NULL;
 
  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgOracle_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_ORACLE_L"));
  } else  {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgOracle.xml"));
    wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_ORACLE"));
  }

  if (!wf) return;

  // Since the topology search is made in the cache, and the cache has only items that
  // are ok to be printed for the current scale, and we want also items for high zoom,
  // we force high zoom and refresh. First we save current scale, of course.
  double oldzoom=MapWindow::zoom.Scale();

  // this 2 km does not grant us that all topology items are really included because
  // the user may have disabled some. In this case, we only use the topology that 
  // is available.  Cant do better than this by now. 
  MapWindow::zoom.EventSetZoom(2);
  MapWindow::RefreshMap(); // maybe fastrefresh is better

  // Make the current nearest invalid
  ResetNearestTopology();

  // We set the flag to start nearest topology calculation
  ForceNearestTopologyCalculation=true;

  // We must wait for data ready, so we shall do it  with timer notify.
  wf->SetTimerNotify(OnTimerNotify);
  wf->ShowModal();

  delete wf;
  wf = NULL;

  // Now we restore old zoom
  MapWindow::zoom.EventSetZoom(oldzoom);
  // And force rescan of topology in the cache
  MapWindow::ForceVisibilityScan=true;
  MapWindow::RefreshMap();

}


