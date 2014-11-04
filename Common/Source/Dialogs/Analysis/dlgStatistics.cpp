/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"
#include "Dialogs.h"
#include "ContestMgr.h"


double Statistics::yscale;
double Statistics::xscale;
double Statistics::y_min;
double Statistics::x_min;
double Statistics::x_max;
double Statistics::y_max;
bool   Statistics::unscaled_x;
bool   Statistics::unscaled_y;


LKPen penThinSignal;

int analysis_page=0;
WndForm *wfa=NULL;
WndOwnerDrawFrame *waGrid=NULL;
WndOwnerDrawFrame *waInfo=NULL;
static WndButton *wCalc=NULL;

extern CContestMgr::TType contestType;
extern void UpdateAnalysis(void);



void Statistics::Reset() {
  ThermalAverage.Reset();
  Wind_x.Reset();
  Wind_y.Reset();
  Altitude.Reset();
  Altitude_Base.Reset();
  Altitude_Ceiling.Reset();
  Task_Speed.Reset();
  Altitude_Terrain.Reset();
  for(int j=0;j<MAXTASKPOINTS;j++) {
    LegStartTime[j] = -1;
  }
}

static int OnTimerNotify(WindowControl *Sender)
{
  static short i=0;

  if(i++ < 5)
    return 0;
  i=0;
   UpdateAnalysis();

  return 0;
}


void Statistics::FormatTicText(TCHAR *text, const double val, const double step) {
  if (step<1.0) {
    _stprintf(text, TEXT("%.1f"), val);
  } else {
    _stprintf(text, TEXT("%.0f"), val);
  }
}



static void SetCalcCaption(const TCHAR* caption) {
  if (wCalc) {
    wCalc->SetCaption(gettext(caption));
  }
}



static void OnAnalysisPaint(WindowControl * Sender, LKSurface& Surface){

  const RECT& rcgfx = Sender->GetBoundRect();
  LKFont hfOld = Surface.SelectObject(LK8PanelUnitFont/* Sender->GetFont()*/);

    if(INVERTCOLORS)
    {
      Sender->SetBackColor(SKY_HORIZON_COL);
      Surface.SetTextColor(RGB_DARKBLUE);
    }
    else
      Surface.SetTextColor(RGB_WHITE);

  Surface.SetBkMode(TRANSPARENT);
//  SetTextColor(hDC, Sender->GetForeColor());
//  SetTextColor(hDC, Sideview_TextColor);


  switch (analysis_page) {
  case ANALYSIS_PAGE_BAROGRAPH:
    SetCalcCaption(gettext(TEXT("_@M885_"))); // Settings
    Statistics::RenderBarograph(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_CLIMB:
    SetCalcCaption(gettext(TEXT("_@M886_"))); // Task calc
    Statistics::RenderClimb(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_WIND:
    SetCalcCaption(gettext(TEXT("_@M887_"))); // Set wind
    Statistics::RenderWind(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_POLAR:
    SetCalcCaption(gettext(TEXT("_@M885_"))); // Settings
    Statistics::RenderGlidePolar(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_TEMPTRACE:
    SetCalcCaption(gettext(TEXT("_@M885_"))); // Settings
    Statistics::RenderTemperature(Surface, rcgfx);
    break;
  case ANALYSIS_PAGE_TASK:
    SetCalcCaption(gettext(TEXT("_@M886_"))); // Task calc
    LockTaskData();
    Statistics::RenderTask(Surface, rcgfx, false);
    UnlockTaskData();
    break;

  case ANALYSIS_PAGE_CONTEST:
    SetCalcCaption(gettext(TEXT("_@M1451_"))); // Change
    LockTaskData();
    Statistics::RenderContest(Surface, rcgfx);
    UnlockTaskData();
    break;


  case ANALYSIS_PAGE_TASK_SPEED:
    SetCalcCaption(gettext(TEXT("_@M886_"))); // Task calc
    LockTaskData();
    Statistics::RenderSpeed(Surface, rcgfx);
    UnlockTaskData();
    break;
  default:
    // should never get here!
    break;
  }
  Surface.SelectObject(hfOld);

}


static void NextPage(int Step){
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
  UpdateAnalysis();
}


static void OnNextClicked(WindowControl * Sender){
	(void)Sender;

  NextPage(+1);
}

static void OnPrevClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(-1);
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;

  wfa->SetModalResult(mrOK);
}


static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
  (void)Sender; (void)lParam;

  if (waGrid->GetFocused())
    return(0);
  
  switch(wParam & 0xffff){
    case VK_LEFT:
    case '6':
      SetFocus(((WndButton *)wfa->FindByName(TEXT("cmdPrev")))->GetHandle());
      NextPage(-1);
      //((WndButton *)wfa->FindByName(TEXT("cmdPrev")))->SetFocused(true, NULL);
    return(0);
    case VK_RIGHT:
    case '7':
      SetFocus(((WndButton *)wfa->FindByName(TEXT("cmdNext")))->GetHandle());
      NextPage(+1);
      //((WndButton *)wfa->FindByName(TEXT("cmdNext")))->SetFocused(true, NULL);
    return(0);
  }
  return(1);
}

static void OnCalcClicked(WindowControl * Sender){
  (void)Sender;
  if (analysis_page==ANALYSIS_PAGE_BAROGRAPH) {
    dlgBasicSettingsShowModal();
  }
  if (analysis_page==ANALYSIS_PAGE_CLIMB) {
    wfa->SetVisible(false);
    dlgTaskCalculatorShowModal();
    wfa->SetVisible(true);
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
    wfa->SetVisible(false);
    dlgTaskCalculatorShowModal();
    wfa->SetVisible(true);
  }
  if (analysis_page==ANALYSIS_PAGE_CONTEST) {
    // Rotate presented contest
    switch(contestType) {
    case CContestMgr::TYPE_OLC_CLASSIC:
      contestType = CContestMgr::TYPE_FAI_TRIANGLE;
      break;
    case CContestMgr::TYPE_FAI_TRIANGLE:
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
      contestType = CContestMgr::TYPE_OLC_CLASSIC;
      break;

    default:
      contestType = CContestMgr::TYPE_OLC_CLASSIC;
    }
  }

  UpdateAnalysis();
}

static void OnAspBearClicked(WindowControl * Sender){
  (void)Sender;

    UpdateAnalysis();
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

  wfa=NULL;
  waGrid=NULL;
  waInfo=NULL;
  wCalc=NULL;
 entered = true;
  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAnalysis_L.xml"));
    wfa = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_ANALYSIS_L"));
  } else  {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAnalysis.xml"));
    wfa = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_ANALYSIS"));
  }

  if (!wfa) return;

  LKColor COL = LKColor(50,243,45);
  if(INVERTCOLORS)
	COL = COL.ChangeBrightness(0.7);
  penThinSignal.Create(PEN_SOLID, IBLSCALE(2) , COL);


  wfa->SetKeyDownNotify(FormKeyDown);

  waGrid = (WndOwnerDrawFrame*)wfa->FindByName(TEXT("frmGrid"));
  LKASSERT(waGrid);
  if (!waGrid) return;

  waInfo = (WndOwnerDrawFrame*)wfa->FindByName(TEXT("frmInfo"));
  LKASSERT(waInfo);
  if (!waInfo) return;

  wCalc = ((WndButton *)wfa->FindByName(TEXT("cmdCalc")));
  LKASSERT(wCalc);
  if (!wCalc) return;

  WndButton *wClose = (WndButton *)wfa->FindByName(TEXT("cmdClose"));
  LKASSERT(wClose);
  if (!wClose) return;
  wClose->SetOnClickNotify(OnCloseClicked);

  WndButton *wNext = (WndButton *)wfa->FindByName(TEXT("cmdNext"));
  LKASSERT(wNext);
  if (!wNext) return;

  WndButton *wPrev = (WndButton *)wfa->FindByName(TEXT("cmdPrev"));
  LKASSERT(wPrev);
  if (!wPrev) return;

  // In portrait mode, buttons are disposed like:
  // CALC
  // < >
  // CLOSE
  if (!ScreenLandscape) {
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

  waGrid->SetWidth( wfa->GetWidth() - waGrid->GetLeft()-6);

  wfa->SetTimerNotify(OnTimerNotify);

//  UpdateAnalysis();

  if (inPage!=ANALYSYS_PAGE_DEFAULT) analysis_page=inPage;
  UpdateAnalysis();

  wfa->ShowModal();

  delete wfa;

  wfa = NULL;

  penThinSignal.Release();


  MapWindow::RequestFastRefresh();
  FullScreen();

  entered = false;
}



