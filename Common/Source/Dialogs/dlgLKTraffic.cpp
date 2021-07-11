/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgLKTraffic.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKInterface.h"
#include "WindowControls.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "FlarmIdFile.h"
#include "resource.h"
#include "Sound/Sound.h"
#include "Radio.h"

static WndForm *wf=NULL;
static void SetValues(int indexid);

static int SelectedTraffic;
static WndButton *buttonTarget=NULL;
const FlarmId* flarmId = nullptr;
static void OnTargetClicked(WndButton* pWnd) {

  if (SelectedTraffic<0 || SelectedTraffic>MAXTRAFFIC) {
	StartupStore(_T("--- Invalid traffic selected to Target, out of range%s"),NEWLINE);
	DoStatusMessage(_T("ERR-126 invalid TARGET traffic"));
	return;
  }
  if ( GPS_INFO.FLARM_Traffic[SelectedTraffic].RadioId < 1) {
	DoStatusMessage(MsgToken(879)); // SORRY TARGET JUST DISAPPEARED
	return;
  }

  if ( LKTraffic[SelectedTraffic].Locked ) {
#if 0
	if (MessageBoxX(MsgToken(880), // UNLOCK current target?
		MsgToken(881), // Target selection
	mbYesNo) == IdYes) {

#endif
	LockFlightData();
	GPS_INFO.FLARM_Traffic[SelectedTraffic].Locked=false;
	UnlockFlightData();
	LKTargetIndex=-1;
	LKTargetType=LKT_TYPE_NONE;
	DoStatusMessage(MsgToken(882)); // TARGET RELEASED
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
	return;
  }
#if 0
  if (MessageBoxX(
    MsgToken(884), // LOCK this target?
	MsgToken(881), // Target selection
  mbYesNo) == IdYes) {
#endif
	// one more check for existance
	if ( GPS_INFO.FLARM_Traffic[SelectedTraffic].RadioId < 1 ) {
		DoStatusMessage(MsgToken(883)); // TARGET DISAPPEARED!
		return;
	}

	LockFlightData();
	// unlock previous target, if any
	if (LKTargetIndex>=0 && LKTargetIndex<MAXTRAFFIC) {
		GPS_INFO.FLARM_Traffic[LKTargetIndex].Locked=false;
	}
	GPS_INFO.FLARM_Traffic[SelectedTraffic].Locked=true;
	UnlockFlightData();
	// LKTOKEN  _@M675_ = "TARGET LOCKED" 
	DoStatusMessage(MsgToken(675));
	LKTargetIndex=SelectedTraffic;
	LKTargetType=LKT_TYPE_MASTER;
	OvertargetMode=OVT_FLARM;
	SetModeType(LKMODE_TRF, IM_TARGET);
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }

}

static void OnRenameClicked(WndButton* pWnd){

  if (SelectedTraffic<0 || SelectedTraffic>MAXTRAFFIC) {
	StartupStore(_T("--- Invalid traffic selected to rename, out of range%s"),NEWLINE);
	DoStatusMessage(_T("ERR-126 invalid traffic"));
	return;
  }
  if ( LKTraffic[SelectedTraffic].RadioId < 1 ) {
	StartupStore(_T("--- Invalid traffic selected to rename, invalid ID%s"),NEWLINE);
	DoStatusMessage(_T("ERR-127 invalid traffic"));
	return;
  }

  TCHAR newName[MAXFLARMNAME+1];
  newName[0] = 0;
  dlgTextEntryShowModal(newName, 7); // 100322 raised from 3 to 6 (+1)
  newName[MAXFLARMNAME] = 0;

  #ifdef DEBUG_LKT
  StartupStore(_T("************  dlgLK RenameClicked for slot=%d id=%lx status=%d oldname=<%s> newName=<%s>\n"),
	SelectedTraffic, 
	GPS_INFO.FLARM_Traffic[SelectedTraffic].ID,
	GPS_INFO.FLARM_Traffic[SelectedTraffic].Status,
	GPS_INFO.FLARM_Traffic[SelectedTraffic].Name,
	newName);
  #endif

  int newnamelen=_tcslen(newName);

  if (newnamelen>0) {

	LockFlightData();

	// Since we don-t know if a new PFLAA with this ID will arrive, and the UpdateNameFlag will be used,
	// we must change the name here now.
	_tcscpy(GPS_INFO.FLARM_Traffic[SelectedTraffic].Name,newName);
	// ... and here also, because SetValues is using this copy 
	_tcscpy(LKTraffic[SelectedTraffic].Name,newName);

	// we assume that no Cn is available, or in any case cannot be kept
	// If newName is smaller or equal to possible Cn, we use it entirely
	if (newnamelen<=MAXFLARMCN) {
		_tcscpy( GPS_INFO.FLARM_Traffic[SelectedTraffic].Cn, newName);
	} else {
		// else we create a fake Cn
		GPS_INFO.FLARM_Traffic[SelectedTraffic].Cn[0]=newName[0];
		GPS_INFO.FLARM_Traffic[SelectedTraffic].Cn[1]=newName[newnamelen-2];
		GPS_INFO.FLARM_Traffic[SelectedTraffic].Cn[2]=newName[newnamelen-1];
		GPS_INFO.FLARM_Traffic[SelectedTraffic].Cn[3]=_T('\0');
	}
	// update it temporarily so that it appears updated leaving the edit page
	_tcscpy(LKTraffic[SelectedTraffic].Cn,GPS_INFO.FLARM_Traffic[SelectedTraffic].Cn);

	// It will be useless, because we already updated the name.. but never mind.
	GPS_INFO.FLARM_Traffic[SelectedTraffic].UpdateNameFlag=true;
	// This will create the local flarmid entry, but won't update the structure in GPS_INFO
	// until a new PFLAA arrives from this ID. 
	AddFlarmLookupItem(GPS_INFO.FLARM_Traffic[SelectedTraffic].RadioId, newName, true);
	UnlockFlightData();

	#ifdef DEBUG_LKT
	StartupStore(_T("...... dlgLK RENAMED slot=%d id=%lx status=%d name=<%s>\n"),
	SelectedTraffic, 
	GPS_INFO.FLARM_Traffic[SelectedTraffic].ID,
	GPS_INFO.FLARM_Traffic[SelectedTraffic].Status,
	GPS_INFO.FLARM_Traffic[SelectedTraffic].Name);
	#endif
	// reload the name...
	SetValues(SelectedTraffic);

  }

}





static void OnFlarmFreqSelectEnter(WndButton*  Sender) {
  TCHAR Tmp[255];
  if(flarmId != NULL)
  {
   if(RadioPara.Enabled)
   {
      double ASFrequency = ExtractFrequency(flarmId->freq);
      if(ValidFrequency(ASFrequency))
      {
        LKSound(TEXT("LK_TICK.WAV"));
        if(_tcslen(flarmId->cn) > 0)
          _tcscpy(Tmp,(TCHAR*)flarmId->cn);
        else
          _tcscpy(Tmp,(TCHAR*)flarmId->reg );
        devPutFreqActive( ASFrequency, Tmp);
      }
    }
  }
}


static void OnFlarmSecFreqSelectEnter(WndButton*  Sender) {
  TCHAR Tmp[255];
  if(flarmId != NULL)
  {
   if(RadioPara.Enabled)
   {
      double ASFrequency = ExtractFrequency(flarmId->freq);
      if(ValidFrequency(ASFrequency))
      {
        LKSound(TEXT("LK_TICK.WAV"));
        if(_tcslen(flarmId->cn) > 0)
          _tcscpy(Tmp,(TCHAR*)flarmId->cn);
        else
          _tcscpy(Tmp,(TCHAR*)flarmId->reg );
        devPutFreqStandby( ASFrequency, Tmp);
      }
    }
  }
}

static void OnCloseClicked(WndButton * pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnRenameClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnTargetClicked),
  ClickNotifyCallbackEntry(OnFlarmFreqSelectEnter),
  ClickNotifyCallbackEntry(OnFlarmSecFreqSelectEnter),
  EndCallBackEntry()
};


static void SetValues(int indexid) {
  //TCHAR *name = 0;
  //TCHAR *cn = 0;
  WndProperty* wp;
  TCHAR buffer[80];
  //TCHAR status[10];
  //static TCHAR Name[MAXFLARMNAME+1];
  //static TCHAR Cn[MAXFLARMCN+1];
  int wlen;

  if (indexid<0 || indexid>MAXTRAFFIC) {
	StartupStore(_T("--- LK setvalues invalid indexid=%d%s"),indexid,NEWLINE);
	// DoStatusMessage(_T("ERR-216 INVALID INDEXID"));
	return;
  }
  if ( LKTraffic[indexid].RadioId <=0 || LKTraffic[indexid].Status <LKT_REAL) {
	StartupStore(_T("--- LK setvalues invalid indexid=%d%s"),indexid,NEWLINE);
	// DoStatusMessage(_T("ERR-217 INVALID INDEXID"));
	return;
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRegName"));
  if (wp) {

	wlen=_tcslen(LKTraffic[indexid].Name);
	// a ? probably
	if (wlen==1) {
		_stprintf(buffer,_T("%06x"),LKTraffic[indexid].RadioId);
		buffer[MAXFLARMNAME]='\0';
	} else {
		LK_tcsncpy(buffer,LKTraffic[indexid].Name,MAXFLARMNAME);
		CharUpper(buffer);
	}
	//name=Name;
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }


#if 0
  wp = (WndProperty*)wf->FindByName(TEXT("prpStatus"));
  if (wp) {
	switch(LKTraffic[indexid].Status) {
		case LKT_REAL:
			_tcscpy(status,_T("LIVE"));
			break;
		case LKT_GHOST:
			_tcscpy(status,_T("GHOST"));
			break;
		case LKT_ZOMBIE:
			_tcscpy(status,_T("ZOMBIE"));
			break;
		default:
			_tcscpy(status,_T("UNKNOWN"));
			break;
	}
	wp->SetText(status);
	wp->RefreshDisplay();
  }
#endif
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpCn"));
  if (wp) {
	if ( _tcslen(LKTraffic[indexid].Cn) == 1 ) {
		if (LKTraffic[indexid].Cn[0] == _T('?')) {
			_tcscpy(buffer,_T(""));
		} else {
			LK_tcsncpy(buffer,LKTraffic[indexid].Cn,MAXFLARMCN);
		}
	} else {
		LK_tcsncpy(buffer,LKTraffic[indexid].Cn,MAXFLARMCN);
	}
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDistance"));
  if (wp) {
	_stprintf(buffer,_T("%.1f %s"),LKTraffic[indexid].Distance*DISTANCEMODIFY, Units::GetDistanceName());
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAltitude"));
  if (wp) {
	_stprintf(buffer,_T("%.0f %s"),LKTraffic[indexid].Altitude*ALTITUDEMODIFY, Units::GetAltitudeName());
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltDiff"));
  if (wp) {
	// this has to be reverted, because it is a relative altitude to us
	_stprintf(buffer,_T("%+.0f %s"),(CALCULATED_INFO.NavAltitude - LKTraffic[indexid].Altitude)*ALTITUDEMODIFY*-1, Units::GetAltitudeName());
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpSpeed"));
  if (wp) {
	_stprintf(buffer,_T("%.0f %s"),LKTraffic[indexid].Speed*SPEEDMODIFY, Units::GetHorizontalSpeedName());
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpVario"));
  if (wp) {
	_stprintf(buffer,_T("%+.1f %s"),LKTraffic[indexid].Average30s*LIFTMODIFY, Units::GetVerticalSpeedName());
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBearing"));
  if (wp) {
	_stprintf(buffer, TEXT(" %d%s"), iround(LKTraffic[indexid].Bearing),MsgToken(2179));
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }

  flarmId = LookupFlarmId(LKTraffic[indexid].RadioId);
  if (flarmId != NULL) {
	wp = (WndProperty*)wf->FindByName(TEXT("prpName"));
	if (wp) {
		_stprintf(buffer,_T("%s"),flarmId->name);
		wp->SetText(buffer);
		wp->RefreshDisplay();
	}
	wp = (WndProperty*)wf->FindByName(TEXT("prpAirfield"));
	if (wp) {
		_stprintf(buffer,_T("%s"),flarmId->airfield);
		wp->SetText(buffer);
		wp->RefreshDisplay();
	}
	wp = (WndProperty*)wf->FindByName(TEXT("prpType"));
	if (wp) {
		_stprintf(buffer,_T("%s"),flarmId->type);
		wp->SetText(buffer);
		wp->RefreshDisplay();
	}


	
	WindowControl* wFreq = wf->FindByName(TEXT("cmdFreq"));
	double ASFrequency = ExtractFrequency(flarmId->freq);
	bool bValidFreq = false;
	if((RadioPara.Enabled) && ValidFrequency(ASFrequency) )
		bValidFreq = true;
	if(wFreq)
	{
		if(bValidFreq )
		{
			_stprintf(buffer,_T("%s %7.3f"),GetActiveStationSymbol(Appearance.UTF8Pictorials), ASFrequency);	
			wFreq->SetCaption(buffer);
			wFreq->SetVisible(true) ;
		}
		else
		{
			wFreq->SetCaption(_T(""));
			wFreq->SetVisible(false) ;
		}
	}
	
	WindowControl* wSecFreq= wf->FindByName(TEXT("cmdSecFreq"));	
	
	if(wSecFreq)
	{
		if(bValidFreq )
		{
			_stprintf(buffer,_T("%s %7.3f"),GetStandyStationSymbol(Appearance.UTF8Pictorials), ASFrequency);
			wSecFreq->SetCaption(buffer);
			wSecFreq->SetVisible(true) ;
		}
		else
		{
			wSecFreq->SetCaption(_T(""));
			wSecFreq->SetVisible(false);
		}
		wSecFreq->Redraw();
	}
}
}


void dlgLKTrafficDetails(int indexid) {

  TCHAR status[80], tpas[30];
  wf = dlgLoadFromXML(CallBackTable, IDR_XML_LKTRAFFICDETAILS);

  if (!wf) return;

  //ASSERT(wf!=NULL);

  // cmdTarget Caption name is normally "TARGET"
  buttonTarget=((WndButton *)wf->FindByName(TEXT("cmdTarget")));
  if (buttonTarget) {
	if ( LKTraffic[indexid].Locked ) {
	// LKTOKEN  _@M754_ = "UNLOCK" 
		_tcscpy(status,MsgToken(754));
		buttonTarget->SetCaption(status);
	}
  }


  SetValues(indexid);

  _tcscpy(status,_T("Traffic: "));
  if (LKTraffic[indexid].Locked) _tcscat(status,_T("TARGET "));
  switch(LKTraffic[indexid].Status) {
	case LKT_REAL:
	// LKTOKEN  _@M394_ = "Live " 
		_tcscat(status,MsgToken(394));
		break;
	case LKT_GHOST:
	// LKTOKEN  _@M323_ = "Ghost " 
		_tcscat(status,MsgToken(323));
		break;
	case LKT_ZOMBIE:
	// LKTOKEN  _@M829_ = "Zombie " 
		_tcscat(status,MsgToken(829));
		break;
	default:
	// LKTOKEN  _@M753_ = "UNKNOWN! " 
		_tcscat(status,MsgToken(753));
		break;
  }
  Units::TimeToTextDown(tpas,(int)(GPS_INFO.Time - LKTraffic[indexid].Time_Fix));
  TCHAR caption[120];
  _stprintf(caption,_T("%s (%s\" old)"),status,tpas);

  wf->SetCaption(caption);
  SelectedTraffic=indexid;

  wf->ShowModal();

  delete wf;
  wf = NULL;
  return;
}

