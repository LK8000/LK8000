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

static void OnAirspacePaintListItem(WindowControl * Sender, HDC hDC){
  
  TCHAR label[40];
  (void)Sender;
  if (DrawListIndex < AIRSPACECLASSCOUNT){
    int i = DrawListIndex;
	_tcsncpy(label, CAirspaceManager::Instance().GetAirspaceTypeText(i), 39);
	label[39]=0;
    int w0, w1, w2, x0;
    if (InfoBoxLayout::landscape) {
      w0 = 202*InfoBoxLayout::scale;
    } else {
      w0 = 225*InfoBoxLayout::scale;
    }
	// LKTOKEN  _@M789_ = "Warn" 
    w1 = GetTextWidth(hDC, gettext(TEXT("_@M789_")))+InfoBoxLayout::scale*10;
	// LKTOKEN  _@M241_ = "Display" 
    w2 = GetTextWidth(hDC, gettext(TEXT("_@M241_")))+InfoBoxLayout::scale*10;
    x0 = w0-w1-w2;

    ExtTextOutClip(hDC, 2*InfoBoxLayout::scale, 2*InfoBoxLayout::scale,
                   label, x0-InfoBoxLayout::scale*10);

    if (colormode) {

      SelectObject(hDC, GetStockObject(WHITE_PEN));
      SelectObject(hDC, GetStockObject(WHITE_BRUSH));
      Rectangle(hDC,
          x0, 2*InfoBoxLayout::scale,
          w0, 22*InfoBoxLayout::scale);
      SetTextColor(hDC,
         MapWindow::GetAirspaceColourByClass(i));
         SetBkColor(hDC,
         RGB(0xFF, 0xFF, 0xFF));
      SelectObject(hDC,
		   MapWindow::GetAirspaceBrushByClass(i));
        Rectangle(hDC,
        x0, 2*InfoBoxLayout::scale,
        w0, 22*InfoBoxLayout::scale);
        
    } else {
    
      bool iswarn;
      bool isdisplay;

      iswarn = (MapWindow::iAirspaceMode[i]>=2);
      isdisplay = ((MapWindow::iAirspaceMode[i]%2)>0);
      if (iswarn) {
	// LKTOKEN  _@M789_ = "Warn" 
        _tcscpy(label, gettext(TEXT("_@M789_")));
        ExtTextOut(hDC,
                   w0-w1-w2,
                   2*InfoBoxLayout::scale,
                   ETO_OPAQUE, NULL,
                   label,
                   _tcslen(label),
                   NULL);
      }
      if (isdisplay) {
	// LKTOKEN  _@M241_ = "Display" 
        _tcscpy(label, gettext(TEXT("_@M241_")));
        ExtTextOut(hDC,
                   w0-w2,
                   2*InfoBoxLayout::scale,
                   ETO_OPAQUE, NULL,
                   label,
                   _tcslen(label),
                   NULL);
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
	SetRegistryColour(ItemIndex,MapWindow::iAirspaceColour[ItemIndex]);
	changed = true;
      }
      int p = dlgAirspacePatternsShowModal();
      if (p>=0) {
	MapWindow::iAirspaceBrush[ItemIndex] = p; 
	SetRegistryBrush(ItemIndex,MapWindow::iAirspaceBrush[ItemIndex]);
	changed = true;
      }
    } else {
      int v = (MapWindow::iAirspaceMode[ItemIndex]+1)%4;
      MapWindow::iAirspaceMode[ItemIndex] = v;
      //  wAirspaceList->Redraw();
      SetRegistryAirspaceMode(ItemIndex);
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
  DeclareCallBackEntry(OnAirspacePaintListItem),
  DeclareCallBackEntry(OnAirspaceListInfo),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnLookupClicked),
  DeclareCallBackEntry(NULL)
};


bool dlgAirspaceShowModal(bool coloredit){

  colormode = coloredit;

  ItemIndex = -1;

  if (!InfoBoxLayout::landscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspace_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACE_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAirspace.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_AIRSPACE"));
  }
  if (!wf) return false;

  ASSERT(wf!=NULL);

  wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceList"));
  ASSERT(wAirspaceList!=NULL);
  wAirspaceList->SetBorderKind(BORDERLEFT);
  wAirspaceList->SetEnterCallback(OnAirspaceListEnter);

  wAirspaceListEntry = (WndOwnerDrawFrame*)wf->
    FindByName(TEXT("frmAirspaceListEntry"));
  ASSERT(wAirspaceListEntry!=NULL);
  wAirspaceListEntry->SetCanFocus(true);

  UpdateList();

  changed = false;

  wf->ShowModal();

  // now retrieve back the properties...
  if (changed) {
    StoreRegistry();
    DoStatusMessage(gettext(TEXT("_@M877_"))); // Configuration saved
  };

  delete wf;

  wf = NULL;

  return changed;
}

