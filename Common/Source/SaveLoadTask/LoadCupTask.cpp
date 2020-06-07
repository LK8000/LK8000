/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   CTaskFileHelper.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 29 janvier 2013, 23:49
 */

#include "externs.h"
#include "Waypointparser.h"
#include "Util/tstring.hpp"
#include "NavFunctions.h"
#include "utils/zzip_stream.h"

#include "WindowControls.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "resource.h"
#include "InputEvents.h"
#include "Util/UTF8.hpp"

int dlgTaskSelectListShowModal(void) ;

#define MAX_TASKS 100
TCHAR* szTaskStrings[MAX_TASKS];


int TaskIndex =0;
int iNO_Tasks =0;
static int TaskDrawListIndex = 0;

typedef std::map<tstring, WAYPOINT> mapCode2Waypoint_t;

class CupObsZoneUpdater {
public:

    CupObsZoneUpdater() : mIdx(), mType(), mR1(), mA1(), mR2(), mA2(), mA12(), mLine() {
    }

    size_t mIdx;
    int mType;
    double mR1;
    double mA1;
    double mR2;
    double mA2;
    double mA12;
    bool mLine;

    void UpdateTask() {
        if (mA1 == 180.0) {
            if (mIdx == 0) {
                StartLine = 0;
                StartRadius = mR1;
            } else if (mIdx == (size_t) getFinalWaypoint()) {
                FinishLine = 0;
                FinishRadius = mR1;
            } else {
                Task[mIdx].AATType = CIRCLE;
                Task[mIdx].AATCircleRadius = mR1;
            }
        } else {

            switch (mType) {
                case 0: // - Fixed value,
                    if (mLine) {
                        StartupStore(_T("..Cup Task : \"Fixed\" LINE Turnpoint is not supported%s"), NEWLINE);
                        UpdateFixedLine();
                    } else {
                        UpdateFixedSector();
                    }
                    break;
                case 1: // - Symmetrical, 
                    if (mLine) {
                        StartupStore(_T("..Cup Task : \"Symmetrical\" LINE Turnpoint is not supported%s"), NEWLINE);
                        UpdateSymLine();
                    } else {
                        UpdateSymSector();
                    }
                    break;
                case 2: // - To next point, 
                    if (mLine) {
                        if (mIdx > 0) {
                            StartupStore(_T("..Cup Task : \"To next point\" LINE Turnpoint is not supported%s"), NEWLINE);
                        }
                        UpdateToNextLine();
                    } else {
                        UpdateToNextSector();
                    }
                    break;
                case 3: // - To previous point, 
                    if (mLine) {
                        if (mIdx < (size_t) getFinalWaypoint()) {
                            StartupStore(_T("..Cup Task : \"To previous point\" LINE Turnpoint is not supported%s"), NEWLINE);
                        }
                        UpdateToPrevLine();
                    } else {
                        UpdateToPrevSector();
                    }
                    break;
                case 4: // - To start point
                    if (mLine) {
                        StartupStore(_T("..Cup Task : \"To start point\" LINE Turnpoint is not supported%s"), NEWLINE);
                        UpdateToStartLine();
                    } else {
                        UpdateToStartSector();
                    }
                    break;
            }
        }
    }

    void UpdateFixedLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateFixedSector();
        }
    }

    void UpdateFixedSector() {
        if (mIdx == 0) {
            StartLine = 2;
            StartRadius = mR1;
        } else if (mIdx == (size_t) getFinalWaypoint()) {
            FinishLine = 2;
            FinishRadius = mR1;
        } else {
            Task[mIdx].AATType = SECTOR;
            Task[mIdx].AATSectorRadius = mR1;
            Task[mIdx].AATStartRadial = mA12 - 180.0 - (mA1);
            Task[mIdx].AATFinishRadial = mA12 - 180.0 + (mA1);
        }
    }

    void UpdateSymLine() {
        if (mIdx == 0) {
            StartLine = 1;
            StartRadius = mR1;
        } else if (mIdx == (size_t) getFinalWaypoint()) {
            FinishLine = 1;
            FinishRadius = mR1;
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateSymSector();
        }
    }

    void UpdateSymSector() {
        if (mIdx == 0) {
            UpdateToNextSector();
        } else if (mIdx == ((size_t) getFinalWaypoint())) {
            UpdateToPrevSector();
        } else {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *PrevPt = TaskWayPoint(mIdx - 1);
            const WAYPOINT *NextPt = TaskWayPoint(mIdx + 1);
            if(CurrPt && PrevPt && NextPt) {
                double InB = 0;
                double OutB = 0;
                // bearing to prev
                DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                                PrevPt->Latitude, PrevPt->Longitude, NULL, &InB);
                // bearing to next
                DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                                NextPt->Latitude, NextPt->Longitude, NULL, &OutB);
                mA12 = BiSector(InB, OutB);

                UpdateFixedSector();
            }
        }
    }

    void UpdateToNextLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateToNextSector();
        }
    }

    void UpdateToNextSector() {
        if (ValidTaskPoint(mIdx + 1)) {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *NextPt = TaskWayPoint(mIdx + 1);
            if(CurrPt && NextPt) {
                // bearing to next
                DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                                NextPt->Latitude, NextPt->Longitude, NULL, &mA12);

                UpdateFixedSector();
            }
        }
    }

    void UpdateToPrevLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateToPrevSector();
        }
    }

    void UpdateToPrevSector() {
        if (ValidTaskPoint(mIdx - 1)) {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *PrevPt = TaskWayPoint(mIdx - 1);
            if(CurrPt && PrevPt) {
                // bearing to prev
                DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                                PrevPt->Latitude, PrevPt->Longitude, NULL, &mA12);

                UpdateFixedSector();
            }
        }
    }

    void UpdateToStartLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateToStartSector();
        }
    }

    void UpdateToStartSector() {
        if (mIdx > 0) {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *StartPt = TaskWayPoint(0);
            if(CurrPt && StartPt) {
                // bearing to prev
                DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                                StartPt->Latitude, StartPt->Longitude, NULL, &mA12);

                UpdateFixedSector();
            }
        }
    }
};



bool ConvertStringToTask( LPCTSTR szTaskSteing,   mapCode2Waypoint_t &mapWaypoint)
{
  std::vector<tstring>  Entries =   CupStringToFieldArray(szTaskSteing);


  size_t Idx =0;
  size_t idxTP = 0;


    for (uint i =0; i < Entries.size() ; i++)
    {
      mapCode2Waypoint_t::iterator It = mapWaypoint.find(Entries[Idx++].c_str());
      if(It != mapWaypoint.end())
      {
        int ix = FindOrAddWaypoint(&(It->second),true);  // first try to find airfield
        if(ix < 0)
          ix= FindOrAddWaypoint(&(It->second),false); // not found try if waypoint exist

        if(ix >= 0)
        {
          if (idxTP < MAXTASKPOINTS)       // space left
          {
            Task[idxTP++].Index = ix;
            if(idxTP > 1)
              if(Task[idxTP-1].Index  == Task[idxTP-2].Index )  // not the same as before?
                idxTP--;
          }
        }
      }
      Task[idxTP].Index = -1;
    }

//#define DEBUG_TASK_LOAD
#ifdef DEBUG_TASK_LOAD
  int i =0;
  while ( Task[i].Index  >=0)
  {
    StartupStore(_T(".......Taskpoint: %u %s\n"),(unsigned)(i), WayPointList[Task[i].Index].Name );
    i++;
  }
#endif


  return (idxTP > 0);
}



bool LoadCupTaskSingle(LPCTSTR szFileName, LPTSTR TaskLine, int SelectedTaskIndex) {


  mapCode2Waypoint_t mapWaypoint;

  bool TaskFound = false;
  bool bLoadComplet = true;
  bool bLastInvalid=true;
  std::vector<tstring> Entries;
  TCHAR szString[READLINE_LENGTH + 1];


  szString[READLINE_LENGTH] = _T('\0');


  memset(szString, 0, sizeof (szString)); // clear Temp Buffer
  WAYPOINT newPoint = {};


  enum {
    none, Waypoint, TaskTp, Option
  } FileSection = none;
  zzip_stream stream(szFileName, "rt");


  LockTaskData();
  ClearTask();

  cup_header_t cup_header;

  bool TaskValid = false;
  FileSection = none;
  int i=0;
  if (stream) {
      while (stream.read_line(szString)) {

          if ((FileSection == none) && ((_tcsncmp(_T("name,code,country"), szString, 17) == 0) ||
              (_tcsncmp(_T("Title,Code,Country"), szString, 18) == 0))) {
              FileSection = Waypoint;
              cup_header = CupStringToHeader(szString);
              continue;
          } else if ((FileSection == Waypoint) && (_tcscmp(szString, _T("-----Related Tasks-----")) == 0)) {
              FileSection = TaskTp;

              continue;
          }
          if( FileSection == TaskTp)
            if(_tcsstr(szString, _T("\",\""))== NULL)   // really a task? (not an option)
              FileSection = Option;
          if( FileSection == Option)
            if(_tcsstr(szString, _T("\",\""))!= NULL)   // really a task! (not an option)
              FileSection = TaskTp;

          const TCHAR *pToken = NULL;

          int hh,mm,ss;

          if(FileSection !=Waypoint)
          {
            Entries = CupStringToFieldArray(szString);
            pToken = Entries[0].c_str() ;
#if TESTBENCH
     //       for(size_t idx=0; idx < Entries.size(); idx ++)
     //         StartupStore(_T(". Task  %i %s %s"), idx, Entries[idx].c_str()  ,NEWLINE);
#endif
          }

          switch (FileSection) {

          case Waypoint:
            memset(&newPoint, 0, sizeof(newPoint));
            if (ParseCUPWayPointString(cup_header, szString, &newPoint)) {
                mapWaypoint[newPoint.Name] = newPoint;
            }
            break;
          case TaskTp:
            // 1. Description
            //       First column is the description of the task. If filled it should be double quoted.
            //       If left empty, then SeeYou will determine the task type on runtime.
            // 2. and all successive columns, separated by commas
            //       Each column represents one waypoint name double quoted. The waypoint name must be exactly the
            //       same as the Long name of a waypoint listed above the Related tasks.


            if (i++ != (SelectedTaskIndex))  // load selected task
            {
              TaskValid = false;
            }
            else
            {
                TaskValid = true;
                TaskFound = true;
                SectorType=SECTOR;  // normal sector by default if no other ObsZone parameter
                gTaskType = TSK_DEFAULT; // racing task by default, if AAT will overwrite this

                if(Entries[0].size() == 0)
                {
                  StartupStore(_T(". no Task name, named it %s%i %s"), MsgToken(699),i, NEWLINE);// _@M699_ "Task"
                  if(TaskLine != NULL)
                    _sntprintf(TaskLine,READLINE_LENGTH,_T("%s%i%s"),MsgToken(699),i,szString);  // _@M699_ "Task"
                }
                else
                {
                  StartupStore(_T(". read Task %s %s"), Entries[0].c_str()  ,NEWLINE);
                  if(TaskLine != NULL)
		              _tcscpy(TaskLine, szString );
                }



                bLoadComplet = ConvertStringToTask( szString, mapWaypoint);
                FileSection = Option;
              }
            break;
          case Option:
            if (TaskValid)  // load option for selected task only
            {
	      if (_tcsstr(pToken, _T("Options")) == pToken)
	      {
		  size_t ParIdx=1;
		  while ( ParIdx <  Entries.size() )
		  {
		    pToken = Entries[ParIdx++].c_str() ;

		    if (_tcsstr(pToken, _T("NoStart=")) == pToken) {
			// Opening of start line
			PGNumberOfGates = 1;
			StrToTime(pToken + 8, &PGOpenTimeH, &PGOpenTimeM);
		    } else if (_tcsstr(pToken, _T("TaskTime=")) == pToken) {
			// Designated Time for the task
			StrToTime(pToken + 9, &hh, &mm, &ss);
//			StartupStore(_T("..Cup Task Time:(%02i:%02i) %imin  %s"),hh,mm,(hh*60+mm), NEWLINE);
			AATTaskLength =  hh*60+mm+ss/60;

			// TODO :
		    } else if (_tcsstr(pToken, _T("WpDis=")) == pToken) {
			// Task distance calculation. False = use fixes, True = use waypoints
			// TODO :
		    } else if (_tcsstr(pToken, _T("NearDis=")) == pToken) {
			// Distance tolerance
			// TODO :
		    } else if (_tcsstr(pToken, _T("NearAlt=")) == pToken) {
			// Altitude tolerance
			// TODO :
		    } else if (_tcsstr(pToken, _T("MinDis=")) == pToken) {
			// Uncompleted leg.
			// False = calculate maximum distance from last observation zone.
			// TODO :
		    } else if (_tcsstr(pToken, _T("RandomOrder=")) == pToken) {
			// if true, then Random order of waypoints is checked
			// TODO :
		    } else if (_tcsstr(pToken, _T("MaxPts=")) == pToken) {
			// Maximum number of points
			// TODO :
		    } else if (_tcsstr(pToken, _T("BeforePts=")) == pToken) {
			// Number of mandatory waypoints at the beginning. 1 means start line only, two means
			//      start line plus first point in task sequence (Task line).
			// TODO :
		    } else if (_tcsstr(pToken, _T("AfterPts=")) == pToken) {
			// Number of mandatory waypoints at the end. 1 means finish line only, two means finish line
			//      and one point before finish in task sequence (Task line).
			// TODO :
		    } else if (_tcsstr(pToken, _T("Bonus=")) == pToken) {
			// Bonus for crossing the finish line
			// TODO :
		    }
		}
                } else if (_tcsstr(pToken, _T("ObsZone=")) == pToken) {
                    TCHAR *sz = NULL;
                    CupObsZoneUpdater TmpZone;
                    TmpZone.mIdx = _tcstol(pToken + 8, NULL, 10);
                    if (TmpZone.mIdx < MAXTASKPOINTS) {
                          size_t ParIdx=1;
                          while ( ParIdx <  Entries.size() )
                          {
                            pToken = Entries[ParIdx++].c_str() ;

                            if (_tcsstr(pToken, _T("Style=")) == pToken) {
                                // Direction. 0 - Fixed value, 1 - Symmetrical, 2 - To next point, 3 - To previous point, 4 - To start point
                                TmpZone.mType = _tcstol(pToken + 6, &sz, 10);
                            } else if (_tcsstr(pToken, _T("R1=")) == pToken) {
                                // Radius 1
                                TmpZone.mR1 = ReadLength(pToken + 3);
                            } else if (_tcsstr(pToken, _T("A1=")) == pToken) {
                                // Angle 1 in degrees
                                TmpZone.mA1 = _tcstod(pToken + 3, &sz);
                                if( TmpZone.mR1 > 179.5)  //  180° = Circle sector
                                  if( TmpZone.mR1 < 180.5)
                                    SectorType=CIRCLE;
                            } else if (_tcsstr(pToken, _T("R2=")) == pToken) {
                                // Radius 2
                                TmpZone.mR2 = ReadLength(pToken + 3);
                            } else if (_tcsstr(pToken, _T("A2=")) == pToken) {
                                // Angle 2 in degrees
                                TmpZone.mA2 = _tcstod(pToken + 3, &sz);
                            } else if (_tcsstr(pToken, _T("A12=")) == pToken) {
                                // Angle 12
                                TmpZone.mA12 = _tcstod(pToken + 4, &sz);
                            } else if (_tcsstr(pToken, _T("AAT=")) == pToken) {
                                // AAT
                        	      gTaskType = TSK_AAT;
                                if( _tcstod(pToken + 4, &sz) > 0) // AAT = 1?
                                  SectorType=CIRCLE;
                            } else if (_tcsstr(pToken, _T("Line=")) == pToken) {
                                // true For Line Turmpoint type
                                // Exist only for start an Goalin LK
                                TmpZone.mLine = (_tcstol(pToken + 5, &sz, 10) == 1);
                            }
                        }
                        if(  TmpZone.mR1 >0)   // if both radius defined
                          if(  TmpZone.mR2 >0) // must be keyhole definition
                            SectorType=DAe;    // the only similar sectortype in LK

                        if( TmpZone.mLine) // line has priority (Start & Finish) if multiple definitions
                        {
                          TmpZone.mA1 = 90;
                          TmpZone.mA2 = 0;
                          TmpZone.mA12 =0;
                        }
                        TmpZone.UpdateTask();
                    }
                }
              }
            break;
          case none:
          default:
            break;
          }
          memset(szString, 0, sizeof (szString)); // clear Temp Buffer
      }
  }
  if(!ISGAAIRCRAFT) {
      // Landing don't exist in LK Task Systems, so Remove It;
      if ( bLoadComplet && !bLastInvalid ) {
          RemoveTaskPoint(getFinalWaypoint());
      }
  }
  UnlockTaskData();
  for (mapCode2Waypoint_t::iterator It = mapWaypoint.begin(); It != mapWaypoint.end(); ++It) {
      if (It->second.Comment) {
          free(It->second.Comment);
      }
      if (It->second.Details) {
          free(It->second.Details);
      }
  }
  mapWaypoint.clear();

  return TaskFound;
}




bool LoadCupTask(LPCTSTR szFileName) {
TCHAR szString[READLINE_LENGTH + 1];
  for (int i =0 ; i< MAX_TASKS;i++)
    szTaskStrings[ i] = NULL;

  iNO_Tasks=0;

  SaveDefaultTask(); // save current task for restore if no task load
  while(LoadCupTaskSingle(szFileName,szString, iNO_Tasks)&&( iNO_Tasks < MAX_TASKS ))
  {
    LockTaskData();
    RefreshTask();
    szTaskStrings[ iNO_Tasks] =  new TCHAR[READLINE_LENGTH + 1];
    size_t NoPts = 0;
    double lengthtotal =0;
    int iLastIdx = -1;
    bool bClosedTask = false;
    for (size_t i=0; i<MAXTASKPOINTS; i++) {
        if (Task[i].Index != -1) {
            lengthtotal += Task[i].Leg;
            NoPts = i;
            iLastIdx = Task[i].Index;
        }
    }
    if(iLastIdx == Task[0].Index )
    {
      bClosedTask = true;
    }

    NoPts-=1;
    if(NoPts > 7) NoPts =7;
    if(NoPts < 0) NoPts =0;
    if(  szTaskStrings[ iNO_Tasks] != NULL)
    {
      if(bClosedTask)
      {
          if (gTaskType == TSK_AAT)
              _sntprintf(szTaskStrings[iNO_Tasks], READLINE_LENGTH, _T("[AAT %.1f%s] %s"), lengthtotal * DISTANCEMODIFY, Units::GetDistanceName(), szString); // _@M699_ "Task"
          else if (CALCULATED_INFO.TaskFAI)
              _sntprintf(szTaskStrings[iNO_Tasks], READLINE_LENGTH, _T("[FAI %s %.1f%s] %s"), MsgToken(2432), lengthtotal * DISTANCEMODIFY, Units::GetDistanceName(), szString); // _@M2432_ "Triangle"
          else
              _sntprintf(szTaskStrings[iNO_Tasks], READLINE_LENGTH, _T("[%s %.1f%s] %s"), MsgToken(2430 + NoPts), lengthtotal * DISTANCEMODIFY, Units::GetDistanceName(), szString);
      }
      else
      {
        _sntprintf(szTaskStrings[ iNO_Tasks] ,READLINE_LENGTH,_T("[%s %.1f%s] %s") , MsgToken(2429) ,lengthtotal*DISTANCEMODIFY,Units::GetDistanceName(), szString); // MsgToken(2432)   ,
      }
    }
    UnlockTaskData();
    iNO_Tasks++;
  }
  TaskIndex =0;
  InputEvents::eventTaskLoad(_T(LKF_DEFAULTASK)); //  restore old task
  dlgTaskSelectListShowModal();
  if((TaskIndex >= 0) && (TaskIndex < MAX_TASKS))
  {     TCHAR file_name[180];
        TCHAR *pWClast = NULL;
        TCHAR * pToken = strsep_r(szTaskStrings[TaskIndex], TEXT(","), &pWClast) ;  // extract taskname
        if((pToken) && (_tcslen (pToken)>1))
	  _sntprintf(file_name,180, TEXT("%s %s ?"), MsgToken(891), pToken ); // Clear old task and load taskname
        else
          _sntprintf(file_name,180, TEXT("%s %s ?"), MsgToken(891), MsgToken(907)); // Clear old task and load task
	if(MessageBoxX(file_name, _T(" "), mbYesNo) == IdYes) {
	  LoadCupTaskSingle(szFileName,szString, TaskIndex); // load new task
	}
  }



  for (int i =0 ; i< MAX_TASKS;i++)    // free dynamic memory
    if(szTaskStrings[i] != NULL)
      {
        // StartupStore(_T("..Cup Task : delete dynamic memoryLine %i %s"), i,NEWLINE);
        delete[] szTaskStrings[i];
        szTaskStrings[i] = NULL;
      }

  return true;

}


static void OnEnterClicked(WndButton* pWnd) {
  (void)pWnd;

  if (TaskIndex >= iNO_Tasks) {
      TaskIndex = iNO_Tasks - 1;
  }
  if (TaskIndex >= 0) {
      if(pWnd) {
          WndForm * pForm = pWnd->GetParentWndForm();
          if(pForm) {
              pForm->SetModalResult(mrOK);
          }
      }
  }
}


static void OnCloseClicked(WndButton* pWnd) {
  TaskIndex = -1;
  if(pWnd) {
      WndForm * pForm = pWnd->GetParentWndForm();
      if(pForm) {
          pForm->SetModalResult(mrCancel);
      }
  }
}


static void UpdateList(WndListFrame* pList) {
  pList->ResetList();
  pList->Redraw();
}

static void SelectItem(WndForm* pForm, int index) {
  WndListFrame* pList = (WndListFrame*)pForm->FindByName(TEXT("frmMultiSelectListList"));
  if(pList) {
    pList->SetItemIndexPos(index);
    pList->Redraw();
  }
  WndOwnerDrawFrame* pListEntry = (WndOwnerDrawFrame*) pForm->FindByName(TEXT("frmMultiSelectListListEntry"));
  if(pListEntry) {
      pListEntry->SetFocus();
  }
}

static void OnUpClicked(WndButton* pWnd) {
  if (TaskIndex > 0) {
      TaskIndex--;
  } else {
      LKASSERT(iNO_Tasks>0);
      TaskIndex = (iNO_Tasks - 1);
  }
  SelectItem(pWnd->GetParentWndForm(), TaskIndex);
}

static void OnDownClicked(WndButton* pWnd) {
  if (TaskIndex < (iNO_Tasks - 1)) {
      TaskIndex++;
  } else {
      TaskIndex = 0;
  }
  SelectItem(pWnd->GetParentWndForm(), TaskIndex);
}





static void OnMultiSelectListListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {

  (void) Sender;

  if (ListInfo->DrawIndex == -1) {
      ListInfo->ItemCount = iNO_Tasks;

  } else {
      TaskDrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
      TaskIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }
}



static void OnMultiSelectListPaintListItem(WindowControl * Sender, LKSurface& Surface) {

#define PICTO_WIDTH 50

  Surface.SetTextColor(RGB_BLACK);
  if (TaskDrawListIndex < iNO_Tasks)  {
      TCHAR *pWClast = NULL;
      TCHAR *pWClast2 = NULL;
      TCHAR text[180] = {TEXT("empty")};
      TCHAR text1[180] = {TEXT("empty")};
      TCHAR text2[180] = {TEXT("empty")};

      // remove all quotations "
      const TCHAR* szSrc = szTaskStrings[TaskDrawListIndex];
      size_t MaxSize = std::min(std::size(text), _tcslen(szSrc));
      TCHAR* out_end = std::remove_copy_if(szSrc, std::next(szSrc, MaxSize),
              std::begin(text), [](TCHAR c) { return c == _T('"'); });
      (*out_end) = _T('\0');
#ifndef UNICODE
      CropIncompleteUTF8(text);
#endif

      TCHAR* pToken = strsep_r(text, TEXT(","), &pWClast);
      _tcscpy(text1, pToken );
      if(*text1 == '\0') {
        _tcscpy(text1, _T("???") );
      }
      strsep_r(pWClast, TEXT(","), &pWClast2) ;  // remove takeof point
      _tcscpy(text2, pWClast2);

      /********************
       * show text
       ********************/
      Surface.SetBackgroundTransparent();
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(DLGSCALE(2), DLGSCALE(2), text1);
      int ytext2 = Surface.GetTextHeight(text1);
      Surface.SetTextColor(RGB_DARKBLUE);
      Surface.DrawText(DLGSCALE(2), DLGSCALE(2) + ytext2, text2);
  }
}



static void OnTaskSelectListListEnter(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {

  TaskIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (TaskIndex >= iNO_Tasks) {
      TaskIndex = iNO_Tasks - 1;
  }

  if (TaskIndex >= 0) {
      if(Sender) {
          WndForm * pForm = Sender->GetParentWndForm();
          if(pForm) {
              pForm->SetModalResult(mrOK);
          }
      }
  }
}


static CallBackTableEntry_t TaskCallBackTable[] = {
    OnPaintCallbackEntry(OnMultiSelectListPaintListItem),
    OnListCallbackEntry(OnMultiSelectListListInfo),
    ClickNotifyCallbackEntry(OnCloseClicked),
    ClickNotifyCallbackEntry(OnUpClicked),
    ClickNotifyCallbackEntry(OnEnterClicked),
    ClickNotifyCallbackEntry(OnDownClicked),
    EndCallBackEntry()
};


int dlgTaskSelectListShowModal() {

  TaskIndex = -1;

  if (iNO_Tasks == 0) {
    return mrCancel;
  }

  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(TaskCallBackTable, ScreenLandscape ? IDR_XML_MULTISELECTLIST_L : IDR_XML_MULTISELECTLIST_P));
  if (!pForm) {
    return mrCancel;
  }

  WndListFrame* pList = (WndListFrame*)pForm->FindByName(TEXT("frmMultiSelectListList"));
  if(pList) {
    pList->SetBorderKind(BORDERLEFT);
    pList->SetEnterCallback(OnTaskSelectListListEnter);
  }

  WndOwnerDrawFrame* pListEntry = (WndOwnerDrawFrame*) pForm->FindByName(TEXT("frmMultiSelectListListEntry"));
  if(pListEntry) {
    /*
    * control height must contains 2 text Line
    * Check and update Height if necessary
    */
    LKWindowSurface windowSurface(*main_window);
    LKBitmapSurface tmpSurface(windowSurface, 1, 1);
    const auto oldFont = tmpSurface.SelectObject(pListEntry->GetFont());
    const int minHeight = 2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
    tmpSurface.SelectObject(oldFont);

    const int wHeight = pListEntry->GetHeight();
    if(minHeight > wHeight) {
      pListEntry->SetHeight(minHeight);
    }
    pListEntry->SetCanFocus(true);
  }

  if(pList) {
    UpdateList(pList);
  }

  int result = pForm->ShowModal();

  iNO_Tasks = 0;

  return result;
}

