/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Process.h"
#include "utils/heapcheck.h"

#define DEBUGLB 1

// Returns false if something went wrong
bool UpdateLogBook(void) {

  FILE *stream;
  TCHAR filename[MAX_PATH];
  TCHAR Temp[100];
  char  line[100];

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKTXT);

  #if TESTBENCH
  StartupStore(_T("... UpdateLogBook <%s>\n"),filename);
  #endif
  //if (CALCULATED_INFO.FlightTime<=0) {
  if (0) {
	#if TESTBENCH
	StartupStore(_T("... UpdateLogBook: flight-time is zero!\n"),filename);
	#endif
	return true; // no problems, just a no-flight trigger
  }

  stream = _wfopen(filename,TEXT("a+"));
  if (stream == NULL) {
	StartupStore(_T(".... ERROR updating LogBook, file open failure!%s"),NEWLINE);
	return false;
  }

  //if (CALCULATED_INFO.FlightTime>0) {
//      Units::TimeToText(Temp,
 //                       (int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));

  //
  // Header line for new note
  //
  sprintf(line,"\r\n[%04d-%02d-%02d %02d:%02d]\r\n",
	GPS_INFO.Year,
	GPS_INFO.Month,
	GPS_INFO.Day,
	GPS_INFO.Hour,
	GPS_INFO.Minute);
  fwrite(line,strlen(line),1,stream);

  // 
  // Note body
  //
  Units::TimeToText(Temp,(int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
  sprintf(line,"%S: %S\r\n",gettext(_T("_@M680_")),Temp);	// takeoff time
  fwrite(line,strlen(line),1,stream);

  if (!CALCULATED_INFO.Flying) {
	Units::TimeToText(Temp,(int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime+CALCULATED_INFO.FlightTime)));
	sprintf(line,"%S: %S\r\n",gettext(_T("_@M386_")),Temp);	// landing time
  } else {
  	#if TESTBENCH
	StartupStore(_T(".... LogBook, logging but still flying!%s"),NEWLINE);
	#endif
	sprintf(line,"%S: -------\r\n",gettext(_T("_@M386_")));
  }
  fwrite(line,strlen(line),1,stream);

  Units::TimeToText(Temp, (int)CALCULATED_INFO.FlightTime);
  sprintf(line,"%S: %S\r\n",gettext(_T("_@M306_")),Temp);	// flight time
  fwrite(line,strlen(line),1,stream);


  fclose(stream);

  return true;

}



