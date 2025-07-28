/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStartPoint.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"

static WndForm *wf=NULL;
static WndListFrame *wStartPointList=NULL;
static WndOwnerDrawFrame *wStartPointListEntry = NULL;

static int ItemIndex = -1;

static void UpdateList(void){
  wStartPointList->ResetList();
  wStartPointList->Redraw();
}

static int DrawListIndex=0;

static void OnStartPointPaintListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){

  TCHAR label[MAX_PATH];

  if (DrawListIndex < MAXSTARTPOINTS){
    int i = DrawListIndex;

    if ((StartPoints[i].Index != -1)&&(StartPoints[i].Active)) {
      lk::strcpy(label, WayPointList[StartPoints[i].Index].Name);
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
        lk::strcpy(label, TEXT("(add waypoint)"));
      } else {
        lk::strcpy(label, TEXT(" "));
      }
    }
    Surface.SetTextColor(RGB_BLACK);
    Surface.DrawText(DLGSCALE(2), DLGSCALE(2), label);
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
    int res = dlgSelectWaypoint();
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


static void OnStartPointListInfo(WndListFrame * Sender,
			       WndListFrame::ListInfo_t *ListInfo){

  if (ListInfo->DrawIndex == -1){
      ListInfo->ItemCount = 0;
      while(ValidStartPoint(ListInfo->ItemCount++)) { }
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnClearClicked(WndButton* pWnd){
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
  CallbackEntry(OnStartPointPaintListItem),
  CallbackEntry(OnStartPointListInfo),
  CallbackEntry(OnCloseClicked),
  CallbackEntry(OnClearClicked),
  EndCallbackEntry()
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

   wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_STARTPOINT_L : IDR_XML_STARTPOINT_P);
  if (!wf) return;

  //ASSERT(wf!=NULL);

  CheckStartPointInTask();

  wStartPointList = wf->FindByName<WndListFrame>(TEXT("frmStartPointList"));
  //ASSERT(wStartPointList!=NULL);
  wStartPointList->SetBorderKind(BORDERLEFT);
  wStartPointList->SetEnterCallback(OnStartPointListEnter);

  wStartPointListEntry = wf->FindByName<WndOwnerDrawFrame>(TEXT("frmStartPointListEntry"));

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
