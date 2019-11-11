/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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

extern void ResetTaskWaypoint(int j);

static WndForm *wf=NULL;
static WndFrame *wfAdvanced=NULL;
static WndListFrame *wTaskList=NULL;
static WndOwnerDrawFrame *wTaskListEntry = NULL;
static bool showAdvanced= false;

static int UpLimit=0;
static int LowLimit=0;

static int ItemIndex = -1;

static int DrawListIndex=0;

static double lengthtotal = 0.0;
static bool fai_ok = false;

static void UpdateFilePointer(void) {
  WndProperty *wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    if (_tcslen(LastTaskFileName)>0) {
      dfe->Lookup(LastTaskFileName);
    } else {
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }
}


static void UpdateCaption (void) {
  TCHAR title[MAX_PATH];
  TCHAR name[MAX_PATH] = TEXT("\0");
  TaskFileName(MAX_PATH, name);

  if (_tcslen(name)>0) {
    _stprintf(title, TEXT("%s: %s"),
	// LKTOKEN  _@M688_ = "Task Overview"
              MsgToken(688),
              name);
  } else {
    _stprintf(title, TEXT("%s"),
	// LKTOKEN  _@M688_ = "Task Overview"
              MsgToken(688));
  }

  if (TaskModified) {
    _tcscat(title, TEXT(" *"));
  }

  wf->SetCaption(title);
}

static void OnTaskPaintListItem(WindowControl * Sender, LKSurface& Surface){

  int n = UpLimit - LowLimit;
  TCHAR sTmp[120];
  TCHAR wpName[120];
  TCHAR landableStr[5] = TEXT(" [X]");
  // LKTOKEN _@M1238_ "L"
  landableStr[2] = MsgToken(1238)[0];
  LockTaskData();

  const PixelRect rcClient(Sender->GetClientRect());
  int w0 = rcClient.GetSize().cx - DLGSCALE(1);
  if(wTaskList) {
    w0 = wTaskList->GetClientWidth()- DLGSCALE(2);}

  const int w1 = Surface.GetTextWidth(TEXT(" 000km"));
  _stprintf(sTmp, _T("  000%s"), MsgToken(2179));
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
  
  if (DrawListIndex < (n-3)){
    int i = LowLimit + DrawListIndex;
//    if ((WayPointList[Task[i].Index].Flags & LANDPOINT) >0)
//      MapWindow::DrawRunway(hDC,  &WayPointList[Task[i].Index],  rc, 3000,true);
    MapWindow::DrawTaskPicto(Surface, DrawListIndex,  rc, 2500);
    if (Task[i].Index>=0) {
      _stprintf(wpName, TEXT("%s%s"),
                WayPointList[Task[i].Index].Name,
                (WayPointList[Task[i].Index].Flags & LANDPOINT) ? landableStr : TEXT(""));

      if (AATEnabled &&  ValidTaskPoint(i+1) && (i>0)) {
        if (Task[i].AATType==0 || Task[i].AATType==3) {
          _stprintf(sTmp, TEXT("%.1f %s"),
                    Task[i].AATCircleRadius*DISTANCEMODIFY, wpName);
        } else {
          if(Task[i].AATType==2 && DoOptimizeRoute()) {
             _stprintf(sTmp, TEXT("%.1f/1 %s"),
                    Task[i].PGConeSlope,wpName);
          } else {
             _stprintf(sTmp, TEXT("%.1f %s"),
                    Task[i].AATSectorRadius*DISTANCEMODIFY,wpName);
          }
        }
      } else {
        if (i == 0)
          _stprintf(sTmp, TEXT("%.1f %s"), StartRadius * DISTANCEMODIFY, wpName);
        else if (i < (n - 4))
          _stprintf(sTmp, TEXT("%.1f %s"), SectorRadius * DISTANCEMODIFY, wpName);
        else
          _stprintf(sTmp, TEXT("%.1f %s"), FinishRadius * DISTANCEMODIFY, wpName);
      }

      Surface.SetBackgroundTransparent();
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawTextClip(rc.right + DLGSCALE(2), TextMargin, sTmp, p1-DLGSCALE(4));

      _stprintf(sTmp, TEXT("%.0f %s"),Task[i].Leg*DISTANCEMODIFY,Units::GetDistanceName());
      Surface.DrawText(rc.right+p1+w1-Surface.GetTextWidth(sTmp), TextMargin, sTmp);

      _stprintf(sTmp, TEXT("%d%s"),  iround(Task[i].InBound),MsgToken(2179));
      Surface.DrawText(rc.right +p2+w2-Surface.GetTextWidth(sTmp), TextMargin, sTmp);

    }

  } else {
    RefreshTaskStatistics();
    Surface.SetTextColor(RGB_BLACK);

     // if (DrawListIndex==n) { // patchout 091126
     if (DrawListIndex==(n-3) && UpLimit < MAXTASKPOINTS) { // patch 091126

	// LKTOKEN  _@M832_ = "add waypoint"
      _stprintf(sTmp, TEXT("  (%s)"), MsgToken(832));
      Surface.DrawText(rc.right +DLGSCALE(2), TextMargin, sTmp);
    } else
      if ((DrawListIndex==(n-2)) && ValidTaskPoint(1)) {

	int Minutes=  (int)(CALCULATED_INFO.TaskTimeToGo )/60;
	if(MACCREADY ==0)
	  Minutes = 24*60;
	int Hours = (int)(Minutes/60);
	Minutes = Minutes - (Hours*60);

      if (!AATEnabled || ISPARAGLIDER) {
        // LKTOKEN  _@M735_ = "Total:"
        Surface.DrawText(rc.right +DLGSCALE(2), TextMargin, MsgToken(735));
	_stprintf(sTmp, TEXT("%s %.0f %s"),  fai_ok?_T(" FAI"):_T(""),lengthtotal*DISTANCEMODIFY, Units::GetDistanceName());
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
		    MsgToken(735),
		    (int)AATTaskLength/60,
		    (int)AATTaskLength%60,
		    DISTANCEMODIFY*lengthtotal,
		    DISTANCEMODIFY*d1,
		    Units::GetDistanceName());
	  Surface.DrawText(rc.right +DLGSCALE(2), TextMargin,   sTmp);
      }
     }
     else
     {
       if ((DrawListIndex==(n-1)) && ValidTaskPoint(1))
       {
	 double dd = CALCULATED_INFO.TaskTimeToGo;
	 if ( (CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying) /*&&(ActiveWayPoint>0)*/) { // patch 091126
	   dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
	 }
	 dd= min(24.0*60.0,dd/60.0);
	 int idd = (int) (dd+0.5);
	 _stprintf(sTmp, TEXT("%s(%s=%3.1f%s): %i:%02ih "),MsgToken(247),  MsgToken(1022)  ,  MACCREADY*LIFTMODIFY,Units::GetVerticalSpeedName(), idd/60, idd%60 );  //_@M247_ ETE
	 Surface.DrawText(rc.right +DLGSCALE(2), TextMargin,   sTmp);
       }
     }
  }
 
  UnlockTaskData();

}


static void OverviewRefreshTask(void) {
  LockTaskData();
  RefreshTask();

  int i;
  // Only need to refresh info where the removal happened
  // as the order of other taskpoints hasn't changed
  UpLimit = 0;
  lengthtotal = 0;
  for (i=0; i<MAXTASKPOINTS; i++) {
    if (Task[i].Index != -1) {
      lengthtotal += Task[i].Leg;
    
      UpLimit = i+1;
    }
  }
  UpLimit +=3;

  fai_ok = CALCULATED_INFO.TaskFAI	;

  RefreshTaskStatistics();
#ifdef OLD_TIME_ESTIMATE
  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATEst"));
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
  int SelectedIndex = wTaskList->GetItemIndex();
  LowLimit =0;
  wTaskList->ResetList();
  wTaskList->SetItemIndex(SelectedIndex);

  UpdateCaption();
  UnlockTaskData();

}



static void UpdateAdvanced(void) {
  if (wfAdvanced) {
    wfAdvanced->SetVisible(showAdvanced);
  }
}


static void OnTaskListEnter(WindowControl * Sender,
		     WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  bool isfinish = false;

  ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;

  // If we are clicking on Add Waypoint
  if(UpLimit>=3)
  if ((ItemIndex>=0) && (ItemIndex == (UpLimit-3)) && ((UpLimit-3)<MAXTASKPOINTS)) {

	// add new waypoint
	if (CheckDeclaration()) {

		if (ItemIndex>0) {
#ifdef LAST_TASKPOINT_QUESTION
			if (MessageBoxX(
			// LKTOKEN  _@M817_ = "Will this be the finish?"
			MsgToken(817),
			// LKTOKEN  _@M54_ = "Add Waypoint"
			MsgToken(54),
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

		int res;
		res = dlgWayPointSelect();

		if (ValidWayPoint(res)){

			LockTaskData();
            ResetTaskWaypoint(ItemIndex);
			Task[ItemIndex].Index = res;
            Task[ItemIndex].PGConeBase = WayPointList[res].Altitude;

			UnlockTaskData();
			if (ItemIndex==0) {
				dlgTaskWaypointShowModal(ItemIndex, 0, true); // start waypoint
			} else if (isfinish) {
				dlgTaskWaypointShowModal(ItemIndex, 2, true); // finish waypoint
			} else {
				if (AATEnabled || DoOptimizeRoute()) {
					// only need to set properties for finish
					dlgTaskWaypointShowModal(ItemIndex, 1, true); // normal waypoint
				}
			}

		} // ValidWaypoint
		OverviewRefreshTask();

	} // CheckDeclaration

	return;

  } // Index==UpLimit, clicking on Add Waypoint
if(UpLimit > 3)
  if (ItemIndex<(UpLimit-3)) {
    if (ItemIndex==0) {
//	StartupStore(_T(". %i %i // start waypoint%s"), ItemIndex, UpLimit,NEWLINE);
      dlgTaskWaypointShowModal(ItemIndex, 0); // start waypoint
    } else {
      if (ItemIndex==(UpLimit-4)) {
//	  StartupStore(_T(". %i %i // finish waypoint%s"), ItemIndex, UpLimit,NEWLINE);
	dlgTaskWaypointShowModal(ItemIndex, 2); // finish waypoint
      } else {
//	  StartupStore(_T(". %i %i // normal waypoint%s"), ItemIndex, UpLimit,NEWLINE);
	dlgTaskWaypointShowModal(ItemIndex, 1); // turnpoint
      }
    }
    OverviewRefreshTask();
  }
 if( ValidTaskPoint(1)) // min 2 waypoints
 {
  if (ItemIndex==(UpLimit-2))
  {
    wf->SetVisible(false);
    dlgAnalysisShowModal(ANALYSIS_PAGE_TASK);
    wf->SetVisible(true);
  }
  
  if (ItemIndex==(UpLimit-1))
  {
    wf->SetVisible(false);
    dlgTaskCalculatorShowModal();
    OverviewRefreshTask();
    wf->SetVisible(true);
  }
 }
} // OnTaskListEnter




static void OnTaskListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
(void)Sender;

  if (ListInfo->DrawIndex == -1) {
	ListInfo->ItemCount = UpLimit-LowLimit+2;
  } else {
	DrawListIndex = ListInfo->DrawIndex +ListInfo->ScrollIndex;
	ItemIndex = ListInfo->ItemIndex +ListInfo->ScrollIndex;
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
                  MsgToken(179),
	// LKTOKEN  _@M178_ = "Clear task"
                  MsgToken(178),
                  mbYesNo) == IdYes) {
    if (CheckDeclaration()) {
      ClearTask();
      UpdateFilePointer();
      OverviewRefreshTask();
      UpdateCaption();
    }
  }
}


static void OnReverseClicked(WndButton* pWnd){


    if (MessageBoxX(
      MsgToken(1852), // LKTOKEN  _@M1852_ = "Reverse task?"
      MsgToken(1851), // LKTOKEN  _@M1851_ = "Reverse task"
      mbYesNo) == IdYes)
    {
      LockTaskData();
      ReverseTask();
      UnlockTaskData();
      OverviewRefreshTask();
    }
}

static void OnCalcClicked(WndButton* pWnd){
  wf->SetVisible(false);
  dlgTaskCalculatorShowModal();
  OverviewRefreshTask();
  wf->SetVisible(true);
}


static void OnAnalysisClicked(WndButton* pWnd){
  wf->SetVisible(false);
  dlgAnalysisShowModal(ANALYSIS_PAGE_TASK);
  wf->SetVisible(true);
}

static void OnTimegatesClicked(WndButton* pWnd){
  wf->SetVisible(false);
  dlgTimeGatesShowModal();
  wf->SetVisible(true);
}

static void OnDeclareClicked(WndButton* pWnd){
  RefreshTask();

  LoggerDeviceDeclare();
  // do something here.
}




static void OnSaveClicked(WndButton* pWnd){

  int file_index;
  TCHAR task_name[MAX_PATH];
  WndProperty* wp;
  DataFieldFileReader *dfe;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;
  dfe = (DataFieldFileReader*)wp->GetDataField();

  file_index = dfe->GetAsInteger();

  // TODO enhancement: suggest a good new name not already in the list
  _tcscpy(task_name,TEXT("NEW"));
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
	_sntprintf(sTmp, array_size(sTmp), TEXT("%s: '%s'"),
	// LKTOKEN  _@M696_ = "Task file already exists"
		MsgToken(696),
		dfe->GetAsString());

		if(MessageBoxX(
			sTmp,
			// LKTOKEN  _@M510_ = "Overwrite?"
			MsgToken(510),
			mbYesNo) != IdYes) {

			return;
		}
  }

  TCHAR file_name[MAX_PATH];
  LocalPath(file_name,TEXT(LKD_TASKS), task_name);

  SaveTask(file_name);
  UpdateCaption();
}



static void OnLoadClicked(WndButton* pWnd){ // 091216
  TCHAR file_name[MAX_PATH];

  WndProperty* wp;
  DataFieldFileReader *dfe;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;

  wp->OnLButtonDown((POINT){0,0});

  dfe = (DataFieldFileReader*) wp->GetDataField();

  int file_index = dfe->GetAsInteger();
   LPCTSTR szFileName = dfe->GetPathFile();
   LPCTSTR wextension = _tcsrchr(szFileName, _T('.'));
    
  if (file_index>0) {
	if (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1) &&   (_tcsicmp(wextension,_T(LKS_WP_CUP))!=0)) {
		_stprintf(file_name, TEXT("%s '%s' ?"), MsgToken(891), dfe->GetAsString()); // Clear old task and load
		if(MessageBoxX(file_name, _T(" "), mbYesNo) == IdNo) {
			return;
		}
	}


      if(wextension) {

          TCHAR szFilePath[MAX_PATH];
          LocalPath(szFilePath, _T(LKD_TASKS), szFileName);

          bool bOK = false;
          if(_tcsicmp(wextension,_T(LKS_TSK))==0) {
              CTaskFileHelper helper;
              bOK = helper.Load(szFilePath);
          }
          else if (_tcsicmp(wextension,_T(LKS_WP_CUP))==0) {
              bOK = LoadCupTask(szFilePath);
          } else if (_tcsicmp(wextension,_T(LKS_WP_GPX))==0) {
              bOK = LoadGpxTask(szFilePath);
          }
          if(!bOK) {
              MessageBoxX(MsgToken(467),_T(" "), mbOk);
              return;
          }
          OverviewRefreshTask();
          UpdateFilePointer();
          UpdateCaption();
      }
  } else {
  	// LKTOKEN  _@M467_ = "No Task to load"
  	MessageBoxX(MsgToken(467),_T(" "), mbOk);
  	return;
    }

}


static void OnDeleteClicked(WndButton* pWnd){

  TCHAR file_name[MAX_PATH];

  WndProperty* wp;
  DataFieldFileReader *dfe;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;

  wp->OnLButtonDown((POINT){0,0});

  dfe = (DataFieldFileReader*) wp->GetDataField();

  int file_index = dfe->GetAsInteger();
  if (file_index>0) {
	_stprintf(file_name, TEXT("%s '%s' ?"), MsgToken(1789), dfe->GetAsString()); // Delete task file?
	if(MessageBoxX(file_name, _T(" "), mbYesNo) == IdNo) {
		return;
	}
  } else {
	MessageBoxX(MsgToken(1790),_T(" "), mbOk); // No task file to delete
	return;
  }

  if (file_index>0) {

    TCHAR file_name[MAX_PATH];
    LocalPath(file_name,TEXT(LKD_TASKS), dfe->GetPathFile());

    lk::filesystem::deleteFile(file_name);
    // Cannot update dfe list, so we force exit.
    ItemIndex = -1;
    if(pWnd) {
      WndForm * pForm = pWnd->GetParentWndForm();
      if(pForm) {
        pForm->SetModalResult(mrOK);
      }
    }
    return;
  }
}



static void OnAdvancedClicked(WndButton* Sender){
  showAdvanced = !showAdvanced;
  UpdateAdvanced();
}

static CallBackTableEntry_t CallBackTable[]={
  OnPaintCallbackEntry(OnTaskPaintListItem),
  OnListCallbackEntry(OnTaskListInfo),
  ClickNotifyCallbackEntry(OnDeclareClicked),
  ClickNotifyCallbackEntry(OnCalcClicked),
  ClickNotifyCallbackEntry(OnClearClicked),
  ClickNotifyCallbackEntry(OnAdvancedClicked),
  ClickNotifyCallbackEntry(OnReverseClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnAdvancedClicked),
  ClickNotifyCallbackEntry(OnSaveClicked),
  ClickNotifyCallbackEntry(OnLoadClicked),
  ClickNotifyCallbackEntry(OnDeleteClicked),
  ClickNotifyCallbackEntry(OnAnalysisClicked),
  ClickNotifyCallbackEntry(OnTimegatesClicked),
  EndCallBackEntry()
};




void dlgTaskOverviewShowModal(int Idx){

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = Idx; //-1;

  showAdvanced = false;
  
  if(MACCREADY < 0.1)
    MACCREADY = GlidePolar::SafetyMacCready;
    
  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_TASKOVERVIEW_L : IDR_XML_TASKOVERVIEW_P);

  if (!wf) return;

  WndButton *wb = (WndButton*)wf->FindByName(TEXT("cmdTimegates"));
  if (wb) wb->SetVisible(false);

  if (ISPARAGLIDER) {
	if (PGOptimizeRoute) AATEnabled=true; // force it on
        EnableMultipleStartPoints=false;
        if (wb) wb->SetVisible(true);
	wb = (WndButton*)wf->FindByName(TEXT("cmdDelete"));
	if (wb) wb->SetVisible(false);
  }

  UpdateCaption();

  wfAdvanced = ((WndFrame *)wf->FindByName(TEXT("frmAdvanced")));

  wTaskList = (WndListFrame*)wf->FindByName(TEXT("frmTaskList"));
  wTaskList->SetBorderKind(BORDERLEFT);
  wTaskList->SetEnterCallback(OnTaskListEnter);

  wTaskListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmTaskListEntry"));
  wTaskListEntry->SetCanFocus(true);

  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (wp) {
    wp->SetVisible(false);
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_TASKS), _T("*" LKS_TSK));
      dfe->ScanDirectoryTop(_T(LKD_TASKS), _T("*" LKS_WP_CUP));
      dfe->ScanDirectoryTop(_T(LKD_TASKS), _T("*" LKS_WP_GPX));
    }
    wp->RefreshDisplay();
  }
  UpdateFilePointer();

  // initialise and turn on the display
  OverviewRefreshTask();

  UpdateAdvanced();



  wTaskList->SetItemIndexPos(Idx);
  wTaskList->Redraw();
  wTaskListEntry->SetFocus();

  wf->ShowModal();

  // now retrieve back the properties...

  RefreshTask();

  delete wf;

  wf = NULL;

}
