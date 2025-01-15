/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTeamCode.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"

static WndForm *wf=NULL;


#include "TeamCodeCalculation.h"


static void Update()
{
  WndProperty* wp;
  TCHAR Text[100];
  double teammateBearing = CALCULATED_INFO.TeammateBearing;
  double teammateRange = CALCULATED_INFO.TeammateRange;


  if(ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid ) {
	double Value = CALCULATED_INFO.TeammateBearing -  GPS_INFO.TrackBearing;

	if (Value < -180.0)
		Value += 360.0;
	else
		if (Value > 180.0)
			Value -= 360.0;

	if (Value > 1)
		_stprintf(Text, TEXT("%2.0f%s>"), Value, MsgToken<2179>());
	else
		if (Value < -1)
			_stprintf(Text, TEXT("<%2.0f%s"), -Value, MsgToken<2179>());
		else
			lk::strcpy(Text, TEXT("<>"));

  } else {
	lk::strcpy(Text, TEXT("---"));
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpRelBearing"));
  if (wp) {
    wp->SetText(Text);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBearing"));
  if (wp) {
	if (TeammateCodeValid) {
		wp->GetDataField()->SetAsFloat(teammateBearing);
	} else {
		wp->GetDataField()->SetAsFloat(0);
	}
	wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpRange"));
  if (wp) {
	if (TeammateCodeValid)
		wp->GetDataField()->SetAsFloat(Units::ToDistance(teammateRange));
	else
		wp->GetDataField()->SetAsFloat(0);
	wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOwnCode"));
  if (wp) {
    LK_tcsncpy(Text,CALCULATED_INFO.OwnTeamCode,5);
    wp->SetText(Text);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMateCode"));
  if (wp) {
    wp->SetText(TeammateCode);
    wp->RefreshDisplay();
  }
}


static void OnCodeClicked(WndButton* pWnd)
{
  TCHAR newTeammateCode[10];

  LK_tcsncpy(newTeammateCode, TeammateCode, 9);
  dlgTextEntryShowModal(newTeammateCode, 7);

  int i= _tcslen(newTeammateCode)-1;
  while (i>=0) {
    if (newTeammateCode[i]!=_T(' '))
      {
	break;
      }
    newTeammateCode[i]=0;
    i--;
  };
  LK_tcsncpy(TeammateCode, newTeammateCode, 9);
  if (_tcslen(TeammateCode)>0) {
	TeammateCodeValid = true;
	OvertargetMode = OVT_MATE;
  } else  {
	TeammateCodeValid = false;
	Update();
  }
}

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static bool OnTimerNotify(WndForm* pWnd) {
  Update();
  return true;
}

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
//  ClickNotifyCallbackEntry(OnFlarmLockClicked),
  EndCallBackEntry()
};


void dlgTeamCodeShowModal(void)
{
  WndProperty* wp = NULL;
  WndButton *buttonCode = NULL;

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_TEAMCODE);
  if (!wf) return;

  TCHAR sTmp[32] = { 0 };
  if( ValidWayPoint(TeamCodeRefWaypoint)) {
    // LKTOKEN _@M1230_ "Team Ref.: "
    _stprintf(sTmp, _T("%s%s"),  MsgToken<1230>(), WayPointList[TeamCodeRefWaypoint].Name);
  } else {
    // LKTOKEN _@M1231_ "Team code: SET REF!"
    LK_tcsncpy(sTmp, MsgToken<1231>(), 30);
  }
  wf->SetCaption(sTmp);

  // set event for button
  buttonCode = (wf->FindByName<WndButton>(TEXT("cmdSetCode")));
  if (buttonCode) {
    buttonCode->SetOnClickNotify(OnCodeClicked);
  }

  // Set unit for range
  wp = wf->FindByName<WndProperty>(TEXT("prpRange"));
  if (wp) {
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
  }

  Update();

  wf->SetTimerNotify(1000, OnTimerNotify);

  wf->ShowModal();

  delete wf;
  wf=NULL;

}
