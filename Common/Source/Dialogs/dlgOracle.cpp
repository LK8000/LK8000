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
#include "Sound/Sound.h"
#include "Topology.h"

static WndForm *wf=NULL;

extern void ResetNearestTopology();

static CallBackTableEntry_t CallBackTable[]={
  EndCallBackEntry()
};

short WaitToCallForce=0;

//#define DEBUG_ORTIMER

WhereAmI _WhereAmI;

// Remember that this function is called at 2hz
static bool OnTimerNotify()
{
    if(!_WhereAmI.IsDone()) {
        return false;
    }

  wf->SetTimerNotify(0, NULL);

  // Bell, and print results
  LKSound(TEXT("LK_GREEN.WAV"));
  MessageBoxX(_WhereAmI.getText(), gettext(_T("_@M1690_")), mbOk, true);

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

  _WhereAmI.Start();
  

  // We must wait for data ready, so we shall do it  with timer notify.
  wf->SetTimerNotify(100, OnTimerNotify);
  wf->ShowModal();

  delete wf;
  wf = NULL;
}


