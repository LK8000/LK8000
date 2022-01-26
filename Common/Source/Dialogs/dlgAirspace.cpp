/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspace.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "LKObjects.h"
#include "resource.h"

static bool colormode = false;


static void UpdateList(WndListFrame* pWnd){
  if(pWnd) {
    pWnd->ResetList();
    pWnd->Redraw();
  }
}

static int DrawListIndex=0;

static void OnAirspacePaintListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){

  TCHAR label[40];

  if (DrawListIndex < AIRSPACECLASSCOUNT){
    int i = DrawListIndex;
	LK_tcsncpy(label, CAirspaceManager::GetAirspaceTypeText(i), 39);
    
    const PixelRect rcClient(Sender->GetClientRect());
    const int w0 = rcClient.GetSize().cx;
	// LKTOKEN  _@M789_ = "Warn"
    const int w1 = Surface.GetTextWidth(MsgToken(789)) + DLGSCALE(10);
	// LKTOKEN  _@M241_ = "Display"
    const int w2 = Surface.GetTextWidth(MsgToken(241)) + DLGSCALE(2);
    
    const int x0 = w0-w1-w2;

    Surface.SetTextColor(RGB_BLACK);
    Surface.DrawTextClip(DLGSCALE(2), DLGSCALE(2), label, x0-DLGSCALE(10));

    if (colormode) {
      PixelRect rcColor = {
          x0, 
          rcClient.top + DLGSCALE(2), 
          rcClient.right - DLGSCALE(2), 
          rcClient.bottom - DLGSCALE(2)
      };

      Surface.SelectObject(LK_WHITE_PEN);
      Surface.SelectObject(LKBrush_White);
      Surface.Rectangle(rcColor.left, rcColor.top, rcColor.right, rcColor.bottom);

      Surface.SetTextColor(MapWindow::GetAirspaceColourByClass(i));
      Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
      Surface.SelectObject(MapWindow::GetAirspaceBrushByClass(i));
      Surface.Rectangle(rcColor.left, rcColor.top, rcColor.right, rcColor.bottom);

    } else {

      bool iswarn;
      bool isdisplay;

      iswarn = (MapWindow::iAirspaceMode[i]>=2);
      isdisplay = ((MapWindow::iAirspaceMode[i]%2)>0);
      if (iswarn) {
        // LKTOKEN  _@M789_ = "Warn"
        Surface.DrawText(x0, DLGSCALE(2), MsgToken(789));
      }
      if (isdisplay) {
        // LKTOKEN  _@M241_ = "Display"
        Surface.DrawText(w0-w2, DLGSCALE(2), MsgToken(241));
      }

    }

  }
}


static bool changed = false;

static void OnAirspaceListEnter(WindowControl * Sender,
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  int ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
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
#ifdef HAVE_HATCHED_BRUSH
      int p = dlgAirspacePatternsShowModal();
      if (p>=0) {
	MapWindow::iAirspaceBrush[ItemIndex] = p;
	changed = true;
      }
#endif
    } else {
      int v = (MapWindow::iAirspaceMode[ItemIndex]+1)%4;
      MapWindow::iAirspaceMode[ItemIndex] = v;
      //  wAirspaceList->Redraw();
      changed = true;
    }
  }
}


static void OnAirspaceListInfo(WndListFrame * Sender,
			       WndListFrame::ListInfo_t *ListInfo){

  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = AIRSPACECLASSCOUNT;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnLookupClicked(WndButton* pWnd) {
  dlgSelectAirspace();
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

  WndForm *wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_AIRSPACE_L : IDR_XML_AIRSPACE_P);
  if (!wf) return false;

  WndListFrame* wAirspaceList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceList"));
  LKASSERT(wAirspaceList!=NULL);
  wAirspaceList->SetBorderKind(BORDERLEFT);
  wAirspaceList->SetEnterCallback(OnAirspaceListEnter);

  WndOwnerDrawFrame* wAirspaceListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceListEntry"));
  if(wAirspaceListEntry) {
    wAirspaceListEntry->SetCanFocus(true);
  }

  UpdateList(wAirspaceList);

  changed = false;

  wf->ShowModal();

  delete wf;

  return changed;
}
