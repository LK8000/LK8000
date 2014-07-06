/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTaskOverview.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "McReady.h"
#include "InfoBoxLayout.h"
#include "LKMapWindow.h"
#include "Dialogs.h"
#include "CTaskFileHelper.h"

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
              gettext(TEXT("_@M688_")),
              name);
  } else {
    _stprintf(title, TEXT("%s"),
	// LKTOKEN  _@M688_ = "Task Overview" 
              gettext(TEXT("_@M688_")));
  }

  if (TaskModified) {
    _tcscat(title, TEXT(" *"));
  } 

  wf->SetCaption(title);
}

static void OnTaskPaintListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
  int n = UpLimit - LowLimit;
  TCHAR sTmp[120];
  TCHAR wpName[120];
  TCHAR landableStr[5] = TEXT(" [X]");
  // LKTOKEN _@M1238_ "L"
  landableStr[2] = gettext(TEXT("_@M1238_"))[0];
  LockTaskData();

  int w0 = Sender->GetWidth()-1;
  int w1 = GetTextWidth(hDC, TEXT(" 000km"));
  _stprintf(sTmp, _T("  000%s"), gettext(_T("_@M2179_")));
  int w2 = GetTextWidth(hDC, sTmp);
  
  int TextMargin = (Sender->GetHeight() - GetTextHeight(hDC, TEXT("A"))) / 2;

  int p1 = w0-w1-w2- Sender->GetHeight()-2;
  int p2 = w0-w2- Sender->GetHeight()-2;
  RECT rc = {0*ScreenScale,  0*ScreenScale, Sender->GetHeight(), Sender->GetHeight()};
  if (DrawListIndex < n){
    int i = LowLimit + DrawListIndex;
//    if ((WayPointList[Task[i].Index].Flags & LANDPOINT) >0)
//      MapWindow::DrawRunway(hDC,  &WayPointList[Task[i].Index],  rc, 3000,true);
    MapWindow::DrawTaskPicto(hDC, DrawListIndex,  rc, 2500);
    if (Task[i].Index>=0) {
      _stprintf(wpName, TEXT("%s%s"),
                WayPointList[Task[i].Index].Name,
                (WayPointList[Task[i].Index].Flags & LANDPOINT) ? landableStr : TEXT(""));
      
      if (AATEnabled && ValidTaskPoint(i+1) && (i>0)) {
        if (Task[i].AATType==0) {
          _stprintf(sTmp, TEXT("%s %.1f"), 
                    wpName, Task[i].AATCircleRadius*DISTANCEMODIFY);
        } else {
          if(Task[i].AATType==2 && DoOptimizeRoute()) {
             _stprintf(sTmp, TEXT("%s %.1f/1"), 
                    wpName, Task[i].PGConeSlope);
          } else {
             _stprintf(sTmp, TEXT("%s %.1f"), 
                    wpName, Task[i].AATSectorRadius*DISTANCEMODIFY);
          }
        }
      } else {
        _stprintf(sTmp, TEXT("%s"), wpName);
      }

      ExtTextOutClip(hDC, Sender->GetHeight()+2*ScreenScale, TextMargin,
		     sTmp, p1-4*ScreenScale);

      _stprintf(sTmp, TEXT("%.0f %s"), 
		Task[i].Leg*DISTANCEMODIFY,
		Units::GetDistanceName());
      ExtTextOut(hDC, Sender->GetHeight()+p1+w1-GetTextWidth(hDC, sTmp),
                 TextMargin,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);

      _stprintf(sTmp, TEXT("%d%s"),  iround(Task[i].InBound),gettext(_T("_@M2179_")));
      ExtTextOut(hDC, Sender->GetHeight()+p2+w2-GetTextWidth(hDC, sTmp),
                 TextMargin,
                 ETO_OPAQUE, NULL,
                 sTmp, _tcslen(sTmp), NULL);
      
    }

  } else {
     // if (DrawListIndex==n) { // patchout 091126
     if (DrawListIndex==n && UpLimit < MAXTASKPOINTS) { // patch 091126

	// LKTOKEN  _@M832_ = "add waypoint" 
      _stprintf(sTmp, TEXT("  (%s)"), gettext(TEXT("_@M832_")));
      ExtTextOut(hDC, Sender->GetHeight()+2*ScreenScale, TextMargin,
		 ETO_OPAQUE, NULL,
		 sTmp, _tcslen(sTmp), NULL);
    } else if ((DrawListIndex==n+1) && ValidTaskPoint(0)) {

      if (!AATEnabled || ISPARAGLIDER) {
	// LKTOKEN  _@M735_ = "Total:" 
	_stprintf(sTmp, gettext(TEXT("_@M735_")));
	ExtTextOut(hDC, Sender->GetHeight()+2*ScreenScale, TextMargin,
		   ETO_OPAQUE, NULL,
		   sTmp, _tcslen(sTmp), NULL);
      
	if (fai_ok) {
	  _stprintf(sTmp, TEXT("%.0f %s FAI"), lengthtotal*DISTANCEMODIFY,
		    Units::GetDistanceName());
	} else {
	  _stprintf(sTmp, TEXT("%.0f %s"), lengthtotal*DISTANCEMODIFY,
		    Units::GetDistanceName());
	}
	ExtTextOut(hDC, Sender->GetHeight()+p1+w1-GetTextWidth(hDC, sTmp),
                  TextMargin,
		   ETO_OPAQUE, NULL,
		   sTmp, _tcslen(sTmp), NULL);

      } else {

      double d1 = CALCULATED_INFO.TaskDistanceToGo;
      if ((CALCULATED_INFO.TaskStartTime>0.0) && (CALCULATED_INFO.Flying) && (ActiveWayPoint>0)) {
                   d1 += CALCULATED_INFO.TaskDistanceCovered;
      }

	if (d1==0.0) {
	  d1 = CALCULATED_INFO.AATTargetDistance;
	}

	_stprintf(sTmp, TEXT("%s %.0f min %.0f (%.0f) %s"), 
	// LKTOKEN  _@M735_ = "Total:" 
                  gettext(TEXT("_@M735_")),
                  AATTaskLength*1.0,
		  DISTANCEMODIFY*lengthtotal,
		  DISTANCEMODIFY*d1,
		  Units::GetDistanceName());
	ExtTextOut(hDC, Sender->GetHeight()+2*ScreenScale, TextMargin,
		   ETO_OPAQUE, NULL,
		   sTmp, _tcslen(sTmp), NULL);
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

  // Simple FAI 2004 triangle rules 
  fai_ok = true;
  if (lengthtotal>0) {
    for (i=0; i<MAXTASKPOINTS; i++) {
      if (Task[i].Index != -1) {
	double lrat = Task[i].Leg/lengthtotal;
	if ((lrat>0.45)||(lrat<0.10)) {
	  fai_ok = false;
	}
      }
    }
  } else {
    fai_ok = false;
  }

  RefreshTaskStatistics();
  
  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATEst"));
  if (wp) {
    double dd = CALCULATED_INFO.TaskTimeToGo;
//    if ((CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying)) { patchout 091126
    if ( (CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying) &&(ActiveWayPoint>0)) { // patch 091126



      dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
    }
    dd= min(24.0*60.0,dd/60.0);
    wp->GetDataField()->SetAsFloat(dd);
    wp->RefreshDisplay();
  }
  
  LowLimit =0;
  wTaskList->ResetList();
  wTaskList->Redraw();

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
  if ((ItemIndex>=0) && (ItemIndex == UpLimit) && (UpLimit<MAXTASKPOINTS)) {

	// add new waypoint
	if (CheckDeclaration()) {

		if (ItemIndex>0) {
#ifdef LAST_TASKPOINT_QUESTION
			if (MessageBoxX(hWndMapWindow,
			// LKTOKEN  _@M817_ = "Will this be the finish?" 
			gettext(TEXT("_@M817_")),
			// LKTOKEN  _@M54_ = "Add Waypoint" 
			gettext(TEXT("_@M54_")),
			MB_YESNO|MB_ICONQUESTION) == IDYES)
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

			UnlockTaskData();
//			wf->SetModalResult(mrOK);
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

  if (ItemIndex<UpLimit) {

//		wf->SetModalResult(mrOK);
	if (ItemIndex==0) {
		dlgTaskWaypointShowModal(ItemIndex, 0); // start waypoint
	} else {
		if (ItemIndex==UpLimit-1) {
			dlgTaskWaypointShowModal(ItemIndex, 2); // finish waypoint
		} else {
			dlgTaskWaypointShowModal(ItemIndex, 1); // turnpoint
		}
	}
	  OverviewRefreshTask();
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



static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  ItemIndex = -1; // to stop FormDown bringing up task details
  wf->SetModalResult(mrOK);
}



static void OnClearClicked(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
	(void)ListInfo; (void)Sender;
  if (MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M179_ = "Clear the task?" 
                  gettext(TEXT("_@M179_")),
	// LKTOKEN  _@M178_ = "Clear task" 
                  gettext(TEXT("_@M178_")),
                  MB_YESNO|MB_ICONQUESTION) == IDYES) {
    if (CheckDeclaration()) {
      ClearTask();
      UpdateFilePointer();
      OverviewRefreshTask();
      UpdateCaption();
    }
  }
}

static void OnCalcClicked(WindowControl * Sender, 
			  WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  (void)ListInfo;

  wf->SetVisible(false);
  dlgTaskCalculatorShowModal();
  OverviewRefreshTask();
  wf->SetVisible(true);
}


static void OnAnalysisClicked(WindowControl * Sender, 
                              WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  (void)ListInfo;

  wf->SetVisible(false);
  dlgAnalysisShowModal(ANALYSIS_PAGE_TASK);
  wf->SetVisible(true);
}

static void OnTimegatesClicked(WindowControl * Sender, 
                              WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  (void)ListInfo;

  wf->SetVisible(false);
  dlgTimeGatesShowModal();
  wf->SetVisible(true);
}

static void OnDeclareClicked(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
	(void)Sender;
	(void)ListInfo;
  RefreshTask();

  LoggerDeviceDeclare();

  // do something here.
}




static void OnSaveClicked(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)ListInfo; (void)Sender;

  int file_index; 
  TCHAR task_name[MAX_PATH];
  TCHAR file_name[MAX_PATH];
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
	LocalPath(file_name,TEXT(LKD_TASKS));
	_tcscat(file_name,TEXT("\\"));
	_tcscat(file_name,task_name); // 091101

	dfe->Lookup(file_name);
	file_index = dfe->GetAsInteger();

	if (file_index==0) {
		// good, this file is unique..
		dfe->addFile(task_name, file_name);
		dfe->Lookup(file_name);
		wp->RefreshDisplay();
	}

  } else {
	// TODO code: report error, task not saved since no name was given
	return;
  }

  if (file_index>0) {
	// file already exists! ask if want to overwrite
	_stprintf(file_name, TEXT("%s: '%s'"), 
	// LKTOKEN  _@M696_ = "Task file already exists" 
		gettext(TEXT("_@M696_")),
		dfe->GetAsString());

		if(MessageBoxX(hWndMapWindow,
			file_name,
			// LKTOKEN  _@M510_ = "Overwrite?" 
			gettext(TEXT("_@M510_")),
			MB_YESNO|MB_ICONQUESTION) != IDYES) {

			return;
		}
  }

  SaveTask(dfe->GetPathFile());
  UpdateCaption();
}



static void OnLoadClicked(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){ // 091216
  (void)ListInfo; (void)Sender;

  TCHAR file_name[MAX_PATH];

  WndProperty* wp;
  DataFieldFileReader *dfe;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;

  HWND hwnd = wp->GetHandle();
  SendMessage(hwnd,WM_LBUTTONDOWN,0,0);
  dfe = (DataFieldFileReader*) wp->GetDataField();

  int file_index = dfe->GetAsInteger();
  if (file_index>0) {
	if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) {
		_stprintf(file_name, TEXT("%s '%s' ?"), gettext(TEXT("_@M891_")), dfe->GetAsString()); // Clear old task and load
		if(MessageBoxX(hWndMapWindow, file_name, _T(" "), MB_YESNO|MB_ICONQUESTION) == IDNO) {
			return;
		}
	}
  } else {
	// LKTOKEN  _@M467_ = "No Task to load" 
	MessageBoxX(hWndMapWindow, gettext(TEXT("_@M467_")),_T(" "), MB_OK|MB_ICONEXCLAMATION);
	return;
  }

  if (file_index>0) {
      LPCTSTR szFileName = dfe->GetPathFile();
      LPCTSTR wextension = _tcsrchr(szFileName, _T('.'));
      if(wextension) {
          bool bOK = false;
          if(_tcsicmp(wextension,_T(LKS_TSK))==0) {
              CTaskFileHelper helper;
              bOK = helper.Load(szFileName);
          } else if (_tcsicmp(wextension,_T(LKS_OLD_TSK))==0) {
              LoadNewTask(szFileName);
              bOK = true;
          } else if (_tcsicmp(wextension,_T(LKS_WP_CUP))==0) {
              bOK = LoadCupTask(szFileName);
          } else if (_tcsicmp(wextension,_T(LKS_WP_GPX))==0) {
              bOK = LoadGpxTask(szFileName);
          }
          if(!bOK) {
              MessageBoxX(hWndMapWindow, gettext(TEXT("_@M467_")),_T(" "), MB_OK|MB_ICONEXCLAMATION);
              return;
          }
          OverviewRefreshTask();
          UpdateFilePointer();
          UpdateCaption();
      }
  }
}


static void OnDeleteClicked(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)ListInfo; (void)Sender;

  TCHAR file_name[MAX_PATH];

  WndProperty* wp;
  DataFieldFileReader *dfe;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;

  HWND hwnd = wp->GetHandle();
  SendMessage(hwnd,WM_LBUTTONDOWN,0,0);
  dfe = (DataFieldFileReader*) wp->GetDataField();

  int file_index = dfe->GetAsInteger();
  if (file_index>0) {
	_stprintf(file_name, TEXT("%s '%s' ?"), MsgToken(1789), dfe->GetAsString()); // Delete task file?
	if(MessageBoxX(hWndMapWindow, file_name, _T(" "), MB_YESNO|MB_ICONQUESTION) == IDNO) {
		return;
	}
  } else {
	MessageBoxX(hWndMapWindow, MsgToken(1790),_T(" "), MB_OK|MB_ICONEXCLAMATION); // No task file to delete
	return;
  }

  if (file_index>0) {
    DeleteFile(dfe->GetPathFile());
    // Cannot update dfe list, so we force exit.
    ItemIndex = -1; 
    wf->SetModalResult(mrOK);
    return;
  }
}



static void OnAdvancedClicked(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)Sender; (void)ListInfo;
  showAdvanced = !showAdvanced;
  UpdateAdvanced();
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnTaskPaintListItem),
  DeclareCallBackEntry(OnTaskListInfo),
  DeclareCallBackEntry(OnDeclareClicked),
  DeclareCallBackEntry(OnCalcClicked),
  DeclareCallBackEntry(OnClearClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnAdvancedClicked),
  DeclareCallBackEntry(OnSaveClicked),
  DeclareCallBackEntry(OnLoadClicked),
  DeclareCallBackEntry(OnDeleteClicked),
  DeclareCallBackEntry(OnAnalysisClicked),
  DeclareCallBackEntry(OnTimegatesClicked),
  DeclareCallBackEntry(NULL)
};




void dlgTaskOverviewShowModal(int Idx){

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = Idx; //-1;

  showAdvanced = false;

  wf = NULL;

  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgTaskOverview_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_TASKOVERVIEW_L"));
  } else {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgTaskOverview.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_TASKOVERVIEW"));
  }

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

  wTaskList->SetWidth(wf->GetWidth() - wTaskList->GetLeft()-2);

  wTaskListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmTaskListEntry"));

   // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
  if ( wTaskList->ScrollbarWidth == -1) {
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wTaskList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);
  }
  wTaskListEntry->SetWidth(wTaskList->GetWidth() - wTaskList->ScrollbarWidth - 5);

  wTaskListEntry = (WndOwnerDrawFrame*)wf-> FindByName(TEXT("frmTaskListEntry"));

  wTaskListEntry->SetCanFocus(true);

  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (wp) {
	DataFieldFileReader* dfe;
	dfe = (DataFieldFileReader*)wp->GetDataField();

	TCHAR suff[10];
	_stprintf(suff,_T("*%s"),_T(LKS_TSK));
	dfe->ScanDirectoryTop(_T(LKD_TASKS),suff);
	_stprintf(suff,_T("*%s"),_T(LKS_OLD_TSK));
	dfe->ScanDirectoryTop(_T(LKD_TASKS),suff);
	_stprintf(suff,_T("*%s"),_T(LKS_WP_CUP));
	dfe->ScanDirectoryTop(_T(LKD_TASKS),suff);
	_stprintf(suff,_T("*%s"),_T(LKS_WP_GPX));
	dfe->ScanDirectoryTop(_T(LKD_TASKS),suff);
	wp->RefreshDisplay();
  }
  UpdateFilePointer();

  // initialise and turn on the display
  OverviewRefreshTask();

  UpdateAdvanced(); 



  wTaskList->SetItemIndexPos(Idx);
  wTaskList->Redraw();
  wTaskListEntry->SetFocused(true,NULL);

  wf->ShowModal();

  // now retrieve back the properties...

  RefreshTask();

  delete wf;

  wf = NULL;

}

