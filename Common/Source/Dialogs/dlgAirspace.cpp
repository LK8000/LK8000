/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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

int dlgAirspaceColoursShowModal(void);


static void UpdateList(WndListFrame* pWnd){
  if(pWnd) {
    pWnd->ResetList();
    pWnd->Redraw();
  }
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

    Surface.SetTextColor(RGB_BLACK);
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
        Surface.DrawText(w0-w1-w2, 2*ScreenScale, label);
      }
      if (isdisplay) {
	// LKTOKEN  _@M241_ = "Display" 
        _tcscpy(label, gettext(TEXT("_@M241_")));
        Surface.DrawText(w0-w2, 2*ScreenScale, label);
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


static void OnAirspaceListInfo(WindowControl * Sender, 
			       WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
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


static void OnLookupClicked(WndButton* pWnd){
  (void)pWnd;
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

