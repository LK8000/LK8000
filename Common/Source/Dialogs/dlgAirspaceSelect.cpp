/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceSelect.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "Airspace.h"
#include "AirspaceWarning.h"
#include "Dialogs.h"
#include "Event/Event.h"
#include "resource.h"

typedef struct{
  CAirspace *airspace;
  double Distance;
  double Direction;
  mutable int    DirectionErr; // modified by sort ...
  unsigned int Type;
  unsigned int FourChars;
} AirspaceSelectInfo_t;

static double Latitude;
static double Longitude;

static WndForm *wf=NULL;
static WndListFrame *wAirspaceList=NULL;
static WndOwnerDrawFrame *wAirspaceListEntry = NULL;

static const TCHAR NameFilter[] = TEXT("*ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
static unsigned NameFilterIdx=0;

static const double DistanceFilter[] = {0.0, 25.0, 50.0, 75.0, 100.0, 150.0, 
                                  250.0, 500.0, 1000.0};
static unsigned DistanceFilterIdx=0;

#define DirHDG -1

static const int DirectionFilter[] = {0, DirHDG, 360, 30, 60, 90, 120, 150, 
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
          wf->SetTimerNotify(0,NULL);
          CAirspaceManager::Instance().PopupAirspaceDetail(airspace);
      }
    }
  } else {
    if(Sender) {
      WndForm * pForm = Sender->GetParentWndForm();
      if(pForm) {
        pForm->SetModalResult(mrCancel);
      }
    }    
  }
}

static int AirspaceNameCompare(const void *elem1, const void *elem2 ){
  if (((const AirspaceSelectInfo_t *)elem1)->FourChars < ((const AirspaceSelectInfo_t *)elem2)->FourChars)
    return (-1);
  if (((const AirspaceSelectInfo_t *)elem1)->FourChars > ((const AirspaceSelectInfo_t *)elem2)->FourChars)
    return (+1);
  // if the first four characters are the same let's do the full comparison
  const TCHAR *name1 = ((const AirspaceSelectInfo_t *)elem1)->airspace->Name();
  const TCHAR *name2 = ((const AirspaceSelectInfo_t *)elem2)->airspace->Name();
  return _tcscmp(name1, name2);
}

static int AirspaceDistanceCompare(const void *elem1, const void *elem2 ){
  if (((const AirspaceSelectInfo_t *)elem1)->Distance < ((const AirspaceSelectInfo_t *)elem2)->Distance)
    return (-1);
  if (((const AirspaceSelectInfo_t *)elem1)->Distance > ((const AirspaceSelectInfo_t *)elem2)->Distance)
    return (+1);
  return (0);
}

static int AirspaceTypeCompare(const void *elem1, const void *elem2 ){
  if (((const AirspaceSelectInfo_t *)elem1)->Type == TypeFilterIdx-1)
    return (-1);
  return (+1);
}


static int AirspaceDirectionCompare(const void *elem1, const void *elem2 ){

  int a, a1, a2;

  a = DirectionFilter[DirectionFilterIdx];
  if (a == DirHDG){
    a = iround(CALCULATED_INFO.Heading);
    lastHeading = a;
  }

  a1 = (int)(((const AirspaceSelectInfo_t *)elem1)->Direction - a);
  a2 = (int)(((const AirspaceSelectInfo_t *)elem2)->Direction - a);

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

  ((const AirspaceSelectInfo_t *)elem1)->DirectionErr = a1;
  ((const AirspaceSelectInfo_t *)elem2)->DirectionErr = a2;

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
	OutOfMemory(_T(__FILE__),__LINE__);
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
    CharUpper(sTmp);

    AirspaceSelectInfo[index].FourChars =
                    (((unsigned)sTmp[0] & 0xff) << 24)
                  + (((unsigned)sTmp[1] & 0xff) << 16)
                  + (((unsigned)sTmp[2] & 0xff) << 8)
                  + (((unsigned)sTmp[3] & 0xff) );

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
    CharUpper(sTmp);
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

  TCHAR sTmp[17];

  if (Sender == NULL){
    Sender = wpDirection->GetDataField();
  }

  if (DirectionFilterIdx == 0) {
    _stprintf(sTmp, TEXT("%c"), '*');
  } else if (DirectionFilterIdx == 1) {
    int a = iround(CALCULATED_INFO.Heading);
    if (a <=0) {
      a += 360;
    }
    _sntprintf(sTmp, array_size(sTmp), TEXT("HDG(%d%s)"), a, MsgToken(2179));
  }else {
    _sntprintf(sTmp, array_size(sTmp), TEXT("%d%s"), DirectionFilter[DirectionFilterIdx], MsgToken(2179));
  }

  sTmp[array_size(sTmp)-1] = _T('\0');

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
	LK_tcsncpy(sTmp, CAirspaceManager::GetAirspaceTypeText(TypeFilterIdx-1), sizeof(sTmp)/sizeof(sTmp[0])-1);
  } else {
	_tcscpy(sTmp, TEXT("*"));
  }
  Sender->Set(sTmp);

}

static int DrawListIndex=0;

static void OnPaintListItem(WindowControl * Sender, LKSurface& Surface){
  (void)Sender;
  int n = UpLimit - LowLimit;
  TCHAR sTmp[12];

  if (DrawListIndex < n){

    int i = LowLimit + DrawListIndex;

    Surface.SetTextColor(RGB_BLACK);

// Poco::Thread::sleep(100);
	const TCHAR *Name = NULL;
	if (AirspaceSelectInfo[i].airspace) Name = AirspaceSelectInfo[i].airspace->Name();
    if (Name) {
      const PixelRect rcClient(Sender->GetClientRect());
      const int w0 = rcClient.GetSize().cx;
      const int w1 = Surface.GetTextWidth(TEXT("XXX")) + DLGSCALE(2);
      
      _stprintf(sTmp, TEXT(" 000%s"), Units::GetDistanceName());
      const int w2 = Surface.GetTextWidth(sTmp) + DLGSCALE(2);
      
      _stprintf(sTmp, _T(" -000%s"), MsgToken(2179));
      const int w3 = Surface.GetTextWidth(sTmp) + DLGSCALE(2);
      
      const int x1 = w0-w1-w2-w3;

      Surface.DrawTextClip(DLGSCALE(2), DLGSCALE(2), Name, x1-DLGSCALE(5));
      
      sTmp[0] = '\0';
      sTmp[1] = '\0';
      sTmp[2] = '\0';
	  LK_tcsncpy(sTmp, CAirspaceManager::GetAirspaceTypeShortText(AirspaceSelectInfo[i].Type), 4);
      // left justified
     
      Surface.DrawText(x1, DLGSCALE(2), sTmp);

      // right justified after airspace type
      _stprintf(sTmp, TEXT("%.0f%s"), AirspaceSelectInfo[i].Distance, Units::GetDistanceName());
      const int x2 = w0-w3-Surface.GetTextWidth(sTmp);
      Surface.DrawText(x2, DLGSCALE(2), sTmp);
      
      // right justified after distance
      _stprintf(sTmp, TEXT("%d%s"),  iround(AirspaceSelectInfo[i].Direction), MsgToken(2179));
      const int x3 = w0-Surface.GetTextWidth(sTmp);
      Surface.DrawText(x3, DLGSCALE(2), sTmp);
    } else {
      // should never get here!
    }
  } else {
    if (DrawListIndex == 0){
      Surface.SetTextColor(RGB_BLACK);
	// LKTOKEN  _@M466_ = "No Match!" 
      Surface.DrawText(DLGSCALE(2), DLGSCALE(2), MsgToken(466));
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


static void OnWPSCloseClicked(WndButton* pWnd){
  ItemIndex = -1;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
}

static bool OnTimerNotify(WndForm* pWnd) {
  if (DirectionFilterIdx == 1){
    const int a = (lastHeading - iround(CALCULATED_INFO.Heading));
    if (abs(a) > 0){
      UpdateList();
      SetDirectionData(NULL);
      wpDirection->RefreshDisplay();
    }
  }
  return true;
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode){
  unsigned NewIndex = TypeFilterIdx;

  switch(KeyCode & 0xffff){
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

  if (TypeFilterIdx != NewIndex){
    TypeFilterIdx = NewIndex;
    FilterMode(false);
    UpdateList();

    WndProperty* wp = ((WndProperty *)wf->FindByName(TEXT("prpFltType")));
    if(wp && wp->GetDataField()) {
        wp->GetDataField()->SetAsString(CAirspaceManager::GetAirspaceTypeText(TypeFilterIdx));
        wp->RefreshDisplay();
    }
    return true;
  }

  return false;
}

static CallBackTableEntry_t CallBackTable[]={
  DataAccessCallbackEntry(OnFilterName),
  DataAccessCallbackEntry(OnFilterDistance),
  DataAccessCallbackEntry(OnFilterDirection),
  DataAccessCallbackEntry(OnFilterType),
  OnPaintCallbackEntry(OnPaintListItem),
  OnListCallbackEntry(OnWpListInfo),
  EndCallBackEntry()
};


void dlgAirspaceSelect(void) {

  StartHourglassCursor();

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = -1;
  NumberOfAirspaces = CAirspaceManager::Instance().NumberofAirspaces();
  
  Latitude = GPS_INFO.Latitude;
  Longitude = GPS_INFO.Longitude;

  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_AIRSPACESELECT_L : IDR_XML_AIRSPACESELECT_P);

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

  wpName = (WndProperty*)wf->FindByName(TEXT("prpFltName"));
  wpDistance = (WndProperty*)wf->FindByName(TEXT("prpFltDistance"));
  wpDirection = (WndProperty*)wf->FindByName(TEXT("prpFltDirection"));

  PrepareData();
  if (AirspaceSelectInfo==NULL){
	StopHourglassCursor();
	goto _return;
  }
  UpdateList();

  wf->SetTimerNotify(1000, OnTimerNotify);

  StopHourglassCursor();
  wf->ShowModal();

_return:
  if (AirspaceSelectInfo!=NULL) free(AirspaceSelectInfo);
  delete wf;
  wf = NULL;
  return;

}

