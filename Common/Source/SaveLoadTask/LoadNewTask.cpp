/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "dlgTools.h"
#include "Waypointparser.h"


extern bool FullResetAsked;


/*******************************************************/
/* this exist only for compatibility with OLD tsk file */

typedef struct _OLD_TASK_POINT
{
  int Index;
  double InBound;
  double OutBound;
  double Bisector;
  double Leg;
  double SectorStartLat;
  double SectorStartLon;
  double SectorEndLat;
  double SectorEndLon;
  POINT	 Start;
  POINT	 End;		
  int	 AATType;
  double AATCircleRadius;
  double AATSectorRadius;
  double AATStartRadial;
  double AATFinishRadial;
  double AATStartLat;
  double AATStartLon;
  double AATFinishLat;
  double AATFinishLon;
  POINT	 AATStart;
  POINT	 AATFinish;
  double AATTargetOffsetRadius;
  double AATTargetOffsetRadial;
  double AATTargetLat;
  double AATTargetLon;
  POINT	 Target;
  bool   AATTargetLocked;
}OLD_TASK_POINT;

#define OLD_MAXTASKPOINTS 20

/*******************************************************/

bool LoadTaskWaypoints(HANDLE hFile) {
  WAYPOINT read_waypoint;
  DWORD dwBytesRead;

  int i;
  for(i=0;i<OLD_MAXTASKPOINTS;i++) {
    if(!ReadFile(hFile,&read_waypoint,sizeof(read_waypoint),&dwBytesRead, (OVERLAPPED *)NULL)
       || (dwBytesRead<sizeof(read_waypoint))) {
      return false;
    }
    if (Task[i].Index != -1) { //  091213 CHECK do not load reserved WP
        
 // WE SHOULD NOT HAVE ANY VALID Details or Comment here.
 // In case of old tsk, we shall have still non-null data, but it is a mistake 
 // and this is why we set it to NULL here, the pointer is fake, saved from old session.
         
      read_waypoint.Comment = NULL;        
      read_waypoint.Details = NULL;
      Task[i].Index = FindOrAddWaypoint(&read_waypoint,false);
    }
  }
  for(i=0;i<MAXSTARTPOINTS;i++) {
    if(!ReadFile(hFile,&read_waypoint,sizeof(read_waypoint),&dwBytesRead, (OVERLAPPED *)NULL)
       || (dwBytesRead<sizeof(read_waypoint))) {
      return false;
    }
    if (StartPoints[i].Index != -1) {
// WE SHOULD NOT HAVE ANY VALID Details or Comment here.
 // In case of old tsk, we shall have still non-null data, but it is a mistake 
 // and this is why we set it to NULL here, the pointer is fake, saved from old session.
      read_waypoint.Comment = NULL;        
      read_waypoint.Details = NULL;
      StartPoints[i].Index = FindOrAddWaypoint(&read_waypoint,false);
    }
  }
  // managed to load everything
  return true;
}


// Loads a new task from scratch.
// This is called on startup by the even manager because in DEFAULT MENU we have a GCE event
// configured to load Default.tsk for STARTUP_SIMULATOR and STARTUP_REAL.
// Until we change this, which would be a good thing because these configuration are unnecessary ,
// we must use the FullResetAsked trick.
void LoadNewTask(LPCTSTR szFileName)
{
  HANDLE hFile;
  TASK_POINT Temp;
  START_POINT STemp;
  DWORD dwBytesRead;
  int i;
  bool TaskInvalid = false;
  bool WaypointInvalid = false;
  bool TaskLoaded = false;
  char taskinfo[LKPREAMBOLSIZE+1]; // 100207
  bool oldversion=false; // 100207
  TCHAR taskFileName[MAX_PATH];

  LockTaskData();

  ClearTask();
  if (FullResetAsked) {
	#if TESTBENCH
	StartupStore(_T("... LoadNewTask detected FullResetAsked, attempt to load DEMO.TSK\n"));
	#endif
	// Clear the flag, forever.
  	FullResetAsked=false;
	_tcscpy(taskFileName,_T("%LOCAL_PATH%\\\\_Tasks\\DEMO.TSK"));
	ExpandLocalPath(taskFileName);

  } else {
	_tcscpy(taskFileName,szFileName);
  }
  
  StartupStore(_T(". LoadNewTask <%s>%s"),taskFileName,NEWLINE);

  hFile = CreateFile(taskFileName,GENERIC_READ,0, (LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);

  if(hFile!= INVALID_HANDLE_VALUE )
    {

      // Defaults
      int   old_StartLine    = StartLine;
      int   old_SectorType   = SectorType;
      DWORD old_SectorRadius = SectorRadius;
      DWORD old_StartRadius  = StartRadius;
      int   old_AutoAdvance  = AutoAdvance;
      double old_AATTaskLength = AATTaskLength;
      BOOL   old_AATEnabled  = AATEnabled;
      DWORD  old_FinishRadius = FinishRadius;
      int    old_FinishLine = FinishLine;
      bool   old_EnableMultipleStartPoints = EnableMultipleStartPoints;

      TaskLoaded = true;

	if(!ReadFile(hFile,&taskinfo,LKPREAMBOLSIZE,&dwBytesRead, (OVERLAPPED *)NULL)) {
		TaskInvalid = true;
		goto goEnd;
	}

	// task version check
	if ( (taskinfo[0]!= 'L') || (taskinfo[1]!= 'K') || (taskinfo[2]!=LKTASKVERSION) ) { 
		TaskInvalid = true;
		oldversion = true;
		goto goEnd;
	}

      for(i=0;i<OLD_MAXTASKPOINTS;i++)
        {
          if(!ReadFile(hFile,&Temp,sizeof(OLD_TASK_POINT),&dwBytesRead, (OVERLAPPED *)NULL))
            {
              TaskInvalid = true;
              break;
            }
          if(i < MAXTASKPOINTS) {
            memcpy(&Task[i],&Temp, sizeof(OLD_TASK_POINT));

            if( !ValidNotResWayPoint(Temp.Index) && (Temp.Index != -1) ) { // 091213
                // Task is only invalid here if the index is out of range
                // of the waypoints and not equal to -1.
                // (Because -1 indicates a null task item)
                WaypointInvalid = true; 
        	}
          }
        }

      if (!TaskInvalid) {

	if (!ReadFile(hFile,&AATEnabled,sizeof(BOOL),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&AATTaskLength,sizeof(double),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	
	// ToDo review by JW
	
	// 20060521:sgi added additional task parameters
	if (!ReadFile(hFile,&FinishRadius,sizeof(FinishRadius),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&FinishLine,sizeof(FinishLine),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&StartRadius,sizeof(StartRadius),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&StartLine,sizeof(StartLine),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&SectorType,sizeof(SectorType),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&SectorRadius,sizeof(SectorRadius),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&AutoAdvance,sizeof(AutoAdvance),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }

        if (!ReadFile(hFile,&EnableMultipleStartPoints,sizeof(bool),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }

        for(i=0;i<MAXSTARTPOINTS;i++)
        {
          if(!ReadFile(hFile,&STemp,sizeof(START_POINT),&dwBytesRead, (OVERLAPPED *)NULL)) {
            TaskInvalid = true;
            break;
          }
	  
          if( ValidNotResWayPoint(STemp.Index) || (STemp.Index==-1) ) { // 091213
            memcpy(&StartPoints[i],&STemp, sizeof(START_POINT));
          } else {
	    WaypointInvalid = true;
		StartupStore(_T("--- LoadNewTask: invalid waypoint=%d found%s"),STemp.Index,NEWLINE); // 091213
	  }
        }

        // search for waypoints...
        if (!TaskInvalid) {
          if (!LoadTaskWaypoints(hFile) && WaypointInvalid) {
            // couldn't lookup the waypoints in the file and we know there are invalid waypoints
            TaskInvalid = true;
            StartupStore(_T(". LoadTaskNew: cant locate waypoint in file, and invalid wp in task file%s"),NEWLINE);
          }
        }

      }
      
        // TimeGate config
        if (!TaskInvalid) {
            TaskInvalid = !ReadFile(hFile, &PGOpenTimeH, sizeof (PGOpenTimeH), &dwBytesRead, (OVERLAPPED*) NULL);
        }
        if (!TaskInvalid) {
            TaskInvalid = !ReadFile(hFile, &PGOpenTimeM, sizeof (PGOpenTimeM), &dwBytesRead, (OVERLAPPED*) NULL);
        }
        if (!TaskInvalid) {
            InitActiveGate();

			// PGOpenTime is Calculated !
			int tmp;
            TaskInvalid = !ReadFile(hFile, &tmp, sizeof (tmp), &dwBytesRead, (OVERLAPPED*) NULL);
        }
        if (!TaskInvalid) {
			PGCloseTime=86399;
			
			// PGCloseTime is Calculated !
			int tmp;
            TaskInvalid = !ReadFile(hFile, &tmp, sizeof (tmp), &dwBytesRead, (OVERLAPPED*) NULL);
        }
        if (!TaskInvalid) {
            TaskInvalid = !ReadFile(hFile, &PGGateIntervalTime, sizeof (PGGateIntervalTime), &dwBytesRead, (OVERLAPPED*) NULL);
        }
        if (!TaskInvalid) {
            TaskInvalid = !ReadFile(hFile, &PGNumberOfGates, sizeof (PGNumberOfGates), &dwBytesRead, (OVERLAPPED*) NULL);
        }
        if (!TaskInvalid) {
            TaskInvalid = !ReadFile(hFile, &PGStartOut, sizeof (PGStartOut), &dwBytesRead, (OVERLAPPED*) NULL);
        }    

goEnd:

      CloseHandle(hFile);

      if (TaskInvalid) {
	if (oldversion)
		StartupStore(_T("------ Task is invalid: old task format%s"),NEWLINE);
	else
		StartupStore(_T("------ Task is invalid%s"),NEWLINE);

        StartLine = old_StartLine;
        SectorType = old_SectorType;
        SectorRadius = old_SectorRadius;
        StartRadius = old_StartRadius;
        AutoAdvance = old_AutoAdvance;
        AATTaskLength = old_AATTaskLength;
        AATEnabled = old_AATEnabled;
        FinishRadius = old_FinishRadius;
        FinishLine = old_FinishLine;
        EnableMultipleStartPoints = old_EnableMultipleStartPoints;
      }

  } else {
    StartupStore(_T("... LoadNewTask: file <%s> not found%s"),taskFileName,NEWLINE); // 091213
    TaskInvalid = true;
  }
  
  if (TaskInvalid) {
    ClearTask();
  } 

  RefreshTask();
  
  if (!ValidTaskPoint(0)) {
    ActiveWayPoint = 0;
  }

  UnlockTaskData();

  if (TaskInvalid && TaskLoaded) {
	if (oldversion) {
	// LKTOKEN  _@M360_ = "Invalid old task format!" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M360_")), 
	// LKTOKEN  _@M396_ = "Load task" 
			gettext(TEXT("_@M396_")), MB_OK|MB_ICONEXCLAMATION);
	} else {
	// LKTOKEN  _@M264_ = "Error in task file!" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M264_")), 
	// LKTOKEN  _@M396_ = "Load task" 
			gettext(TEXT("_@M396_")), MB_OK|MB_ICONEXCLAMATION);
	}
  } else {
	#if TESTBENCH
	StartupStore(_T("------ Task is Loaded%s"),NEWLINE);
	#endif
	TaskModified = false; 
	TargetModified = false;
	_tcscpy(LastTaskFileName, taskFileName);
  }

}
