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
#include "Sound/Sound.h"

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
#define NAMEFILTERLEN 10
static TCHAR sNameFilter[NAMEFILTERLEN+1];
static WndButton *wpnewName = NULL;
static unsigned NameFilterIdx=0;

static bool FullFlag=false; // 100502

static const double DistanceFilter[] = {0.0, 25.0, 50.0, 75.0, 100.0, 150.0, 
                                  250.0, 500.0, 1000.0};
static unsigned DistanceFilterIdx=0;

#define DirHDG -1
static int *StrIndex=NULL;
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




static void OnEnableClicked(WndButton* pWnd)
{

  if (ItemIndex != -1) {
     const size_t i = (FullFlag) ? StrIndex[LowLimit + ItemIndex] : (LowLimit + ItemIndex);
    if ((UpLimit-LowLimit>0)
        && (ItemIndex >= 0)  // JMW fixed bug, was >0
        && (ItemIndex < (UpLimit - LowLimit))) {

      CAirspace *airspace = AirspaceSelectInfo[i].airspace;
      if (airspace) {
          wf->SetTimerNotify(0,NULL);
          LKSound(TEXT("LK_TICK.WAV"));
          CAirspaceManager::Instance().PopupAirspaceDetail(airspace);
      }
    }
  } else {
    if(pWnd) {
      WndForm * pForm = pWnd->GetParentWndForm();
      if(pForm) {
        pForm->SetModalResult(mrCancel);
      }
    }    
  }
}





static void OnAirspaceListEnter(WindowControl * Sender,  WndListFrame::ListInfo_t *ListInfo)
{

if (ItemIndex != -1) {

if ((UpLimit-LowLimit>0)
&& (ItemIndex >= 0)  // JMW fixed bug, was >0
&& (ItemIndex < (UpLimit - LowLimit))) {
const size_t i = (FullFlag) ? StrIndex[LowLimit + ItemIndex] : (LowLimit + ItemIndex);
CAirspace *airspace = AirspaceSelectInfo[i].airspace;
if (airspace) {

  if(airspace->Enabled())
  {
      LKSound(TEXT("LK_BEEP0.WAV"));
     CAirspaceManager::Instance().AirspaceDisable(*airspace);
  }else{
      LKSound(TEXT("LK_BEEP1.WAV"));
     CAirspaceManager::Instance().AirspaceEnable(*airspace);
  }

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

static int AirspaceDisabledCompare(const void *elem1, const void *elem2 ){
  if(((AirspaceSelectInfo_t *)elem1)->airspace !=NULL)
    if (((const AirspaceSelectInfo_t *)elem1)->airspace->Enabled())
      return (+1);
  return (-1);
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


  _tcscpy(sNameFilter,_T(""));
  AirspaceSelectInfo = (AirspaceSelectInfo_t*)
    malloc(sizeof(AirspaceSelectInfo_t) * NumberOfAirspaces);

  if (AirspaceSelectInfo==NULL) {
	OutOfMemory(_T(__FILE__),__LINE__);
	return;
  }

  StrIndex = (int*)malloc(sizeof(int)*(NumberOfAirspaces+1));
  if (StrIndex==NULL) {
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
  int i;
 ItemIndex = 0;

  UpLimit= NumberOfAirspaces;
  LowLimit =0;
  FullFlag=false;
  if (TypeFilterIdx>0) {


    if(TypeFilterIdx == AIRSPACECLASSCOUNT+1)
    {
        qsort(AirspaceSelectInfo, NumberOfAirspaces,
               sizeof(AirspaceSelectInfo_t), AirspaceDisabledCompare);
      for (i=0; i<(int)NumberOfAirspaces; i++){
        if (AirspaceSelectInfo[i].airspace->Enabled()  ){
          UpLimit = i;
          break;
        }
    }
    }
    else
    {
      qsort(AirspaceSelectInfo, NumberOfAirspaces,
          sizeof(AirspaceSelectInfo_t), AirspaceTypeCompare);
      for (i=0; i<(int)NumberOfAirspaces; i++){
        if (!(AirspaceSelectInfo[i].Type == TypeFilterIdx-1)){
          UpLimit = i;
          break;
        }
      }
    }
  }

  if (DistanceFilterIdx != 0){

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

    qsort(AirspaceSelectInfo, UpLimit,
        sizeof(AirspaceSelectInfo_t), AirspaceDirectionCompare);
    for (i=0; i<UpLimit; i++){
      if (AirspaceSelectInfo[i].DirectionErr > 18){
        UpLimit = i;
        break;
      }
    }
  }


  TCHAR sTmp[NAMEFILTERLEN+1];
  TCHAR wname[NAME_SIZE+1];
  LowLimit = UpLimit;
  qsort(AirspaceSelectInfo, UpLimit,
      sizeof(AirspaceSelectInfo_t), AirspaceNameCompare);

  LK_tcsncpy(sTmp,sNameFilter, NAMEFILTERLEN);
  CharUpper(sTmp);
  int iFilterLen = _tcslen(sNameFilter);

  if (iFilterLen<GC_SUB_STRING_THRESHOLD)
  {
  for (i=0; i<UpLimit; i++){
    // compare entire name which may be more than 4 chars

      LKASSERT(i>=0 && i< NumberOfAirspaces);
      LK_tcsncpy(wname,AirspaceSelectInfo[i].airspace->Name() , NAME_SIZE);
      CharUpper(wname);

    if (_tcsnicmp(wname,sTmp,iFilterLen) >= 0) {
      LowLimit = i;
      break;
    }
  }

  if (_tcscmp(sTmp, TEXT("")) != 0) { // if it's blanks, then leave UpLimit at end of list
    for (; i<UpLimit; i++){

        LKASSERT(i>=0 && i< NumberOfAirspaces);
      LK_tcsncpy(wname,AirspaceSelectInfo[i].airspace->Name(), NAME_SIZE);
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
      // the AirspaceSelectInfo list has been sorted by filters, and then sorted by name. 0-UpLimit is the size.
      // now we create a secondary index pointing to this list
      for (i=0, matches=0; i<UpLimit; i++) {

          LKASSERT(i>=0 && i< NumberOfAirspaces);
              LK_tcsncpy(wname,AirspaceSelectInfo[i].airspace->Name(), NAME_SIZE);
              CharUpper(wname);

              if ( _tcsstr(  wname,sTmp ) ) {
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

  wAirspaceList->ResetList();
  wAirspaceList->Redraw();

}



//static WndProperty *wpName;
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
/*    if (wpName) {
      wpName->GetDataField()->Set(TEXT("**"));
      wpName->RefreshDisplay();
    }*/
  }
}


static void SetNameCaption(const TCHAR* tFilter) {

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

static void OnFilterName(WndButton* pWnd) {

  int SelectedAsp=-1;
  int CursorPos=0;
TCHAR newNameFilter[NAMEFILTERLEN+1];

LK_tcsncpy(newNameFilter, sNameFilter, NAMEFILTERLEN);
SelectedAsp =  dlgTextEntryShowModalAirspace(newNameFilter, NAMEFILTERLEN);



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
          SetNameCaption(TEXT("*"));
      } else {
          SetNameCaption(sNameFilter);
      }
}

FilterMode(true);
UpdateList();

if((SelectedAsp>=0) && (SelectedAsp < NumberOfAirspaces))
{
    CursorPos = SelectedAsp;
    /*
 for (i=0; i<UpLimit; i++)
 {

     if(AirspaceSelectInfo[i].Index == SelectedAsp)
     {
           CursorPos = i;
     }
     */
 }


 wAirspaceList->SetFocus();
wAirspaceList->SetItemIndexPos(CursorPos);
wAirspaceList->Redraw();
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
      if (TypeFilterIdx > (AIRSPACECLASSCOUNT+1)) TypeFilterIdx = 0;		//Need to limit+1 because idx shifted with +1
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if (TypeFilterIdx == 0)
        TypeFilterIdx = AIRSPACECLASSCOUNT+1;		//Need to limit+1 because idx shifted with +1
      else
        TypeFilterIdx--;
      FilterMode(false);
      UpdateList();
    break;
  case DataField::daSpecial:
    break;
  }

  if (TypeFilterIdx>0) {
    if( TypeFilterIdx == AIRSPACECLASSCOUNT+1)
      _tcscpy(sTmp, MsgToken(239));
    else
      LK_tcsncpy(sTmp, CAirspaceManager::GetAirspaceTypeText(TypeFilterIdx-1), sizeof(sTmp)/sizeof(sTmp[0])-1);
  } else {
	_tcscpy(sTmp, TEXT("*"));
  }
  Sender->Set(sTmp);

}



static unsigned int DrawListIndex=0;

// Painting elements after init

static void OnPaintListItem(WindowControl * Sender, LKSurface& Surface) {
    if (!Sender) {
        return;
    }

    unsigned int n = UpLimit - LowLimit;
    TCHAR sTmp[50];

    Surface.SetTextColor(RGB_BLACK);

    const int LineHeight = Sender->GetHeight();
    const int TextHeight = Surface.GetTextHeight(_T("dp"));

    const int TextPos = (LineHeight - TextHeight) / 2; // offset for text vertical center

    if (DrawListIndex < n) {

        const size_t i = (FullFlag) ? StrIndex[LowLimit + DrawListIndex] : (LowLimit + DrawListIndex);

        // Poco::Thread::sleep(100);

        const int width = Sender->GetWidth(); // total width

        const int w0 = LineHeight; // Picto Width
        const int w2 = Surface.GetTextWidth(TEXT(" 000km")); // distance Width
        _stprintf(sTmp, _T(" 000%s "), MsgToken(2179));
        const int w3 = Surface.GetTextWidth(sTmp); // bearing width

        const int w1 = width - w0 - 2*w2 - w3; // Max Name width

        // Draw Picto
        const RECT PictoRect = {0, 0, w0, LineHeight};

        AirspaceSelectInfo[i].airspace->DrawPicto(Surface, PictoRect);


        // Draw Name
        Surface.DrawTextClip(w0, TextPos, AirspaceSelectInfo[i].airspace->Name() , w1);

        LK_tcsncpy(sTmp,  CAirspaceManager::GetAirspaceTypeShortText(AirspaceSelectInfo[i].Type) , 4);
        const int w4 = Surface.GetTextWidth(sTmp);

        Surface.DrawTextClip(w1+w2, TextPos, sTmp,w4);

        // Draw Distance : right justified after waypoint Name
        _stprintf(sTmp, TEXT("%.0f%s"), AirspaceSelectInfo[i].Distance  , Units::GetDistanceName());
        const int x2 = width - w3 - Surface.GetTextWidth(sTmp);
        Surface.DrawText(x2, TextPos, sTmp);

        // Draw Bearing right justified after distance
        _stprintf(sTmp, TEXT("%d%s"), iround(AirspaceSelectInfo[i].Direction), MsgToken(2179));
        const int x3 = width - Surface.GetTextWidth(sTmp);
        Surface.DrawText(x3, TextPos, sTmp);
    } else {
        if (DrawListIndex == 0) {
            // LKTOKEN  _@M466_ = "No Match!"
            Surface.DrawText(IBLSCALE(2), TextPos, MsgToken(466));
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
  wf->SetTimerNotify(0,NULL);

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
  DataAccessCallbackEntry(OnFilterDistance),
  DataAccessCallbackEntry(OnFilterDirection),
  DataAccessCallbackEntry(OnFilterType),
  OnPaintCallbackEntry(OnPaintListItem),
  OnListCallbackEntry(OnWpListInfo),
  EndCallBackEntry()
};


void dlgAirspaceSelect(void) {

//  StartHourglassCursor();

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

  ((WndButton *)wf->
   FindByName(TEXT("cmdSelect")))->
    SetOnClickNotify(OnEnableClicked);

  ((WndButton *)wf->
   FindByName(TEXT("cmdName")))->
    SetOnClickNotify(OnFilterName);

  wpnewName = (WndButton*)wf->FindByName(TEXT("cmdName"));

  wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceList"));
  LKASSERT(wAirspaceList!=NULL);
  wAirspaceList->SetBorderKind(BORDERLEFT);
  wAirspaceList->SetEnterCallback(OnAirspaceListEnter);


  wAirspaceListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceListEntry"));
  LKASSERT(wAirspaceListEntry!=NULL);
  wAirspaceListEntry->SetCanFocus(true);

//  wpName = (WndProperty*)wf->FindByName(TEXT("prpFltName"));

  wpDistance = (WndProperty*)wf->FindByName(TEXT("prpFltDistance"));
  wpDirection = (WndProperty*)wf->FindByName(TEXT("prpFltDirection"));

  PrepareData();

  if (AirspaceSelectInfo==NULL){
	StopHourglassCursor();
	goto _return;
  }
  UpdateList();


  if ((wf->ShowModal() == mrOK) && (UpLimit - LowLimit > 0) && (ItemIndex >= 0)
   && (ItemIndex < (UpLimit - LowLimit))) {

        if (FullFlag)
                ItemIndex = StrIndex[LowLimit + ItemIndex];
        else
                ItemIndex = LowLimit + ItemIndex;
  }else
        ItemIndex = -1;

  wf->SetTimerNotify(10000, OnTimerNotify);



_return:
  if (AirspaceSelectInfo!=NULL) free(AirspaceSelectInfo);
  if (StrIndex!=NULL) free(StrIndex);
  StrIndex = NULL;
  delete wf;
  wf = NULL;

  return;

}

