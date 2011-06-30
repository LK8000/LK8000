/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include "lk8000.h"
#include "Utils.h"
#include "Utils2.h"
#include "dlgTools.h"
#include "externs.h"
#include "InfoBoxLayout.h"

static WndForm *wf=NULL;
     

#include "TeamCodeCalculation.h"

#include "utils/heapcheck.h"

static void Update() 
{
  WndProperty* wp;
  TCHAR Text[100];
  double teammateBearing = CALCULATED_INFO.TeammateBearing;
  double teammateRange = CALCULATED_INFO.TeammateRange;

  if((TeamCodeRefWaypoint >=0)&&(WayPointList) && TeammateCodeValid ) {
	double Value = CALCULATED_INFO.TeammateBearing -  GPS_INFO.TrackBearing;
      
	if (Value < -180.0)
		Value += 360.0;
	else
		if (Value > 180.0)
			Value -= 360.0;
      
	if (Value > 1)
		_stprintf(Text, TEXT("%2.0f")TEXT(DEG)TEXT(">"), Value);
	else
		if (Value < -1)
			_stprintf(Text, TEXT("<%2.0f")TEXT(DEG), -Value);
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
    _tcsncpy(Text,CALCULATED_INFO.OwnTeamCode,5);
    Text[5] = '\0';
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
  _tcsncpy(newTeammateCode, TeammateCode, 10);
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
  _tcsncpy(TeammateCode, newTeammateCode, 10);
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
  Update();
  return 0;
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnTimerNotify),
//  DeclareCallBackEntry(OnFlarmLockClicked),
  DeclareCallBackEntry(NULL)
};


void dlgTeamCodeShowModal(void) 
{
  WndProperty* wp = NULL;
  WndButton *buttonCode = NULL;
  wf = NULL;
  char filename[MAX_PATH];
  if (InfoBoxLayout::landscape) 
    {
      LocalPathS(filename, TEXT("dlgTeamCode_L.xml"));
      wf = dlgLoadFromXML(CallBackTable, 

			  filename, 
			  hWndMainWindow,
			  TEXT("IDR_XML_TEAMCODE_L"));
      if (!wf) return;
    }
  else
    {
      LocalPathS(filename, TEXT("dlgTeamCode.xml"));
      wf = dlgLoadFromXML(CallBackTable, 

			  filename, 
			  hWndMainWindow,
			  TEXT("IDR_XML_TEAMCODE"));
      if (!wf) return;
    }

  TCHAR sTmp[32] = { 0 };
  if( WayPointList && ValidWayPoint(TeamCodeRefWaypoint)) {
	// LKTOKEN _@M1230_ "Team Ref.: "
	_tcsncpy(sTmp, gettext(TEXT("_@M1230_")), 20);
	_tcsncat(sTmp, WayPointList[TeamCodeRefWaypoint].Name,10);
  } else {
	// LKTOKEN _@M1231_ "Team code: SET REF!"
	_tcsncpy(sTmp, gettext(TEXT("_@M1231_")), 30);
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
