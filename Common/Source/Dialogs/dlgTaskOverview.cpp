/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTaskOverview.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "McReady.h"
#include "LKMapWindow.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "CTaskFileHelper.h"
#include "resource.h"
#include "utils/printf.h"
#include "Waypoints/SetHome.h"
#include "LocalPath.h"

#ifdef ANDROID
#include "Android/LK8000Activity.h"
#endif

extern void ResetTaskWaypoint(int j);

static bool showAdvanced= false;

static int UpLimit=0;
static int LowLimit=0;

static int ItemIndex = -1;

static int DrawListIndex=0;

static double lengthtotal = 0.0;
static bool fai_ok = false;

static void UpdateFilePointer(WndForm* pWnd) {
  WndProperty *wp = pWnd->FindByName<WndProperty>(TEXT("prpFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    if (_tcslen(LastTaskFileName)>0) {
      dfe->Lookup(LastTaskFileName);
    } else {
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }
}


static void UpdateCaption (WndForm* pWnd) {
  TCHAR title[MAX_PATH];
  TCHAR name[MAX_PATH] = TEXT("\0");
  TaskFileName(MAX_PATH, name);

  if (_tcslen(name)>0) {
  	// LKTOKEN  _@M688_ = "Task Overview"
    lk::snprintf(title, _T("%s: %s"), MsgToken<688>(), name);
  } else {
    _stprintf(title, TEXT("%s"),
	// LKTOKEN  _@M688_ = "Task Overview"
              MsgToken<688>());
  }

  if (TaskModified) {
    _tcscat(title, TEXT(" *"));
  }

  pWnd->SetCaption(title);
}

static void OnTaskPaintListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){
  TCHAR sTmp[120];
  TCHAR wpName[40];
  TCHAR landableStr[] = TEXT(" [X]");
  // LKTOKEN _@M1238_ "L"
  landableStr[2] = MsgToken<1238>()[0]; // TODO : to fix : only work if string[0] is usascii char...
  LockTaskData();

  const PixelRect rcClient(Sender->GetClientRect());
  
  const int w0 = rcClient.GetSize().cx - DLGSCALE(1);
  const int w1 = Surface.GetTextWidth(TEXT(" 000km"));
  _stprintf(sTmp, _T("  000%s"), MsgToken<2179>());
  const int w2 = Surface.GetTextWidth(sTmp);

  const int TextMargin = (rcClient.GetSize().cy - Surface.GetTextHeight(TEXT("A"))) / 2;

  const int p1 = w0-w1-w2- rcClient.GetSize().cy - DLGSCALE(2);
  const int p2 = w0-w2- rcClient.GetSize().cy - DLGSCALE(2);
  
  const PixelRect rc = {
      0, 
      0,
      rcClient.GetSize().cy, 
      rcClient.GetSize().cy
  };
  
  if (DrawListIndex < UpLimit){
    int i = LowLimit + DrawListIndex;
    MapWindow::DrawTaskPicto(Surface, DrawListIndex,  rc);
    if (Task[i].Index>=0) {
      _stprintf(wpName, TEXT("%s%s"),
                WayPointList[Task[i].Index].Name,
                (WayPointList[Task[i].Index].Flags & LANDPOINT) ? landableStr : TEXT(""));

      if (UseAATTarget() && ValidTaskPoint(i+1) && (i>0)) {
        if (Task[i].AATType == sector_type_t::CIRCLE || Task[i].AATType == sector_type_t::ESS_CIRCLE) {
          _stprintf(sTmp, TEXT("%.1f %s"),
                    Units::ToDistance(Task[i].AATCircleRadius), wpName);
        } else {
          _stprintf(sTmp, TEXT("%.1f %s"),
                    Units::ToDistance(Task[i].AATSectorRadius),wpName);
        }
      } else {
        if (i == 0) // start
          _stprintf(sTmp, TEXT("%.1f %s"), Units::ToDistance(StartRadius), wpName);
        else if (i == (UpLimit - 1)) //Finish
          _stprintf(sTmp, TEXT("%.1f %s"), Units::ToDistance(FinishRadius), wpName);
        else // turnpoint
          _stprintf(sTmp, TEXT("%.1f %s"), Units::ToDistance(SectorRadius), wpName);
      }

      Surface.SetBackgroundTransparent();
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawTextClip(rc.right + DLGSCALE(2), TextMargin, sTmp, p1-DLGSCALE(4));

      _stprintf(sTmp, TEXT("%.0f %s"),
                Units::ToDistance(Task[i].Leg),
                Units::GetDistanceName());
      Surface.DrawText(rc.right+p1+w1-Surface.GetTextWidth(sTmp), TextMargin, sTmp);

      _stprintf(sTmp, TEXT("%d%s"),  iround(Task[i].InBound),MsgToken<2179>());
      Surface.DrawText(rc.right +p2+w2-Surface.GetTextWidth(sTmp), TextMargin, sTmp);

    }

  } else {
    RefreshTaskStatistics();
    Surface.SetTextColor(RGB_BLACK);

    if (DrawListIndex == UpLimit) {
	    // LKTOKEN  _@M832_ = "add waypoint"
      _stprintf(sTmp, TEXT("  (%s)"), MsgToken<832>());
      Surface.DrawText(rc.right +DLGSCALE(2), TextMargin, sTmp);
    } else if ((DrawListIndex == (UpLimit + 1)) && ValidTaskPoint(1)) {

      if (gTaskType!=TSK_AAT) {
        // LKTOKEN  _@M735_ = "Total:"
        Surface.DrawText(rc.right +DLGSCALE(2), TextMargin, MsgToken<735>());
        _stprintf(sTmp, TEXT("%s %.0f %s"),  fai_ok?_T(" FAI"):_T(""), Units::ToDistance(lengthtotal), Units::GetDistanceName());
        Surface.DrawText(rc.right +p1+w1-Surface.GetTextWidth(sTmp), TextMargin, sTmp);
      }
      else
      {
        double d1 = CALCULATED_INFO.TaskDistanceToGo;
        if ((CALCULATED_INFO.TaskStartTime>0.0) && (CALCULATED_INFO.Flying) && (ActiveTaskPoint>0)) {
          d1 += CALCULATED_INFO.TaskDistanceCovered;
        }
        if (d1==0.0) {
          d1 = CALCULATED_INFO.AATTargetDistance;
        }
        _stprintf(sTmp, TEXT("%s %2i:%02ih %.0f (%.0f) %s"),
                  // LKTOKEN  _@M735_ = "Total:"
                  MsgToken<735>(),
                  (int)AATTaskLength/60,
                  (int)AATTaskLength%60,
                  Units::ToDistance(lengthtotal),
                  Units::ToDistance(d1),
                  Units::GetDistanceName());
        Surface.DrawText(rc.right +DLGSCALE(2), TextMargin,   sTmp);
      }
     }
     else if ((DrawListIndex == (UpLimit + 2)) && ValidTaskPoint(1)) {
          double dd = CALCULATED_INFO.TaskTimeToGo;
          if ( (CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying) /*&&(ActiveWayPoint>0)*/) { // patch 091126
            dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
          }
          dd= min(24.0*60.0,dd/60.0);
          int idd = (int) (dd+0.5);
          _stprintf(sTmp, TEXT("%s(%s=%3.1f%s): %i:%02ih "),MsgToken<247>(), MsgToken<1022>(), Units::ToVerticalSpeed(MACCREADY), Units::GetVerticalSpeedName(), idd/60, idd%60 );  //_@M247_ ETE
          Surface.DrawText(rc.right +DLGSCALE(2), TextMargin,   sTmp);
     }
  }
 
  UnlockTaskData();

}

static 
tstring GetHomeWaypointName() {
  ScopeLock lock(CritSec_TaskData);
  if (ValidWayPointFast(HomeWaypoint)) {
    return WayPointList[HomeWaypoint].Name;
  } else {
    return _T(" ? ");
  }
}

static 
void UpdateHomeWaypoint(WndProperty * pWnd) {
  if (pWnd) {
    DataField* df = pWnd->GetDataField();
    if (df) {
      df->Set(GetHomeWaypointName().c_str());
    }
    pWnd->RefreshDisplay();
  }
}

static
void OnSelectHomeWaypoint(WndProperty * pWnd) {
  SetNewHome(dlgSelectWaypoint());
  UpdateHomeWaypoint(pWnd);
}


static void OverviewRefreshTask(WndForm* pWnd) {
  LockTaskData();
  RefreshTask();

  // Only need to refresh info where the removal happened
  // as the order of other taskpoints hasn't changed
  lengthtotal = 0;
  for(UpLimit = 0; ValidTaskPointFast(UpLimit); ++UpLimit) {
    lengthtotal += Task[UpLimit].Leg;
  }

  fai_ok = CALCULATED_INFO.TaskFAI	;

  RefreshTaskStatistics();
#ifdef OLD_TIME_ESTIMATE
  WndProperty* wp;

  wp = wf->FindByName<WndProperty>(TEXT("prpAATEst"));
  if (wp) {
    double dd = CALCULATED_INFO.TaskTimeToGo;
//    if ((CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying)) { patchout 091126
    if ( (CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying) &&(ActiveTaskPoint>0)) { // patch 091126



      dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
    }
    dd= min(24.0*60.0,dd/60.0);
    wp->GetDataField()->SetAsFloat(dd);
    wp->RefreshDisplay();
  }
#endif
  LowLimit =0;

  UpdateHomeWaypoint(pWnd->FindByName<WndProperty>(_T("prpHome")));

  WndListFrame* wTaskList = pWnd->FindByName<WndListFrame>(TEXT("frmTaskList"));
  if(wTaskList) {
    int SelectedItem = wTaskList->GetItemIndex();
    wTaskList->ResetList();
    wTaskList->SetItemIndex(SelectedItem);
  }

  EnableMultipleStartPoints = (gTaskType != TSK_GP);

  WindowControl *wTimeGates = pWnd->FindByName(TEXT("cmdTimegates"));
  if (wTimeGates) {
    wTimeGates->SetVisible(gTaskType == TSK_GP);
  }

  WindowControl *wDelete = pWnd->FindByName(TEXT("cmdDelete"));
  if (wDelete) {
    wDelete->SetVisible(gTaskType != TSK_GP);
  }

  UpdateCaption(pWnd);
  UnlockTaskData();
}


static void OnTaskListEnter(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {

  WndForm* pForm = Sender->GetParentWndForm();
  bool isfinish = false;

  ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;

  // If we are clicking on Add Waypoint
  if (ItemIndex == UpLimit) {

	// add new waypoint
	if (CheckDeclaration()) {

		if (ItemIndex>0) {
#ifdef LAST_TASKPOINT_QUESTION
			if (MessageBoxX(
			// LKTOKEN  _@M817_ = "Will this be the finish?"
			MsgToken<817>(),
			// LKTOKEN  _@M54_ = "Add Waypoint"
			MsgToken<54>(),
			mbYesNo) == IdYes)
#else
		    if(0)
#endif
			{

				isfinish = true;

				// Set initial wp as the finish by default, or home if nonex
				LockTaskData();
                // ItemIndex is already checked for > 0 no need to test twice
				Task[ItemIndex].Index = Task[0].Index;
				UnlockTaskData();

			} else {
				isfinish = false;
			}
		}

		int res = dlgSelectWaypoint();
		if (ValidWayPoint(res)){

			LockTaskData();
			ResetTaskWaypoint(ItemIndex);
			Task[ItemIndex].Index = res;

			UnlockTaskData();
			if (ItemIndex==0) {
				dlgTaskWaypointShowModal(ItemIndex, 0, true); // start waypoint
			} else if (isfinish) {
				dlgTaskWaypointShowModal(ItemIndex, 2, true); // finish waypoint
			} else {
				if (UseAATTarget()) {
					// only need to set properties for finish
					dlgTaskWaypointShowModal(ItemIndex, 1, true); // normal waypoint
				}
			}

		} // ValidWaypoint
		OverviewRefreshTask(pForm);

	} // CheckDeclaration

	return;

  } // Index==UpLimit, clicking on Add Waypoint

  if (ItemIndex < UpLimit) {
    if (ItemIndex == 0) {
      dlgTaskWaypointShowModal(ItemIndex, 0); // start waypoint
    } else if (ItemIndex==(UpLimit-1)) {
      dlgTaskWaypointShowModal(ItemIndex, 2); // finish waypoint
    } else {
      dlgTaskWaypointShowModal(ItemIndex, 1); // turnpoint
    }
    OverviewRefreshTask(pForm);
  }

  if( ValidTaskPoint(1)) { // min 2 waypoints
    if (ItemIndex == (UpLimit + 1)) {
      pForm->SetVisible(false);
      dlgAnalysisShowModal(ANALYSIS_PAGE_TASK);
      pForm->SetVisible(true);
    }
  
    if (ItemIndex == (UpLimit + 2)) {
      pForm->SetVisible(false);
      dlgTaskCalculatorShowModal();
      OverviewRefreshTask(pForm);
      pForm->SetVisible(true);
    }
  }
} // OnTaskListEnter




static void OnTaskListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t* ListInfo){
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = UpLimit - LowLimit + 3;
  } else {
    DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(WndButton* pWnd) {
  ItemIndex = -1; // to stop FormDown bringing up task details
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      SaveDefaultTask(); // save  changed task
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnClearClicked(WndButton* pWnd){
  if (MessageBoxX(
	// LKTOKEN  _@M179_ = "Clear the task?"
                  MsgToken<179>(),
	// LKTOKEN  _@M178_ = "Clear task"
                  MsgToken<178>(),
                  mbYesNo) == IdYes) {
    if (CheckDeclaration()) {
      ClearTask();
      SetHome(true);  // force home reload

      WndForm* pForm = pWnd->GetParentWndForm();

      UpdateFilePointer(pForm);
      OverviewRefreshTask(pForm);
      UpdateCaption(pForm);
    }
  }
}


static void OnReverseClicked(WndButton* pWnd){


    if (MessageBoxX(
      MsgToken<1852>(), // LKTOKEN  _@M1852_ = "Reverse task?"
      MsgToken<1851>(), // LKTOKEN  _@M1851_ = "Reverse task"
      mbYesNo) == IdYes)
    {
      LockTaskData();
      ReverseTask();
      UnlockTaskData();
      OverviewRefreshTask(pWnd->GetParentWndForm());
    }
}

static void OnCalcClicked(WndButton* pWnd){
  WndForm* pForm = pWnd->GetParentWndForm();
  pForm->SetVisible(false);
  dlgTaskCalculatorShowModal();
  OverviewRefreshTask(pForm);
  pForm->SetVisible(true);
}


static void OnAnalysisClicked(WndButton* pWnd){
  WndForm* pForm = pWnd->GetParentWndForm();
  pForm->SetVisible(false);
  dlgAnalysisShowModal(ANALYSIS_PAGE_TASK);
  pForm->SetVisible(true);
}

static void OnTimegatesClicked(WndButton* pWnd){
  WndForm* pForm = pWnd->GetParentWndForm();
  pForm->SetVisible(false);
  dlgTimeGatesShowModal();
  pForm->SetVisible(true);
}

static void OnDeclareClicked(WndButton* pWnd){
  RefreshTask();

  LoggerDeviceDeclare();
  // do something here.
}




static void OnSaveClicked(WndButton* pWnd){
  TCHAR task_name[MAX_PATH];

  WndForm* pForm = pWnd->GetParentWndForm();
  WndProperty* wp = pForm->FindByName<WndProperty>(TEXT("prpFile"));
  if (!wp) return;
  DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
  int file_index = dfe->GetAsInteger();

  // TODO enhancement: suggest a good new name not already in the list
  lk::strcpy(task_name,TEXT("NEW"));
  dlgTextEntryShowModal(task_name, 10); // max length

  if (_tcslen(task_name)>0) {

	_tcscat(task_name, TEXT(LKS_TSK));

	dfe->Lookup(task_name);
	file_index = dfe->GetAsInteger();

	if (file_index==0) {
		// good, this file is unique..
		dfe->addFile(task_name, task_name);
		dfe->Lookup(task_name);
		wp->RefreshDisplay();
	}

  } else {
	// TODO code: report error, task not saved since no name was given
	return;
  }

  if (file_index>0) {
	// file already exists! ask if want to overwrite
        TCHAR sTmp[500];
	_sntprintf(sTmp, std::size(sTmp), TEXT("%s: '%s'"),
	// LKTOKEN  _@M696_ = "Task file already exists"
		MsgToken<696>(),
		dfe->GetAsString());

		if(MessageBoxX(
			sTmp,
			// LKTOKEN  _@M510_ = "Overwrite?"
			MsgToken<510>(),
			mbYesNo) != IdYes) {

			return;
		}
  }

  TCHAR file_name[MAX_PATH];
  LocalPath(file_name,TEXT(LKD_TASKS), task_name);

  SaveTask(file_name);
  UpdateCaption(pForm);
}



static void OnLoadClicked(WndButton* pWnd){ // 091216
  TCHAR file_name[MAX_PATH];
  WndForm* pForm = pWnd->GetParentWndForm();
  WndProperty* wp = pForm->FindByName<WndProperty>(TEXT("prpFile"));
  if (!wp) return;

  wp->OnLButtonDown((POINT){0,0});

  DataFieldFileReader* dfe = (DataFieldFileReader*) wp->GetDataField();

  int file_index = dfe->GetAsInteger();
  LPCTSTR szFileName = dfe->GetPathFile();

#ifdef ANDROID
  if (_tcscmp(szFileName, _T("QRCODE")) == 0) {

    bool load_task = true;
    if (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1)) {
      TCHAR msg[180];
      _sntprintf(msg,180, TEXT("%s %s ?"), MsgToken<891>(), MsgToken<907>()); // Clear old task and load task
      if (MessageBoxX(msg, _T(" "), mbYesNo) != IdYes) {
        load_task = false;
      }
    }

    if (load_task) {
      LK8000Activity* activity = LK8000Activity::Get();
      assert(activity);
      if(activity) {
        activity->ScanQRCode();
        return;
      }
    }
    return;
  }
#endif

  LPCTSTR wextension = _tcsrchr(szFileName, _T('.'));

  if (file_index>0) {
    if (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1) && (_tcsicmp(wextension,_T(LKS_WP_CUP))!=0)) {
      _stprintf(file_name, TEXT("%s '%s' ?"), MsgToken<891>(), dfe->GetAsString()); // Clear old task and load
      if(MessageBoxX(file_name, _T(" "), mbYesNo) == IdNo) {
        return;
      }
    }

    if(wextension) {
      TCHAR szFilePath[MAX_PATH];
      LocalPath(szFilePath, _T(LKD_TASKS), szFileName);

      bool bOK = false;
      if(_tcsicmp(wextension,_T(LKS_TSK))==0) {
        bOK = CTaskFileHelper().Load(szFilePath);
      }
      else if (_tcsicmp(wextension,_T(LKS_WP_CUP))==0) {
        bOK = LoadCupTask(szFilePath);
      } else if (_tcsicmp(wextension,_T(LKS_WP_GPX))==0) {
        bOK = LoadGpxTask(szFilePath);
      } else if (_tcsicmp(wextension,_T(LKS_XCTSK))==0) {
        bOK = LoadXctrackTask(szFilePath);
      }
      if(!bOK) {
        MessageBoxX(MsgToken<467>(),_T(" "), mbOk);
      }
      OverviewRefreshTask(pForm);
      UpdateFilePointer(pForm);
      UpdateCaption(pForm);
    }
  } else {
    // LKTOKEN  _@M467_ = "No Task to load"
    MessageBoxX(MsgToken<467>(),_T(" "), mbOk);
  }
}


static void OnDeleteClicked(WndButton* pWnd){

  TCHAR file_name[MAX_PATH];

  WndForm* pForm = pWnd->GetParentWndForm();
  WndProperty* wp = pForm->FindByName<WndProperty>(TEXT("prpFile"));
  if (!wp) return;
  wp->OnLButtonDown((POINT){0,0});

  DataFieldFileReader* dfe = (DataFieldFileReader*) wp->GetDataField();

  int file_index = dfe->GetAsInteger();
  if (file_index>0) {
	_stprintf(file_name, TEXT("%s '%s' ?"), MsgToken<1789>(), dfe->GetAsString()); // Delete task file?
	if(MessageBoxX(file_name, _T(" "), mbYesNo) == IdNo) {
		return;
	}
  } else {
	MessageBoxX(MsgToken<1790>(),_T(" "), mbOk); // No task file to delete
	return;
  }

  if (file_index>0) {

    TCHAR file_name[MAX_PATH];
    LocalPath(file_name,TEXT(LKD_TASKS), dfe->GetPathFile());

    lk::filesystem::deleteFile(file_name);
    // Cannot update dfe list, so we force exit.
    ItemIndex = -1;


    pForm->SetModalResult(mrOK);
  }
}


static bool OnUser(WndForm * pWndForm, unsigned id) {
  switch (id) {
    case UM_UPDATE_TASK_OVERVIEW:
      OverviewRefreshTask(pWndForm);
      UpdateFilePointer(pWndForm);
      UpdateCaption(pWndForm);
      return true;
  }
  return false;
}

static CallBackTableEntry_t CallBackTable[]={
  OnPaintCallbackEntry(OnTaskPaintListItem),
  OnListCallbackEntry(OnTaskListInfo),
  ClickNotifyCallbackEntry(OnDeclareClicked),
  ClickNotifyCallbackEntry(OnCalcClicked),
  ClickNotifyCallbackEntry(OnClearClicked),
  ClickNotifyCallbackEntry(OnReverseClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnSaveClicked),
  ClickNotifyCallbackEntry(OnLoadClicked),
  ClickNotifyCallbackEntry(OnDeleteClicked),
  ClickNotifyCallbackEntry(OnAnalysisClicked),
  ClickNotifyCallbackEntry(OnTimegatesClicked),
  OnHelpCallbackEntry(OnSelectHomeWaypoint),
  EndCallBackEntry()
};




void dlgTaskOverviewShowModal(int Idx){

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = Idx; //-1;

  showAdvanced = false;
  
  if(MACCREADY < 0.1) {
    CheckSetMACCREADY(GlidePolar::SafetyMacCready, nullptr);
  }
    
  WndForm* wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_TASKOVERVIEW_L : IDR_XML_TASKOVERVIEW_P);

  if (!wf) return;

  wf->SetOnUser(OnUser);

  UpdateCaption(wf);

  WndListFrame* wTaskList = wf->FindByName<WndListFrame>(TEXT("frmTaskList"));
  wTaskList->SetBorderKind(BORDERLEFT);
  wTaskList->SetEnterCallback(OnTaskListEnter);

  WndOwnerDrawFrame* wTaskListEntry = wf->FindByName<WndOwnerDrawFrame>(TEXT("frmTaskListEntry"));
  wTaskListEntry->SetCanFocus(true);

  WndProperty* wp = wf->FindByName<WndProperty>(TEXT("prpFile"));
  if (wp) {
    wp->SetVisible(false);
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      const TCHAR* suffix_filters[] = {
        _T(LKS_TSK),
        _T(LKS_WP_CUP),
        _T(LKS_WP_GPX),
        _T(LKS_XCTSK)
      };
      dfe->Clear();
#ifdef ANDROID      
      dfe->addFile(_T("< Scan QRCode >"), _T("QRCODE"));
#endif
      dfe->ScanDirectoryTop(_T(LKD_TASKS), suffix_filters, dfe->GetNumFiles());
    }
    wp->RefreshDisplay();
  }
  UpdateFilePointer(wf);

  // initialise and turn on the display
  OverviewRefreshTask(wf);

  wTaskList->SetItemIndexPos(Idx);
  wTaskList->Redraw();
  wTaskListEntry->SetFocus();

  wf->ShowModal();

  // now retrieve back the properties...

  RefreshTask();

  delete wf;
}
