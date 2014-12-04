/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspacePatterns.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "LKObjects.h"

static WndForm *wf=NULL;
static WndListFrame *wAirspacePatternsList=NULL;
static WndOwnerDrawFrame *wAirspacePatternsListEntry = NULL;

static int ItemIndex = -1;


static void UpdateList(void){
  wAirspacePatternsList->ResetList();
  wAirspacePatternsList->Redraw();
}

static int DrawListIndex=0;

static void OnAirspacePatternsPaintListItem(WindowControl * Sender, LKSurface& Surface) {
    (void) Sender;
    if ((DrawListIndex < NUMAIRSPACEBRUSHES) &&(DrawListIndex >= 0)) {
        int i = DrawListIndex;
        Surface.SelectObject(LKBrush_White);
        Surface.SelectObject(LK_BLACK_PEN);
        Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
        Surface.SelectObject(MapWindow::GetAirspaceBrush(i));
        Surface.SetTextColor(LKColor(0x00, 0x00, 0x00));
        Surface.Rectangle(100 * ScreenScale, 2 * ScreenScale, 180 * ScreenScale, 22 * ScreenScale);
    }
}


static void OnAirspacePatternsListEnter(WindowControl * Sender, 
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=NUMAIRSPACEBRUSHES) {
    ItemIndex = NUMAIRSPACEBRUSHES-1;
  }
  if (ItemIndex>=0) {
    wf->SetModalResult(mrOK);
  }
}


static void OnAirspacePatternsListInfo(WindowControl * Sender, 
			       WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = NUMAIRSPACEBRUSHES;
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
  OnPaintCallbackEntry(OnAirspacePatternsPaintListItem),
  OnListCallbackEntry(OnAirspacePatternsListInfo),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


int dlgAirspacePatternsShowModal(void){

  ItemIndex = -1;

  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspacePatterns_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_AIRSPACEPATTERNS_L"));
  } else {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspacePatterns.xml"));
    wf = dlgLoadFromXML(CallBackTable,                      
                        filename, 
                        TEXT("IDR_XML_AIRSPACEPATTERNS"));
  }

  if (!wf) return -1;

  //ASSERT(wf!=NULL);

  wAirspacePatternsList = (WndListFrame*)wf->FindByName(TEXT("frmAirspacePatternsList"));
  //ASSERT(wAirspacePatternsList!=NULL);
  wAirspacePatternsList->SetBorderKind(BORDERLEFT);
  wAirspacePatternsList->SetEnterCallback(OnAirspacePatternsListEnter);

  wAirspacePatternsListEntry = (WndOwnerDrawFrame*)wf->
    FindByName(TEXT("frmAirspacePatternsListEntry"));
  //ASSERT(wAirspacePatternsListEntry!=NULL);
  wAirspacePatternsListEntry->SetCanFocus(true);

  UpdateList();

  wf->ShowModal();

  // now retrieve back the properties...

  delete wf;

  wf = NULL;

  return ItemIndex;
}

