/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStartPoint.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>
#include "Dialogs.h"

#include "InfoBoxLayout.h"


static WndForm *wf=NULL;
static WndListFrame *wStartPointList=NULL;
static WndOwnerDrawFrame *wStartPointListEntry = NULL;

static int ItemIndex = -1;

static void UpdateList(void){
  wStartPointList->ResetList();
  wStartPointList->Redraw();
}

static int DrawListIndex=0;

static void OnStartPointPaintListItem(WindowControl * Sender, HDC hDC){
	(void)Sender;

  TCHAR label[MAX_PATH];

  if (DrawListIndex < MAXSTARTPOINTS){
    int i = DrawListIndex;

    if ((StartPoints[i].Index != -1)&&(StartPoints[i].Active)) {
      _tcscpy(label, WayPointList[StartPoints[i].Index].Name);
    } else {
      int j;
      int i0=0;
      for (j=MAXSTARTPOINTS-1; j>=0; j--) {
        if ((StartPoints[j].Index!= -1)&&(StartPoints[j].Active)) {
          i0=j+1;
          break;
        }
      }
      if (i==i0) {
        _tcscpy(label, TEXT("(add waypoint)"));
      } else {
        _tcscpy(label, TEXT(" "));
      }
    }
    ExtTextOut(hDC,
	       2*ScreenScale,
	       2*ScreenScale,
	       ETO_OPAQUE, NULL,
	       label,
	       _tcslen(label),
	       NULL);
  }
}


static bool changed = false;

static void OnStartPointListEnter(WindowControl * Sender, 
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=MAXSTARTPOINTS) {
    ItemIndex = -1;
    while(ValidStartPoint(ItemIndex++)) { }
  }
  if (ItemIndex>=0) {
    int res;
    res = dlgWayPointSelect();
    if (res>=0) {
      // TODO bug: don't add it if it's already present!
      LockTaskData();
      StartPoints[ItemIndex].Index = res;
      StartPoints[ItemIndex].Active = true;
      UnlockTaskData();
      changed = true;
      UpdateList();
    }
  }
}


static void OnStartPointListInfo(WindowControl * Sender, 
			       WndListFrame::ListInfo_t *ListInfo){
	(void)Sender;
  if (ListInfo->DrawIndex == -1){
      ListInfo->ItemCount = 0;
      while(ValidStartPoint(ListInfo->ItemCount++)) { }
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static void OnClearClicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  for (int i=0; i<MAXSTARTPOINTS; i++) {
    StartPoints[i].Index = -1;
    StartPoints[i].Active = false;
  }
  StartPoints[0].Index = Task[0].Index;
  StartPoints[0].Active = true;
  changed = true;
  UnlockTaskData();
  UpdateList();
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnStartPointPaintListItem),
  DeclareCallBackEntry(OnStartPointListInfo),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnClearClicked),
  DeclareCallBackEntry(NULL)
};


static void CheckStartPointInTask(void) {
  LockTaskData();
  if (Task[0].Index != -1) {
    // ensure current start point is in task
    int index_last = 0;
    for (int i=MAXSTARTPOINTS-1; i>=0; i--) {
      if (StartPoints[i].Index == Task[0].Index) {
	index_last = -1;
	break;
      }
      if ((StartPoints[i].Index>=0) && (index_last==0)) {
	index_last = i;
      }
    }
    if (index_last>=0) {
      if (StartPoints[index_last].Index>= 0) {
	index_last = min(MAXSTARTPOINTS-1,index_last+1);
      }
      // it wasn't, so make sure it's added now
      StartPoints[index_last].Index = Task[0].Index;
      StartPoints[index_last].Active = true;
    }
  }
  UnlockTaskData();
}


void dlgStartPointShowModal(void) {

  ItemIndex = -1;

  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgStartPoint_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename,
                        hWndMainWindow,
                        TEXT("IDR_XML_STARTPOINT_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgStartPoint.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_STARTPOINT"));
  }
  if (!wf) return;

  //ASSERT(wf!=NULL);
  
  CheckStartPointInTask();

  wStartPointList = (WndListFrame*)wf->FindByName(TEXT("frmStartPointList"));
  //ASSERT(wStartPointList!=NULL);
  wStartPointList->SetBorderKind(BORDERLEFT);
  wStartPointList->SetEnterCallback(OnStartPointListEnter);
  wStartPointList->SetWidth(wf->GetWidth() - wStartPointList->GetLeft()-2);

  wStartPointListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmStartPointListEntry"));
  if ( wStartPointList->ScrollbarWidth == -1) {
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wStartPointList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);
  }
  wStartPointListEntry->SetWidth(wStartPointList->GetWidth() - wStartPointList->ScrollbarWidth - 5);

  //ASSERT(wStartPointListEntry!=NULL);
  wStartPointListEntry->SetCanFocus(true);



  UpdateList();

  changed = false;

  wf->ShowModal();

  // now retrieve back the properties...
  if (changed) {
    LockTaskData();
    TaskModified = true;
    RefreshTask();
    UnlockTaskData();
  };

  delete wf;

  wf = NULL;

}

