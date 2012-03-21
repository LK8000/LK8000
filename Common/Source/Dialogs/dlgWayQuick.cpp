/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayQuick.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>
#include "InfoBoxLayout.h"
#include "LKProfiles.h"


static WndForm *wf=NULL;
#define WPLSEL WayPointList[SelectedWaypoint]


static short retStatus;

static void OnCancelClicked(WindowControl * Sender){
(void)Sender;
        wf->SetModalResult(mrOK);
}

static void OnSetAlt1Clicked(WindowControl * Sender){
  (void)Sender;
  LockTaskData();
  Alternate1 = SelectedWaypoint;
  RefreshTask();
  UnlockTaskData();
  if (ValidWayPoint(Alternate1))
  	DoStatusMessage(_T("Altern.1="),WayPointList[Alternate1].Name);
  retStatus=3;
  wf->SetModalResult(mrOK);
}

static void OnSetAlt2Clicked(WindowControl * Sender){
  (void)Sender;
  LockTaskData();
  Alternate2 = SelectedWaypoint;
  RefreshTask();
  UnlockTaskData();
  if (ValidWayPoint(Alternate2))
  	DoStatusMessage(_T("Altern.2="),WayPointList[Alternate2].Name);
  retStatus=4;
  wf->SetModalResult(mrOK);
}

static void OnGotoClicked(WindowControl * Sender){
  (void)Sender;
  GotoWaypoint(SelectedWaypoint);
  retStatus=2;
  wf->SetModalResult(mrOK);
}

static void OnDetailsClicked(WindowControl * Sender){
  (void)Sender;
  retStatus=1;
  wf->SetModalResult(mrOK);
}

static void OnTaskClicked(WindowControl * Sender){
  (void)Sender;
  retStatus=5;
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnGotoClicked),
  DeclareCallBackEntry(OnSetAlt1Clicked),
  DeclareCallBackEntry(OnSetAlt2Clicked),
  DeclareCallBackEntry(OnTaskClicked),
  DeclareCallBackEntry(OnCancelClicked),
  DeclareCallBackEntry(NULL)
};

// Will return 0 if cancel or error, 1 if details needed, 2 if goto, 3 if alt1, 4 if alt2
short dlgWayQuickShowModal(void){

  wf = NULL;

  char filename[MAX_PATH];
  TCHAR sTmp[128];

  if (ScreenLandscape) {
	LocalPathS(filename, TEXT("dlgWayQuick.xml"));
	wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_WAYPOINTQUICK"));
  } else {
	LocalPathS(filename, TEXT("dlgWayQuick_P.xml"));
	wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_WAYPOINTQUICK_P"));
  }

  if (!wf) return 0;

  ((WndButton *)wf->FindByName(TEXT("cmdGoto"))) ->SetOnClickNotify(OnGotoClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSetAlt1"))) ->SetOnClickNotify(OnSetAlt1Clicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSetAlt2"))) ->SetOnClickNotify(OnSetAlt2Clicked);
  ((WndButton *)wf->FindByName(TEXT("cmdDetails"))) ->SetOnClickNotify(OnDetailsClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdTask"))) ->SetOnClickNotify(OnTaskClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdCancel"))) ->SetOnClickNotify(OnCancelClicked);

  retStatus=0;
  if (WPLSEL.Format == LKW_CUP) {
        TCHAR ttmp[50];
        // and it is landable
        if ((WPLSEL.Style>1) && (WPLSEL.Style<6) ) {

                _stprintf(sTmp, TEXT("%s "), WPLSEL.Name);
		if (_tcslen(sTmp)>9) {
			sTmp[9]='\0';
			_tcscat(sTmp, _T(" "));
		}

                if ( _tcslen(WPLSEL.Freq)>0 )  {
                        _stprintf(ttmp,_T("%s "),WPLSEL.Freq);
                        _tcscat(sTmp, ttmp);
                }

                if ( WPLSEL.RunwayDir>=0 )  {
                        _stprintf(ttmp,_T("RW %d "),WPLSEL.RunwayDir);
                        _tcscat(sTmp, ttmp);
                }
                if ( WPLSEL.RunwayLen>0 )  {
                        // we use Altitude instead of distance, to keep meters and feet
                        _stprintf(ttmp,_T("%.0f%s"),Units::ToUserAltitude((double)WPLSEL.RunwayLen), Units::GetAltitudeName());
                        _tcscat(sTmp, ttmp);
                }

        } else {
		_stprintf(sTmp, _T(" %s"),WayPointList[SelectedWaypoint].Name);
        }
  } else {
	_stprintf(sTmp, _T(" %s"),WayPointList[SelectedWaypoint].Name);
  }
  wf->SetCaption(sTmp);

  unsigned int offset=0;
  if (ScreenLandscape) {
	offset=(ScreenSizeX-IBLSCALE(320))/2;
	((WndButton *)wf->FindByName(TEXT("cmdGoto"))) ->SetLeft(offset);
	((WndButton *)wf->FindByName(TEXT("cmdSetAlt1"))) ->SetLeft(offset);
	((WndButton *)wf->FindByName(TEXT("cmdSetAlt2"))) ->SetLeft(offset+IBLSCALE(160));
	((WndButton *)wf->FindByName(TEXT("cmdDetails"))) ->SetLeft(offset);
	((WndButton *)wf->FindByName(TEXT("cmdTask"))) ->SetLeft(offset+IBLSCALE(160));
	((WndButton *)wf->FindByName(TEXT("cmdCancel"))) ->SetLeft(offset);
  } else {
	offset=(ScreenSizeY-IBLSCALE(320))/2;
	((WndButton *)wf->FindByName(TEXT("cmdGoto"))) ->SetTop(offset);
  }


  wf->ShowModal();

  delete wf;

  wf = NULL;

  return retStatus;

}
