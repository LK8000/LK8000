/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include <aygshell.h>

#include "lk8000.h"

#include "externs.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "MapWindow.h"

#include "utils/heapcheck.h"


static WndForm *wf=NULL;
static WndListFrame *wAirspacePatternsList=NULL;
static WndOwnerDrawFrame *wAirspacePatternsListEntry = NULL;

static int ItemIndex = -1;


static void UpdateList(void){
  wAirspacePatternsList->ResetList();
  wAirspacePatternsList->Redraw();
}

static int DrawListIndex=0;

static void OnAirspacePatternsPaintListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
  if ((DrawListIndex < NUMAIRSPACEBRUSHES) &&(DrawListIndex>=0)) {
    int i = DrawListIndex;
    SelectObject(hDC, GetStockObject(WHITE_BRUSH));
    SelectObject(hDC, GetStockObject(BLACK_PEN));
    SetBkColor(hDC, 
	       RGB(0xFF, 0xFF, 0xFF));
    SelectObject(hDC, 
		 MapWindow::GetAirspaceBrush(i)); 
    SetTextColor(hDC, RGB(0x00,0x00, 0x00));
    Rectangle(hDC, 
              100*InfoBoxLayout::scale, 
              2*InfoBoxLayout::scale,
              180*InfoBoxLayout::scale,
              22*InfoBoxLayout::scale);
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

static void OnCloseClicked(WindowControl * Sender){
  (void)Sender;
  ItemIndex = -1;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAirspacePatternsPaintListItem),
  DeclareCallBackEntry(OnAirspacePatternsListInfo),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


int dlgAirspacePatternsShowModal(void){

  ItemIndex = -1;

  if (!InfoBoxLayout::landscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspacePatterns_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACEPATTERNS_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspacePatterns.xml"));
    wf = dlgLoadFromXML(CallBackTable,                      
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACEPATTERNS"));
  }

  if (!wf) return -1;

  ASSERT(wf!=NULL);

  wAirspacePatternsList = (WndListFrame*)wf->FindByName(TEXT("frmAirspacePatternsList"));
  ASSERT(wAirspacePatternsList!=NULL);
  wAirspacePatternsList->SetBorderKind(BORDERLEFT);
  wAirspacePatternsList->SetEnterCallback(OnAirspacePatternsListEnter);

  wAirspacePatternsListEntry = (WndOwnerDrawFrame*)wf->
    FindByName(TEXT("frmAirspacePatternsListEntry"));
  ASSERT(wAirspacePatternsListEntry!=NULL);
  wAirspacePatternsListEntry->SetCanFocus(true);

  UpdateList();

  wf->ShowModal();

  // now retrieve back the properties...

  delete wf;

  wf = NULL;

  return ItemIndex;
}

