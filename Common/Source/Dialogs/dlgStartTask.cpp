/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStartTask.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "LKProcess.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"
#include "Library/TimeFunctions.h"


static WndForm *wf=NULL;

bool startIsValid = false;

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void StartTaskAnyway(bool valid) {
  startIsValid = valid;
}

static void OnStartTaskAnywayClicked(WndButton* pWnd) {
    StartTaskAnyway(true);
    wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnStartTaskAnywayClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};

void dlgStartTaskShowModal(bool *validStart, double Time, double Speed, double Altitude){

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_STARTTASK);

  if (wf) {
    WndProperty* wp;

    TCHAR Temp[80];

    wp = (WndProperty*)wf->FindByName(TEXT("prpTime"));
    if (wp) {
      Units::TimeToText(Temp, LocalTime(Time));
      wp->SetText(Temp);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpSpeed"));
    if (wp) {
      _stprintf(Temp, TEXT("%.0f %s"),
                   Units::ToTaskSpeed(Speed),
                   Units::GetTaskSpeedName());
      wp->SetText(Temp);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpAltitude"));
    if (wp) {
      _stprintf(Temp, TEXT("%.0f %s"),
                Units::ToAltitude(Altitude),
                Units::GetAltitudeName());
      wp->SetText(Temp);
    }

    wf->ShowModal();

    delete wf;
  }
  wf = NULL;

  *validStart = startIsValid;
}
