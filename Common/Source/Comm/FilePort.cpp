/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   FilePort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on February 22, 2016, 8:29 PM
 */
#include "externs.h"
#include "FilePort.h"
#include <functional>
using namespace std::placeholders;


FilePort::FilePort (int idx, const tstring& sName) : ComPort(idx, sName)
{
// not yet needed
}

FilePort::~FilePort() {
	// not yet needed
}

bool FilePort::Initialize() {
TCHAR szReplayFileName [MAX_PATH];

  LocalPath(szReplayFileName,TEXT(LKD_LOGS),Replay_FileName[GetPortIndex()]);
	FileStream = _tfopen(szReplayFileName, TEXT("rt"));

	if((!FileStream) || (!ComPort::Initialize()))
	{
		 StartupStore(_T(". FilePort  %u failed to open file %s Port <%s> %s"), (unsigned)GetPortIndex() + 1, Replay_FileName[GetPortIndex()], GetPortName(), NEWLINE);
		 StatusMessage(mbOk, NULL, TEXT("%s %s"), MsgToken(762), GetPortName());
		 return false;
	}

	StartupStore(_T(". FilePort  %u open file %s Port <%s> OK%s"), (unsigned)GetPortIndex() + 1, Replay_FileName[GetPortIndex()], GetPortName(), NEWLINE);
	m_dwWaitTime =0;
	return true;
}


int FilePort::SetRxTimeout(int TimeOut) {
// not yet needed
return 0;
}

int FilePort::ReadLine(void *pString, size_t size) {
char* szString = (char*)pString;

	if(!FileStream) {
			return -1;
	}

	if (fgets((char*) szString, size, FileStream)==NULL) {
		szString[0] = '\0';
		return -1;
	}
	return strlen(szString);;
}


bool FilePort::Close() {
	ComPort::Close();
	fclose(FileStream);
	FileStream = NULL;
	StartupStore(_T(". FilePort %u %s closed Ok.%s"), (unsigned)GetPortIndex(),GetPortName(), NEWLINE);
	return true;
}

bool FilePort::Write(const void *data, size_t length) {
  return true;
}

/*
void FilePort::CancelWaitEvent(void)
{
  StartupStore(_T(". FilePort CancelWaitEvent %u m_dwWaitTime :"), (unsigned) m_dwWaitTime );
  m_dwWaitTime = 5;

}
*/
//FilePort::SendSectrion()





 int32_t GGA_RMC_seconds(const char *StrTime)
{int32_t Hour=0,Minute=0,Second=0, sec;

  if (_istdigit(StrTime[0]) && _istdigit(StrTime[1])) {
      Hour = (StrTime[0] - '0')*10 + (StrTime[1] - '0');
  }
  if (_istdigit(StrTime[2]) && _istdigit(StrTime[3])) {
      Minute = (StrTime[2] - '0')*10 + (StrTime[3] - '0');
  }
  if (_istdigit(StrTime[4]) && _istdigit(StrTime[5])) {
      Second = (StrTime[4] - '0')*10 + (StrTime[5] - '0');
  }
//  StartupStore(_T("...... RMC_seconds %s =====> %02ih%02im%02is%s"),StrTime, Hour, Minute, Second,NEWLINE);
  sec = Hour*3600L+ Minute * 60L + Second;
//  StartupStore(TEXT("RMC_seconds =====> %d %s"), sec, NEWLINE);
return sec; 
}



unsigned FilePort::RxThread()
{

PeriodClock Timer;

char szString[MAX_NMEA_LEN];
char szRef[MAX_NMEA_LEN]="$GPGGA";


int  nRecv;
int32_t LastTimeSeconds=0;
int32_t TimeInSeconds=0;


int32_t i_skip = 1;
  int32_t ms =0;
  while (!StopEvt.tryWait(5) && FileStream ) // call every 5ms
  {
    int32_t speed = ReplaySpeed[GetPortIndex()];
#define THRESHOLD 10 // max 10Hz GPS Map refresh
  
      nRecv =  ReadLine(szString, MAX_NMEA_LEN-1);
   
      if(!FileStream) // end of File?
      {
        Close();      // restart file
        Initialize();
      }

      if( strncmp (szString,szRef, 6) ==0)
      {
        TimeInSeconds = GGA_RMC_seconds(&szString[7]);
         
        if(speed ==0)
        {
          m_dwWaitTime = 10000;  //  ( 0 = 0.1Hz GPS  )
          i_skip = 1;
        }
        else
        {
          if(speed <= THRESHOLD)  
          {
            m_dwWaitTime = 1000/speed; // up to 10Hz
            i_skip = 1;
          }
          else
          {
            m_dwWaitTime = 1000/5;  //  ( 5Hz refresh  )
            i_skip = speed/5;       // skip all data in between      
          }
        }
                   
        long LineCnt=0;
        while ((nRecv >= 0) && ((TimeInSeconds-LastTimeSeconds) < i_skip ) && (LineCnt < (i_skip *10)))
        {
          nRecv =  ReadLine(szString, MAX_NMEA_LEN-1);
          LineCnt++;
          if( strncmp (szString,szRef, 6) ==0)
          {
            TimeInSeconds = GGA_RMC_seconds(&szString[7]);
          }
         
        }

        ms = m_dwWaitTime - Timer.ElapsedUpdate();         
        if((ms > 0)&&(ms < 10000))
        {
          Poco::Thread::sleep(ms); 
          Timer.Update();
        }    
        LastTimeSeconds = TimeInSeconds;     
      }


      if (nRecv > 0)
      {
#define BYTE_BY_BYTE
#ifdef BYTE_BY_BYTE
          std::for_each(std::begin(szString), std::begin(szString) + nRecv, std::bind(&FilePort::ProcessChar, this, _1));
#else
          LockFlightData();
          devParseNMEA(GetPortIndex(), (TCHAR*)szString, &GPS_INFO);
          UnlockFlightData();
#endif
          AddStatTx(nRecv);
          UpdateStatus();
      }
       
    }
  
  return 0UL;
}
