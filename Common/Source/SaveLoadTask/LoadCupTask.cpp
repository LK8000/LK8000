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

int dlgTaskSelectListShowModal(void) ;

static WndForm *wf = NULL;
static WndListFrame *wTaskSelectListList = NULL;
static WndOwnerDrawFrame *wTaskSelectListListEntry = NULL;
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

bool LoadCupTask(LPCTSTR szFileName) {
  //  LockTaskData();

  mapCode2Waypoint_t mapWaypoint;

  //  ClearTask();
  size_t idxTP = 0;
  bool bTakeOff = true;
  bool bLoadComplet = true;
  bool bLastInvalid=true;
  std::vector<tstring> Entries;
  TCHAR szString[READLINE_LENGTH + 1];
  TCHAR TpCode[NAME_SIZE + 1];

  szString[READLINE_LENGTH] = _T('\0');
  TpCode[NAME_SIZE] = _T('\0');

  memset(szString, 0, sizeof (szString)); // clear Temp Buffer
  WAYPOINT newPoint = {0};
  WAYPOINT* WPtoAdd=NULL;

  enum {
    none, Waypoint, TaskTp, Option
  } FileSection = none;
  zzip_stream stream(szFileName, "rt");
  iNO_Tasks =0;
  TaskIndex =0;
  for (int i =0 ; i< MAX_TASKS;i++)
    szTaskStrings[ i] = NULL;
#define MULTITASKS_CUP
#ifdef MULTITASKS_CUP
    if (stream) {
      while (stream.read_line(szString)) {

          if ((FileSection == none) && ((_tcsncmp(_T("name,code,country"), szString, 17) == 0) ||
              (_tcsncmp(_T("Title,Code,Country"), szString, 18) == 0))) {
              FileSection = Waypoint;
              continue;
          } else if ((FileSection == Waypoint) && (_tcscmp(szString, _T("-----Related Tasks-----")) == 0)) {
              FileSection = TaskTp;
              continue;
          }


          if(  FileSection == TaskTp)
            {
              if(_tcsstr(szString, _T("\",\""))!= NULL)   // really a task? (not an option)
                {
                  if(iNO_Tasks < MAX_TASKS)   // Space in List left
                    {//[READLINE_LENGTH + 1];
                      szTaskStrings[ iNO_Tasks] =  new TCHAR[READLINE_LENGTH + 1];
                      if(  szTaskStrings[ iNO_Tasks] != NULL)
                      {
                        _tcscpy(szTaskStrings[ iNO_Tasks] , szString);  // copy task string
                         // StartupStore(_T("..Cup Task : %s  %s"), szTaskStrings[ iNO_Tasks], NEWLINE);
                        iNO_Tasks++;
                      }
                      else
                        StartupStore(_T("..Cup Task: no memory %s"), NEWLINE);
                    }
                  else
                    StartupStore(_T("..Cup Task Too many Tasks (more than %i) %s"), MAX_TASKS, NEWLINE);
                }
            }
      }
      stream.close();
      StartupStore(_T("..Cup Selected Task:%i %s  %s"), TaskIndex, szTaskStrings[ TaskIndex] , NEWLINE);
    }

  int res = 0;
  if(iNO_Tasks >1)   // Selection only if more than one task found
    res = dlgTaskSelectListShowModal();

  for (int i =0 ; i< MAX_TASKS;i++)    // free dynamic memory
    if(szTaskStrings[i] != NULL)
      {
        // StartupStore(_T("..Cup Task : delete dynamic memoryLine %i %s"), i,NEWLINE);
        delete[] szTaskStrings[i];
        szTaskStrings[i] = NULL;
      }
   if(res == mrCancel)
     return false;

  /***********************************************************************************/

  LockTaskData();
  ClearTask();
  stream.open(szFileName, "rt");
#endif

  FileSection = none;
  int i=0;
  if (stream) {
      while (stream.read_line(szString)) {

          if ((FileSection == none) && ((_tcsncmp(_T("name,code,country"), szString, 17) == 0) ||
              (_tcsncmp(_T("Title,Code,Country"), szString, 18) == 0))) {
              FileSection = Waypoint;
              continue;
          } else if ((FileSection == Waypoint) && (_tcscmp(szString, _T("-----Related Tasks-----")) == 0)) {
              FileSection = TaskTp;

              continue;
          }

          TCHAR *pToken = NULL;
          TCHAR *pWClast = NULL;

          switch (FileSection) {
          case Waypoint:
            memset(&newPoint, 0, sizeof(newPoint));
            if (ParseCUPWayPointString(szString, &newPoint)) {
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
            WPtoAdd=NULL;
            Entries =   CupStringToFieldArray(szString);
            if(Entries[0].size() == 0)
              StartupStore(_T(". no Task name %s"), NEWLINE);
            else
              StartupStore(_T(". Task name %s %s"), Entries[0].c_str()  ,NEWLINE);

            if (i++ == TaskIndex)  // load selected task
              {
                uint Idx =1;

                while (bLoadComplet && (Idx < (Entries.size()))) {
                    if (idxTP < MAXTASKPOINTS) {
                    _sntprintf(TpCode,NAME_SIZE, _T("%s"), Entries[Idx++].c_str() );
                        mapCode2Waypoint_t::iterator It = mapWaypoint.find(TpCode);
                        if(!ISGAAIRCRAFT) {
                            if (It != mapWaypoint.end()) {
                                if (bTakeOff) {
                                    // skip TakeOff Set At Home Waypoint
                                    int ix = FindOrAddWaypoint(&(It->second),false);
                                    if (ix>=0) {
#if 0 // REMOVE
                                        // We must not change HomeWaypoint without user knowing!
                                        // The takeoff and homewaypoint are independent from task.
                                        // In addition, this is a bug because on next run the index is invalid
                                        // and we have no more HowWaypoint!
                                        HomeWaypoint = ix;
#endif
                                        bTakeOff = false;
                                    }
#if BUGSTOP
                                    else LKASSERT(0); // .. else is unmanaged, TODO
#endif
                                } else {
                                  if(Idx < Entries.size())
                                    if( _tcscmp(Entries[Idx].c_str(), TpCode) !=0) // doublets?
                                    {
                                      int ix =  FindOrAddWaypoint(&(It->second),false);
                                      if (ix>=0) Task[idxTP++].Index = ix;
                                    }
                                }
                                bLastInvalid=false;
                            } else {
                                // An invalid takeoff, probably a "???" , which we ignore
#if TESTBENCH
                                if (bTakeOff) StartupStore(_T("....... CUP Takeoff not found: <%s>\n"),TpCode);
#endif
                                // in any case bTakeOff now is false
                                bTakeOff=false;
                                bLastInvalid=true;
                            }
                        } else { //ISGAIRRCRAFT
                            if(It != mapWaypoint.end()) {
                                if(WPtoAdd!=NULL) {
                                    //add what we found in previous cycle: it was not the last one
                                    int ix = FindOrAddWaypoint(WPtoAdd,false);
                                    if (ix>=0) Task[idxTP++].Index = ix;
#if BUGSTOP
                                    else LKASSERT(0); // .. else is unmanaged, TODO
#endif
                                }
                                if (bTakeOff) { //it's the first: may be we have a corresponding airfield
                                    //look for departure airfield and add it
                                    int ix = FindOrAddWaypoint(&(It->second),true);
                                    if (ix>=0) {
                                        Task[idxTP++].Index = ix;
                                        bTakeOff = false;
                                    }
#if BUGSTOP
                                    else LKASSERT(0); // .. else is unmanaged, TODO
#endif
                                } else WPtoAdd=&(It->second); //store it for next cycle (may be it is the last one)
                            }
                        }
                    } else {
                        bLoadComplet = false;
                    }
                }
                if(ISGAAIRCRAFT) { //For GA: check if we have an airport corresponding to the last WP
                    if(WPtoAdd!=NULL) { //if we have the last one (probably an airfield) still to add...
                        if(idxTP<MAXTASKPOINTS) {
                            int ix=FindOrAddWaypoint(WPtoAdd,true); //look for arrival airport and add it
                            if (ix>=0) {
                                Task[idxTP++].Index= ix;
                            }
#if BUGSTOP
                            else LKASSERT(0); // .. else is unmanaged, TODO
#endif
                        }
                        else bLoadComplet=false;
                    }
                }
                FileSection = Option;
              }
            break;
          case Option:
            if ((pToken = strsep_r(szString, TEXT(","), &pWClast)) != NULL) {
                if (_tcscmp(pToken, _T("Options")) == 0) {
                    while ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                        if (_tcsstr(pToken, _T("NoStart=")) == pToken) {
                            // Opening of start line
                            PGNumberOfGates = 1;
                            StrToTime(pToken + 8, &PGOpenTimeH, &PGOpenTimeM);
                        } else if (_tcsstr(pToken, _T("TaskTime=")) == pToken) {
                            // Designated Time for the task
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
                    TmpZone.mIdx = _tcstol(pToken + 8, &sz, 10);
                    if (TmpZone.mIdx < MAXTASKPOINTS) {
                        while ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                            if (_tcsstr(pToken, _T("Style=")) == pToken) {
                                // Direction. 0 - Fixed value, 1 - Symmetrical, 2 - To next point, 3 - To previous point, 4 - To start point
                                TmpZone.mType = _tcstol(pToken + 6, &sz, 10);
                            } else if (_tcsstr(pToken, _T("R1=")) == pToken) {
                                // Radius 1
                                TmpZone.mR1 = ReadLength(pToken + 3);
                            } else if (_tcsstr(pToken, _T("A1=")) == pToken) {
                                // Angle 1 in degrees
                                TmpZone.mA1 = _tcstod(pToken + 3, &sz);
                            } else if (_tcsstr(pToken, _T("R2=")) == pToken) {
                                // Radius 2
                                TmpZone.mR2 = ReadLength(pToken + 3);
                            } else if (_tcsstr(pToken, _T("A2=")) == pToken) {
                                // Angle 2 in degrees
                                TmpZone.mA2 = _tcstod(pToken + 3, &sz);
                            } else if (_tcsstr(pToken, _T("A12=")) == pToken) {
                                // Angle 12
                                TmpZone.mA12 = _tcstod(pToken + 4, &sz);
                            } else if (_tcsstr(pToken, _T("Line=")) == pToken) {
                                // true For Line Turmpoint type
                                // Exist only for start an Goalin LK
                                TmpZone.mLine = (_tcstol(pToken + 5, &sz, 10) == 1);
                            }
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

  return ValidTaskPoint(0);
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


static void UpdateList(void) {
  wTaskSelectListList->ResetList();
  wTaskSelectListList->Redraw();
}

static void OnUpClicked(WndButton* Sender) {
  if (TaskIndex > 0) {
      TaskIndex--;
  } else {
      LKASSERT(iNO_Tasks>0);
      TaskIndex = (iNO_Tasks - 1);
  }
  wTaskSelectListList->SetItemIndexPos(TaskIndex);
  wTaskSelectListList->Redraw();
  wTaskSelectListListEntry->SetFocus();
}

static void OnDownClicked(WndButton* pWnd) {

  (void)pWnd;

  if (TaskIndex < (iNO_Tasks - 1)) {
      TaskIndex++;
  } else {
      TaskIndex = 0;
  }
  wTaskSelectListList->SetItemIndexPos(TaskIndex);
  wTaskSelectListList->Redraw();
  wTaskSelectListListEntry->SetFocus();
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
      TCHAR *pToken = NULL;
      TCHAR *pWClast = NULL;
      TCHAR *pWClast2 = NULL;
      TCHAR text[180] = {TEXT("empty")};
      TCHAR text1[180] = {TEXT("empty")};
      TCHAR text2[180] = {TEXT("empty")};

      _tcscpy(text, szTaskStrings [TaskDrawListIndex] );
    unsigned int i=0;
    while (i < _tcslen(text) )  // remove all quotations "
    {
        if(text[i]== '"')    //  quotations found ?
        {
            for (unsigned int j= i ; j < _tcslen(text); j++)
            text[j] =  text[j+1];
        }
        i++;
    }
    pToken = strsep_r(text, TEXT(","), &pWClast) ;
    _tcscpy(text1, pToken );
    if(*text1 == '\0')   _tcscpy(text1, _T("???") );

    pToken = strsep_r(pWClast, TEXT(","), &pWClast2) ;  // remove takeof point
    _tcscpy(text2, pWClast2);

      Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));


      PixelRect rc = {
          0,
          0,
          0, // DLGSCALE(PICTO_WIDTH),
          static_cast<PixelScalar>(Sender->GetHeight())
      };

      /********************
       * show text
       ********************/
      Surface.SetBackgroundTransparent();
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(rc.right + DLGSCALE(2), DLGSCALE(2), text1);
      int ytext2 = Surface.GetTextHeight(text1);
      Surface.SetTextColor(RGB_DARKBLUE);
      Surface.DrawText(rc.right + DLGSCALE(2), ytext2, text2);
  }
}



static void OnTaskSelectListListEnter(WindowControl * Sender,
    WndListFrame::ListInfo_t *ListInfo) {
  (void) Sender;

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


int dlgTaskSelectListShowModal(void) {

  TaskIndex = -1;

  if (iNO_Tasks == 0) return mrCancel;


  wf = dlgLoadFromXML(TaskCallBackTable, ScreenLandscape ? IDR_XML_MULTISELECTLIST_L : IDR_XML_MULTISELECTLIST_P);

  if (!wf)   return mrCancel;

  wTaskSelectListList = (WndListFrame*) wf->FindByName(TEXT("frmMultiSelectListList"));
  LKASSERT(wTaskSelectListList != NULL);
  wTaskSelectListList->SetBorderKind(BORDERLEFT);
  wTaskSelectListList->SetEnterCallback(OnTaskSelectListListEnter);

  wTaskSelectListListEntry = (WndOwnerDrawFrame*) wf->FindByName(TEXT("frmMultiSelectListListEntry"));
  if(wTaskSelectListListEntry) {
      /*
       * control height must contains 2 text Line
       * Check and update Height if necessary
       */
      LKWindowSurface windowSurface(MainWindow);
      LKBitmapSurface tmpSurface(windowSurface, 1, 1);
      const auto oldFont = tmpSurface.SelectObject(wTaskSelectListListEntry->GetFont());
      const int minHeight = 2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
      tmpSurface.SelectObject(oldFont);
      const int wHeight = wTaskSelectListListEntry->GetHeight();
      if(minHeight > wHeight) {
          wTaskSelectListListEntry->SetHeight(minHeight);
      }
      wTaskSelectListListEntry->SetCanFocus(true);
  } else LKASSERT(0);

  UpdateList();

  int result = wf->ShowModal();
  wTaskSelectListList->Redraw();
  delete wf;

  wf = NULL;

  iNO_Tasks = 0;

  return result;
}


