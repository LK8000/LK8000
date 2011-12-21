/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceColours.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>

#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "MapWindow.h"



static WndForm *wf=NULL;
static WndListFrame *wAirspaceColoursList=NULL;
static WndOwnerDrawFrame *wAirspaceColoursListEntry = NULL;

static int ItemIndex = -1;


static void UpdateList(void){
  wAirspaceColoursList->ResetList();
  wAirspaceColoursList->Redraw();
}

static int DrawListIndex=0;

static void OnAirspaceColoursPaintListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
  if ((DrawListIndex < NUMAIRSPACECOLORS) &&(DrawListIndex>=0)) {
    int i = DrawListIndex;
    SelectObject(hDC, GetStockObject(WHITE_BRUSH));
    SelectObject(hDC, GetStockObject(BLACK_PEN));
    SetBkColor(hDC, 
	       RGB(0xFF, 0xFF, 0xFF));
    SelectObject(hDC, 
		 MapWindow::GetAirspaceBrush(1)); // this is the solid brush
    SetTextColor(hDC, 
		 MapWindow::GetAirspaceColour(i));
    Rectangle(hDC, 
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

static void OnCloseClicked(WindowControl * Sender){
  (void)Sender;
  ItemIndex = -1;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAirspaceColoursPaintListItem),
  DeclareCallBackEntry(OnAirspaceColoursListInfo),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


int dlgAirspaceColoursShowModal(void){

  ItemIndex = -1;

  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspaceColours_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACECOLOURS_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspaceColours.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACECOLOURS"));
  }

  if (!wf) return -1;

  //ASSERT(wf!=NULL);

  wAirspaceColoursList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceColoursList"));
  //ASSERT(wAirspaceColoursList!=NULL);
  wAirspaceColoursList->SetBorderKind(BORDERLEFT);
  wAirspaceColoursList->SetEnterCallback(OnAirspaceColoursListEnter);

  wAirspaceColoursListEntry = (WndOwnerDrawFrame*)wf->
    FindByName(TEXT("frmAirspaceColoursListEntry"));
  //ASSERT(wAirspaceColoursListEntry!=NULL);
  wAirspaceColoursListEntry->SetCanFocus(true);

  UpdateList();

  wf->ShowModal();

  // now retrieve back the properties...

  delete wf;

  wf = NULL;

  return ItemIndex;
}


