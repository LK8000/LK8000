/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayPointSelect.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>
#include "Dialogs.h"
#include "InfoBoxLayout.h"
#include "WindowControls.h"
#include "DoInits.h"
#include "TraceThread.h"
#include <ctype.h>

#ifdef WAYPOINT_ICONS
  #define PICTO_OFFSET 28
#else
  #define PICTO_OFFSET 0
#endif



typedef struct{
  int Index;
  double Distance;
  double Direction;
  int    DirectionErr;
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

static const TCHAR NameFilter[] = TEXT("*ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
static unsigned NameFilterIdx=0;

static double DistanceFilter[] = {0.0, 25.0, 50.0, 75.0, 100.0, 150.0, 250.0, 500.0, 1000.0};
static unsigned DistanceFilterIdx=0;

#define DirHDG -1
static int DirectionFilter[] = {0, DirHDG, 360, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330};
static unsigned DirectionFilterIdx=0;
static int lastHeading=0;

#define TYPEFILTERSNUM	6
static const TCHAR *TypeFilter[TYPEFILTERSNUM];
static unsigned TypeFilterIdx=0;

static int UpLimit=0;
static int LowLimit=0;

static bool FullFlag=false; // 100502

static int ItemIndex = -1;

static int SelectedWayPointFileIdx = 0;

extern char ToUpper(char in);

static void OnWaypointListEnter(WindowControl * Sender, 
				WndListFrame::ListInfo_t *ListInfo){
	(void)Sender; (void)ListInfo;
  if (ItemIndex != -1) {
    wf->SetModalResult(mrOK);
  }
  else
    wf->SetModalResult(mrCancle);
}


static WayPointSelectInfo_t *WayPointSelectInfo=NULL;
static int *StrIndex=NULL;

static int _cdecl WaypointNameCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->FourChars < ((WayPointSelectInfo_t *)elem2)->FourChars)
    return (-1);
  if (((WayPointSelectInfo_t *)elem1)->FourChars > ((WayPointSelectInfo_t *)elem2)->FourChars)
    return (+1);
  return (0);
}

static int _cdecl WaypointDistanceCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Distance < ((WayPointSelectInfo_t *)elem2)->Distance)
    return (-1);
  if (((WayPointSelectInfo_t *)elem1)->Distance > ((WayPointSelectInfo_t *)elem2)->Distance)
    return (+1);
  return (0);
}

static int _cdecl WaypointAirportCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Type & (AIRPORT))
    return (-1);
  return (+1);
}

static int _cdecl WaypointLandableCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Type & (AIRPORT | LANDPOINT))
    return (-1);
  return (+1);
}

static int _cdecl WaypointWayPointCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Type & (TURNPOINT))
    return (-1);
  return (+1);
}

static int _cdecl WaypointFileIdxCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->FileIdx != SelectedWayPointFileIdx)
    return (+1);
  return (-1);
}

static int _cdecl WaypointDirectionCompare(const void *elem1, const void *elem2 ){

  int a, a1, a2;

  a = DirectionFilter[DirectionFilterIdx];
  if (a == DirHDG){
    a = iround(CALCULATED_INFO.Heading);
    lastHeading = a;
  }

  a1 = (int)(((WayPointSelectInfo_t *)elem1)->Direction - a);
  a2 = (int)(((WayPointSelectInfo_t *)elem2)->Direction - a);

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

  ((WayPointSelectInfo_t *)elem1)->DirectionErr = a1;
  ((WayPointSelectInfo_t *)elem2)->DirectionErr = a2;

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

  if (!WayPointList) return;

  sNameFilter[0]='\0';
  SetWPNameCaption(TEXT("*"));
  WayPointSelectInfo = (WayPointSelectInfo_t*)malloc(sizeof(WayPointSelectInfo_t) * NumberOfWayPoints);
  if (WayPointSelectInfo==NULL) {
	OutOfMemory(__FILE__,__LINE__);
	return;
  }
  memset(WayPointSelectInfo, 0, sizeof(WayPointSelectInfo_t) * NumberOfWayPoints);

  StrIndex = (int*)malloc(sizeof(int)*(NumberOfWayPoints+1));
  if (StrIndex==NULL) {
	OutOfMemory(__FILE__,__LINE__);
	free(WayPointSelectInfo);
	WayPointSelectInfo=NULL;
	return;
  }

  for (int i=0; i<(int)NumberOfWayPoints; i++){

    LKASSERT(numvalidwp<=NumberOfWayPoints);

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
    _tcsupr(sTmp);

    WayPointSelectInfo[numvalidwp].FourChars =
                    (((DWORD)sTmp[0] & 0xff) << 24)
                  + (((DWORD)sTmp[1] & 0xff) << 16)
                  + (((DWORD)sTmp[2] & 0xff) << 8)
                  + (((DWORD)sTmp[3] & 0xff) );

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
  } else if (TypeFilterIdx == 4 || TypeFilterIdx == 5){
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
    TCHAR TmpCmpr[NAME_SIZE+1];
    unsigned int k;
    LowLimit = UpLimit;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointNameCompare);

	for(k =0; k < NAMEFILTERLEN; k++)
		sTmp[k] = ToUpper(sNameFilter[k]);
    int iFilterLen = _tcslen(sNameFilter);

    if (iFilterLen<GC_SUB_STRING_THRESHOLD)
    {
    for (i=0; i<UpLimit; i++){
      // compare entire name which may be more than 4 chars

    	for(k =0; k < NAME_SIZE; k++)
    		TmpCmpr[k] = ToUpper(WayPointList[WayPointSelectInfo[i].Index].Name[k]);


      if (_tcsnicmp(TmpCmpr,sTmp,iFilterLen) >= 0) {
        LowLimit = i;
        break;
      }
    }

    if (_tcscmp(sTmp, TEXT("")) != 0) { // if it's blanks, then leave UpLimit at end of list
      for (; i<UpLimit; i++){

      	for(k =0; k < NAME_SIZE; k++)
      		TmpCmpr[k] = ToUpper(WayPointList[WayPointSelectInfo[i].Index].Name[k]);
        if (_tcsnicmp(TmpCmpr,sTmp,iFilterLen) != 0) {
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

    	for( k =0; k < NAME_SIZE; k++)
    		TmpCmpr[k] = ToUpper(WayPointList[WayPointSelectInfo[i].Index].Name[k]);
 
		if ( _tcsstr(  TmpCmpr,sTmp ) ) {
			StrIndex[matches++]=i;
		}
	}
	// No need to set flag if no results
	if (matches>0) {
		LowLimit=0;
		UpLimit=matches;
/*
		for (i=0; i<UpLimit; i++) 
			StartupStore(_T("StrIndex[%d] = %d <%s>\n"), i, StrIndex[i], WayPointList[WayPointSelectInfo[StrIndex[i]].Index].Name);
*/
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
    _tcsupr(sTmp);
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
    NameFilterIdx=0;
#if 0 // 100503 OBSOLETED
    if (wpName) {
      wpName->GetDataField()->Set(TEXT("**"));
      wpName->RefreshDisplay();
    }
#endif
  }
}


static void OnFilterNameButton(WindowControl *Sender) {
	 int SelectedWp=-1;
	 int CursorPos=0;
  TCHAR newNameFilter[NAMEFILTERLEN+1];

  LK_tcsncpy(newNameFilter, sNameFilter, NAMEFILTERLEN);
  SelectedWp =  dlgTextEntryShowModal(newNameFilter, NAMEFILTERLEN, true);



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
#define SELECT_FOUND
#ifdef SELECT_FOUND
  if((SelectedWp>=0) && (SelectedWp < (int)NumberOfWayPoints))
  {
	for (i=0; i<UpLimit; i++)
	{

  	    if(WayPointSelectInfo[StrIndex[i]].Index == SelectedWp)
  	    {
  	  	  CursorPos = i;
  	    }
	}

    wWayPointListEntry->SetFocused(true,NULL);
    wWayPointList->SetItemIndexPos(CursorPos);
    wWayPointList->Redraw();
  }
#endif
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
    _stprintf(sTmp, TEXT("%c"), '*');
  else
    _stprintf(sTmp, TEXT("%.0f%s"), 
              DistanceFilter[DistanceFilterIdx],
              Units::GetDistanceName());
  Sender->Set(sTmp);
}


static void SetDirectionData(DataField *Sender){

  TCHAR sTmp[20];

  if (Sender == NULL){
    Sender = wpDirection->GetDataField();
  }

  if (DirectionFilterIdx == 0)
    _stprintf(sTmp, TEXT("%c"), '*');
  else if (DirectionFilterIdx == 1){
    int a = iround(CALCULATED_INFO.Heading);
    if (a <=0)
      a += 360;
	//LKTOKEN _@M1229_ "HDG"
    _stprintf(sTmp, TEXT("%s(%d%s)"), gettext(TEXT("_@M1229_")), a, TEXT(DEG));
  }else
    _stprintf(sTmp, TEXT("%d")TEXT(DEG), DirectionFilter[DirectionFilterIdx]);

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

  TCHAR sTmp[12];

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

  _stprintf(sTmp, TEXT("%s"), TypeFilter[TypeFilterIdx]);

  Sender->Set(sTmp);

}

static int DrawListIndex=0;

// Painting elements after init
static void OnPaintListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
  int n = UpLimit - LowLimit;
  TCHAR sTmp[12];
#if 100124
  static int w0;

  if (DoInit[MDI_ONPAINTLISTITEM]) {
        w0=wWayPointList->GetWidth() - wWayPointList->ScrollbarWidth - 4;
	DoInit[MDI_ONPAINTLISTITEM]=false;
  }
#endif


  if (DrawListIndex < n){

    int i;
    if (FullFlag) {
	i = StrIndex[DrawListIndex];	// 100502
/*
	StartupStore(_T("UpLimit=%d LowLimit=%d n=%d DrawListIndex=%d StrIndex[%d]=i=%d Index=%d <%s>\n"),
	UpLimit,LowLimit,n, DrawListIndex, DrawListIndex, i, WayPointSelectInfo[i].Index,
	WayPointList[WayPointSelectInfo[i].Index].Name);
*/
    }else
	i = LowLimit + DrawListIndex;

// Sleep(100);


    int w1, w2, w3, x1, x2, x3;
    WndListFrame *wlf = (WndListFrame *)wf->FindByName(TEXT("frmWayPointList"));
    if (wlf) {
   	 w0=wlf->GetWidth() - wlf->ScrollbarWidth - 4;
    }

    w1 = GetTextWidth(hDC, TEXT("XXX"));
    w2 = GetTextWidth(hDC, TEXT(" 000km"));
    w3 = GetTextWidth(hDC, TEXT(" 000")TEXT(DEG));

    x1 = w0-w1-w2-w3;

    ExtTextOutClip(hDC, (int)(PICTO_OFFSET+2)*ScreenScale, 2*ScreenScale,
                   WayPointList[WayPointSelectInfo[i].Index].Name,
                   x1-ScreenScale*10);

    sTmp[0] = '\0';
    sTmp[1] = '\0';
    sTmp[2] = '\0';

#ifdef WAYPOINT_ICONS
    RECT rc = {0,  0, (int)(PICTO_OFFSET*1)*ScreenScale,   20*ScreenScale};
    int idx = WayPointSelectInfo[i].Index;
     if (WayPointCalc[idx].IsLandable )
  	  MapWindow::DrawRunway(hDC,&WayPointList[idx],  rc, 1500*ScreenScale, true);
     else
     {   rc.right = rc.right/2;
   //  rc.top += (rc.bottom)/2;
     rc.bottom = 5;
       MapWindow::DrawWaypointPicto(hDC,  rc, &WayPointList[idx]);
     }
#else
    if (WayPointList[WayPointSelectInfo[i].Index].Flags & HOME){
		// LKTOKEN _@M1236_ "H"
      sTmp[0] = gettext(TEXT("_@M1236_"))[0];
    }else
    if (WayPointList[WayPointSelectInfo[i].Index].Flags & AIRPORT){
		// LKTOKEN _@M1237_ "A"
      sTmp[0] = gettext(TEXT("_@M1237_"))[0];
    }else
    if (WayPointList[WayPointSelectInfo[i].Index].Flags & LANDPOINT){
		// LKTOKEN _@M1238_ "L"
      sTmp[0] = gettext(TEXT("_@M1238_"))[0];
    }

    if (WayPointList[WayPointSelectInfo[i].Index].Flags & TURNPOINT){
      if (sTmp[0] == '\0')
		// LKTOKEN _@M1239_ "T"
        sTmp[0] = gettext(TEXT("_@M1239_"))[0];
      else
		// LKTOKEN _@M1239_ "T"
        sTmp[1] = gettext(TEXT("_@M1239_"))[0];
    }
#endif
    // left justified
    ExtTextOut(hDC, x1, 2*ScreenScale,
               ETO_OPAQUE, NULL,
               sTmp, _tcslen(sTmp), NULL);

    // right justified after waypoint flags
    _stprintf(sTmp, TEXT("%.0f%s"), 
              WayPointSelectInfo[i].Distance,
              Units::GetDistanceName());
    x2 = w0-w3-GetTextWidth(hDC, sTmp);
    ExtTextOut(hDC, x2, 2*ScreenScale,
      ETO_OPAQUE, NULL,
      sTmp, _tcslen(sTmp), NULL);

    // right justified after distance
    _stprintf(sTmp, TEXT("%d")TEXT(DEG),  
	      iround(WayPointSelectInfo[i].Direction));
    x3 = w0-GetTextWidth(hDC, sTmp);
    ExtTextOut(hDC, x3, 2*ScreenScale,
               ETO_OPAQUE, NULL,
               sTmp, _tcslen(sTmp), NULL);
  } else {
    if (DrawListIndex == 0){
	// LKTOKEN  _@M466_ = "No Match!" 
      _stprintf(sTmp, TEXT("%s"), gettext(TEXT("_@M466_")));
      ExtTextOut(hDC, 2*ScreenScale, 2*ScreenScale,
        ETO_OPAQUE, NULL,
        sTmp, _tcslen(sTmp), NULL);
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

static void OnWPSSelectClicked(WindowControl * Sender){
       (void)Sender;
  OnWaypointListEnter(NULL, NULL);
}

static void OnWPSCloseClicked(WindowControl * Sender){
	(void)Sender;
  ItemIndex = -1;
  wf->SetModalResult(mrCancle);
}

static int OnTimerNotify(WindowControl * Sender) {
  (void)Sender;

  static short i=0;
  if(i++ % 2 == 0) return 0;

  if (DirectionFilterIdx == 1){
    int a;
    a = (lastHeading - iround(CALCULATED_INFO.Heading));
    if (abs(a) > 0){
      UpdateList();
      SetDirectionData(NULL);
      wpDirection->RefreshDisplay();
    }
  }
  wWayPointList->Redraw();
  return 0;
}

static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){

  WndProperty* wp;
  unsigned NewIndex = TypeFilterIdx;

  (void)lParam;
  (void)Sender;
  wp = ((WndProperty *)wf->FindByName(TEXT("prpFltType")));

  switch(wParam & 0xffff){
    case VK_F1:
      NewIndex = 0;
    break;
    case VK_F2:
      NewIndex = 2;
    break;
    case VK_F3:
      NewIndex = 3;
    break;
  }

  if (TypeFilterIdx != NewIndex){
    TypeFilterIdx = NewIndex;
    FilterMode(false);
    UpdateList();
    wp->GetDataField()->SetAsString(TypeFilter[TypeFilterIdx]);
    wp->RefreshDisplay();
  }

  return(1);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnFilterDistance),
  DeclareCallBackEntry(OnFilterDirection),
  DeclareCallBackEntry(OnFilterType),
  DeclareCallBackEntry(OnPaintListItem),
  DeclareCallBackEntry(OnWpListInfo),


  DeclareCallBackEntry(NULL)
};


int dlgWayPointSelect(double lon, double lat, int type, int FilterNear){

  SHOWTHREAD(_T("dlgWayPointSelect"));

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = -1;
	//Manual fillup, do not change indexes!
	//If you add more items don't forget to change TYPEFILTERSNUM and UpdateList() also
	TypeFilter[0] = gettext(TEXT("*"));
	// LKTOKEN _@M1224_ "Airport"
	TypeFilter[1] = gettext(TEXT("_@M1224_"));
	// LKTOKEN _@M1225_ "Landable"
	TypeFilter[2] = gettext(TEXT("_@M1225_"));
	// LKTOKEN _@M1226_ "Turnpoint"
	TypeFilter[3] = gettext(TEXT("_@M1226_"));
	// LKTOKEN _@M1227_ "File 1"
	TypeFilter[4] = gettext(TEXT("_@M1227_"));
	// LKTOKEN _@M1228_ "File 2"
	TypeFilter[5] = gettext(TEXT("_@M1228_"));


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
    DistanceFilterIdx = 1;
  }

  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWayPointSelect_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_WAYPOINTSELECT_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWayPointSelect.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_WAYPOINTSELECT"));
  }

  if (!wf) return -1;

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnWPSCloseClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdName")))->SetOnClickNotify(OnFilterNameButton);
  ((WndButton *)wf->FindByName(TEXT("cmdSelect")))->SetOnClickNotify(OnWPSSelectClicked);

  wWayPointList = (WndListFrame*)wf->FindByName(TEXT("frmWayPointList"));
  LKASSERT(wWayPointList!=NULL);
  wWayPointList->SetBorderKind(BORDERLEFT);
  wWayPointList->SetEnterCallback(OnWaypointListEnter);
  wWayPointList->SetWidth(wf->GetWidth() - wWayPointList->GetLeft()-2);

  wWayPointListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmWayPointListEntry"));
  LKASSERT(wWayPointListEntry!=NULL);

  wWayPointListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmWayPointListEntry"));
  LKASSERT(wWayPointListEntry!=NULL);

  wWayPointListEntry->SetCanFocus(true);
   // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
   if ( wWayPointList->ScrollbarWidth == -1) {  
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wWayPointList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);

   }
  wWayPointListEntry->SetWidth(wWayPointList->GetWidth() - wWayPointList->ScrollbarWidth - 5);

  wpDistance = (WndProperty*)wf->FindByName(TEXT("prpFltDistance"));
  wpDirection = (WndProperty*)wf->FindByName(TEXT("prpFltDirection"));
  wpnewName = (WndButton*)wf->FindByName(TEXT("cmdName"));

  PrepareData();
  if (WayPointSelectInfo==NULL) goto _return; // Will be null also if strindex was null
  UpdateList();
  wf->SetTimerNotify(OnTimerNotify);

  if ((wf->ShowModal() == mrOK) && (UpLimit - LowLimit > 0) && (ItemIndex >= 0) 
   && (ItemIndex < (UpLimit - LowLimit))) {

	if (FullFlag)
		ItemIndex = WayPointSelectInfo[StrIndex[LowLimit + ItemIndex]].Index;
	else
		ItemIndex = WayPointSelectInfo[LowLimit + ItemIndex].Index;
  }else
	ItemIndex = -1;

_return:
  if (WayPointSelectInfo!=NULL) free(WayPointSelectInfo);
  if (StrIndex!=NULL) free(StrIndex);

  delete wf;

  wf = NULL;

  return(ItemIndex);

}
