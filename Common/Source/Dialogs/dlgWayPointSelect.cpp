/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayPointSelect.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "DoInits.h"
#include "TraceThread.h"
#include <ctype.h>
#include "dlgTools.h"
#include "Event/Event.h"
#include "resource.h"
#include "NavFunctions.h"


typedef struct{
  int Index;
  double Distance;
  double Direction;
  mutable int    DirectionErr;
  int    Type;
  int    FileIdx;
  unsigned int FourChars;
} WayPointSelectInfo_t;

#define NAMEFILTERLEN 10
static TCHAR sNameFilter[NAMEFILTERLEN+1];
static WndButton *wpnewName;

static double Latitude;
static double Longitude;

static WndForm *wf=NULL;
static WndListFrame *wWayPointList=NULL;
static WndOwnerDrawFrame *wWayPointListEntry = NULL;

//static const TCHAR NameFilter[] = TEXT("*ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
//static unsigned NameFilterIdx=0;

static double DistanceFilter[] = {0.0, 25.0, 50.0, 75.0, 100.0, 150.0, 250.0, 500.0, 1000.0};
static unsigned DistanceFilterIdx=0;

#define DirNoFilter 0
#define DirHDG -1
#define DirBRG  -2
#define DirAhead -3
static int DirectionFilter[] = {DirNoFilter, DirHDG, DirBRG, 360, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330};
static unsigned DirectionFilterIdx=0;
static int lastHeading=0;

#define TYPEFILTERSNUM	(4+NO_WP_FILES)
static   TCHAR TypeFilter[TYPEFILTERSNUM][50];
static unsigned TypeFilterIdx=0;

static int UpLimit=0;
static int LowLimit=0;

static bool FullFlag=false; // 100502

static int ItemIndex = -1;

static int SelectedWayPointFileIdx = 0;

static void OnWaypointListEnter(WindowControl * Sender,
				WndListFrame::ListInfo_t *ListInfo){
	(void)Sender; (void)ListInfo;

  if(Sender) {
    WndForm * pForm = Sender->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult((ItemIndex != -1) ? mrOK : mrCancel);
    }
  }
}


static WayPointSelectInfo_t *WayPointSelectInfo=NULL;
static int *StrIndex=NULL;


int GetTaskBearing() {
	int value=0;
	if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
		int index = Task[ActiveTaskPoint].Index;
		if (index>=0) {
			if (UseAATTarget())
				value=CALCULATED_INFO.WaypointBearing;
			else
				value = WayPointCalc[index].Bearing;
		}
	}
	return AngleLimit360(value);
}

static int WaypointNameCompare(const void *elem1, const void *elem2 ){
  if (((const WayPointSelectInfo_t *)elem1)->FourChars < ((const WayPointSelectInfo_t *)elem2)->FourChars)
    return (-1);
  if (((const WayPointSelectInfo_t *)elem1)->FourChars > ((const WayPointSelectInfo_t *)elem2)->FourChars)
    return (+1);
  return (0);
}

static int WaypointDistanceCompare(const void *elem1, const void *elem2 ){
  if (((const WayPointSelectInfo_t *)elem1)->Distance < ((const WayPointSelectInfo_t *)elem2)->Distance)
    return (-1);
  if (((const WayPointSelectInfo_t *)elem1)->Distance > ((const WayPointSelectInfo_t *)elem2)->Distance)
    return (+1);
  return (0);
}

static int WaypointAirportCompare(const void *elem1, const void *elem2 ){
  if (((const WayPointSelectInfo_t *)elem1)->Type & (AIRPORT))
    return (-1);
  return (+1);
}

static int WaypointLandableCompare(const void *elem1, const void *elem2 ){
  if (((const WayPointSelectInfo_t *)elem1)->Type & (AIRPORT | LANDPOINT))
    return (-1);
  return (+1);
}

static int WaypointWayPointCompare(const void *elem1, const void *elem2 ){
  if (((const WayPointSelectInfo_t *)elem1)->Type & (TURNPOINT))
    return (-1);
  return (+1);
}

static int WaypointFileIdxCompare(const void *elem1, const void *elem2 ){
  if (((const WayPointSelectInfo_t *)elem1)->FileIdx != SelectedWayPointFileIdx)
    return (+1);
  return (-1);
}

static int WaypointDirectionCompare(const void *elem1, const void *elem2 ){

  int a, a1, a2;


  switch (DirectionFilter[DirectionFilterIdx])
  {

    case DirHDG:
    case DirAhead:
      a = iround(CALCULATED_INFO.Heading);
    break;

    case DirBRG:
    	a = GetTaskBearing();
	  break;
    case DirNoFilter:
    default:
      a = DirectionFilter[DirectionFilterIdx];
    break;
  }

  lastHeading = a;

  a1 = (int)(((const WayPointSelectInfo_t *)elem1)->Direction - a);
  a2 = (int)(((const WayPointSelectInfo_t *)elem2)->Direction - a);

  if (a1 > 180)
    a1 -=360;

  if (a1 < -180)
    a1 +=360;

  if (a2 > 180)
    a2 -=360;

  if (a2 < -180)
    a2 +=360;

  a1 = abs(a1);
  a2 = abs(a2);

  ((const WayPointSelectInfo_t *)elem1)->DirectionErr = a1;
  ((const WayPointSelectInfo_t *)elem2)->DirectionErr = a2;

  if (a1 < a2)
    return (-1);
  if (a1 > a2)
    return (+1);

  return (0);
}

static void SetWPNameCaption(const TCHAR* tFilter) {

  TCHAR namfilter[50];
  if ( _tcscmp(tFilter,_T("*")) == 0)
	_tcscpy(namfilter,_T("*"));
  else {
	if (_tcslen(tFilter) <GC_SUB_STRING_THRESHOLD)
		_stprintf(namfilter,_T("%s*"),tFilter);
	else
	  {
		if (_tcslen(tFilter) <6)
			_stprintf(namfilter,_T("*%s*"),tFilter);
		else {
			_tcscpy(namfilter,_T("*"));
			_tcsncat(namfilter,tFilter,5);
			namfilter[5]='\0';
			_tcscat(namfilter,_T("..*"));
		}
	}

  }
//  StartupStore(_T("SETCAPTION <%s>\n"),namfilter);
  wpnewName->SetCaption(namfilter);

}

unsigned int numvalidwp=0;

static void PrepareData(void){

  TCHAR sTmp[20];
  numvalidwp=0; // Reset them on entry!!

  if (WayPointList.empty()) return;

  sNameFilter[0]='\0';
  SetWPNameCaption(TEXT("*"));
  WayPointSelectInfo = (WayPointSelectInfo_t*)malloc(sizeof(WayPointSelectInfo_t) * WayPointList.size());
  if (WayPointSelectInfo==NULL) {
	OutOfMemory(_T(__FILE__),__LINE__);
	return;
  }
  memset(WayPointSelectInfo, 0, sizeof(WayPointSelectInfo_t) * WayPointList.size());

  StrIndex = (int*)malloc(sizeof(int)*(WayPointList.size()+1));
  if (StrIndex==NULL) {
	OutOfMemory(_T(__FILE__),__LINE__);
	free(WayPointSelectInfo);
	WayPointSelectInfo=NULL;
	return;
  }

  for (int i=0; i<(int)WayPointList.size(); i++){

    LKASSERT(numvalidwp<=WayPointList.size());

    if (WayPointList[i].Latitude==RESWP_INVALIDNUMBER) continue;
    WayPointSelectInfo[numvalidwp].Index = i;

    DistanceBearing(Latitude,
                    Longitude,
                    WayPointList[i].Latitude,
                    WayPointList[i].Longitude,
                    &(WayPointSelectInfo[numvalidwp].Distance),
                    &(WayPointSelectInfo[numvalidwp].Direction));
    WayPointSelectInfo[numvalidwp].Distance *= DISTANCEMODIFY;

    LK_tcsncpy(sTmp, WayPointList[i].Name, 4);
    CharUpper(sTmp);

    WayPointSelectInfo[numvalidwp].FourChars =
                    (((unsigned)sTmp[0] & 0xff) << 24)
                  + (((unsigned)sTmp[1] & 0xff) << 16)
                  + (((unsigned)sTmp[2] & 0xff) << 8)
                  + (((unsigned)sTmp[3] & 0xff) );

    WayPointSelectInfo[numvalidwp].Type = WayPointList[i].Flags;

    WayPointSelectInfo[numvalidwp].FileIdx = WayPointList[i].FileNum;
    numvalidwp++;

  }
  // we exit with numvalidwp pointing to next *not valid* position
  // so all checks must be done using <numvalidwp

  qsort(WayPointSelectInfo, UpLimit,
      sizeof(WayPointSelectInfo_t), WaypointNameCompare);

}


static void UpdateList(void){

  // no waypoints, no party
  if (numvalidwp<1) {
	LowLimit=0;
	UpLimit=0;
	return;
  }

//  TCHAR sTmp[128];
  int i;
  bool distancemode = false;

  ItemIndex = 0;

  UpLimit=numvalidwp;
  LKASSERT(UpLimit>0);
  LowLimit =0;

  FullFlag=false; // 100502

  if (TypeFilterIdx == 1){
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointAirportCompare);
    for (i=0; i<UpLimit; i++){
      if (!(WayPointSelectInfo[i].Type & (AIRPORT))){
        UpLimit = i;
        break;
      }
    }
  } else if (TypeFilterIdx == 2){
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointLandableCompare);
    for (i=0; i<UpLimit; i++){
      if (!(WayPointSelectInfo[i].Type & (AIRPORT | LANDPOINT))){
        UpLimit = i;
        break;
      }
    }
  } else if (TypeFilterIdx == 3){
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointWayPointCompare);
    for (i=0; i<UpLimit; i++){
      if (!(WayPointSelectInfo[i].Type & (TURNPOINT))){
        UpLimit = i;
        break;
      }
    }
  } else if (TypeFilterIdx >= 4 && TypeFilterIdx < (4+NO_WP_FILES)){
    // distancemode = true;
    SelectedWayPointFileIdx = TypeFilterIdx-4;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointFileIdxCompare);
    for (i=0; i<UpLimit; i++){
      if (WayPointSelectInfo[i].FileIdx != SelectedWayPointFileIdx){
        UpLimit = i;
        break;
      }
    }
  }

  if (DistanceFilterIdx != 0){
    distancemode = true;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointDistanceCompare);
    for (i=0; i<UpLimit; i++){
      if (WayPointSelectInfo[i].Distance > DistanceFilter[DistanceFilterIdx]){
        UpLimit = i;
        break;
      }
    }
  }

  if (DirectionFilterIdx != 0){
    distancemode = true;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointDirectionCompare);
    for (i=0; i<UpLimit; i++){
      if (WayPointSelectInfo[i].DirectionErr > 18){
        UpLimit = i;
        break;
      }
    }
  }

#if 100502
    TCHAR sTmp[NAMEFILTERLEN+1];
    TCHAR wname[EXT_NAMESIZE];
    LowLimit = UpLimit;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointNameCompare);

    LK_tcsncpy(sTmp,sNameFilter, NAMEFILTERLEN);
    CharUpper(sTmp);
    int iFilterLen = _tcslen(sNameFilter);

    if (iFilterLen<GC_SUB_STRING_THRESHOLD)
    {
    for (i=0; i<UpLimit; i++){
      // compare entire name which may be more than 4 chars

	LKASSERT(WayPointSelectInfo[i].Index>=0 && WayPointSelectInfo[i].Index<(signed)WayPointList.size());
	LK_tcsncpy(wname,WayPointList[WayPointSelectInfo[i].Index].Name, NAME_SIZE);
	CharUpper(wname);

      if (_tcsnicmp(wname,sTmp,iFilterLen) >= 0) {
        LowLimit = i;
        break;
      }
    }

    if (_tcscmp(sTmp, TEXT("")) != 0) { // if it's blanks, then leave UpLimit at end of list
      for (; i<UpLimit; i++){

	LKASSERT(WayPointSelectInfo[i].Index>=0 && WayPointSelectInfo[i].Index<(signed)WayPointList.size());
	LK_tcsncpy(wname,WayPointList[WayPointSelectInfo[i].Index].Name, NAME_SIZE);
	CharUpper(wname);

        if (_tcsnicmp(wname,sTmp,iFilterLen) != 0) {
          UpLimit = i;
          break;
        }
      }
    }
    } else { // iFilterLen>3, fulltext search 100502
	FullFlag=true;
	int matches=0;
	// the WayPointSelectInfo list has been sorted by filters, and then sorted by name. 0-UpLimit is the size.
	// now we create a secondary index pointing to this list
	for (i=0, matches=0; i<UpLimit; i++) {

		LKASSERT(WayPointSelectInfo[i].Index>=0 && WayPointSelectInfo[i].Index<(signed)WayPointList.size());

		_sntprintf(wname,EXT_NAMESIZE, _T("%s %s"),
						WayPointList[WayPointSelectInfo[i].Index].Name,
						WayPointList[WayPointSelectInfo[i].Index].Code );

		CharUpper(wname);

		if ( _tcsstr(  wname,sTmp ) ) {
			StrIndex[matches++]=i;
		}
	}
	// No need to set flag if no results
	if (matches>0) {
		LowLimit=0;
		UpLimit=matches;
	} else {
		LowLimit=0;
		UpLimit=0;
	}

    }

#else
  if (NameFilterIdx != 0){
    TCHAR sTmp[8];
    LowLimit = UpLimit;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointNameCompare);
    sTmp[0] = NameFilter[NameFilterIdx];
    sTmp[1] = '\0';
    sTmp[2] = '\0';
    CharUpper(sTmp);
    for (i=0; i<UpLimit; i++){
      if ((BYTE)(WayPointSelectInfo[i].FourChars >> 24) >= (sTmp[0]&0xff)){
        LowLimit = i;
        break;
      }
    }

    for (; i<UpLimit; i++){
      if ((BYTE)(WayPointSelectInfo[i].FourChars >> 24) != (sTmp[0]&0xff)){
        UpLimit = i;
        break;
      }
    }
  }

#endif

  if (!distancemode) {
    if (!FullFlag) {
	// do NOT change the order of array if we are using FullFlag: we have indexed the old order!!!
	qsort(&WayPointSelectInfo[LowLimit], UpLimit-LowLimit, sizeof(WayPointSelectInfo_t), WaypointNameCompare);
    }
  } else {
	if (!FullFlag) {
		qsort(&WayPointSelectInfo[LowLimit], UpLimit-LowLimit, sizeof(WayPointSelectInfo_t), WaypointDistanceCompare);
	}
  }

  wWayPointList->ResetList();
  wWayPointList->Redraw();

}


static WndProperty *wpDistance;
static WndProperty *wpDirection;

static void FilterMode(bool direction) {
  if (direction) {
    DistanceFilterIdx=0;
    DirectionFilterIdx=0;
    if (wpDistance) {
      wpDistance->GetDataField()->Set(TEXT("*"));
      wpDistance->RefreshDisplay();
    }
    if (wpDirection) {
      wpDirection->GetDataField()->Set(TEXT("*"));
      wpDirection->RefreshDisplay();
    }
  } else {
#if 0 // 100503 OBSOLETED
    NameFilterIdx=0;
    if (wpName) {
      wpName->GetDataField()->Set(TEXT("**"));
      wpName->RefreshDisplay();
    }
#endif
  }
}


static void OnFilterNameButton(WndButton* pWnd) {
	 int SelectedWp=-1;
	 int CursorPos=0;
  TCHAR newNameFilter[NAMEFILTERLEN+1];

  LK_tcsncpy(newNameFilter, sNameFilter, NAMEFILTERLEN);
  SelectedWp =  dlgTextEntryShowModalWaypoint(newNameFilter, NAMEFILTERLEN);



  int i= _tcslen(newNameFilter)-1;
  while (i>=0) {
	if (newNameFilter[i]!=_T(' ')) {
		break;
	}
	newNameFilter[i]=0;
	i--;
  };

  LK_tcsncpy(sNameFilter, newNameFilter, NAMEFILTERLEN);

  if (wpnewName) {
	if (sNameFilter[0]=='\0') {
		SetWPNameCaption(TEXT("*"));
	} else {
		SetWPNameCaption(sNameFilter);
	}
  }
  FilterMode(true);
  UpdateList();
  wWayPointListEntry->SetFocus();
  wWayPointList->SetItemIndexPos(0);
  if((SelectedWp>=0) && (SelectedWp < (int)WayPointList.size()))
  {
    for (i=0; i<UpLimit; i++)
    {
      if(StrIndex != NULL)
      {
        if(( StrIndex[i] >= 0 ) && (StrIndex[i] < (int)WayPointList.size()))
        {
          if(WayPointSelectInfo[StrIndex[i]].Index == SelectedWp)
          {
            CursorPos = i;
            wWayPointList->SetItemIndexPos(CursorPos);
            wWayPointList->CenterScrollCursor();
          }
        }
      }
    }
  }
  wWayPointList->Redraw();

}

static void OnFilterDistance(DataField *Sender, DataField::DataAccessKind_t Mode){

  TCHAR sTmp[12];

  switch(Mode){
    case DataField::daGet:
      Sender->Set(TEXT("25"));
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      DistanceFilterIdx++;
      if (DistanceFilterIdx > (signed)(sizeof(DistanceFilter)/sizeof(DistanceFilter[0])-1))
        DistanceFilterIdx = 0;
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if(DistanceFilterIdx == 0)
        DistanceFilterIdx = sizeof(DistanceFilter)/sizeof(DistanceFilter[0])-1;
      else
        DistanceFilterIdx--;
      FilterMode(false);
      UpdateList();
    break;
  case DataField::daSpecial:
    break;
  }

  if (DistanceFilterIdx == 0)
    _tcsncpy(sTmp,_T("*"),NAMEFILTERLEN);
  else
    _stprintf(sTmp, TEXT("%.0f%s"),
              DistanceFilter[DistanceFilterIdx],
              Units::GetDistanceName());
  Sender->Set(sTmp);
}


static void SetDirectionData(DataField *Sender){

  TCHAR sTmp[30];

  if (Sender == NULL){
    Sender = wpDirection->GetDataField();
  }



	//LKTOKEN _@M1229_ "HDG"
    int a = iround(CALCULATED_INFO.Heading);  if (a <=0)   a += 360;
    switch (DirectionFilter[DirectionFilterIdx] )
    {
    	case DirNoFilter: _stprintf(sTmp, TEXT("%c"), '*');
    	break;
      case DirHDG:
      	{
      	_stprintf(sTmp, TEXT("%s(%d%s)"), MsgToken(1229), a, MsgToken(2179));  // _@1229 HDG  _@M2179 °
      	}
      break;
      case DirAhead:
      	{
      	_stprintf(sTmp, TEXT("%s(%d%s)"), MsgToken(2470), a, MsgToken(2179)); // _@2470 Ahead  _@M2179 °
      	}
      break;
      case DirBRG:
      	{
         a = iround(GetTaskBearing());
      	_stprintf(sTmp, TEXT("%s(%d%s)"), MsgToken(154), a, MsgToken(2179));  // _@M154 Brg  _@M2179 °
      	}
      break;

    	default: _stprintf(sTmp, TEXT("%d%s"), DirectionFilter[DirectionFilterIdx],MsgToken(2179));
    	break;
    }

  Sender->Set(sTmp);

}

static void OnFilterDirection(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
      Sender->Set(TEXT("*"));
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      DirectionFilterIdx++;
      if (DirectionFilterIdx > sizeof(DirectionFilter)/sizeof(DirectionFilter[0])-1)
        DirectionFilterIdx = 0;
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if(DirectionFilterIdx == 0)
        DirectionFilterIdx = sizeof(DirectionFilter)/sizeof(DirectionFilter[0])-1;
      else
        DirectionFilterIdx--;
      FilterMode(false);
      UpdateList();
    break;
  case DataField::daSpecial:
    break;
  }

  SetDirectionData(Sender);

}



static void OnFilterType(DataField *Sender, DataField::DataAccessKind_t Mode){

  TCHAR sTmp[NAMEFILTERLEN];

  switch(Mode){
    case DataField::daGet:
      Sender->Set(TEXT("*"));
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      TypeFilterIdx++;
      if (TypeFilterIdx >= TYPEFILTERSNUM)
        TypeFilterIdx = 0;
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if (TypeFilterIdx) TypeFilterIdx--; else TypeFilterIdx = TYPEFILTERSNUM-1;
      FilterMode(false);
      UpdateList();
    break;
  case DataField::daSpecial:
    break;
  }

  _tcsncpy(sTmp, TypeFilter[TypeFilterIdx],NAMEFILTERLEN);

  Sender->Set(sTmp);

}

static unsigned int DrawListIndex=0;

// Painting elements after init

extern int FindFirstIn(const TCHAR Txt[] ,const TCHAR Sub[]);

static void OnPaintListItem(WindowControl * Sender, LKSurface& Surface) {
    if (!Sender) {
        return;
    }

    unsigned int n = UpLimit - LowLimit;
    TCHAR sTmp[50];
    TCHAR TmpName[EXT_NAMESIZE];
    Surface.SetTextColor(RGB_BLACK);

    const int LineHeight = Sender->GetHeight();
    const int TextHeight = Surface.GetTextHeight(_T("dp"));

    const int TextPos = (LineHeight - TextHeight) / 2; // offset for text vertical center

    if (DrawListIndex < n) {

        const size_t i = (FullFlag) ? StrIndex[DrawListIndex] : (LowLimit + DrawListIndex);

        // Poco::Thread::sleep(100);

        LKASSERT(i < WayPointList.size());

        const int width = Sender->GetWidth(); // total width

        const int w0 = LineHeight; // Picto Width
        const int w2 = Surface.GetTextWidth(TEXT(" 000km")); // distance Width
        _stprintf(sTmp, _T(" 000%s "), MsgToken(2179));
        const int w3 = Surface.GetTextWidth(sTmp); // bearing width

        const int w1 = width - w0 - w2 - w3; // Max Name width


        int idx = WayPointSelectInfo[i].Index;

        // Draw Name
        if( _tcslen(WayPointList[WayPointSelectInfo[i].Index].Code ) >1)
          _sntprintf(TmpName, EXT_NAMESIZE, _T("%s (%s)"), WayPointList[WayPointSelectInfo[i].Index].Name,  WayPointList[WayPointSelectInfo[i].Index].Code);
        else
          _sntprintf(TmpName, EXT_NAMESIZE, _T("%s"), WayPointList[WayPointSelectInfo[i].Index].Name);
        Surface.DrawTextClip(w0, TextPos, TmpName, w1);

        _tcsncpy(sTmp, TmpName ,EXT_NAMESIZE);

        int Start = FindFirstIn(sTmp ,sNameFilter);
        if(Start >= 0) {
          const int iFilterLen = _tcslen(sNameFilter);
          sTmp[Start + iFilterLen]=0;
          const int subend = std::min(w1, w0 + Surface.GetTextWidth(sTmp));
          sTmp[Start]=0;
          const int substart = std::max(w0, w0 + Surface.GetTextWidth(sTmp));

          if(substart < subend) {
            int h =  LineHeight - IBLSCALE(4);
            const auto hOldPen = Surface.SelectObject(LKPen_Black_N1);
            Surface.DrawLine(substart, h, subend, h);
            Surface.SelectObject(hOldPen);
          }
        }


        // Draw Distance : right justified after waypoint Name
        _stprintf(sTmp, TEXT("%.0f%s"), WayPointSelectInfo[i].Distance, Units::GetDistanceName());
        const int x2 = width - w3 - Surface.GetTextWidth(sTmp);
        Surface.DrawText(x2, TextPos, sTmp);

        // Draw Bearing right justified after distance
        _stprintf(sTmp, TEXT("%d%s"), iround(WayPointSelectInfo[i].Direction), MsgToken(2179));
        const int x3 = width - Surface.GetTextWidth(sTmp);
        Surface.DrawText(x3, TextPos, sTmp);

        // Draw Picto
        const RECT PictoRect = {0, 0, w0, LineHeight};
        if (WayPointCalc[idx].IsLandable) {
            MapWindow::DrawRunway(Surface, &WayPointList[idx], PictoRect, nullptr, 1 , true);
        } else {
           MapWindow::DrawWaypointPicto(Surface, PictoRect, &WayPointList[idx]);
        }        

    } else {
        if (DrawListIndex == 0) {
            // LKTOKEN  _@M466_ = "No Match!"
            Surface.DrawText(IBLSCALE(2), TextPos, MsgToken(466));
        }
    }
}

static void OnWpListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
	ListInfo->ItemCount = UpLimit-LowLimit;
  } else {
	DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
	ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}

static void OnWPSSelectClicked(WndButton* pWnd) {
  WndForm* pForm = pWnd->GetParentWndForm();
  if(pForm) {
    OnWaypointListEnter(pForm->FindByName(TEXT("frmWayPointList")), NULL);
  }
}

static void OnWPSCloseClicked(WndButton* pWnd) {
  ItemIndex = -1;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
}

static bool OnTimerNotify(WndForm* pWnd) {
	int a=-1;
	switch(DirectionFilter[DirectionFilterIdx] )
	{
		case DirHDG:
			a = iround(CALCULATED_INFO.Heading);
			break;

		case DirBRG:
			a = GetTaskBearing();
			break;
	}

	if(a >= 0) {
		if (abs(a-lastHeading) > 10) {
			lastHeading = a;
			UpdateList();
			SetDirectionData(NULL);
			wpDirection->RefreshDisplay();
			wWayPointList->Redraw();
		}
	}
	return true;
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {

    WndProperty* wp = ((WndProperty *) pWnd->FindByName(TEXT("prpFltType")));
    if(wp) {
       unsigned NewIndex = TypeFilterIdx;
       switch (KeyCode & 0xffff) {
            case KEY_F1:
                NewIndex = 0;
                break;
            case KEY_F2:
                NewIndex = 2;
                break;
            case KEY_F3:
                NewIndex = 3;
                break;
       }

        if (TypeFilterIdx != NewIndex) {
           TypeFilterIdx = NewIndex;
           FilterMode(false);
           UpdateList();
           wp->GetDataField()->SetAsString(TypeFilter[TypeFilterIdx]);
           wp->RefreshDisplay();
        }
    }
    return false;
}

static CallBackTableEntry_t CallBackTable[]={
  DataAccessCallbackEntry(OnFilterDistance),
  DataAccessCallbackEntry(OnFilterDirection),
  DataAccessCallbackEntry(OnFilterType),
  OnPaintCallbackEntry(OnPaintListItem),
  OnListCallbackEntry(OnWpListInfo),


  EndCallBackEntry()
};


int dlgWayPointSelect(double lon, double lat, int type, int FilterNear){

  SHOWTHREAD(_T("dlgWayPointSelect"));

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = -1;
	//Manual fillup, do not change indexes!
	//If you add more items don't forget to change TYPEFILTERSNUM and UpdateList() also
//	TypeFilter[0] = gettext(TEXT("*"));
	 _stprintf(TypeFilter[0], TEXT("*"));
	// LKTOKEN _@M1224_ "Airport"
//	TypeFilter[1] = MsgToken(1224);
	 _stprintf(TypeFilter[1], TEXT("%s"), MsgToken(1224));
	// LKTOKEN _@M1225_ "Landable"
//	TypeFilter[2] = MsgToken(1225);
	 _stprintf(TypeFilter[2], TEXT("%s"), MsgToken(1225));
	// LKTOKEN _@M1226_ "Turnpoint"
//	TypeFilter[3] = MsgToken(1226);
	 _stprintf(TypeFilter[3], TEXT("%s"), MsgToken(1226));
	// LKTOKEN _@M2342_ "File"
	for (int i = 0 ; i < NO_WP_FILES; i++)
	  _stprintf(TypeFilter[4+i], TEXT("%s %i"), MsgToken(2342), i+1 );


//	// LKTOKEN _@M1228_ "File 2"
//	TypeFilter[5] = MsgToken(1228);



  if (lon==0.0 && lat==90.0) {
    Latitude = GPS_INFO.Latitude;
    Longitude = GPS_INFO.Longitude;
  } else {
    Latitude = lat;
    Longitude = lon;
  }
  if (type > -1){
    TypeFilterIdx = type;
  }
  if (FilterNear){
    DistanceFilterIdx = FilterNear;
  }

  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_WAYPOINTSELECT_L : IDR_XML_WAYPOINTSELECT_P);

  if (!wf) return -1;

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnWPSCloseClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdName")))->SetOnClickNotify(OnFilterNameButton);
  ((WndButton *)wf->FindByName(TEXT("cmdSelect")))->SetOnClickNotify(OnWPSSelectClicked);

  wWayPointList = (WndListFrame*)wf->FindByName(TEXT("frmWayPointList"));
  LKASSERT(wWayPointList!=NULL);
  wWayPointList->SetBorderKind(BORDERLEFT);
  wWayPointList->SetEnterCallback(OnWaypointListEnter);

  wWayPointListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmWayPointListEntry"));
  LKASSERT(wWayPointListEntry!=NULL);

  wWayPointListEntry->SetCanFocus(true);

  wpDistance = (WndProperty*)wf->FindByName(TEXT("prpFltDistance"));
  wpDirection = (WndProperty*)wf->FindByName(TEXT("prpFltDirection"));
  wpnewName = (WndButton*)wf->FindByName(TEXT("cmdName"));

  PrepareData();
  if (WayPointSelectInfo==NULL) goto _return; // Will be null also if strindex was null
  UpdateList();
  wf->SetTimerNotify(500, OnTimerNotify);

  if ((wf->ShowModal() == mrOK) && (UpLimit - LowLimit > 0) && (ItemIndex >= 0)
   && (ItemIndex < (UpLimit - LowLimit))) {

	if (FullFlag)
		ItemIndex = WayPointSelectInfo[StrIndex[LowLimit + ItemIndex]].Index;
	else
		ItemIndex = WayPointSelectInfo[LowLimit + ItemIndex].Index;
  }else
	ItemIndex = -1;

_return:
  wf->SetTimerNotify(0, NULL);
  if (WayPointSelectInfo!=NULL) free(WayPointSelectInfo);
  if (StrIndex!=NULL) free(StrIndex);

  delete wf;

  wf = NULL;

  return(ItemIndex);

}
