/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceSelect.cpp,v 8.3 2010/12/13 12:26:00 root Exp root $
*/


#include "StdAfx.h"
#include <aygshell.h>

#include "XCSoar.h"

#include "Statistics.h"
#include "externs.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "Airspace.h"
#include "AirspaceWarning.h"

#include "utils/heapcheck.h"


typedef struct{
#ifdef LKAIRSPACE
  CAirspace *airspace;
#else
  int Index_Circle;
  int Index_Area;
#endif
  double Distance;
  double Direction;
  int    DirectionErr;
  unsigned int Type;
  unsigned int FourChars;
} AirspaceSelectInfo_t;

static double Latitude;
static double Longitude;

static WndForm *wf=NULL;
static WndListFrame *wAirspaceList=NULL;
static WndOwnerDrawFrame *wAirspaceListEntry = NULL;

static TCHAR NameFilter[] = TEXT("*ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
static unsigned NameFilterIdx=0;

static double DistanceFilter[] = {0.0, 25.0, 50.0, 75.0, 100.0, 150.0, 
                                  250.0, 500.0, 1000.0};
static unsigned DistanceFilterIdx=0;

#define DirHDG -1

static int DirectionFilter[] = {0, DirHDG, 360, 30, 60, 90, 120, 150, 
                                180, 210, 240, 270, 300, 330};
static unsigned DirectionFilterIdx=0;
static int lastHeading=0;

static int NumberOfAirspaces = 0;

#ifndef LKAIRSPACE
// FIXV2
static const TCHAR *TypeFilter[] = {TEXT("*"), 
				    TEXT("Other"),
				    TEXT("Restricted areas"),
				    TEXT("Prohibited areas"),
				    TEXT("Danger areas"),
				    TEXT("Class A"), 
				    TEXT("Class B"), 
				    TEXT("Class C"), 
				    TEXT("Class D"), 
				    TEXT("No gliders"),
				    TEXT("CTR"),
				    TEXT("Wave"),
				    TEXT("AAT"),
				    TEXT("Class E"),
				    TEXT("Class F"),
				    TEXT("Class G"),
};
#endif
static unsigned TypeFilterIdx=0;

static int UpLimit=0;
static int LowLimit=0;

static int ItemIndex = -1;

static AirspaceSelectInfo_t *AirspaceSelectInfo=NULL;

#ifdef LKAIRSPACE
static void OnAirspaceListEnter(WindowControl * Sender, 
				WndListFrame::ListInfo_t *ListInfo){
  (void)Sender; (void)ListInfo;

  if (ItemIndex != -1) {

    if ((UpLimit-LowLimit>0)
        && (ItemIndex >= 0)  // JMW fixed bug, was >0
        && (ItemIndex < (UpLimit - LowLimit))) {

      CAirspace *airspace = AirspaceSelectInfo[LowLimit+ItemIndex].airspace;
      if (airspace) {
        
        const TCHAR *Name = airspace->Name();
        if (Name) {
		  UINT answer;
          answer = MessageBoxX(hWndMapWindow,
			       Name,
					// LKTOKEN  _@M51_ = "Acknowledge for day?" 
			       gettext(TEXT("_@M51_")),
			       MB_YESNOCANCEL|MB_ICONQUESTION);
		  if (answer == IDYES) {
			if (airspace) CAirspaceManager::Instance().AirspaceAckDaily(*airspace);
          } else if (answer == IDNO) {
			// this will cancel a daily ack
			if (airspace) CAirspaceManager::Instance().AirspaceAckDailyCancel(*airspace);
		  }
        }
      }
    }
  } else {
    wf->SetModalResult(mrCancle);
  }
}
#else
static void OnAirspaceListEnter(WindowControl * Sender, 
				WndListFrame::ListInfo_t *ListInfo){
  (void)Sender; (void)ListInfo;

  if (ItemIndex != -1) {

    if ((UpLimit-LowLimit>0)
        && (ItemIndex >= 0)  // JMW fixed bug, was >0
        && (ItemIndex < (UpLimit - LowLimit))) {

      int index_circle = AirspaceSelectInfo[LowLimit+ItemIndex].Index_Circle;
      int index_area = AirspaceSelectInfo[LowLimit+ItemIndex].Index_Area;
      
      if ((index_circle>=0) || (index_area>=0)) {
        
        TCHAR *Name = NULL;
        if (index_circle>=0) {
          Name = AirspaceCircle[index_circle].Name;
        } else if (index_area>=0) {
          Name = AirspaceArea[index_area].Name;
        }
        if (Name) {
	  UINT answer;
          answer = MessageBoxX(hWndMapWindow,
			       Name,
	// LKTOKEN  _@M51_ = "Acknowledge for day?" 
			       gettext(TEXT("_@M51_")),
			       MB_YESNOCANCEL|MB_ICONQUESTION);
	  if (answer == IDYES) {
	    if (index_circle>=0) {
              AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, false, true, 
                                  index_circle, true);
            } else if (index_area>=0) {
              AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, false, false, 
                                  index_area, true);
            }
          } else if (answer == IDNO) {
	    // this will cancel a daily ack
	    if (index_circle>=0) {
              AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, true, true, 
                                  index_circle, true);
            } else if (index_area>=0) {
              AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, true, false, 
                                  index_area, true);
            }
	  }
        }
      }
    }
  } else {
    wf->SetModalResult(mrCancle);
  }
}
#endif


static int _cdecl AirspaceNameCompare(const void *elem1, const void *elem2 ){
  if (((AirspaceSelectInfo_t *)elem1)->FourChars < ((AirspaceSelectInfo_t *)elem2)->FourChars)
    return (-1);
  if (((AirspaceSelectInfo_t *)elem1)->FourChars > ((AirspaceSelectInfo_t *)elem2)->FourChars)
    return (+1);
  return (0);
}

static int _cdecl AirspaceDistanceCompare(const void *elem1, const void *elem2 ){
  if (((AirspaceSelectInfo_t *)elem1)->Distance < ((AirspaceSelectInfo_t *)elem2)->Distance)
    return (-1);
  if (((AirspaceSelectInfo_t *)elem1)->Distance > ((AirspaceSelectInfo_t *)elem2)->Distance)
    return (+1);
  return (0);
}

static int _cdecl AirspaceTypeCompare(const void *elem1, const void *elem2 ){
  if (((AirspaceSelectInfo_t *)elem1)->Type == TypeFilterIdx-1)
    return (-1);
  return (+1);
}


static int _cdecl AirspaceDirectionCompare(const void *elem1, const void *elem2 ){

  int a, a1, a2;

  a = DirectionFilter[DirectionFilterIdx];
  if (a == DirHDG){
    a = iround(CALCULATED_INFO.Heading);
    lastHeading = a;
  }

  a1 = (int)(((AirspaceSelectInfo_t *)elem1)->Direction - a);
  a2 = (int)(((AirspaceSelectInfo_t *)elem2)->Direction - a);

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

  ((AirspaceSelectInfo_t *)elem1)->DirectionErr = a1;
  ((AirspaceSelectInfo_t *)elem2)->DirectionErr = a2;

  if (a1 < a2)
    return (-1);
  if (a1 > a2)
    return (+1);

  return (0);
}

#ifdef LKAIRSPACE
static void PrepareData(void){

  TCHAR sTmp[5];

  if (NumberOfAirspaces==0) return;

  AirspaceSelectInfo = (AirspaceSelectInfo_t*)
    malloc(sizeof(AirspaceSelectInfo_t) * NumberOfAirspaces);

  if (AirspaceSelectInfo==NULL) { // 100101
	StartupStore(_T("------ Airspace malloc SelectInfo Failed!!%s"), NEWLINE);
	return;
  }

  int index=0;
  double bearing;
  double distance;
  CAirspaceList Airspaces = CAirspaceManager::Instance().GetAllAirspaces();
  CAirspaceList::const_iterator it;
  for (it=Airspaces.begin(); it != Airspaces.end(); ++it) {
    AirspaceSelectInfo[index].airspace = *it;

	distance = DISTANCEMODIFY * (*it)->Range(Longitude, Latitude, bearing);
	if (distance<0) distance=0;
    AirspaceSelectInfo[index].Distance = distance;
	AirspaceSelectInfo[index].Direction = bearing;

    _tcsncpy(sTmp, (*it)->Name(), 4);
    sTmp[4] = '\0';
    _tcsupr(sTmp);

    AirspaceSelectInfo[index].FourChars =
                    (((DWORD)sTmp[0] & 0xff) << 24)
                  + (((DWORD)sTmp[1] & 0xff) << 16)
                  + (((DWORD)sTmp[2] & 0xff) << 8)
                  + (((DWORD)sTmp[3] & 0xff) );

    AirspaceSelectInfo[index].Type = (*it)->Type();

    index++;
  }

  qsort(AirspaceSelectInfo, UpLimit,
      sizeof(AirspaceSelectInfo_t), AirspaceNameCompare);

}
#else
static void PrepareData(void){

  TCHAR sTmp[5];

  if (!AirspaceCircle && !AirspaceArea) return;

  AirspaceSelectInfo = (AirspaceSelectInfo_t*)
    malloc(sizeof(AirspaceSelectInfo_t) * NumberOfAirspaces);

  if (AirspaceSelectInfo==NULL) { // 100101
	StartupStore(_T("------ Airspace malloc SelectInfo Failed!!%s"), NEWLINE);
  }

  int index=0;
  int i;

  for (i=0; i<(int)NumberOfAirspaceCircles; i++){
    AirspaceSelectInfo[index].Index_Circle = i;
    AirspaceSelectInfo[index].Index_Area = -1;

    AirspaceSelectInfo[index].Distance = DISTANCEMODIFY*
      RangeAirspaceCircle(Longitude, Latitude, i);

    DistanceBearing(Latitude,
                    Longitude,
                    AirspaceCircle[i].Latitude, 
                    AirspaceCircle[i].Longitude,
                    NULL, &AirspaceSelectInfo[index].Direction);

    _tcsncpy(sTmp, AirspaceCircle[i].Name, 4);
    sTmp[4] = '\0';
    _tcsupr(sTmp);

    AirspaceSelectInfo[index].FourChars =
                    (((DWORD)sTmp[0] & 0xff) << 24)
                  + (((DWORD)sTmp[1] & 0xff) << 16)
                  + (((DWORD)sTmp[2] & 0xff) << 8)
                  + (((DWORD)sTmp[3] & 0xff) );

    AirspaceSelectInfo[index].Type = AirspaceCircle[i].Type;

    index++;
  }

  for (i=0; i<(int)NumberOfAirspaceAreas; i++){
    AirspaceSelectInfo[index].Index_Circle = -1;
    AirspaceSelectInfo[index].Index_Area = i;
    
    AirspaceSelectInfo[index].Distance = DISTANCEMODIFY*
      RangeAirspaceArea(Longitude, Latitude, i, 
                        &AirspaceSelectInfo[index].Direction);

    _tcsncpy(sTmp, AirspaceArea[i].Name, 4);
    sTmp[4] = '\0';
    _tcsupr(sTmp);

    AirspaceSelectInfo[index].FourChars =
                    (((DWORD)sTmp[0] & 0xff) << 24)
                  + (((DWORD)sTmp[1] & 0xff) << 16)
                  + (((DWORD)sTmp[2] & 0xff) << 8)
                  + (((DWORD)sTmp[3] & 0xff) );

    AirspaceSelectInfo[index].Type = AirspaceArea[i].Type;

    index++;
  }

  qsort(AirspaceSelectInfo, UpLimit,
      sizeof(AirspaceSelectInfo_t), AirspaceNameCompare);

}
#endif

static void UpdateList(void){

//  TCHAR sTmp[128];
  int i;
  bool distancemode = false;

  ItemIndex = 0;

  UpLimit= NumberOfAirspaces;
  LowLimit =0;

  if (TypeFilterIdx>0) {

    qsort(AirspaceSelectInfo, NumberOfAirspaces,
        sizeof(AirspaceSelectInfo_t), AirspaceTypeCompare);
    for (i=0; i<(int)NumberOfAirspaces; i++){
      if (!(AirspaceSelectInfo[i].Type == TypeFilterIdx-1)){
        UpLimit = i;
        break;
      }
    }
  }

  if (DistanceFilterIdx != 0){
    distancemode = true;
    qsort(AirspaceSelectInfo, UpLimit,
        sizeof(AirspaceSelectInfo_t), AirspaceDistanceCompare);
    for (i=0; i<(int)UpLimit; i++){
      if (AirspaceSelectInfo[i].Distance > DistanceFilter[DistanceFilterIdx]){
        UpLimit = i;
        break;
      }
    }
  }

  if (DirectionFilterIdx != 0){
    distancemode = true;
    qsort(AirspaceSelectInfo, UpLimit,
        sizeof(AirspaceSelectInfo_t), AirspaceDirectionCompare);
    for (i=0; i<UpLimit; i++){
      if (AirspaceSelectInfo[i].DirectionErr > 18){
        UpLimit = i;
        break;
      }
    }
  }

  if (NameFilterIdx != 0){
    TCHAR sTmp[8];
    LowLimit = UpLimit;
    qsort(AirspaceSelectInfo, UpLimit,
        sizeof(AirspaceSelectInfo_t), AirspaceNameCompare);
    sTmp[0] = NameFilter[NameFilterIdx];
    sTmp[1] = '\0';
    sTmp[2] = '\0';
    _tcsupr(sTmp);
    for (i=0; i<UpLimit; i++){
      if ((BYTE)(AirspaceSelectInfo[i].FourChars >> 24) >= (sTmp[0]&0xff)){
        LowLimit = i;
        break;
      }
    }

    for (; i<UpLimit; i++){
      if ((BYTE)(AirspaceSelectInfo[i].FourChars >> 24) != (sTmp[0]&0xff)){
        UpLimit = i;
        break;
      }
    }
  }

  if (!distancemode) {
    qsort(&AirspaceSelectInfo[LowLimit], UpLimit-LowLimit,
	  sizeof(AirspaceSelectInfo_t), AirspaceNameCompare);
  } else {
    qsort(&AirspaceSelectInfo[LowLimit], UpLimit-LowLimit,
	  sizeof(AirspaceSelectInfo_t), AirspaceDistanceCompare);
  }

  wAirspaceList->ResetList();
  wAirspaceList->Redraw();

}


static WndProperty *wpName;
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
    if (wpName) {
      wpName->GetDataField()->Set(TEXT("**"));
      wpName->RefreshDisplay();
    }
  }
}



static void OnFilterName(DataField *Sender, DataField::DataAccessKind_t Mode){

  TCHAR sTmp[12];
  int siz;

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      NameFilterIdx++;
	siz=sizeof(NameFilter[0]);
	if (siz==0) FailStore(_T("Division by zero onFilterName A"));
      if (siz>0) // 100101
      if (NameFilterIdx > sizeof(NameFilter)/sizeof(NameFilter[0])-2)
        NameFilterIdx = 1;
      FilterMode(true);
      UpdateList();
    break;
    case DataField::daDec:
      if (NameFilterIdx == 0) {
	siz=sizeof(NameFilter[0]);
	if (siz==0) FailStore(_T("Division by zero onFilterName A"));
        if (siz>0) // 100101
		NameFilterIdx = sizeof(NameFilter)/sizeof(NameFilter[0])-1;
      }
      else
        NameFilterIdx--;
      FilterMode(true);
      UpdateList();
    break;
  case DataField::daSpecial:
    break;
  }

  _stprintf(sTmp, TEXT("%c*"), NameFilter[NameFilterIdx]);
  Sender->Set(sTmp);

}



static void OnFilterDistance(DataField *Sender, 
                             DataField::DataAccessKind_t Mode) {

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
      if (DistanceFilterIdx > sizeof(DistanceFilter)/sizeof(DistanceFilter[0])-1)
        DistanceFilterIdx = 0;
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if (DistanceFilterIdx == 0)
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

  TCHAR sTmp[12];

  if (Sender == NULL){
    Sender = wpDirection->GetDataField();
  }

  if (DirectionFilterIdx == 0)
    _stprintf(sTmp, TEXT("%c"), '*');
  else if (DirectionFilterIdx == 1){
    int a = iround(CALCULATED_INFO.Heading);
    if (a <=0)
      a += 360;
    _stprintf(sTmp, TEXT("HDG(%d")TEXT(DEG)TEXT(")"), a);
  }else
    _stprintf(sTmp, TEXT("%d")TEXT(DEG), DirectionFilter[DirectionFilterIdx]);

  Sender->Set(sTmp);

}

static void OnFilterDirection(DataField *Sender, 
                              DataField::DataAccessKind_t Mode){

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
      if (DirectionFilterIdx == 0)
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

static void OnFilterType(DataField *Sender, 
                         DataField::DataAccessKind_t Mode) {

  TCHAR sTmp[20];

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
#ifdef LKAIRSPACE
      if (TypeFilterIdx > AIRSPACECLASSCOUNT) TypeFilterIdx = 0;		//Need to limit+1 because idx shifted with +1
#else
      if (TypeFilterIdx > sizeof(TypeFilter)/sizeof(TypeFilter[0])-1)
        TypeFilterIdx = 0;
#endif
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
#ifdef LKAIRSPACE
      if (TypeFilterIdx == 0)
        TypeFilterIdx = AIRSPACECLASSCOUNT;		//Need to limit+1 because idx shifted with +1
      else
        TypeFilterIdx--;
#else
      if (TypeFilterIdx == 0)
        TypeFilterIdx = sizeof(TypeFilter)/sizeof(TypeFilter[0])-1;
      else
        TypeFilterIdx--;
#endif
      FilterMode(false);
      UpdateList();
    break;
  case DataField::daSpecial:
    break;
  }

#ifdef LKAIRSPACE
  if (TypeFilterIdx>0) {
	_tcsncpy(sTmp, CAirspaceManager::Instance().GetAirspaceTypeText(TypeFilterIdx-1), sizeof(sTmp)/sizeof(sTmp[0]));
  } else {
	_tcscpy(sTmp, TEXT("*"));
  }
#else
  _stprintf(sTmp, TEXT("%s"), TypeFilter[TypeFilterIdx]);
#endif
  Sender->Set(sTmp);

}

static int DrawListIndex=0;

static void OnPaintListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
  int n = UpLimit - LowLimit;
  TCHAR sTmp[12];

  if (DrawListIndex < n){

    int i = LowLimit + DrawListIndex;

// Sleep(100);
#ifdef LKAIRSPACE
	TCHAR *Name = NULL;
	if (AirspaceSelectInfo[i].airspace) Name = (TCHAR*)AirspaceSelectInfo[i].airspace->Name();
#else
    TCHAR *Name = 0;
    if (AirspaceSelectInfo[i].Index_Circle>=0) {
      Name = AirspaceCircle[AirspaceSelectInfo[i].Index_Circle].Name;
    }
    if (AirspaceSelectInfo[i].Index_Area>=0) {
      Name = AirspaceArea[AirspaceSelectInfo[i].Index_Area].Name;
    }
#endif
    if (Name) {

      int w0, w1, w2, w3, x1, x2, x3;
      if (InfoBoxLayout::landscape) {
        w0 = 202*InfoBoxLayout::scale;
      } else {
        w0 = 225*InfoBoxLayout::scale;
      }
      w1 = GetTextWidth(hDC, TEXT("XXX"));
      w2 = GetTextWidth(hDC, TEXT(" 000km"));
      w3 = GetTextWidth(hDC, TEXT(" 000")TEXT(DEG));
      
      x1 = w0-w1-w2-w3;

      ExtTextOutClip(hDC, 2*InfoBoxLayout::scale, 2*InfoBoxLayout::scale,
                     (TCHAR*)Name, x1-InfoBoxLayout::scale*5); 
      
      sTmp[0] = '\0';
      sTmp[1] = '\0';
      sTmp[2] = '\0';
#ifdef LKAIRSPACE
	  _tcsncpy(sTmp, CAirspaceManager::Instance().GetAirspaceTypeShortText(AirspaceSelectInfo[i].Type), 4);
#else
      switch(AirspaceSelectInfo[i].Type) {
      case CLASSA: 
        _tcscpy(sTmp, TEXT("A"));
        break;
      case CLASSB: 
        _tcscpy(sTmp, TEXT("B"));
        break;
      case CLASSC: 
        _tcscpy(sTmp, TEXT("C"));
        break;
      case CLASSD: 
        _tcscpy(sTmp, TEXT("D"));
        break;
      case CLASSE: 
        _tcscpy(sTmp, TEXT("E"));
        break;
      case CLASSF: 
        _tcscpy(sTmp, TEXT("F"));
        break;
      case CLASSG: 
        _tcscpy(sTmp, TEXT("G"));
        break;
      case PROHIBITED: 
        _tcscpy(sTmp, TEXT("Prb"));
        break;
      case DANGER: 
        _tcscpy(sTmp, TEXT("Dgr"));
        break;
      case RESTRICT: 
        _tcscpy(sTmp, TEXT("Res"));
        break;
      case CTR: 
        _tcscpy(sTmp, TEXT("CTR"));
        break;
      case NOGLIDER: 
        _tcscpy(sTmp, TEXT("NoGl"));
        break;
      case WAVE:
        _tcscpy(sTmp, TEXT("Wav"));
        break;
      case OTHER:
        _tcscpy(sTmp, TEXT("?"));
        break;
      case AATASK:
        _tcscpy(sTmp, TEXT("AAT"));
        break;
      default:
        break;
      }
#endif
      // left justified
      
      ExtTextOut(hDC, x1, 2*InfoBoxLayout::scale,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);

      // right justified after airspace type
      _stprintf(sTmp, TEXT("%.0f%s"), 
                AirspaceSelectInfo[i].Distance,
                Units::GetDistanceName());
      x2 = w0-w3-GetTextWidth(hDC, sTmp);
      ExtTextOut(hDC, x2, 2*InfoBoxLayout::scale,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);
      
      // right justified after distance
      _stprintf(sTmp, TEXT("%d")TEXT(DEG),  iround(AirspaceSelectInfo[i].Direction));
      x3 = w0-GetTextWidth(hDC, sTmp);
      ExtTextOut(hDC, x3, 2*InfoBoxLayout::scale,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);
    } else {
      // should never get here!
    }
  } else {
    if (DrawListIndex == 0){
	// LKTOKEN  _@M466_ = "No Match!" 
      _stprintf(sTmp, TEXT("%s"), gettext(TEXT("_@M466_")));
      ExtTextOut(hDC, 2*InfoBoxLayout::scale, 2*InfoBoxLayout::scale,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);
    }
  }
}

// DrawListIndex = number of things to draw
// ItemIndex = current selected item


static void OnWpListInfo(WindowControl * Sender, 
                         WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
	if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = UpLimit-LowLimit;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}


static void OnWPSCloseClicked(WindowControl * Sender){
	(void)Sender;
  ItemIndex = -1;
  wf->SetModalResult(mrCancle);
}

static int OnTimerNotify(WindowControl * Sender) {
  (void)Sender;
  if (DirectionFilterIdx == 1){
    int a;
    a = (lastHeading - iround(CALCULATED_INFO.Heading));
    if (abs(a) > 0){
      UpdateList();
      SetDirectionData(NULL);
      wpDirection->RefreshDisplay();
    }
  }
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
#ifdef LKAIRSPACE
    wp->GetDataField()->SetAsString(CAirspaceManager::Instance().GetAirspaceTypeText(TypeFilterIdx));
#else
    wp->GetDataField()->SetAsString(TypeFilter[TypeFilterIdx]);
#endif
    wp->RefreshDisplay();
  }

  return(1);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnFilterName),
  DeclareCallBackEntry(OnFilterDistance),
  DeclareCallBackEntry(OnFilterDirection),
  DeclareCallBackEntry(OnFilterType),
  DeclareCallBackEntry(OnPaintListItem),
  DeclareCallBackEntry(OnWpListInfo),
  DeclareCallBackEntry(NULL)
};


void dlgAirspaceSelect(void) {

  StartHourglassCursor();

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = -1;
#ifdef LKAIRSPACE
  NumberOfAirspaces = CAirspaceManager::Instance().NumberofAirspaces();
#else
  NumberOfAirspaces = NumberOfAirspaceCircles + NumberOfAirspaceAreas;
#endif
  
  Latitude = GPS_INFO.Latitude;
  Longitude = GPS_INFO.Longitude;

  if (!InfoBoxLayout::landscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspaceSelect_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACESELECT_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspaceSelect.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACESELECT"));
  }

  if (!wf) return;

  ASSERT(wf!=NULL);

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->
   FindByName(TEXT("cmdClose")))->
    SetOnClickNotify(OnWPSCloseClicked);

  wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceList"));
  ASSERT(wAirspaceList!=NULL);
  wAirspaceList->SetBorderKind(BORDERLEFT);
  wAirspaceList->SetEnterCallback(OnAirspaceListEnter);

  wAirspaceListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceListEntry"));
  ASSERT(wAirspaceListEntry!=NULL);
  wAirspaceListEntry->SetCanFocus(true);

  wpName = (WndProperty*)wf->FindByName(TEXT("prpFltName"));
  wpDistance = (WndProperty*)wf->FindByName(TEXT("prpFltDistance"));
  wpDirection = (WndProperty*)wf->FindByName(TEXT("prpFltDirection"));

  PrepareData();
  UpdateList();

  wf->SetTimerNotify(OnTimerNotify);

  StopHourglassCursor();
  wf->ShowModal();

  free(AirspaceSelectInfo);

  delete wf;

  wf = NULL;

  return;

}

