/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//
// 110628  The new LKAIRSPACE by Kalman with 2.4 page does not need this dialog anymore
// LKairspace will trigger only one event at a time, each one overriding previous.
// This ShowDlg was already disabled in lk8000.cpp and was still here only from manual button inside statistics.
// There we call airspace warning configuration, now. So no need to keep this.

//
#if USEOLDASPWARNINGS

#include "StdAfx.h"
#include <aygshell.h>

#include "InfoBoxLayout.h"

#include "externs.h"
#include "Units.h"
#include "LKAirspace.h"
using std::min;
using std::max;
#include "MapWindow.h"

#include "dlgTools.h"

#include "utils/heapcheck.h"



extern HWND   hWndMainWindow;
extern HWND   hWndMapWindow;
static WndForm *wf=NULL;
static WndListFrame *wAirspaceList=NULL;
static WndOwnerDrawFrame *wAirspaceListEntry = NULL;
static HBRUSH hBrushInsideBk;
static HBRUSH hBrushNearBk;
static HBRUSH hBrushInsideAckBk;
static HBRUSH hBrushNearAckBk;
//static HWND   hActiveWindow;

static int Count=0;
static int ItemIndex=-1;
static int DrawListIndex=-1;
static int FocusedIdx = -1;    // Currently socused airspace List Index
static int SelectedIdx = -1;   // Currently selected airspace List Index
static bool fDialogOpen = false;
CAirspaceList airspaces;
static HBRUSH hBrushNormal;

//void dlgAirspaceWarningNotify(AirspaceWarningNotifyAction_t Action, CAirspace *Airspace);

static void DoAck(int Ack){
  int Idx;

  Idx = SelectedIdx;				//use selected list entry
  if (Idx < 0) Idx = FocusedIdx;	//if none selected use focused
  if (Idx < 0) Idx = 0;				//if none focused?
  if (Idx>=(int)airspaces.size()) return;
  
  CAirspace *p;
  p = airspaces[Idx];
  
	switch (Ack) {
	  default:
	  case -1:		//-1 Ack warning
		CAirspaceManager::Instance().AirspaceAckWarn(*p);
		break;

	  case 3:		//3 Ack airspace
		CAirspaceManager::Instance().AirspaceAckSpace(*p);
		break;

	  case 4:		//4 Dailyack clicked 
		CAirspaceManager::Instance().AirspaceDisable(*p);
		break;

	  case 0:		//0 Enable clicked 
		CAirspaceManager::Instance().AirspaceEnable(*p);
		break;
	}//sw
    wAirspaceList->Redraw();
}

static void OnAckClicked(WindowControl * Sender){
  (void)Sender;
  DoAck(3);
}

static void OnAck1Clicked(WindowControl * Sender){
  (void)Sender;
  DoAck(-1);
}

static void OnAck2Clicked(WindowControl * Sender){
  (void)Sender;
  DoAck(4);
}

static void OnEnableClicked(WindowControl * Sender){
  (void)Sender;
  DoAck(0);
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetVisible(false);
  MapWindow::RequestFastRefresh();

//  SetFocus(hWndMainWindow);
//  SetFocus(hWndMapWindow);

  wf->SetModalResult(mrOK);

}

static int OnTimer(WindowControl * Sender){
  (void)Sender;
  airspaces.clear();
  airspaces = CAirspaceManager::Instance().GetAirspacesInWarning();
  Count = airspaces.size();
  wAirspaceList->Redraw();
  return(0);
}

static int OnKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
  (void)lParam;
	switch(wParam){
    case VK_RETURN:
      if (wAirspaceListEntry->GetFocused()){
        SelectedIdx = FocusedIdx;
        wAirspaceList->Redraw();
        return(0);
      }
      return(1);
    case VK_ESCAPE:
      OnCloseClicked(Sender);
      return(0);
  }

  return(1);
  
}

/*
static void OnDistroy(WindowControl * Sender){
  (void)Sender;
  // TODO code: This currently isn't called!

  AirspaceWarnListRemoveNotifier(AirspaceWarningNotify);
  DeleteObject(hBrushInsideBk);
  DeleteObject(hBrushNearBk);
  DeleteObject(hBrushInsideAckBk);
  DeleteObject(hBrushNearAckBk);

  delete wf;
  wf = NULL;
}
*/



static TCHAR GetAckIndicator(AirspaceWarningLevel_t ackstate)
{
	switch (ackstate) {
	  default:
	  case awNone:
		return _TEXT(" ")[0];			// No ack
	  case awYellow:
		return _TEXT("+")[0];			// Predicted warning acked
	  case awRed:
		return _TEXT("#")[0];			// Warning acked
	}
}

static void OnAirspaceListItemPaint(WindowControl * Sender, HDC hDC){
  TCHAR sTmp[128];

  if (Count != 0){

	TCHAR sName[21];
    TCHAR sTop[32];
    TCHAR sBase[32];
    TCHAR sType[32];
    AIRSPACE_ALT Base;
    AIRSPACE_ALT Top;
    int i = DrawListIndex;
    int          Type;
    int          TextHeight = 12;
    int          TextTop = 1;
    int          Col0Left = 3;
    int          Col1Left = 120;
    RECT         rc;
    RECT         rcTextClip;
    HBRUSH       hBrushBk = NULL;

    if (i>=Count) return;

    CopyRect(&rc, Sender->GetBoundRect());
    CopyRect(&rcTextClip, Sender->GetBoundRect());
    rcTextClip.right = IBLSCALE(Col1Left - 2);

    InflateRect(&rc, IBLSCALE(-2), IBLSCALE(-2));
	if (i>=(int)airspaces.size()) return;
	// Method: get an airspace object copy from CAirspaceManager to use it in this thread indepently
	CAirspace airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspaces[i]);
    if (ItemIndex == DrawListIndex) {
      FocusedIdx = ItemIndex;
    }

	_tcsncpy(sName, airspace_copy.Name(), sizeof(sName)/sizeof(sName[0]));
    sName[sizeof(sName)/sizeof(sName[0])-1] = '\0';
	memcpy(&Base, airspace_copy.Base(), sizeof(Base));
	memcpy(&Top, airspace_copy.Top(), sizeof(Top));
	Type = airspace_copy.Type();

	CAirspaceManager::Instance().GetAirspaceAltText(sTop, sizeof(sTop)/sizeof(sTop[0]), &Top); 
	CAirspaceManager::Instance().GetAirspaceAltText(sBase, sizeof(sBase)/sizeof(sBase[0]), &Base); 
	_tcsncpy(sType, CAirspaceManager::Instance().GetAirspaceTypeShortText(Type), 4);

	// Select brush based on warning and ack level
	hBrushBk = hBrushNormal;
	  if (airspace_copy.WarningLevel() == awYellow) {
		if (airspace_copy.WarningAckLevel() >= awYellow) {
		  hBrushBk = hBrushNearAckBk;
		} else {
		  hBrushBk = hBrushNearBk;
		}
	  }
	  
	  if (airspace_copy.WarningLevel() == awRed) {
		if (airspace_copy.WarningAckLevel() >= awRed) {
		  hBrushBk = hBrushInsideAckBk;
		} else {
		  hBrushBk = hBrushInsideBk;
		}
	  }
    
    if (SelectedIdx == DrawListIndex){
      InflateRect(&rc, 1, 1);
      SelectObject(hDC, (HPEN)GetStockObject(BLACK_PEN));
      Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    } else {
      FillRect(hDC, &rc, hBrushBk);
    }

    if ((airspace_copy.WarningAckLevel() > 0) && (airspace_copy.WarningAckLevel() >= airspace_copy.WarningLevel())) {
      SetTextColor(hDC, clGray);
    }

    #if defined(DEBUG)
    wsprintf(sTmp, TEXT("%-20s%d"), sName , airspace_copy.WarningLevel() - airspace_copy.WarningAckLevel() );
    #else
    if (_tcslen(sName)>0) wsprintf(sTmp, TEXT("%-20s"), sName);  //@ FIX ATTEMPT 101027
	else _tcscpy(sTmp,_T("UNKNOWN")); //@ 101027
    #endif
	
    ExtTextOut(hDC, IBLSCALE(Col0Left), IBLSCALE(TextTop),
      ETO_CLIPPED, &rcTextClip, sTmp, _tcslen(sTmp), NULL);

    wsprintf(sTmp, TEXT("%-20s"), sTop);

    ExtTextOut(hDC, IBLSCALE(Col1Left), IBLSCALE(TextTop),
      ETO_OPAQUE, NULL, sTmp, _tcslen(sTmp), NULL);

    wsprintf(sTmp, TEXT("%-20s"), sBase);

    ExtTextOut(hDC, IBLSCALE(Col1Left), IBLSCALE(TextTop+TextHeight),
      ETO_OPAQUE, NULL, sTmp, _tcslen(sTmp), NULL);
	int hdistance;
	int vdistance;
	int bearing;
	bool inside;
	
	// Unfortunatelly virtual methods don't work on copied instances
	// we have to ask airspacemanager to perform the required calculations
	//inside = airspace_copy.CalculateDistance(&hdistance, &bearing, &vdistance);
	//inside = CAirspaceManager::Instance().AirspaceCalculateDistance(airspaces[i], &hdistance, &bearing, &vdistance);
	bool distances_ready = airspace_copy.GetDistanceInfo(inside, hdistance, bearing, vdistance);
	
	switch (airspace_copy.WarningLevel()) {
	  default:
		if (inside) {
		  wsprintf(sTmp, TEXT("> %c %s"), GetAckIndicator(airspace_copy.WarningAckLevel()), sType );
		} else {
		  wsprintf(sTmp, TEXT("< %c %s"), GetAckIndicator(airspace_copy.WarningAckLevel()), sType );
		}
		break;
	  case awRed:
		if (inside) {
		  wsprintf(sTmp, TEXT("> %c %s %s"), GetAckIndicator(airspace_copy.WarningAckLevel()), sType, gettext(TEXT("_@M789_")));	//LKTOKEN _@M789_ "Warn"
		} else {
		  wsprintf(sTmp, TEXT("< %c %s %s"), GetAckIndicator(airspace_copy.WarningAckLevel()), sType, gettext(TEXT("_@M789_")));	//LKTOKEN _@M789_ "Warn"
		}
		break;
		
	  case awYellow:
		TCHAR DistanceText[MAX_PATH];
		if (!airspace_copy.Flyzone()) {
		  //Non fly zone
		  if (distances_ready) {
			if (hdistance <= 0) {
			  // Directly above or below airspace
			  Units::FormatUserAltitude(fabs(vdistance),DistanceText, 7);
			  if (vdistance > 0) {
				wsprintf(sTmp, TEXT("< %c %s ab %s"), 
						GetAckIndicator(airspace_copy.WarningAckLevel()), 
						sType, DistanceText);
			  } else {
				wsprintf(sTmp, TEXT("< %c %s bl %s"), 
						GetAckIndicator(airspace_copy.WarningAckLevel()), 
						sType, DistanceText);
			  }
			} else {
				// Horizontally separated
				Units::FormatUserDistance(fabs(hdistance),DistanceText, 7);
				wsprintf(sTmp, TEXT("< %c %s H %s"), GetAckIndicator(airspace_copy.WarningAckLevel()),
						sType, DistanceText);
			}
		  } else {
			// distance info not available
			wsprintf(sTmp, TEXT("< %c %s ---"), GetAckIndicator(airspace_copy.WarningAckLevel()), sType );
		  }
		} else {
		  //Fly zone
		  if (distances_ready) {
			if ( abs(hdistance) > abs(vdistance)*30) {
			  // vDist smaller than horizontal
			  Units::FormatUserAltitude(fabs(vdistance),DistanceText, 7);
			  if (vdistance > 0) {
				wsprintf(sTmp, TEXT("> %c %s ab %s"), 
						GetAckIndicator(airspace_copy.WarningAckLevel()), 
						sType, DistanceText);
			  } else {
				wsprintf(sTmp, TEXT("> %c %s bl %s"), 
						GetAckIndicator(airspace_copy.WarningAckLevel()), 
						sType, DistanceText);
			  }
			} else {
				// Horizontally separated
				Units::FormatUserDistance(fabs(hdistance),DistanceText, 7);
				wsprintf(sTmp, TEXT("> %c %s H %s"), GetAckIndicator(airspace_copy.WarningAckLevel()),
						sType, DistanceText);
			}
		  } else {
			// distance info not available
			wsprintf(sTmp, TEXT("> %c %s ---"), GetAckIndicator(airspace_copy.WarningAckLevel()), sType );
		  }
		}
		break;
    }//sw
    ExtTextOut(hDC, IBLSCALE(Col0Left), IBLSCALE(TextTop+TextHeight),
      ETO_CLIPPED, &rcTextClip, sTmp, _tcslen(sTmp), NULL);

  } else {
    if (DrawListIndex == 0){
	// LKTOKEN  _@M469_ = "No Warnings" 
      _stprintf(sTmp, TEXT("%s"), gettext(TEXT("_@M469_")));
      ExtTextOut(hDC, NIBLSCALE(2), NIBLSCALE(2),
        ETO_OPAQUE, NULL,
        sTmp, _tcslen(sTmp), NULL);
    }
  }
}

static void OnAirspaceListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    if (FocusedIdx < 0) {
      FocusedIdx = 0;
    }
    ListInfo->ItemIndex = FocusedIdx;
    ListInfo->ItemCount = max(1,Count);

    ListInfo->DrawIndex = 0;
    ListInfo->ScrollIndex = 0; // JMW bug fix
    DrawListIndex = 0;

  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    FocusedIdx = ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}


int UserMsgNotify(WindowControl *Sender, MSG *msg){

  if (msg->message != WM_USER+1)
    return(1);

  if (!wf->GetVisible())
    return(0);
  // this is our message, we have handled it.
  return(0);
}


extern bool RequestAirspaceWarningDialog;


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAckClicked),
  DeclareCallBackEntry(OnAck1Clicked),
  DeclareCallBackEntry(OnAck2Clicked),
  DeclareCallBackEntry(OnEnableClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnAirspaceListInfo),
  DeclareCallBackEntry(OnAirspaceListItemPaint),
  DeclareCallBackEntry(NULL)
};

/*
bool dlgAirspaceWarningShow(void){
  if (Count == 0)
    return(false);
  actShow = true;
  PostMessage(wf->GetHandle(), WM_USER+1, 0, 0);
  return(true);
}
*/

bool dlgAirspaceWarningIsEmpty(void) {
    return (airspaces.size() == 0);
}

// WARING: may only be called from MapWindow event loop!
// JMW this is now called from ProcessCommon (main GUI loop)

bool dlgAirspaceWarningVisible(void) {
  return fDialogOpen;
}


int dlgAirspaceWarningInit(void){

  int res = 0;

#ifdef HAVEEXCEPTIONS
  __try{
#endif

//    hActiveWindow = GetActiveWindow();

    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspaceWarning.xml")); // unused, removable

  if (!InfoBoxLayout::landscape)
	wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_AIRSPACEWARNING_L"));
  else
	wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_AIRSPACEWARNING"));

    if (wf) {

      wf->SetKeyDownNotify(OnKeyDown);
      wf->SetUserMsgNotify(UserMsgNotify);
      wf->SetTimerNotify(OnTimer);
	  hBrushNormal = (HBRUSH)CreateSolidBrush(RGB(197,223,194));
      hBrushInsideBk = (HBRUSH)CreateSolidBrush(RGB(254,50,50));
      hBrushNearBk = (HBRUSH)CreateSolidBrush(RGB(254,254,50));
      hBrushInsideAckBk = (HBRUSH)CreateSolidBrush(RGB(254,100,100));
      hBrushNearAckBk = (HBRUSH)CreateSolidBrush(RGB(254,254,100));

      wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceWarningList"));
      wAirspaceListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceWarningListEntry"));
      wAirspaceListEntry->SetCanFocus(true);
      wf->Close();  // hide the window

    } else StartupStore(_T("------ AirspaceWarning setup FAILED!%s"),NEWLINE); //@ 101027


#ifdef HAVEEXCEPTIONS
  }__except(EXCEPTION_EXECUTE_HANDLER ){

    res = 0;
    // ToDo: log that problem

  };
#endif

  return(res);

}

int dlgAirspaceWarningDeInit(void){

  if (wf)
    wf->SetVisible(false);

  // 110106  missing delete brush objects here. Minor malis.
  delete wf;
  wf = NULL;

  return(0);
  
}

#endif // USEOLDASPWARNINGS
