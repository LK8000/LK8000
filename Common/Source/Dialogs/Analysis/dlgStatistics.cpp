/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "ContestMgr.h"
#include "dlgTools.h"
#include "Event/Event.h"
#include "resource.h"
#include "Asset.hpp"

double Statistics::yscale;
double Statistics::xscale;
double Statistics::y_min;
double Statistics::x_min;
double Statistics::x_max;
double Statistics::y_max;
bool   Statistics::unscaled_x;
bool   Statistics::unscaled_y;


LKPen penThinSignal;

int analysis_page=ANALYSYS_PAGE_DEFAULT;

extern CContestMgr::TType contestType;
extern void UpdateAnalysis(WndForm* wfa);



void Statistics::Reset() {
  ThermalAverage.Reset();
  Wind_x.Reset();
  Wind_y.Reset();
  Altitude.Reset();
  Altitude_Base.Reset();
  Altitude_Ceiling.Reset();
  Task_Speed.Reset();
  Altitude_Terrain.Reset();
  for (int j = 0; j < MAXTASKPOINTS; j++) {
    LegStartTime[j] = -1;
  }

  if (AdditionalContestRule) {  // If there is an additional contest activated we start with this.
    contestType = CContestMgr::TYPE_XC_FREE_TRIANGLE;
    analysis_page = ANALYSIS_PAGE_CONTEST;
  }
}

static bool OnTimerNotify(WndForm* pWnd) {
    UpdateAnalysis(pWnd);
    return true;
}


void Statistics::FormatTicText(TCHAR *text, const double val, const double step) {
  if (step<1.0) {
    _stprintf(text, TEXT("%.1f"), val);
  } else {
    _stprintf(text, TEXT("%.0f"), val);
  }
}

static void OnAnalysisPaint(WindowControl * Sender, LKSurface& Surface){

  WndForm* pForm = Sender->GetParentWndForm();

  const RECT rcgfx = Sender->GetClientRect();
  const auto hfOld = Surface.SelectObject(LK8PanelUnitFont/* Sender->GetFont()*/);

    if(INVERTCOLORS || IsDithered()) {
      if(Sender->GetBackColor() != SKY_HORIZON_COL) {
        Sender->SetBackColor(SKY_HORIZON_COL);
        // we need to fill background if BackColor change because background are already filled.
        Surface.FillRect(&rcgfx, Sender->GetBackBrush());
      }
      Surface.SetTextColor(RGB_DARKBLUE);
    } else {
      Surface.SetTextColor(RGB_WHITE);
    }

    Surface.SetBkColor(Sender->GetBackColor());

    Surface.SetBackgroundTransparent();


const TCHAR* CalcCaption = NULL;
switch (analysis_page) {
  case ANALYSIS_PAGE_BAROGRAPH:
    CalcCaption = TEXT("_@M885_"); // Settings
    Statistics::RenderBarograph(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_CLIMB:
    CalcCaption = TEXT("_@M886_"); // Task calc
    Statistics::RenderClimb(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_WIND:
    CalcCaption = TEXT("_@M887_"); // Set wind
    Statistics::RenderWind(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_POLAR:
    CalcCaption = TEXT("_@M885_"); // Settings
    Statistics::RenderGlidePolar(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_TEMPTRACE:
    CalcCaption = TEXT("_@M885_"); // Settings
    Statistics::RenderTemperature(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_TASK:
    CalcCaption = TEXT("_@M886_"); // Task calc
    LockTaskData();
    Statistics::RenderTask(Surface, rcgfx, false);
    UnlockTaskData();
    break;

  case ANALYSIS_PAGE_CONTEST:
    CalcCaption = TEXT("_@M1451_"); // Change
    LockTaskData();
    Statistics::RenderContest(Surface, rcgfx);
    UnlockTaskData();
    break;


  case ANALYSIS_PAGE_TASK_SPEED:
    CalcCaption = TEXT("_@M886_"); // Task calc
    LockTaskData();
    Statistics::RenderSpeed(Surface, rcgfx);
    UnlockTaskData();
    break;
  default:
    // should never get here!
    break;
  }

  if(CalcCaption) {
    WindowControl *wCalc = pForm->FindByName(TEXT("cmdCalc"));
    if (wCalc) {
      wCalc->SetCaption(LKGetText(CalcCaption));
    }
  }

  Surface.SelectObject(hfOld);

}


static void NextPage(WndForm* pForm, int Step){
  analysis_page += Step;

LockFlightData(); /* skip Temperature Page if no OAT available */
  if(analysis_page == ANALYSIS_PAGE_TEMPTRACE)
    if(GPS_INFO.TemperatureAvailable!=TRUE)
      analysis_page += Step;
UnlockFlightData();

  if (analysis_page > MAXPAGE)
    analysis_page = 0;
  if (analysis_page < 0)
    analysis_page = MAXPAGE;
  UpdateAnalysis(pForm);
}


static void OnNextClicked(WndButton* pWnd){
    NextPage(pWnd->GetParentWndForm(), +1);
}

static void OnPrevClicked(WndButton* pWnd){
    NextPage(pWnd->GetParentWndForm(), -1);
}

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {
    WindowControl* pGrid = pWnd->FindByName(TEXT("frmGrid"));
    if (pGrid->HasFocus())
        return false;

    Window * pBtn = NULL;

    switch (KeyCode & 0xffff) {
        case KEY_LEFT:
        case '6':
            pBtn = pWnd->FindByName(TEXT("cmdPrev"));
            NextPage(pWnd, -1);
            break;
        case KEY_RIGHT:
        case '7':
            pBtn = pWnd->FindByName(TEXT("cmdNext"));
            NextPage(pWnd, +1);
            break;;
    }
    if (pBtn) {
        pBtn->SetFocus();
        return true;
    }

    return false;
}

static void OnCalcClicked(WndButton* pWnd){
  WndForm* pForm = pWnd->GetParentWndForm();

  if (analysis_page==ANALYSIS_PAGE_BAROGRAPH) {
    dlgBasicSettingsShowModal();
  }
  if (analysis_page==ANALYSIS_PAGE_CLIMB) {
    pForm->SetVisible(false);
    dlgTaskCalculatorShowModal();
    pForm->SetVisible(true);
  }
  if (analysis_page==ANALYSIS_PAGE_WIND) {
    dlgWindSettingsShowModal();
  }
  if (analysis_page==ANALYSIS_PAGE_POLAR) {
    dlgBasicSettingsShowModal();
  }
  if (analysis_page==ANALYSIS_PAGE_TEMPTRACE) {
    dlgBasicSettingsShowModal();
  }
  if ((analysis_page==ANALYSIS_PAGE_TASK) || (analysis_page==ANALYSIS_PAGE_TASK_SPEED)) {
    pForm->SetVisible(false);
    dlgTaskCalculatorShowModal();
    pForm->SetVisible(true);
  }
  if (analysis_page==ANALYSIS_PAGE_CONTEST) {

    // Rotate presented contest
    switch(contestType) {
    case CContestMgr::TYPE_OLC_CLASSIC:
      contestType = CContestMgr::TYPE_FAI_TRIANGLE;
      break;

    case CContestMgr::TYPE_FAI_TRIANGLE:
      contestType = CContestMgr::TYPE_FAI_TRIANGLE4;
      FAI_OptimizerMode = 4;
      CContestMgr::Instance().RefreshFAIOptimizer();
      break;
    case CContestMgr::TYPE_FAI_TRIANGLE4:
#ifdef  FIVEPOINT_OPTIMIZER
      contestType = CContestMgr::TYPE_FAI_TRIANGLE5;
      FAI_OptimizerMode = 5;

      CContestMgr::Instance().RefreshFAIOptimizer();
      break;

    case CContestMgr::TYPE_FAI_TRIANGLE5:
#endif
      contestType = CContestMgr::TYPE_OLC_FAI;
      break;
    case CContestMgr::TYPE_OLC_FAI:
      contestType = CContestMgr::TYPE_OLC_CLASSIC_PREDICTED;
      break;
    case CContestMgr::TYPE_OLC_CLASSIC_PREDICTED:
      contestType = CContestMgr::TYPE_OLC_FAI_PREDICTED;
      break;
    case CContestMgr::TYPE_OLC_FAI_PREDICTED:
      contestType = CContestMgr::TYPE_OLC_LEAGUE;
      break;
    case CContestMgr::TYPE_OLC_LEAGUE:
      contestType = CContestMgr::TYPE_FAI_3_TPS;
      break;
    case CContestMgr::TYPE_FAI_3_TPS:
      contestType = CContestMgr::TYPE_FAI_3_TPS_PREDICTED;
      break;
    case CContestMgr::TYPE_FAI_3_TPS_PREDICTED:
      if ( !AdditionalContestRule)
        contestType = CContestMgr::TYPE_OLC_CLASSIC;
      else
        contestType = CContestMgr::TYPE_XC_FREE_TRIANGLE;
      break;
      case  CContestMgr::TYPE_XC_FREE_TRIANGLE:
      contestType = CContestMgr::TYPE_XC_FAI_TRIANGLE;
      break;
    case  CContestMgr::TYPE_XC_FAI_TRIANGLE:
        contestType = CContestMgr::TYPE_XC_FREE_FLIGHT;
        break;
    case  CContestMgr::TYPE_XC_FREE_FLIGHT:
        contestType = CContestMgr::TYPE_OLC_CLASSIC;
        break;
    default:
      if ( !AdditionalContestRule)
        contestType = CContestMgr::TYPE_OLC_CLASSIC;
      else
        contestType = CContestMgr::TYPE_XC_FAI_TRIANGLE;
    }
  }

  UpdateAnalysis(pForm);
}

static void OnAspBearClicked(WndButton* pWnd){
    UpdateAnalysis(pWnd->GetParentWndForm());
}


static CallBackTableEntry_t CallBackTable[]={
  OnPaintCallbackEntry(OnAnalysisPaint),
  ClickNotifyCallbackEntry(OnNextClicked),
  ClickNotifyCallbackEntry(OnPrevClicked),
  ClickNotifyCallbackEntry(OnCalcClicked),
  ClickNotifyCallbackEntry(OnAspBearClicked),
  EndCallBackEntry()
};





void dlgAnalysisShowModal(int inPage){
static bool entered = false;



if (entered == true) /* prevent re entrance */
	return;

unsigned int iTmpMainMapOptMode = FAI_OptimizerMode ; /* remember optimizer mode of main map */
  entered = true;

  WndForm* pForm = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_ANALYSIS_L : IDR_XML_ANALYSIS_P);
  if (!pForm) return;

  LKColor COL = LKColor(50,243,45);
  if(INVERTCOLORS || IsDithered())
	COL = COL.ChangeBrightness(0.7);
  penThinSignal.Create(PEN_SOLID, IBLSCALE(2) , COL);


  pForm->SetKeyDownNotify(FormKeyDown);

  WndButton *wClose = (WndButton *)pForm->FindByName(TEXT("cmdClose"));
  LKASSERT(wClose);
  if (!wClose) return;
  wClose->SetOnClickNotify(OnCloseClicked);

  WndButton *wNext = (WndButton *)pForm->FindByName(TEXT("cmdNext"));
  LKASSERT(wNext);
  if (!wNext) return;

  WndButton *wPrev = (WndButton *)pForm->FindByName(TEXT("cmdPrev"));
  LKASSERT(wPrev);
  if (!wPrev) return;

  // In portrait mode, buttons are disposed like:
  // CALC
  // < >
  // CLOSE
  if (!ScreenLandscape) {
    WindowControl *wCalc = pForm->FindByName(TEXT("cmdCalc"));
    if(wCalc) {
      int ytop=wCalc->GetTop();
      int ysize=ScreenSizeY-ytop-NIBLSCALE(20);
      int sep=NIBLSCALE(1);
      int hei=(ysize-sep*2)/3;
      wCalc->SetHeight(hei);
      wNext->SetHeight(hei);
      wPrev->SetHeight(hei);
      wClose->SetHeight(hei);

      wNext->SetTop(ytop+hei+sep);
      wPrev->SetTop(ytop+hei+sep);

      wClose->SetTop(ytop+hei+hei+sep+sep);
    }
  }

  /*
    Does Not Work, Because wfa->GetHeigth() Is WindowHeight and ClientRect Is Calculated By OnPaint
    Why WndForm Do Not Use NonClientArea for Border and Caption ?
  */
  /*
  WndFrame *wBtFrm = (WndFrame*)wfa->FindByName(TEXT("frmButton"));
  if(wBtFrm) {
    wBtFrm->SetTop(wfa->GetHeigth()- wBtFrm->GetHeight());
    if(waInfo) {
      waInfo->SetHeight(wBtFrm->GetTop()-waInfo->GetTop());
    }
  }
  */

  pForm->SetTimerNotify(5000, OnTimerNotify);

//  UpdateAnalysis();

  if (inPage!=ANALYSYS_PAGE_DEFAULT) analysis_page=inPage;
  UpdateAnalysis(pForm);

  pForm->ShowModal();

  delete pForm;

  pForm = NULL;

  penThinSignal.Release();

  if(FAI_OptimizerMode != iTmpMainMapOptMode) /* Analysis let in a different optimizer mode */
  {
    FAI_OptimizerMode = iTmpMainMapOptMode; /* restore optimizer mode for main map */
	CContestMgr::Instance().Result(CContestMgr::TYPE_FAI_TRIANGLE, false); /* recalc optimizer for main map */
	CContestMgr::Instance().RefreshFAIOptimizer();
  }

  MapWindow::RequestFastRefresh();

  FullScreen();

  entered = false;
}
