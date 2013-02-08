/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceSelect.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include <aygshell.h>
#include "InfoBoxLayout.h"
#include "Airspace.h"
#include "AirspaceWarning.h"
#include "Dialogs.h"


typedef struct{
  CAirspace *airspace;
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

static unsigned TypeFilterIdx=0;

static int UpLimit=0;
static int LowLimit=0;

static int ItemIndex = -1;

static AirspaceSelectInfo_t *AirspaceSelectInfo=NULL;

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
          if (airspace->Enabled()) {
            answer = MessageBoxX(hWndMapWindow,
                    Name,
                      // LKTOKEN  _@M1284_ "Disable this airspace?" 
                    gettext(TEXT("_@M1284_")),
                    MB_YESNOCANCEL|MB_ICONQUESTION);
            if (answer == IDYES) {
              CAirspaceManager::Instance().AirspaceDisable(*airspace);
            }
          } else {
            answer = MessageBoxX(hWndMapWindow,
                    Name,
                    // LKTOKEN  _@M1280_ "Enable this airspace?"
                    gettext(TEXT("_@M1280_")),
                    MB_YESNOCANCEL|MB_ICONQUESTION);
            if (answer == IDYES) {
              // this will cancel a daily ack
              CAirspaceManager::Instance().AirspaceEnable(*airspace);
            }
          }
        }
      }
    }
  } else {
    wf->SetModalResult(mrCancle);
  }
}

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

static void PrepareData(void){

  TCHAR sTmp[5];

  if (NumberOfAirspaces==0) return;

  AirspaceSelectInfo = (AirspaceSelectInfo_t*)
    malloc(sizeof(AirspaceSelectInfo_t) * NumberOfAirspaces);

  if (AirspaceSelectInfo==NULL) {
	OutOfMemory(__FILE__,__LINE__);
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

    LK_tcsncpy(sTmp, (*it)->Name(), 4);
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
	LKASSERT(siz>0);
      if (siz>0) // 100101
      if (NameFilterIdx > sizeof(NameFilter)/sizeof(NameFilter[0])-2)
        NameFilterIdx = 1;
      FilterMode(true);
      UpdateList();
    break;
    case DataField::daDec:
      if (NameFilterIdx == 0) {
	siz=sizeof(NameFilter[0]);
	LKASSERT(siz>0);
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
      if (TypeFilterIdx > AIRSPACECLASSCOUNT) TypeFilterIdx = 0;		//Need to limit+1 because idx shifted with +1
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if (TypeFilterIdx == 0)
        TypeFilterIdx = AIRSPACECLASSCOUNT;		//Need to limit+1 because idx shifted with +1
      else
        TypeFilterIdx--;
      FilterMode(false);
      UpdateList();
    break;
  case DataField::daSpecial:
    break;
  }

  if (TypeFilterIdx>0) {
	LK_tcsncpy(sTmp, CAirspaceManager::Instance().GetAirspaceTypeText(TypeFilterIdx-1), sizeof(sTmp)/sizeof(sTmp[0])-1);
  } else {
	_tcscpy(sTmp, TEXT("*"));
  }
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
	TCHAR *Name = NULL;
	if (AirspaceSelectInfo[i].airspace) Name = (TCHAR*)AirspaceSelectInfo[i].airspace->Name();
    if (Name) {

      int w0, w1, w2, w3, x1, x2, x3;
      if (ScreenLandscape) {
        w0 = 202*ScreenScale;
      } else {
        w0 = 225*ScreenScale;
      }
      w1 = GetTextWidth(hDC, TEXT("XXX"));
      w2 = GetTextWidth(hDC, TEXT(" 000km"));
      w3 = GetTextWidth(hDC, TEXT(" 000")TEXT(DEG));
      
      x1 = w0-w1-w2-w3;

      ExtTextOutClip(hDC, 2*ScreenScale, 2*ScreenScale,
                     (TCHAR*)Name, x1-ScreenScale*5); 
      
      sTmp[0] = '\0';
      sTmp[1] = '\0';
      sTmp[2] = '\0';
	  LK_tcsncpy(sTmp, CAirspaceManager::Instance().GetAirspaceTypeShortText(AirspaceSelectInfo[i].Type), 4);
      // left justified
     
      ExtTextOut(hDC, x1, 2*ScreenScale,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);

      // right justified after airspace type
      _stprintf(sTmp, TEXT("%.0f%s"), 
                AirspaceSelectInfo[i].Distance,
                Units::GetDistanceName());
      x2 = w0-w3-GetTextWidth(hDC, sTmp);
      ExtTextOut(hDC, x2, 2*ScreenScale,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);
      
      // right justified after distance
      _stprintf(sTmp, TEXT("%d")TEXT(DEG),  iround(AirspaceSelectInfo[i].Direction));
      x3 = w0-GetTextWidth(hDC, sTmp);
      ExtTextOut(hDC, x3, 2*ScreenScale,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);
    } else {
      // should never get here!
    }
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
    wp->GetDataField()->SetAsString(CAirspaceManager::Instance().GetAirspaceTypeText(TypeFilterIdx));
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
  NumberOfAirspaces = CAirspaceManager::Instance().NumberofAirspaces();
  
  Latitude = GPS_INFO.Latitude;
  Longitude = GPS_INFO.Longitude;

  if (!ScreenLandscape) {
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

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->
   FindByName(TEXT("cmdClose")))->
    SetOnClickNotify(OnWPSCloseClicked);

  wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceList"));
  LKASSERT(wAirspaceList!=NULL);
  wAirspaceList->SetBorderKind(BORDERLEFT);
  wAirspaceList->SetEnterCallback(OnAirspaceListEnter);


  wAirspaceListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceListEntry"));
  LKASSERT(wAirspaceListEntry!=NULL);
  wAirspaceListEntry->SetCanFocus(true);

  // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
  if ( wAirspaceList->ScrollbarWidth == -1) {
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wAirspaceList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);

  }
  wAirspaceListEntry->SetWidth(wAirspaceList->GetWidth() - wAirspaceList->ScrollbarWidth - 5);


  wpName = (WndProperty*)wf->FindByName(TEXT("prpFltName"));
  wpDistance = (WndProperty*)wf->FindByName(TEXT("prpFltDistance"));
  wpDirection = (WndProperty*)wf->FindByName(TEXT("prpFltDirection"));

  PrepareData();
  if (AirspaceSelectInfo==NULL){
	StopHourglassCursor();
	goto _return;
  }
  UpdateList();

  wf->SetTimerNotify(OnTimerNotify);

  StopHourglassCursor();
  wf->ShowModal();

_return:
  if (AirspaceSelectInfo!=NULL) free(AirspaceSelectInfo);
  delete wf;
  wf = NULL;
  return;

}

