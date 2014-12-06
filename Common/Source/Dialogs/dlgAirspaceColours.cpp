/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceColours.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "LKObjects.h"


static WndForm *wf=NULL;
static WndListFrame *wAirspaceColoursList=NULL;
static WndOwnerDrawFrame *wAirspaceColoursListEntry = NULL;

static int ItemIndex = -1;


static void UpdateList(void){
  wAirspaceColoursList->ResetList();
  wAirspaceColoursList->Redraw();
}

static int DrawListIndex=0;

static void OnAirspaceColoursPaintListItem(WindowControl * Sender, LKSurface& Surface){
  (void)Sender;
  if ((DrawListIndex < NUMAIRSPACECOLORS) &&(DrawListIndex>=0)) {
    int i = DrawListIndex;
    Surface.SelectObject(LKBrush_White);
    Surface.SelectObject(LK_BLACK_PEN);
    Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
#ifdef HAVE_HATCHED_BRUSH          
    Surface.SelectObject(MapWindow::GetAirspaceBrush(1)); // this is the solid brush
#else
#warning "TODO : maybe we need solid brush or that !"
#endif
    Surface.SetTextColor(MapWindow::GetAirspaceColour(i));
    Surface.Rectangle(
              100*ScreenScale, 
              2*ScreenScale,
              180*ScreenScale,
              22*ScreenScale);
  }
}


static void OnAirspaceColoursListEnter(WindowControl * Sender, 
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=NUMAIRSPACECOLORS) {
    ItemIndex = NUMAIRSPACECOLORS-1;
  }
  if (ItemIndex>=0) {
    wf->SetModalResult(mrOK);
  }
}


static void OnAirspaceColoursListInfo(WindowControl * Sender, 
			       WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = NUMAIRSPACECOLORS;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(Window* pWnd){
  (void)pWnd;
  ItemIndex = -1;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  OnPaintCallbackEntry(OnAirspaceColoursPaintListItem),
  OnListCallbackEntry(OnAirspaceColoursListInfo),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


int dlgAirspaceColoursShowModal(void){

  ItemIndex = -1;

  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspaceColours_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_AIRSPACECOLOURS_L"));
  } else {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspaceColours.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_AIRSPACECOLOURS"));
  }

  if (!wf) return -1;

  wAirspaceColoursList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceColoursList"));
  LKASSERT(wAirspaceColoursList!=NULL);
  wAirspaceColoursList->SetBorderKind(BORDERLEFT);
  wAirspaceColoursList->SetEnterCallback(OnAirspaceColoursListEnter);
  wAirspaceColoursList->SetWidth(wf->GetWidth() - wAirspaceColoursList->GetLeft()-2);

  wAirspaceColoursListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceColoursListEntry"));
  LKASSERT(wAirspaceColoursListEntry!=NULL);
  wAirspaceColoursListEntry->SetCanFocus(true);

   // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
  if ( wAirspaceColoursList->ScrollbarWidth == -1) {
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wAirspaceColoursList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);

  }
  wAirspaceColoursListEntry->SetWidth(wAirspaceColoursList->GetWidth() - wAirspaceColoursList->ScrollbarWidth - 5);


  UpdateList();

  wf->ShowModal();

  delete wf;

  wf = NULL;

  return ItemIndex;
}


