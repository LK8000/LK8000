/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "StdAfx.h"
#include <aygshell.h>

#include "InfoBoxLayout.h"

#include "externs.h"
#include "Units.h"
#ifdef LKAIRSPACE
#include "LKAirspace.h"
#endif
#if defined(LKAIRSPACE) || defined(NEW_OLC)
using std::min;
using std::max;
#else
#include "Airspace.h"
#endif
#include "MapWindow.h"

#include "dlgTools.h"

#include "utils/heapcheck.h"

// 110102 Note from paolo
// the entire airspace warning system in xcsoar 5.2.4 which is still in use within lk8000 should be trashed.
// The fake mechanism of message queues here does not work, and it is also shared between two different threads.
// TODO ASAP entirely with a more simple approach.
// This system may still cause crashes, despite my attempts to fix here and there possible conflicts.
// Sadly we cannot simply disable airspace warnings right now.

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
#ifndef LKAIRSPACE
static int FocusedID = -1;     // Currently focused airspace ID
#endif
static int FocusedIdx = -1;    // Currently socused airspace List Index
#ifndef LKAIRSPACE
static int SelectedID = -1;    // Currently selected airspace ID
#endif
static int SelectedIdx = -1;   // Currently selected airspace List Index
static bool fDialogOpen = false;
#ifdef LKAIRSPACE
CAirspaceList airspaces;
static HBRUSH hBrushNormal;

//void dlgAirspaceWarningNotify(AirspaceWarningNotifyAction_t Action, CAirspace *Airspace);
#else
void AirspaceWarningNotify(AirspaceWarningNotifyAction_t Action, AirspaceInfo_c *AirSpace);
#endif

#ifdef LKAIRSPACE
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
#else
static void DoAck(int Ack){
  AirspaceInfo_c pAS;
  int Idx;

  if (!wAirspaceListEntry->GetFocused())
    Idx = SelectedIdx;
  else
    Idx = ItemIndex;

  if (Idx < 0)
    Idx = 0;
  if (AirspaceWarnGetItem(Idx, pAS)){
    AirspaceWarnDoAck(pAS.ID, Ack);
    wAirspaceList->Redraw();
  }
}
#endif

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
#ifdef LKAIRSPACE
  airspaces.clear();
  airspaces = CAirspaceManager::Instance().GetAirspacesInWarning();
  Count = airspaces.size();
  wAirspaceList->Redraw();
#endif
  return(0);
}

static int OnKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
  (void)lParam;
	switch(wParam){
    case VK_RETURN:
      if (wAirspaceListEntry->GetFocused()){
#ifndef LKAIRSPACE
        SelectedID = FocusedID;
#endif
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

#ifndef LKAIRSPACE
static void getAirspaceType(TCHAR *buf, int Type){
  switch (Type)
    {
    case RESTRICT:
      _tcscpy(buf, TEXT("LxR"));
      return;
    case PROHIBITED:
      _tcscpy(buf, TEXT("LxP"));
      return;
    case DANGER:
      _tcscpy(buf, TEXT("LxD"));
      return;
    case CLASSA:
      _tcscpy(buf, TEXT("A"));
      return;
    case CLASSB:
      _tcscpy(buf, TEXT("B"));
      return;
    case CLASSC:
      _tcscpy(buf, TEXT("C"));
      return;
    case CLASSD:
      _tcscpy(buf, TEXT("D"));
      return;
    case CLASSE:
      _tcscpy(buf, TEXT("E"));
      return;
    case CLASSF:
      _tcscpy(buf, TEXT("F"));
      return;
    case CLASSG:
      _tcscpy(buf, TEXT("G"));
      return;
    case NOGLIDER:
      _tcscpy(buf, TEXT("NoGld"));
      return;
    case CTR:
      _tcscpy(buf, TEXT("CTR"));
      return;
    case WAVE:
      _tcscpy(buf, TEXT("Wav"));
      return;
    case CLASSTMZ:
      _tcscpy(buf, TEXT("TMZ"));
      return;
    default:
      _tcscpy(buf, TEXT("?"));
      return;
    }
}

static double FLAltRounded(double alt) {
  int f = iround(alt/10)*10;
  return (double)f;
}
#endif

#ifndef LKAIRSPACE
static TCHAR *fmtAirspaceAlt(TCHAR *Buffer, AIRSPACE_ALT *alt){

  TCHAR sUnitBuffer[24];
  TCHAR sAltUnitBuffer[24];

  Units::FormatUserAltitude(alt->Altitude, sUnitBuffer, 
			    sizeof(sUnitBuffer)/sizeof(sUnitBuffer[0]));
  Units::FormatAlternateUserAltitude(alt->Altitude, 
				     sAltUnitBuffer, 
		       sizeof(sAltUnitBuffer)/sizeof(sAltUnitBuffer[0]));

  switch (alt->Base){
    case abUndef:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(Buffer, TEXT("%s %s"), sUnitBuffer, sAltUnitBuffer);
      } else {
	_stprintf(Buffer, TEXT("%s"), sUnitBuffer);
      }
    break;
    case abMSL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(Buffer, TEXT("%s %s MSL"), sUnitBuffer, sAltUnitBuffer);
      } else {
	_stprintf(Buffer, TEXT("%s MSL"), sUnitBuffer);
      }
    break;
    case abAGL:
      if (alt->Altitude == 0)
        _stprintf(Buffer, TEXT("SFC"));
      else {
	Units::FormatUserAltitude(alt->AGL, sUnitBuffer, 
				  sizeof(sUnitBuffer)/sizeof(sUnitBuffer[0]));
	Units::FormatAlternateUserAltitude(alt->AGL, sAltUnitBuffer, 
			    sizeof(sAltUnitBuffer)/sizeof(sAltUnitBuffer[0]));
	if (Units::GetUserAltitudeUnit() == unMeter) {
	  _stprintf(Buffer, TEXT("%s %s AGL"), sUnitBuffer, sAltUnitBuffer);
	} else {
	  _stprintf(Buffer, TEXT("%s AGL"), sUnitBuffer);
	}
      }
    break;
    case abFL:
      /*AltitudeToQNHAltitude(alt->Altitude)*/
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(Buffer, TEXT("FL%.0f %.0f m %.0f ft"), 
		  alt->FL, FLAltRounded(alt->Altitude),
		  FLAltRounded(alt->Altitude*TOFEET));
      } else {
	_stprintf(Buffer, TEXT("FL%.0f %.0f ft"), 
		  alt->FL, FLAltRounded(alt->Altitude*TOFEET));
      }
    break;
  }
  return(Buffer);
}
#endif

#ifdef LKAIRSPACE
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
#endif

static void OnAirspaceListItemPaint(WindowControl * Sender, HDC hDC){
  TCHAR sTmp[128];

  if (Count != 0){

#ifndef LKAIRSPACE
    TCHAR sAckIndicator[6] = TEXT(" -++*");
#endif
	TCHAR sName[21];
    TCHAR sTop[32];
    TCHAR sBase[32];
    TCHAR sType[32];
    AIRSPACE_ALT Base;
    AIRSPACE_ALT Top;
    int i = DrawListIndex;
#ifndef LKAIRSPACE
    AirspaceInfo_c pAS;
#endif
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
#ifdef LKAIRSPACE
	if (i>=(int)airspaces.size()) return;
	// Method: get an airspace object copy from CAirspaceManager to use it in this thread indepently
	CAirspace airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspaces[i]);
    if (ItemIndex == DrawListIndex) {
      FocusedIdx = ItemIndex;
    }
#else
    if (!AirspaceWarnGetItem(i, pAS)) return;
    if (ItemIndex == DrawListIndex){
      FocusedID = pAS.ID;
    }
#endif

#ifdef LKAIRSPACE
	_tcsncpy(sName, airspace_copy.Name(), sizeof(sName)/sizeof(sName[0]));
    sName[sizeof(sName)/sizeof(sName[0])-1] = '\0';
	memcpy(&Base, airspace_copy.Base(), sizeof(Base));
	memcpy(&Top, airspace_copy.Top(), sizeof(Top));
	Type = airspace_copy.Type();

	CAirspaceManager::Instance().GetAirspaceAltText(sTop, sizeof(sTop)/sizeof(sTop[0]), &Top); 
	CAirspaceManager::Instance().GetAirspaceAltText(sBase, sizeof(sBase)/sizeof(sBase[0]), &Base); 
	_tcsncpy(sType, CAirspaceManager::Instance().GetAirspaceTypeShortText(Type), 4);
#else
    if (pAS.IsCircle){
      _tcsncpy(sName, AirspaceCircle[pAS.AirspaceIndex].Name, 
	       sizeof(sName)/sizeof(sName[0]));
      Base = AirspaceCircle[pAS.AirspaceIndex].Base;
      Top  = AirspaceCircle[pAS.AirspaceIndex].Top;
      Type = AirspaceCircle[pAS.AirspaceIndex].Type;
    } else {
      _tcsncpy(sName, AirspaceArea[pAS.AirspaceIndex].Name, 
	       sizeof(sName)/sizeof(sName[0]));
      Base = AirspaceArea[pAS.AirspaceIndex].Base;
      Top  = AirspaceArea[pAS.AirspaceIndex].Top;
      Type = AirspaceArea[pAS.AirspaceIndex].Type;
    }

    if (_tcslen(sName)>0) // 100324
    sName[sizeof(sName)/sizeof(sName[0])-1] = '\0';

    fmtAirspaceAlt(sTop, &Top);
    fmtAirspaceAlt(sBase, &Base);
    getAirspaceType(sType, Type);
#endif

#ifdef LKAIRSPACE
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
#else
    if (pAS.Inside){
      if (pAS.Acknowledge >= 3)
        hBrushBk = hBrushInsideAckBk;
      else
        hBrushBk = hBrushInsideBk;
    } else { 
      if ((pAS.hDistance < 2500) && (abs(pAS.vDistance) < 250)) {
        if (pAS.Acknowledge >= 1) 
          hBrushBk = hBrushNearAckBk;
        else
          hBrushBk = hBrushNearBk;
	  }
    }
#endif
    
    if (SelectedIdx == DrawListIndex){
      InflateRect(&rc, 1, 1);
      SelectObject(hDC, (HPEN)GetStockObject(BLACK_PEN));
      Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    } else {
      FillRect(hDC, &rc, hBrushBk);
    }

#ifdef LKAIRSPACE
    if ((airspace_copy.WarningAckLevel() > 0) && (airspace_copy.WarningAckLevel() >= airspace_copy.WarningLevel())) {
      SetTextColor(hDC, clGray);
    }

    #if defined(DEBUG)
    wsprintf(sTmp, TEXT("%-20s%d"), sName , airspace_copy.WarningLevel() - airspace_copy.WarningAckLevel() );
    #else
    if (_tcslen(sName)>0) wsprintf(sTmp, TEXT("%-20s"), sName);  //@ FIX ATTEMPT 101027
	else _tcscpy(sTmp,_T("UNKNOWN")); //@ 101027
    #endif
#else
    if ((pAS.Acknowledge > 0) && (pAS.Acknowledge >= pAS.WarnLevel)){
      SetTextColor(hDC, clGray);
    }

    #if defined(DEBUG)
    wsprintf(sTmp, TEXT("%-20s%d"), sName , pAS.WarnLevel - pAS.Acknowledge);
    #else
    if (_tcslen(sName)>0) wsprintf(sTmp, TEXT("%-20s"), sName);  //@ FIX ATTEMPT 101027
	else _tcscpy(sTmp,_T("UNKNOWN")); //@ 101027
    #endif
#endif
	
    ExtTextOut(hDC, IBLSCALE(Col0Left), IBLSCALE(TextTop),
      ETO_CLIPPED, &rcTextClip, sTmp, _tcslen(sTmp), NULL);

    wsprintf(sTmp, TEXT("%-20s"), sTop);

    ExtTextOut(hDC, IBLSCALE(Col1Left), IBLSCALE(TextTop),
      ETO_OPAQUE, NULL, sTmp, _tcslen(sTmp), NULL);

    wsprintf(sTmp, TEXT("%-20s"), sBase);

    ExtTextOut(hDC, IBLSCALE(Col1Left), IBLSCALE(TextTop+TextHeight),
      ETO_OPAQUE, NULL, sTmp, _tcslen(sTmp), NULL);
#ifdef LKAIRSPACE
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
#else
    if (pAS.Inside){
      wsprintf(sTmp, TEXT("> %c %s"), sAckIndicator[pAS.Acknowledge], sType);
    } else {
      TCHAR DistanceText[MAX_PATH];
      if (pAS.hDistance == 0) {

        // Directly above or below airspace

        Units::FormatUserAltitude(fabs((double)pAS.vDistance),DistanceText, 7);
        if (pAS.vDistance > 0) {
          wsprintf(sTmp, TEXT("< %c %s ab %s"), 
                   sAckIndicator[pAS.Acknowledge], 
                   sType, DistanceText);
        }
        if (pAS.vDistance < 0) {
          Units::FormatUserAltitude(fabs((double)pAS.vDistance),DistanceText, 7);
          wsprintf(sTmp, TEXT("< %c %s bl %s"), 
                   sAckIndicator[pAS.Acknowledge], 
                   sType, DistanceText);
        }
      } else {
        if ((pAS.vDistance == 0) || 
            (pAS.hDistance < abs(pAS.vDistance)*30 )) {

          // Close to airspace altitude, horizontally separated

          Units::FormatUserDistance(fabs((double)pAS.hDistance),DistanceText, 7);
          wsprintf(sTmp, TEXT("< %c %s H %s"), sAckIndicator[pAS.Acknowledge],
                   sType, DistanceText);
        } else {

          // Effectively above or below airspace, steep climb or descent 
          // necessary to enter

          Units::FormatUserAltitude(fabs((double)pAS.vDistance),DistanceText, 7);
          if (pAS.vDistance > 0) {
            wsprintf(sTmp, TEXT("< %c %s ab %s"), 
                     sAckIndicator[pAS.Acknowledge], 
                     sType, DistanceText); 
          } else {
            wsprintf(sTmp, TEXT("< %c %s bl %s"), 
                     sAckIndicator[pAS.Acknowledge], sType, 
                     DistanceText);
          }
        }
      }
    }
#endif  
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


#ifndef LKAIRSPACE
bool actShow = false;
bool actListSizeChange = false;
bool actListChange = false;

static bool FindFocus() {
  bool do_refocus = false;

  FocusedIdx = 0;
#ifdef LKAIRSPACE
  FocusedIdx = CAirspaceManager::Instance().AirspaceWarnFindIndexByID(FocusedID);
#else
  FocusedIdx = AirspaceWarnFindIndexByID(FocusedID);
#endif
  if (FocusedIdx < 0) {
    FocusedIdx = 0;
    FocusedID = -1; // JMW bug fix
      
    if (wAirspaceListEntry->GetFocused()) {
      // JMW attempt to find fix...
      do_refocus = true;
    }
  }
#ifdef LKAIRSPACE
  SelectedIdx = CAirspaceManager::Instance().AirspaceWarnFindIndexByID(SelectedID);
#else
  SelectedIdx = AirspaceWarnFindIndexByID(SelectedID);
#endif
  if (SelectedIdx < 0){
    SelectedID = -1;
  }
  return do_refocus;
}
#endif

int UserMsgNotify(WindowControl *Sender, MSG *msg){

  if (msg->message != WM_USER+1)
    return(1);

  if (!wf->GetVisible())
    return(0);
#ifndef LKAIRSPACE
  bool do_refocus = false;

  if (actListSizeChange){
    actListSizeChange = false;
#ifdef LKAIRSPACE
    Count = airspaces.size();
#else
    Count = AirspaceWarnGetItemCount();
#endif
    do_refocus = FindFocus();

    wAirspaceList->ResetList();

    if (Count == 0) {
      // auto close
      OnCloseClicked(Sender);
    }
  }

  if (actShow){
    actShow = false;
    if (!do_refocus) {
      do_refocus = FindFocus();
    }

    /*
    if (!wf->GetVisible()){
      Count = AirspaceWarnGetItemCount();
      wAirspaceList->ResetList();
      FocusedIdx = 0;
      FocusedID = -1;
      wf->Show();
      SetFocus(wAirspaceListEntry->GetHandle());
    } else {
      SetFocus(wAirspaceListEntry->GetHandle());
    }
    */
    //    return(0);
  }

  if (actListChange) {
    actListChange = false;
    wAirspaceList->Redraw();
  }

  if (do_refocus) {
    SetFocus(wAirspaceListEntry->GetHandle());    
  }
#endif
  // this is our message, we have handled it.
  return(0);
}


extern bool RequestAirspaceWarningDialog;

// WARNING: this is NOT called from the windows thread!
#ifdef LKAIRSPACE
/*void dlgAirspaceWarningNotify(AirspaceWarningNotifyAction_t Action, 
                           CAirspace *Airspace) {

  if ((Action == asaItemAdded) || (Action == asaWarnLevelIncreased)) {
    actShow = true;
  }

  if ((Action == asaItemAdded) || (Action == asaItemRemoved) 
      || (Action == asaClearAll)) {
    actListSizeChange = true;
  }

  if ((Action == asaItemChanged) || (Action == asaWarnLevelIncreased)){
    actListChange = true;
  }

  if ((Action == asaProcessEnd) && (actShow || actListSizeChange || actListChange)){
    if (fDialogOpen) {
      PostMessage(wf->GetHandle(), WM_USER+1, 0, 0);
    }
    else {
	  if (actShow) RequestAirspaceWarningDialog= true;
      // JMW this is bad! PostMessage(hWndMapWindow, WM_USER+1, 0, 0);  
      // (Makes it serviced by the main gui thread, much better)
    }
    // sync dialog with MapWindow (event processing etc)
  }

}*/
#else
void AirspaceWarningNotify(AirspaceWarningNotifyAction_t Action, 
                           AirspaceInfo_c *AirSpace) {
  (void)AirSpace;
  if ((Action == asaItemAdded) || (Action == asaItemRemoved) 
      || (Action == asaWarnLevelIncreased)) {
    actShow = true;
  }

  if ((Action == asaItemAdded) || (Action == asaItemRemoved) 
      || (Action == asaClearAll)) {
    actListSizeChange = true;
  }

  if ((Action == asaItemChanged) || (Action == asaWarnLevelIncreased)){
    actListChange = true;
  }

  if ((Action == asaProcessEnd) && (actShow || actListSizeChange || actListChange)){
    if (fDialogOpen) {
      PostMessage(wf->GetHandle(), WM_USER+1, 0, 0);
    }
    else {
      RequestAirspaceWarningDialog= true;
      // JMW this is bad! PostMessage(hWndMapWindow, WM_USER+1, 0, 0);  
      // (Makes it serviced by the main gui thread, much better)
    }
    // sync dialog with MapWindow (event processing etc)
  }

}
#endif


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
#ifdef LKAIRSPACE
    return (airspaces.size() == 0);
#else
  return (AirspaceWarnGetItemCount()==0);
#endif
}

// WARING: may only be called from MapWindow event loop!
// JMW this is now called from ProcessCommon (main GUI loop)

bool dlgAirspaceWarningVisible(void) {
  return fDialogOpen;
}

#if LKAIRSPACE
// LKairspace will trigger only one event at a time, each one overriding previous.
// This ShowDlg was already disabled in lk8000.cpp and was still here only from manual button inside statistics.
// There we call airspace warning configuration, now. So no need to keep this.
#else
bool dlgAirspaceWarningShowDlg(bool Force){

  if (fDialogOpen)
    return(false);
#ifndef LKAIRSPACE
  if (!actShow && !Force)
    return false;
#endif
#ifdef LKAIRSPACE
	airspaces = CAirspaceManager::Instance().GetAirspacesInWarning();
    Count = airspaces.size();
#else
  Count = AirspaceWarnGetItemCount();
#endif
  if (Count == 0)
    return(false);

  if (wf==NULL) {
	StartupStore(_T("------ BAD SW ERROR AirspaceWarningDlg problem no window!%s"),NEWLINE);
	FailStore(_T("BAD SW ERROR AirspaceWarningDlg problem no window!"));
	return(false);
  }
  if (wAirspaceList==NULL) {
	StartupStore(_T("------ BAD SW ERROR AirspaceList empty!%s"),NEWLINE);
	FailStore(_T("BAD SW ERROR AirspaceList empty!"));
	return(false);
  }

  wAirspaceList->ResetList();

  if (!fDialogOpen) {
    fDialogOpen = true;
    #ifndef DISABLEAUDIO
    if (EnableSoundModes) LKSound(_T("LK_AIRSPACE.WAV")); // 100819
    #endif
    HWND oldFocusHwnd = GetFocus();
    wf->ShowModal();
    if (oldFocusHwnd) {
      SetFocus(oldFocusHwnd);
    }


    // JMW need to deselect everything on new reopening of dialog
#ifndef LKAIRSPACE
    SelectedID = -1;
#endif
    SelectedIdx = -1;
#ifndef LKAIRSPACE
    FocusedID = -1;
#endif
    FocusedIdx = -1;
#ifdef LKAIRSPACE
	airspaces.clear();
#endif
    fDialogOpen = false;

    //    SetFocus(hWndMapWindow);
    // JMW why do this? --- not necessary?
    // concerned it may interfere if a dialog is already open
  }

  return(true);
}
#endif


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
#ifdef LKAIRSPACE
	  hBrushNormal = (HBRUSH)CreateSolidBrush(RGB(197,223,194));
#endif
      hBrushInsideBk = (HBRUSH)CreateSolidBrush(RGB(254,50,50));
      hBrushNearBk = (HBRUSH)CreateSolidBrush(RGB(254,254,50));
      hBrushInsideAckBk = (HBRUSH)CreateSolidBrush(RGB(254,100,100));
      hBrushNearAckBk = (HBRUSH)CreateSolidBrush(RGB(254,254,100));

      wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceWarningList"));
      wAirspaceListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceWarningListEntry"));
      wAirspaceListEntry->SetCanFocus(true);
#ifdef LKAIRSPACE
#else
      AirspaceWarnListAddNotifier(AirspaceWarningNotify);
#endif
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
#ifdef LKAIRSPACE
#else
  AirspaceWarnListRemoveNotifier(AirspaceWarningNotify);
#endif
  delete wf;
  wf = NULL;

  return(0);
  
}
