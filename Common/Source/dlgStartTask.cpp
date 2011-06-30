/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#include "externs.h"
#include "Units.h"
#include "dlgTools.h"
#ifdef OLDPPC
#include "LK8000Process.h"
#else
#include "Process.h"
#endif

#include "utils/heapcheck.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;

bool startIsValid = false;

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static void StartTaskAnyway(bool valid) {
  startIsValid = valid;
}


static void OnStartTaskAnywayClicked(WindowControl * Sender){
	(void)Sender;
        StartTaskAnyway(true);
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnStartTaskAnywayClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

void dlgStartTaskShowModal(bool *validStart, double Time, double Speed, double Altitude){

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgStartTask.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
		      
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_STARTTASK"));

  if (wf) {
    WndProperty* wp;

    TCHAR Temp[80];

    wp = (WndProperty*)wf->FindByName(TEXT("prpTime"));
    if (wp) {
      Units::TimeToText(Temp, (int)TimeLocal((int)Time));
      wp->SetText(Temp);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpSpeed"));
    if (wp) {
      _stprintf(Temp, TEXT("%.0f %s"),
                (double) TASKSPEEDMODIFY * Speed, Units::GetTaskSpeedName());
      wp->SetText(Temp);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpAltitude"));
    if (wp) {
      _stprintf(Temp, TEXT("%.0f %s"),
                (double) Altitude*ALTITUDEMODIFY, Units::GetAltitudeName());
      wp->SetText(Temp);
    }

    wf->ShowModal();
    
    delete wf;
  }
  wf = NULL;

  *validStart = startIsValid;
}

