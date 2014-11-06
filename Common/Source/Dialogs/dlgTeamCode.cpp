/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTeamCode.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InfoBoxLayout.h"
#include "Dialogs.h"


static WndForm *wf=NULL;
     

#include "TeamCodeCalculation.h"


static void Update() 
{
  WndProperty* wp;
  TCHAR Text[100];
  double teammateBearing = CALCULATED_INFO.TeammateBearing;
  double teammateRange = CALCULATED_INFO.TeammateRange;

  if((TeamCodeRefWaypoint >=0) && TeammateCodeValid ) {
	double Value = CALCULATED_INFO.TeammateBearing -  GPS_INFO.TrackBearing;
      
	if (Value < -180.0)
		Value += 360.0;
	else
		if (Value > 180.0)
			Value -= 360.0;
      
	if (Value > 1)
		_stprintf(Text, TEXT("%2.0f%s>"), Value, gettext(_T("_@M2179_")));
	else
		if (Value < -1)
			_stprintf(Text, TEXT("<%2.0f%s"), -Value, gettext(_T("_@M2179_")));
		else
			_tcscpy(Text, TEXT("<>"));
      
  } else {
	_tcscpy(Text, TEXT("---"));
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRelBearing"));
  if (wp) {
    wp->SetText(Text);
    wp->RefreshDisplay();
  } 

  wp = (WndProperty*)wf->FindByName(TEXT("prpBearing"));
  if (wp) {
	if (TeammateCodeValid) {
		wp->GetDataField()->SetAsFloat(teammateBearing);
	} else {
		wp->GetDataField()->SetAsFloat(0);
	}
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRange"));
  if (wp) {
	if (TeammateCodeValid) 
		wp->GetDataField()->SetAsFloat(teammateRange*DISTANCEMODIFY);
	else
		wp->GetDataField()->SetAsFloat(0);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOwnCode"));
  if (wp) {
    LK_tcsncpy(Text,CALCULATED_INFO.OwnTeamCode,5);
    wp->SetText(Text);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMateCode"));
  if (wp) {
    wp->SetText(TeammateCode);
    wp->RefreshDisplay();
  }
}


static void OnCodeClicked(WindowControl *Sender) 
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

static void OnCloseClicked(WindowControl * Sender)
{
  (void)Sender;
  wf->SetModalResult(mrOK);
}

static int OnTimerNotify(WindowControl * Sender) {
  (void)Sender;
  static short i=0;
  if(i++ % 2 == 0) return 0;

  Update();
  return 0;
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
  wf = NULL;
  TCHAR filename[MAX_PATH];
  if (ScreenLandscape) 
    {
      LocalPathS(filename, TEXT("dlgTeamCode_L.xml"));
      wf = dlgLoadFromXML(CallBackTable, 
			  filename, 
			  TEXT("IDR_XML_TEAMCODE_L"));
      if (!wf) return;
    }
  else
    {
      LocalPathS(filename, TEXT("dlgTeamCode.xml"));
      wf = dlgLoadFromXML(CallBackTable, 
			  filename, 
			  TEXT("IDR_XML_TEAMCODE"));
      if (!wf) return;
    }

  TCHAR sTmp[32] = { 0 };
  if( ValidWayPoint(TeamCodeRefWaypoint)) {
	// LKTOKEN _@M1230_ "Team Ref.: "
	LK_tcsncpy(sTmp, gettext(TEXT("_@M1230_")), 20);
	_tcsncat(sTmp, WayPointList[TeamCodeRefWaypoint].Name,10);
  } else {
	// LKTOKEN _@M1231_ "Team code: SET REF!"
	LK_tcsncpy(sTmp, gettext(TEXT("_@M1231_")), 30);
  }
  wf->SetCaption(sTmp);

  // set event for button
  buttonCode = ((WndButton *)wf->FindByName(TEXT("cmdSetCode")));
  if (buttonCode) {
    buttonCode->SetOnClickNotify(OnCodeClicked);
  }  

  // Set unit for range
  wp = (WndProperty*)wf->FindByName(TEXT("prpRange"));
  if (wp) {
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
  }

  Update();

  wf->SetTimerNotify(OnTimerNotify);

  wf->ShowModal();

  delete wf;
  wf=NULL; 

}
