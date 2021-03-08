/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayQuick.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "TeamCodeCalculation.h"
#include "NavFunctions.h"
#include "resource.h"
#include "LKStyle.h"
#include "Radio.h"

#define WPLSEL WayPointList[SelectedWaypoint]


static short retStatus;

static void OnPaintWaypointPicto(WindowControl * Sender, LKSurface& Surface) {
    if (Sender) {
        const RECT rc = Sender->GetClientRect();

        MapWindow::DrawWaypointPictoBg(Surface, rc);
        LockTaskData();
        LKASSERT(ValidWayPointFast(SelectedWaypoint));
        if (WayPointCalc[SelectedWaypoint].IsLandable) {
            MapWindow::DrawRunway(Surface, &WayPointList[SelectedWaypoint], rc, nullptr, 1.5 , true);
        } else {
            MapWindow::DrawWaypointPicto(Surface, rc, &WayPointList[SelectedWaypoint]);
        }
        UnlockTaskData();
    }
}

static void OnCancelClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnSetAlt1Clicked(WndButton* pWnd){
  LockTaskData();
  Alternate1 = SelectedWaypoint;
  RefreshTask();
  UnlockTaskData();
  if (ValidWayPoint(Alternate1))
	DoStatusMessage(_T("Altern.1="),WayPointList[Alternate1].Name);
  retStatus=3;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnSetAlt2Clicked(WndButton* pWnd){
  LockTaskData();
  Alternate2 = SelectedWaypoint;
  RefreshTask();
  UnlockTaskData();
  if (ValidWayPoint(Alternate2))
	DoStatusMessage(_T("Altern.2="),WayPointList[Alternate2].Name);
  retStatus=4;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnGotoClicked(WndButton* pWnd){
  GotoWaypoint(SelectedWaypoint);
  retStatus=2;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnDetailsClicked(WndButton* pWnd){
  retStatus=1;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnTaskClicked(WndButton* pWnd){
  retStatus=5;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void SetRadioFrequency(WndButton* pWnd, bool bActive)
{
  double Ferquency;
  LKASSERT(SelectedWaypoint>=0);
  LockTaskData();
  LKASSERT(ValidWayPointFast(SelectedWaypoint));
  Ferquency = StrToDouble(WayPointList[SelectedWaypoint].Freq,NULL);
  if(bActive) {
    devPutFreqActive(Ferquency, WayPointList[SelectedWaypoint].Name);
  } else {
    devPutFreqStandby(Ferquency, WayPointList[SelectedWaypoint].Name);
  }

#ifdef STATUS_RADIO   // Status Popup not realy needed
  TCHAR szFreq[60];
  _stprintf(szFreq,_T("Standby %6.3f ") ,Ferquency);

  DoStatusMessage(_T(""), WayPointList[SelectedWaypoint].Name );
  DoStatusMessage(_T(""), szFreq );

  retStatus=3;
#endif

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
  UnlockTaskData();
}

static void OnRadioFrequencyClicked(WndButton* pWnd){
	SetRadioFrequency(pWnd, true);
}



static void OnRadioFrequencySBClicked(WndButton* pWnd){
	SetRadioFrequency(pWnd, false);
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnGotoClicked),
  ClickNotifyCallbackEntry(OnSetAlt1Clicked),
  ClickNotifyCallbackEntry(OnSetAlt2Clicked),
  ClickNotifyCallbackEntry(OnTaskClicked),
  ClickNotifyCallbackEntry(OnCancelClicked),
  ClickNotifyCallbackEntry(OnDetailsClicked),
  ClickNotifyCallbackEntry(OnRadioFrequencyClicked),
  ClickNotifyCallbackEntry(OnRadioFrequencySBClicked),
  OnPaintCallbackEntry(OnPaintWaypointPicto),
  EndCallBackEntry()
};

// Will return 0 if cancel or error, 1 if details needed, 2 if goto, 3 if alt1, 4 if alt2
short dlgWayQuickShowModal(void){

  TCHAR sTmp[128];

  std::unique_ptr<WndForm> wf(dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_WAYPOINTQUICK_L : IDR_XML_WAYPOINTQUICK_P));

  if (!wf) return 0;
  TCHAR buffer2[80];
  WindowControl* wFreq = wf->FindByName(TEXT("cmdRadioFreq"));
  WindowControl* wFreqSB = wf->FindByName(TEXT("cmdRadioFreqSB"));
  _stprintf(buffer2,_T("%s %s"),	SEL_ACTIVE_SYMBOL(Appearance.UTF8Pictorials), WPLSEL.Freq );

  wFreq->SetCaption(buffer2);
  wFreq->Redraw();

  _stprintf(buffer2,_T("%s %s"),	SEL_STANDBY_SYMBOL(Appearance.UTF8Pictorials), WPLSEL.Freq );

  wFreqSB->SetCaption(buffer2);
  wFreqSB->Redraw();
  retStatus=0;
  if ((WPLSEL.Format == LKW_CUP  || WPLSEL.Format == LKW_OPENAIP)&& WPLSEL.Style >= STYLE_AIRFIELDGRASS && WPLSEL.Style <= STYLE_AIRFIELDSOLID) {
        TCHAR ttmp[50];

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
     TCHAR code[20];
     double wpdistance = 0;
     double wpbearing = 0;

     if (TeamCodeRefWaypoint >= 0) {
        DistanceBearing(WayPointList[TeamCodeRefWaypoint].Latitude,
                  WayPointList[TeamCodeRefWaypoint].Longitude,
                  WayPointList[SelectedWaypoint].Latitude,
                  WayPointList[SelectedWaypoint].Longitude,
                  &wpdistance, &wpbearing);

        GetTeamCode(code,wpbearing, wpdistance);
        _stprintf(sTmp, TEXT(" %s  (%s)"), WayPointList[SelectedWaypoint].Name, code);
     } else {
        _stprintf(sTmp, TEXT(" %s"), WayPointList[SelectedWaypoint].Name);
     }
  }
  wf->SetCaption(sTmp);

  const bool bRadioFreq = (_tcstol(WayPointList[SelectedWaypoint].Freq, nullptr, 10) > 0) && RadioPara.Enabled;

  if (ScreenLandscape) {
    PixelScalar left = 0;
    WindowControl* pWnd = wf->FindByName(TEXT("cmdGoto"));
    if(pWnd) {
      left = pWnd->GetLeft();
      pWnd->SetWidth(ScreenSizeX-NIBLSCALE(5)-left);
    }


	pWnd = wf->FindByName(TEXT("cmdSetAlt1"));
    if(pWnd) {
      if(  Alternate2 == RESWP_EXT_TARGET)  // used for external logger target?
      {
	pWnd->SetWidth((ScreenSizeX)-NIBLSCALE(7));
      }
      else
      {
        pWnd->SetWidth((ScreenSizeX/2)-NIBLSCALE(5));
	pWnd->SetLeft(NIBLSCALE(3));
      }
    }


	pWnd = wf->FindByName(TEXT("cmdSetAlt2"));
    if(pWnd) {

      if( Alternate2 == RESWP_EXT_TARGET)  // used for external logger target?
      {
	pWnd->SetHeight(1);
	pWnd->SetWidth(1);
	pWnd->SetLeft(1);
      }
      else
      {
        pWnd->SetWidth((ScreenSizeX/2)-NIBLSCALE(7));
	pWnd->SetLeft((ScreenSizeX/2)+NIBLSCALE(2));
      }
    }
	pWnd = wf->FindByName(TEXT("cmdDetails"));
    if(pWnd) {
      pWnd->SetWidth((ScreenSizeX/2)-NIBLSCALE(5));
	  pWnd->SetLeft(NIBLSCALE(3));
    }
	pWnd = wf->FindByName(TEXT("cmdTask"));
    if(pWnd) {
      pWnd->SetWidth((ScreenSizeX/2)-NIBLSCALE(7));
	  pWnd->SetLeft((ScreenSizeX/2)+NIBLSCALE(2));
    }

	if(bRadioFreq) {
	  pWnd = wf->FindByName(TEXT("cmdCancel"));
      if(pWnd) {
        pWnd->SetLeft(NIBLSCALE(3));
	    pWnd->SetWidth((ScreenSizeX/2)-NIBLSCALE(5));
      }

	  pWnd = wf->FindByName(TEXT("cmdRadioFreq"));
      if(pWnd) {
        pWnd->SetWidth((ScreenSizeX/4)-NIBLSCALE(4));
	    pWnd->SetLeft((ScreenSizeX/4*3)-NIBLSCALE(2));
      }
	  pWnd = wf->FindByName(TEXT("cmdRadioFreqSB"));
      if(pWnd) {
        pWnd->SetWidth((ScreenSizeX/4)-NIBLSCALE(4));
	    pWnd->SetLeft((ScreenSizeX/2)+NIBLSCALE(2));
      }
	} else {
      pWnd = wf->FindByName(TEXT("cmdCancel"));
      if(pWnd) {
        pWnd->SetLeft(NIBLSCALE(3));
        pWnd->SetWidth((ScreenSizeX)-NIBLSCALE(8));
      }

	  pWnd = wf->FindByName(TEXT("cmdRadioFreq"));
      if(pWnd) {
        pWnd->SetVisible(false);
	  }
    }
  } else {
	if(bRadioFreq) {
    } else {
      WindowControl* pWndCancel = wf->FindByName(TEXT("cmdCancel"));
      WindowControl* pWndFreq = wf->FindByName(TEXT("cmdRadioFreq"));
      WindowControl* pWndFreqSB = wf->FindByName(TEXT("cmdRadioFreqSB"));
      if(pWndCancel && pWndFreq) {
        pWndCancel->SetTop(pWndFreq->GetTop());
      }
      if(pWndFreq) {
          pWndFreq->SetVisible(false);
      }
      if(pWndFreqSB) {
          pWndFreqSB->SetVisible(false);
      }
    }
  }

  if(SelectedWaypoint < RESWP_FIRST_MARKER)
  {
    WindowControl*pWnd = wf->FindByName(TEXT("cmdTask"));
    if(pWnd) {    pWnd->SetVisible(false);}
  }
  wf->ShowModal();

  return retStatus;

}
