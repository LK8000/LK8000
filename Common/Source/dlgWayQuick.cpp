/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayQuick.cpp,v 1.1 2010/12/13 16:54:49 root Exp root $
*/

#include "StdAfx.h"
#include <aygshell.h>

#include "lk8000.h"

//#include "Statistics.h"
#include "externs.h"
#include "Dialogs.h"
#include "Utils2.h"
//#include "Logger.h"
//#include "McReady.h"
//#include "dlgTools.h"
#include "InfoBoxLayout.h"

#include "utils/heapcheck.h"


static WndForm *wf=NULL;
#define WPLSEL WayPointList[SelectedWaypoint]


short retStatus;

static void OnCancelClicked(WindowControl * Sender){
(void)Sender;
        wf->SetModalResult(mrOK);
}

static void OnSetAlt1Clicked(WindowControl * Sender){
  (void)Sender;
  LockTaskData();
  Alternate1 = SelectedWaypoint;
  SetToRegistry(szRegistryAlternate1, Alternate1);
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
  SetToRegistry(szRegistryAlternate2, Alternate2);
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

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnGotoClicked),
  DeclareCallBackEntry(OnSetAlt1Clicked),
  DeclareCallBackEntry(OnSetAlt2Clicked),
  DeclareCallBackEntry(OnCancelClicked),
  DeclareCallBackEntry(NULL)
};

// Will return 0 if cancel or error, 1 if details needed, 2 if goto, 3 if alt1, 4 if alt2
short dlgWayQuickShowModal(void){

  wf = NULL;

  char filename[MAX_PATH];
  TCHAR sTmp[128];

  LocalPathS(filename, TEXT("dlgWayQuick.xml"));
  wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_WAYPOINTQUICK"));

  if (!wf) return 0;

  ASSERT(wf!=NULL);

  ((WndButton *)wf->FindByName(TEXT("cmdGoto"))) ->SetOnClickNotify(OnGotoClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSetAlt1"))) ->SetOnClickNotify(OnSetAlt1Clicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSetAlt2"))) ->SetOnClickNotify(OnSetAlt2Clicked);
  ((WndButton *)wf->FindByName(TEXT("cmdDetails"))) ->SetOnClickNotify(OnDetailsClicked);
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
  wf->SetLeft((ScreenSizeX-IBLSCALE(230))/2);

  wf->ShowModal();

  delete wf;

  wf = NULL;

  return retStatus;

}
