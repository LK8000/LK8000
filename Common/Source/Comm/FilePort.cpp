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

unsigned FilePort::RxThread()
{

PeriodClock Timer;

char szString[MAX_NMEA_LEN];
int  nRecv;

  while (!StopEvt.tryWait(5) /*&&  FileStream */) // call every 5ms
  {

#define THRESHOLD 10  // max 10Hz GPS Map refresh
  
      nRecv =  ReadLine(szString, MAX_NMEA_LEN-1);
   
      if(!FileStream) // end of File?
      {
        Close();      // restart file
        Initialize();
      }

      if(strncmp (szString,"$GPRMC", 6) ==0) // wait until next GPS fix or at least every 25 sentences
      {
        if(ReplaySpeed[GetPortIndex()] ==0)
        {
          m_dwWaitTime = 10000;  //  ( 0 = 0.1Hz GPS  )
        }
        else
        {
          if(ReplaySpeed[GetPortIndex()] <THRESHOLD)  
            m_dwWaitTime = 1000/ReplaySpeed[GetPortIndex()]; // up to 10Hz
          else
          {  // very fast replay? (skip NMEA sentences
            long GPRMCCnt=0;
            long LineCnt=0;
            do
            {
              nRecv =  ReadLine(szString, MAX_NMEA_LEN-1);
              LineCnt++;
              if(strncmp (szString,"$GPRMC", 6) == 0)
              {
                LineCnt =0;
                GPRMCCnt++;
              }
            }
            while ((nRecv >= 0) &&  (GPRMCCnt < ReplaySpeed[GetPortIndex()]/THRESHOLD) && (LineCnt < 50));
            m_dwWaitTime = 1000/THRESHOLD;  //  ( 0 = 0.1Hz GPS  )
          }
        }

        unsigned ms = std::clamp<unsigned>(m_dwWaitTime - Timer.ElapsedUpdate(), 1U, 100000U);
        Poco::Thread::sleep(ms); // needed for windows bug
        Timer.Update();

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
