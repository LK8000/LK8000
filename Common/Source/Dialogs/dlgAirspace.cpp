/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspace.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"

#include "InfoBoxLayout.h"
#include "LKObjects.h"


static WndForm *wf=NULL;
static WndListFrame *wAirspaceList=NULL;
static WndOwnerDrawFrame *wAirspaceListEntry = NULL;

static int ItemIndex = -1;
static bool colormode = false;

int dlgAirspaceColoursShowModal(void);
int dlgAirspacePatternsShowModal(void);

static void UpdateList(void){
  wAirspaceList->ResetList();
  wAirspaceList->Redraw();
}

static int DrawListIndex=0;

static void OnAirspacePaintListItem(WindowControl * Sender, LKSurface& Surface){
  
  TCHAR label[40];
  (void)Sender;
  if (DrawListIndex < AIRSPACECLASSCOUNT){
    int i = DrawListIndex;
	LK_tcsncpy(label, CAirspaceManager::Instance().GetAirspaceTypeText(i), 39);
    int w0, w1, w2, x0;
    if (ScreenLandscape) {
      w0 = 202*ScreenScale;
    } else {
      w0 = 225*ScreenScale;
    }
	// LKTOKEN  _@M789_ = "Warn" 
    w1 = Surface.GetTextWidth(gettext(TEXT("_@M789_")))+ScreenScale*10;
	// LKTOKEN  _@M241_ = "Display" 
    w2 = Surface.GetTextWidth(gettext(TEXT("_@M241_")))+ScreenScale*10;
    x0 = w0-w1-w2;

    Surface.DrawTextClip(2*ScreenScale, 2*ScreenScale,
                   label, x0-ScreenScale*10);

    if (colormode) {

      Surface.SelectObject(LK_WHITE_PEN);
      Surface.SelectObject(LKBrush_White);
      Surface.Rectangle(x0, 2*ScreenScale,w0, 22*ScreenScale);
      Surface.SetTextColor(MapWindow::GetAirspaceColourByClass(i));
      Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
      Surface.SelectObject(MapWindow::GetAirspaceBrushByClass(i));
      Surface.Rectangle(x0, 2*ScreenScale,w0, 22*ScreenScale);
        
    } else {
    
      bool iswarn;
      bool isdisplay;

      iswarn = (MapWindow::iAirspaceMode[i]>=2);
      isdisplay = ((MapWindow::iAirspaceMode[i]%2)>0);
      if (iswarn) {
	// LKTOKEN  _@M789_ = "Warn" 
        _tcscpy(label, gettext(TEXT("_@M789_")));
        Surface.DrawText(w0-w1-w2, 2*ScreenScale, label, _tcslen(label));
      }
      if (isdisplay) {
	// LKTOKEN  _@M241_ = "Display" 
        _tcscpy(label, gettext(TEXT("_@M241_")));
        Surface.DrawText(w0-w2, 2*ScreenScale, label, _tcslen(label));
      }

    }

  }
}


static bool changed = false;

static void OnAirspaceListEnter(WindowControl * Sender, 
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=AIRSPACECLASSCOUNT) {
    ItemIndex = AIRSPACECLASSCOUNT-1;
  }
  if (ItemIndex>=0) {

    if (colormode) {
      int c = dlgAirspaceColoursShowModal();
      if (c>=0) {
	MapWindow::iAirspaceColour[ItemIndex] = c; 
	changed = true;
      }
      int p = dlgAirspacePatternsShowModal();
      if (p>=0) {
	MapWindow::iAirspaceBrush[ItemIndex] = p; 
	changed = true;
      }
    } else {
      int v = (MapWindow::iAirspaceMode[ItemIndex]+1)%4;
      MapWindow::iAirspaceMode[ItemIndex] = v;
      //  wAirspaceList->Redraw();
      changed = true;
    }
  }
}


static void OnAirspaceListInfo(WindowControl * Sender, 
			       WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = AIRSPACECLASSCOUNT;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(WindowControl * Sender){
    (void)Sender;
	wf->SetModalResult(mrOK);
}


static void OnLookupClicked(WindowControl * Sender){
  (void)Sender;
  dlgAirspaceSelect();
}


static CallBackTableEntry_t CallBackTable[]={
  OnPaintCallbackEntry(OnAirspacePaintListItem),
  OnListCallbackEntry(OnAirspaceListInfo),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnLookupClicked),
  EndCallBackEntry()
};


bool dlgAirspaceShowModal(bool coloredit){

  colormode = coloredit;

  ItemIndex = -1;

  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspace_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_AIRSPACE_L"));
  } else {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspace.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_AIRSPACE"));
  }
  if (!wf) return false;

  wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceList"));
  LKASSERT(wAirspaceList!=NULL);
  wAirspaceList->SetBorderKind(BORDERLEFT);
  wAirspaceList->SetEnterCallback(OnAirspaceListEnter);

  wAirspaceListEntry = (WndOwnerDrawFrame*)wf->
  FindByName(TEXT("frmAirspaceListEntry"));
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


  UpdateList();

  changed = false;

  wf->ShowModal();


  delete wf;

  wf = NULL;

  return changed;
}

