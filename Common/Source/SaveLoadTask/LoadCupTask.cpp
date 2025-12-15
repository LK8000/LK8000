/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "utils/zzip_file_stream.h"

#include "WindowControls.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "resource.h"
#include "InputEvents.h"
#include "Util/UTF8.hpp"
#include "utils/tokenizer.h"
#include "utils/printf.h"
#include "utils/charset_helper.h"
#include "Util/Clamp.hpp"
#include "Calc/Task/TimeGates.h"

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
                StartLine = sector_type_t::CIRCLE;
                StartRadius = mR1;
            } else if (mIdx == (size_t) getFinalWaypoint()) {
                FinishLine = sector_type_t::CIRCLE;
                FinishRadius = mR1;
            } else {
                Task[mIdx].AATType = sector_type_t::CIRCLE;
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
            StartLine = sector_type_t::SECTOR;
            StartRadius = mR1;
        } else if (mIdx == (size_t) getFinalWaypoint()) {
            FinishLine = sector_type_t::SECTOR;
            FinishRadius = mR1;
        } else {
            Task[mIdx].AATType = sector_type_t::SECTOR;
            Task[mIdx].AATSectorRadius = mR1;
            Task[mIdx].AATStartRadial = mA12 - 180.0 - (mA1);
            Task[mIdx].AATFinishRadial = mA12 - 180.0 + (mA1);
        }
    }

    void UpdateSymLine() {
        if (mIdx == 0) {
            StartLine = sector_type_t::LINE;
            StartRadius = mR1;
        } else if (mIdx == (size_t) getFinalWaypoint()) {
            FinishLine = sector_type_t::LINE;
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

namespace {

using field_iterator = std::vector<std::string>::const_iterator;

bool ConvertStringToTask(field_iterator begin, field_iterator end, mapCode2Waypoint_t &mapWaypoint) {
    size_t idxTP = 0;

    std::for_each(begin, end, [&](const auto& code) {
        if (idxTP >= MAXTASKPOINTS) {
            return;
        }
        mapCode2Waypoint_t::iterator It = mapWaypoint.find(from_unknown_charset(code.c_str()));
        if (It != mapWaypoint.end()) {
            int ix = FindOrAddWaypoint(&(It->second), true);  // first try to find airfield
            if (ix < 0) {
                ix = FindOrAddWaypoint(&(It->second), false); // not found try if waypoint exist
            }
            if (ix >= 0) { // valid TP
                Task[idxTP++].Index = ix;
                if (idxTP > 1) {
                    if (Task[idxTP - 1].Index == Task[idxTP - 2].Index) {  // not the same as before?
                        --idxTP;
                    }
                }
            }
        }
    });
    Task[idxTP].Index = -1;
    return (idxTP > 0);
}

template<size_t size>
bool LoadCupTaskSingle(LPCTSTR szFileName, TCHAR (&TaskLine)[size], int SelectedTaskIndex) {


  mapCode2Waypoint_t mapWaypoint;

  bool TaskFound = false;
  bool bLoadComplet = true;
  std::vector<std::string> Entries;
  WAYPOINT newPoint = {};

  enum { none, Waypoint, TaskTp, Option } FileSection = none;

  zzip_file_stream stream(szFileName, "rt");


  LockTaskData();
  ClearTask();

  cup_header_t cup_header;

  bool TaskValid = false;
  FileSection = none;
  int i=0;
  if (stream) {
    std::istream in(&stream);
    std::string src_line;

    while (std::getline(in, src_line)) {
      if ((FileSection == none) &&
          (src_line.starts_with("name,code,country") ||
           src_line.starts_with("Title,Code,Country"))) {
        FileSection = Waypoint;
        cup_header = CupStringToHeader(src_line);
        continue;
      }
      else if ((FileSection == Waypoint) &&
               (src_line == "-----Related Tasks-----")) {
        FileSection = TaskTp;

        continue;
      }
      if (FileSection == TaskTp) {
        if (src_line.find("\",\"") == std::string::npos) {  // really a task? (not an option)
          FileSection = Option;
        }
      }
      if (FileSection == Option) {
        if (src_line.find("\",\"") != std::string::npos) {  // really a task? (not an option)
          FileSection = TaskTp;
        }
      }
      std::vector<std::string>::const_iterator pToken;

      int hh, mm, ss;

      if (FileSection != Waypoint) {
        Entries = CupStringToFieldArray(src_line);
        pToken = Entries.begin();
      }

      switch (FileSection) {
        case Waypoint:
          newPoint = {};
          if (ParseCUPWayPointString(cup_header, src_line, &newPoint)) {
            mapWaypoint[newPoint.Name] = newPoint;
          }
          break;
        case TaskTp:
          // 1. Description
          //       First column is the description of the task. If filled it
          //       should be double quoted. If left empty, then SeeYou will
          //       determine the task type on runtime.
          // 2. and all successive columns, separated by commas
          //       Each column represents one waypoint name double quoted. The
          //       waypoint name must be exactly the same as the Long name of a
          //       waypoint listed above the Related tasks.

          if (i++ != (SelectedTaskIndex)) {
            TaskValid = false; // load selected task
          }
          else {
            TaskValid = true;
            TaskFound = true;
            SectorType = sector_type_t::SECTOR;  // normal sector by default if
                                                 // no other ObsZone parameter
            gTaskType = task_type_t::DEFAULT;  // racing task by default, if AAT
                                               // will overwrite this

            if (Entries.empty()) {
              TaskValid = false;
              break;
            }
            if (Entries[0].size() == 0) {
              StartupStore(_T(". no Task name, named it %s%i"), MsgToken<699>(),
                           i);  // _@M699_ "Task"
              tstring string = from_unknown_charset(src_line.c_str());
              lk::snprintf(TaskLine, _T("%s%i%s"),
                           MsgToken<699>(), i,
                           string.c_str());  // _@M699_ "Task"
            }
            else {
              tstring string = from_unknown_charset(src_line.c_str());
              lk::strcpy(TaskLine, string.c_str());
            }

            bLoadComplet = ConvertStringToTask(std::next(Entries.begin()),
                                               Entries.end(), mapWaypoint);
            FileSection = Option;
          }
          break;
        case Option:
          if (TaskValid)  // load option for selected task only
          {
            if (pToken->starts_with("Options")) {
              while (++pToken != Entries.end()) {
                if (pToken->starts_with("NoStart=")) {
                  TimeGates::GateType = TimeGates::fixed_gates;

                  // Opening of start line
                  TimeGates::PGNumberOfGates = 1;
                  StrToTime(pToken->c_str() + 8, &TimeGates::PGOpenTimeH,
                            &TimeGates::PGOpenTimeM);
                }
                else if (pToken->starts_with("NoFinish=")) {
                  // Designated Time for the task
                  StrToTime(pToken->c_str() + 9, &hh, &mm, &ss);
                  //			StartupStore(_T("..Cup Task
                  //Time:(%02i:%02i) %imin  %s"),hh,mm,(hh*60+mm), NEWLINE);
                  AATTaskLength = hh * 60 + mm + ss / 60;

                  // TODO :
                }
                else if (pToken->starts_with("WpDis=")) {
                  // Task distance calculation. False = use fixes, True = use
                  // waypoints
                  // TODO :
                }
                else if (pToken->starts_with("NearDis=")) {
                  // Distance tolerance
                  // TODO :
                }
                else if (pToken->starts_with("NearAlt=")) {
                  // Altitude tolerance
                  // TODO :
                }
                else if (pToken->starts_with("MinDis=")) {
                  // Uncompleted leg.
                  // False = calculate maximum distance from last observation
                  // zone.
                  // TODO :
                }
                else if (pToken->starts_with("RandomOrder=")) {
                  // if true, then Random order of waypoints is checked
                  // TODO :
                }
                else if (pToken->starts_with("MaxPts=")) {
                  // Maximum number of points
                  // TODO :
                }
                else if (pToken->starts_with("BeforePts=")) {
                  // Number of mandatory waypoints at the beginning. 1 means
                  // start line only, two means
                  //      start line plus first point in task sequence (Task
                  //      line).
                  // TODO :
                }
                else if (pToken->starts_with("AfterPts=")) {
                  // Number of mandatory waypoints at the end. 1 means finish
                  // line only, two means finish line
                  //      and one point before finish in task sequence (Task
                  //      line).
                  // TODO :
                }
                else if (pToken->starts_with("Bonus=")) {
                  // Bonus for crossing the finish line
                  // TODO :
                }
              }
            }
            else if (pToken->starts_with("ObsZone=")) {
              CupObsZoneUpdater TmpZone;
              TmpZone.mIdx = strtol(pToken->c_str() + 8, NULL, 10);
              if (TmpZone.mIdx < MAXTASKPOINTS) {
                while (++pToken != Entries.end()) {
                  if (pToken->starts_with("Style=")) {
                    // Direction. 0 - Fixed value, 1 - Symmetrical, 2 - To next
                    // point, 3 - To previous point, 4 - To start point
                    TmpZone.mType = strtol(pToken->c_str() + 6, nullptr, 10);
                  }
                  else if (pToken->starts_with("R1=")) {
                    // Radius 1
                    TmpZone.mR1 = ReadLength(pToken->c_str() + 3);
                  }
                  else if (pToken->starts_with("A1=")) {
                    // Angle 1 in degrees
                    TmpZone.mA1 = strtod(pToken->c_str() + 3, nullptr);
                    if (TmpZone.mA1 > 179.5)  //  180° = Circle sector
                      if (TmpZone.mA1 < 180.5)
                        SectorType = sector_type_t::CIRCLE;
                  }
                  else if (pToken->starts_with("R2=")) {
                    // Radius 2
                    TmpZone.mR2 = ReadLength(pToken->c_str() + 3);
                  }
                  else if (pToken->starts_with("A2=")) {
                    // Angle 2 in degrees
                    TmpZone.mA2 = strtod(pToken->c_str() + 3, nullptr);
                  }
                  else if (pToken->starts_with("A12=")) {
                    // Angle 12
                    TmpZone.mA12 = strtod(pToken->c_str() + 4, nullptr);
                  }
                  else if (pToken->starts_with("AAT=")) {
                    // AAT
                    gTaskType = task_type_t::AAT;
                    if (strtod(pToken->c_str() + 4, nullptr) > 0)  // AAT = 1?
                      SectorType = sector_type_t::CIRCLE;
                  }
                  else if (pToken->starts_with("Line=")) {
                    // true For Line Turmpoint type
                    // Exist only for start an Goalin LK
                    TmpZone.mLine = (strtol(pToken->c_str() + 5, nullptr, 10) == 1);
                  }
                }
                if (TmpZone.mR1 > 0)    // if both radius defined
                  if (TmpZone.mR2 > 0)  // must be keyhole definition
                    SectorType = sector_type_t::DAe;  // the only similar
                                                      // sectortype in LK

                if (TmpZone.mLine)  // line has priority (Start & Finish) if
                                    // multiple definitions
                {
                  TmpZone.mA1 = 90;
                  TmpZone.mA2 = 0;
                  TmpZone.mA12 = 0;
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
          src_line.clear(); // clear Temp Buffer
    }
  }
  if(!ISGAAIRCRAFT) {
      // Landing don't exist in LK Task Systems, so Remove It;
      if ( bLoadComplet ) {
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

const TCHAR* TaskTypeLabel(size_t count) {
  static const MsgToken_t label_array[] = {
          &MsgToken<2430>,
          &MsgToken<2431>,
          &MsgToken<2432>,
          &MsgToken<2433>,
          &MsgToken<2434>,
          &MsgToken<2435>,
          &MsgToken<2436>,
          &MsgToken<2437>,
  };

  size_t label_index = Clamp<size_t>(count, 1U, std::size(label_array)) - 1;
  return label_array[label_index]();
}

} // namespace

bool LoadCupTask(LPCTSTR szFileName) {
TCHAR szString[READLINE_LENGTH + 1];
  for (int i =0 ; i< MAX_TASKS;i++)
    szTaskStrings[ i] = NULL;

  iNO_Tasks=0;

  SaveDefaultTask(); // save current task to restore if no task load
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

    if(  szTaskStrings[ iNO_Tasks] != NULL)
    {
      if(bClosedTask)
      {
          if (gTaskType == task_type_t::AAT)
              lk::snprintf(szTaskStrings[iNO_Tasks], READLINE_LENGTH, _T("[AAT %.1f%s] %s"), Units::ToDistance(lengthtotal), Units::GetDistanceName(), szString); // _@M699_ "Task"
          else if (CALCULATED_INFO.TaskFAI)
              lk::snprintf(szTaskStrings[iNO_Tasks], READLINE_LENGTH, _T("[FAI %s %.1f%s] %s"), MsgToken<2432>(), Units::ToDistance(lengthtotal), Units::GetDistanceName(), szString); // _@M2432_ "Triangle"
          else
              lk::snprintf(szTaskStrings[iNO_Tasks], READLINE_LENGTH, _T("[%s %.1f%s] %s"), TaskTypeLabel(NoPts), Units::ToDistance(lengthtotal), Units::GetDistanceName(), szString);
      }
      else
      {
        lk::snprintf(szTaskStrings[ iNO_Tasks] ,READLINE_LENGTH, _T("[%s %.1f%s] %s"), MsgToken<2429>() ,Units::ToDistance(lengthtotal), Units::GetDistanceName(), szString);
      }
    }
    UnlockTaskData();
    iNO_Tasks++;
  }
  TaskIndex =0;
  InputEvents::eventTaskLoad(_T(LKF_DEFAULTASK)); //  restore old task
  dlgTaskSelectListShowModal();
  if((TaskIndex >= 0) && (TaskIndex < MAX_TASKS))
  {
      TCHAR file_name[180];
      const TCHAR * pToken = lk::tokenizer<TCHAR>(szTaskStrings[TaskIndex]).Next({_T(',')});
      if((pToken) && (_tcslen (pToken)>1)) {
          lk::snprintf(file_name, _T("%s %s ?"), MsgToken<891>(), pToken); // Clear old task and load taskname
      } else {
          lk::snprintf(file_name, _T("%s %s ?"), MsgToken<891>(), MsgToken<907>()); // Clear old task and load task
      }
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
  WndListFrame* pList = pForm->FindByName<WndListFrame>(TEXT("frmMultiSelectListList"));
  if(pList) {
    pList->SetItemIndexPos(index);
    pList->Redraw();
  }
  WndOwnerDrawFrame* pListEntry = pForm->FindByName<WndOwnerDrawFrame>(TEXT("frmMultiSelectListListEntry"));
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





static void OnMultiSelectListListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t *ListInfo) {

  if (ListInfo->DrawIndex == -1) {
      ListInfo->ItemCount = iNO_Tasks;

  } else {
      TaskDrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
      TaskIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }
}



static void OnMultiSelectListPaintListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface) {

#define PICTO_WIDTH 50

  Surface.SetTextColor(RGB_BLACK);
  if (TaskDrawListIndex < iNO_Tasks)  {
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

      lk::tokenizer<TCHAR> tok(text);
      TCHAR* pToken = tok.Next({_T(',')});
      if(pToken) {
        lk::strcpy(text1, pToken);
        if(*text1 == '\0') {
          lk::strcpy(text1, _T("???") );
        }
        tok.Next({_T(',')}); // remove takeof point
        lk::strcpy(text2, tok.Remaining());
      }
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
    CallbackEntry(OnMultiSelectListPaintListItem),
    CallbackEntry(OnMultiSelectListListInfo),
    CallbackEntry(OnCloseClicked),
    CallbackEntry(OnUpClicked),
    CallbackEntry(OnEnterClicked),
    CallbackEntry(OnDownClicked),
    EndCallbackEntry()
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

  WndListFrame* pList = pForm->FindByName<WndListFrame>(TEXT("frmMultiSelectListList"));
  if(pList) {
    pList->SetBorderKind(BORDERLEFT);
    pList->SetEnterCallback(OnTaskSelectListListEnter);
  }

  WndOwnerDrawFrame* pListEntry = pForm->FindByName<WndOwnerDrawFrame>(TEXT("frmMultiSelectListListEntry"));
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

