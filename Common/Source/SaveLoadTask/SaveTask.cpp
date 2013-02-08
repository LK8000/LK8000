/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "dlgTools.h"

void RenameIfVirtual(const unsigned int i);

void SaveTask(TCHAR *szFileName)
{
  HANDLE hFile;
  DWORD dwBytesWritten;
  char taskinfo[LKPREAMBOLSIZE+1];

  if (!WayPointList) return; // this should never happen, but just to be safe...

  LockTaskData();
  StartupStore(_T(". SaveTask: saving <%s>%s"),szFileName,NEWLINE); // 091112
        
  hFile = CreateFile(szFileName,GENERIC_WRITE,0, (LPSECURITY_ATTRIBUTES)NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
        
  if(hFile!=INVALID_HANDLE_VALUE ) {

	// add a version string inside task files
	sprintf(taskinfo,"LK%c%02d%02d___x_________x_________x_________x_________",LKTASKVERSION,MAXTASKPOINTS,MAXSTARTPOINTS);
	WriteFile(hFile,&taskinfo,LKPREAMBOLSIZE,&dwBytesWritten,(OVERLAPPED *)NULL);

	WriteFile(hFile,&Task[0],sizeof(TASK_POINT)*MAXTASKPOINTS,&dwBytesWritten,(OVERLAPPED *)NULL);
	WriteFile(hFile,&AATEnabled,sizeof(BOOL),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&AATTaskLength,sizeof(double),&dwBytesWritten,(OVERLAPPED*)NULL);
    
	// 20060521:sgi added additional task parameters
	WriteFile(hFile,&FinishRadius,sizeof(FinishRadius),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&FinishLine,sizeof(FinishLine),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&StartRadius,sizeof(StartRadius),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&StartLine,sizeof(StartLine),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&SectorType,sizeof(SectorType),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&SectorRadius,sizeof(SectorRadius),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&AutoAdvance,sizeof(AutoAdvance),&dwBytesWritten,(OVERLAPPED*)NULL);
	    
	WriteFile(hFile,&EnableMultipleStartPoints,sizeof(bool),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&StartPoints[0],sizeof(START_POINT)*MAXSTARTPOINTS,&dwBytesWritten,(OVERLAPPED*)NULL);
    
	// JMW added writing of waypoint data, in case it's missing
	int i;

	for(i=0;i<MAXTASKPOINTS;i++) {
		if (ValidWayPoint(Task[i].Index)) {
			RenameIfVirtual(Task[i].Index);
			WriteFile(hFile,&WayPointList[Task[i].Index],
			sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		} else {
			// dummy data..
			WriteFile(hFile,&WayPointList[NUMRESWP],
			sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		}
	}

	for(i=0;i<MAXSTARTPOINTS;i++) {
		if (ValidWayPoint(StartPoints[i].Index)) { // 091223
			RenameIfVirtual(StartPoints[i].Index);
			WriteFile(hFile,&WayPointList[StartPoints[i].Index], sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		} else {
			// dummy data..
			WriteFile(hFile,&WayPointList[NUMRESWP], sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		}
	}
    
    // Add TimeGate config /////////////////////////////////////////////////////////////////////
    WriteFile(hFile, &PGOpenTimeH, sizeof(PGOpenTimeH),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile, &PGOpenTimeM, sizeof(PGOpenTimeM),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile, &PGOpenTime, sizeof(PGOpenTime),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile, &PGCloseTime, sizeof(PGCloseTime),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile, &PGGateIntervalTime, sizeof(PGGateIntervalTime),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile, &PGNumberOfGates, sizeof(PGNumberOfGates),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile, &PGStartOut, sizeof(PGStartOut),&dwBytesWritten,(OVERLAPPED*)NULL);    
    ////////////////////////////////////////////////////////////////////////////////////////////
    
    
	CloseHandle(hFile);
	TaskModified = false; // task successfully saved
	TargetModified = false;
	_tcscpy(LastTaskFileName, szFileName);
	#if TESTBENCH
	StartupStore(_T(".... SaveTask: Ok%s"),NEWLINE);
	#endif

  } else {
    
	MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M263_ = "Error in saving task!" 
	gettext(TEXT("_@M263_")), gettext(TEXT("Save task")), MB_OK|MB_ICONEXCLAMATION);
	StartupStore(_T("++++++ SaveTask: ERROR saving task!%s"),NEWLINE);
  }
  UnlockTaskData();
}


//
// If it is a virtual waypoint, rename it before saving to task
//
void RenameIfVirtual(const unsigned int i) {

  if (i>RESWP_END) return;
  LKASSERT(i<=NumberOfWayPoints);

  if ( _tcslen(WayPointList[i].Name)> (NAME_SIZE-5)) return;

  TCHAR tmp[NAME_SIZE+10];
  _stprintf(tmp,_T("TSK_%s"),WayPointList[i].Name);
  LK_tcsncpy(WayPointList[i].Name,tmp,NAME_SIZE);
  if (WayPointList[i].Latitude==RESWP_INVALIDNUMBER && WayPointList[i].Longitude==RESWP_INVALIDNUMBER) {
	if (WayPointList[RESWP_TAKEOFF].Latitude!=RESWP_INVALIDNUMBER) {
		WayPointList[i].Latitude=WayPointList[RESWP_TAKEOFF].Latitude;
		WayPointList[i].Longitude=WayPointList[RESWP_TAKEOFF].Longitude;
		WayPointList[i].Altitude=WayPointList[RESWP_TAKEOFF].Altitude;
	}
  }
 
} 




