/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgLKTraffic.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKInterface.h"
#include <aygshell.h>
#include "InfoBoxLayout.h"
#include "Dialogs.h"

#include "FlarmIdFile.h"
extern FlarmIdFile *file;

static WndForm *wf=NULL;
static void SetValues(int indexid);

static int SelectedTraffic;
static WndButton *buttonTarget=NULL;

static void OnTargetClicked(WindowControl * Sender) {
  (void)Sender;

  if (SelectedTraffic<0 || SelectedTraffic>MAXTRAFFIC) {
	StartupStore(_T("--- Invalid traffic selected to Target, out of range%s"),NEWLINE);
	DoStatusMessage(_T("ERR-126 invalid TARGET traffic"));
	return;
  }
  if ( GPS_INFO.FLARM_Traffic[SelectedTraffic].ID <1 ) {
	DoStatusMessage(gettext(TEXT("_@M879_"))); // SORRY TARGET JUST DISAPPEARED
	return;
  }

  if ( LKTraffic[SelectedTraffic].Locked ) {
#if 0
	if (MessageBoxX(hWndMapWindow,
		gettext(TEXT("_@M880_")), // UNLOCK current target?
		gettext(TEXT("_@M881_")), // Target selection
	MB_YESNO|MB_ICONQUESTION) == IDYES) {

#endif
	LockFlightData();
	GPS_INFO.FLARM_Traffic[SelectedTraffic].Locked=false;
	UnlockFlightData();
	LKTargetIndex=-1;
	LKTargetType=LKT_TYPE_NONE;
	WayPointList[RESWP_FLARMTARGET].Latitude   = RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Longitude  = RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Altitude   = RESWP_INVALIDNUMBER;
	_tcscpy(WayPointList[RESWP_FLARMTARGET].Name,_T(RESWP_FLARMTARGET_NAME) );
	DoStatusMessage(gettext(TEXT("_@M882_"))); // TARGET RELEASED
	wf->SetModalResult(mrOK);
	return;
  }
#if 0
  if (MessageBoxX(hWndMapWindow, 
	gettext(TEXT("_@M884_")), // LOCK this target?
	gettext(TEXT("_@M881_")), // Target selection
  MB_YESNO|MB_ICONQUESTION) == IDYES) {
#endif
	// one more check for existance
	if ( GPS_INFO.FLARM_Traffic[SelectedTraffic].ID <1 ) {
		DoStatusMessage(gettext(TEXT("_@M883_"))); // TARGET DISAPPEARED!
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
	DoStatusMessage(gettext(TEXT("_@M675_")));
	LKTargetIndex=SelectedTraffic;
	LKTargetType=LKT_TYPE_MASTER;

	// Remember that we need to update the virtual waypoint constantly when we receive FLARM data of target
	// Probably name is not updated if changed from Flarm menu...
	OvertargetMode=OVT_FLARM;
	WayPointList[RESWP_FLARMTARGET].Latitude   = GPS_INFO.FLARM_Traffic[LKTargetIndex].Latitude;
	WayPointList[RESWP_FLARMTARGET].Longitude  = GPS_INFO.FLARM_Traffic[LKTargetIndex].Longitude;
	WayPointList[RESWP_FLARMTARGET].Altitude   = GPS_INFO.FLARM_Traffic[LKTargetIndex].Altitude;
	if (_tcslen(GPS_INFO.FLARM_Traffic[LKTargetIndex].Name) == 1) {
		_stprintf(WayPointList[RESWP_FLARMTARGET].Name,_T("%0x"),GPS_INFO.FLARM_Traffic[LKTargetIndex].ID);
	} else {
		_stprintf(WayPointList[RESWP_FLARMTARGET].Name,_T("%s"),GPS_INFO.FLARM_Traffic[LKTargetIndex].Name);
	}
	SetModeType(LKMODE_TRF, IM_TARGET);
  wf->SetModalResult(mrOK);

}

static void OnRenameClicked(WindowControl * Sender){
  (void)Sender;

  if (SelectedTraffic<0 || SelectedTraffic>MAXTRAFFIC) {
	StartupStore(_T("--- Invalid traffic selected to rename, out of range%s"),NEWLINE);
	DoStatusMessage(_T("ERR-126 invalid traffic"));
	return;
  }
  if ( LKTraffic[SelectedTraffic].ID <1 ) {
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
	AddFlarmLookupItem(GPS_INFO.FLARM_Traffic[SelectedTraffic].ID, newName, true);
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


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnRenameClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnTargetClicked),
  DeclareCallBackEntry(NULL)
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
  if ( LKTraffic[indexid].ID <=0 || LKTraffic[indexid].Status <LKT_REAL) {
	StartupStore(_T("--- LK setvalues invalid indexid=%d%s"),indexid,NEWLINE);
	// DoStatusMessage(_T("ERR-217 INVALID INDEXID"));
	return;
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRegName"));
  if (wp) {

	wlen=wcslen(LKTraffic[indexid].Name);
	// a ? probably
	if (wlen==1) {
		_stprintf(buffer,_T("%06x"),LKTraffic[indexid].ID);
		buffer[MAXFLARMNAME]='\0';
	} else {
		LK_tcsncpy(buffer,LKTraffic[indexid].Name,MAXFLARMNAME);
		ConvToUpper(buffer);
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
	_stprintf(buffer, TEXT(" %d")TEXT(DEG), iround(LKTraffic[indexid].Bearing));
	wp->SetText(buffer);
	wp->RefreshDisplay();
  }

  FlarmId* flarmId = file->GetFlarmIdItem(LKTraffic[indexid].ID);
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
	wp = (WndProperty*)wf->FindByName(TEXT("prpFreq"));
	if (wp) {
		_stprintf(buffer,_T("%s"),flarmId->freq);
		wp->SetText(buffer);
		wp->RefreshDisplay();
	}


  }
		


}


void dlgLKTrafficDetails(int indexid) {

  TCHAR status[80], tpas[30];
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgLKTraffic.xml"));
  wf = dlgLoadFromXML(CallBackTable,
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_LKTRAFFICDETAILS"));

  if (!wf) return;

  //ASSERT(wf!=NULL);

  // cmdTarget Caption name is normally "TARGET"
  buttonTarget=((WndButton *)wf->FindByName(TEXT("cmdTarget")));
  if (buttonTarget) {
	if ( LKTraffic[indexid].Locked ) {
	// LKTOKEN  _@M754_ = "UNLOCK" 
		_tcscpy(status,gettext(TEXT("_@M754_")));
		buttonTarget->SetCaption(status);
	}
  }


  SetValues(indexid);

  _tcscpy(status,_T("Traffic: "));
  if (LKTraffic[indexid].Locked) _tcscat(status,_T("TARGET "));
  switch(LKTraffic[indexid].Status) {
	case LKT_REAL:
	// LKTOKEN  _@M394_ = "Live " 
		_tcscat(status,gettext(TEXT("_@M394_")));
		break;
	case LKT_GHOST:
	// LKTOKEN  _@M323_ = "Ghost " 
		_tcscat(status,gettext(TEXT("_@M323_")));
		break;
	case LKT_ZOMBIE:
	// LKTOKEN  _@M829_ = "Zombie " 
		_tcscat(status,gettext(TEXT("_@M829_")));
		break;
	default:
	// LKTOKEN  _@M753_ = "UNKNOWN! " 
		_tcscat(status,gettext(TEXT("_@M753_")));
		break;
  }
  Units::TimeToTextDown(tpas,(int)(GPS_INFO.Time - LKTraffic[indexid].Time_Fix));
  TCHAR caption[80];
  _stprintf(caption,_T("%s (%s\" old)"),status,tpas);

  wf->SetCaption(caption);
  SelectedTraffic=indexid;

  wf->ShowModal();

  delete wf;
  wf = NULL;
  return;
}

