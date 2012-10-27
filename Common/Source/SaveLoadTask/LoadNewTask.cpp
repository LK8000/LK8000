/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "dlgTools.h"


bool LoadTaskWaypoints(HANDLE hFile);


// loads a new task from scratch.
void LoadNewTask(TCHAR *szFileName)
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

  LockTaskData();

  ActiveWayPoint = -1;
  for(i=0;i<MAXTASKPOINTS;i++) {
	Task[i].Index = -1;
  }
  
 StartupStore(_T(". LoadNewTask <%s>%s"),szFileName,NEWLINE);
  hFile = CreateFile(szFileName,GENERIC_READ,0,
                     (LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,NULL);
  
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

      for(i=0;i<MAXTASKPOINTS;i++)
        {
          if(!ReadFile(hFile,&Temp,sizeof(TASK_POINT),&dwBytesRead, (OVERLAPPED *)NULL))
            {
              TaskInvalid = true;
              break;
            }
	  memcpy(&Task[i],&Temp, sizeof(TASK_POINT));

          if( !ValidNotResWayPoint(Temp.Index) && (Temp.Index != -1) ) { // 091213
            // Task is only invalid here if the index is out of range
            // of the waypoints and not equal to -1.
            // (Because -1 indicates a null task item)
	        WaypointInvalid = true; 
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
    StartupStore(_T("... LoadNewTask: file <%s> not found%s"),szFileName,NEWLINE); // 091213
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
	TaskModified = false; 
	TargetModified = false;
	_tcscpy(LastTaskFileName, szFileName);
  }

}
