/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "utils/stringext.h"

using namespace std::placeholders;


FilePort::FilePort (unsigned idx, const tstring& sName) : ComPort(idx, sName)
{
  FileStream = NULL;
}

FilePort::~FilePort() {
	// not yet needed
}

bool FilePort::Initialize() {
  const TCHAR* file_name = PortConfig[GetPortIndex()].Replay_FileName;

  TCHAR szReplayFileName [MAX_PATH];
  LocalPath(szReplayFileName,TEXT(LKD_LOGS), file_name);
	FileStream = _tfopen(szReplayFileName, TEXT("rt"));

	if (!FileStream) {
		 StartupStore(_T(". FilePort  %u failed to open file %s Port <%s>"), GetPortIndex() + 1, file_name, GetPortName());
		 StatusMessage(_T("%s %s"), MsgToken(762), GetPortName());
		 return false;
	}

  SetPortStatus(CPS_OPENOK);
	StartupStore(_T(". FilePort  %u open file %s Port <%s> OK"), GetPortIndex() + 1, file_name, GetPortName());
	m_dwWaitTime =0;
	return true;
}


int FilePort::SetRxTimeout(int TimeOut) {
// not yet needed
return 0;
}

int FilePort::ReadLine(char* pString, size_t size) {
	if(!FileStream) {
		return -1;
	}

	if (fgets(pString, size, FileStream)==NULL) {
		pString[0] = '\0';
		return -1;
	}
	return strlen(pString);;
}


bool FilePort::Close() {
	ComPort::Close();
	fclose(FileStream);
	FileStream = NULL;
	StartupStore(_T(". FilePort %u %s closed Ok.%s"), GetPortIndex(), GetPortName(), NEWLINE);
	return true;
}

int32_t GGA_RMC_seconds(const char *StrTime) {
  int32_t Hour=0, Minute=0, Second=0;

  if (_istdigit(StrTime[0]) && _istdigit(StrTime[1])) {
      Hour = (StrTime[0] - '0')*10 + (StrTime[1] - '0');
  }
  if (_istdigit(StrTime[2]) && _istdigit(StrTime[3])) {
      Minute = (StrTime[2] - '0')*10 + (StrTime[3] - '0');
  }
  if (_istdigit(StrTime[4]) && _istdigit(StrTime[5])) {
      Second = (StrTime[4] - '0')*10 + (StrTime[5] - '0');
  }
  return Hour*3600+ Minute * 60 + Second;
}



unsigned FilePort::RxThread()
{

PeriodClock Timer;

char szString[MAX_NMEA_LEN];
char szRef[3][MAX_NMEA_LEN]= {{"nosync"},{"$GPRMC"}, {"$GPGGA"}};


int  nRecv =0;
int32_t LastTimeSeconds=0;
int32_t TimeInSeconds=0;

  auto& Port = PortConfig[GetPortIndex()];

int32_t i_skip = 1;
  int32_t ms =0;
  while (!StopEvt.tryWait(5) /*&& FileStream*/ ) 
  {
    int32_t speed = Port.ReplaySpeed;
#define THRESHOLD 10 // max 10Hz GPS Map refresh

    nRecv =  ReadLine(szString, MAX_NMEA_LEN-1);

    if(speed == 0)  
      m_dwWaitTime = 10000;  //  ( 0 = 0.1Hz GPS  )
    else
      m_dwWaitTime = 1000/speed; // default factor as Hz

    if(Port.ReplaySync == 0) // Timer only?
    {
      ms = m_dwWaitTime - Timer.ElapsedUpdate();
      if((ms > 0)&&(ms < 10000))
      {
        Poco::Thread::sleep(ms); 
        Timer.Update();
      }
      LastTimeSeconds = TimeInSeconds;     
    }
    else
    {
      if( strncmp (szString,szRef[Port.ReplaySync], 6) ==0)
      {
        TimeInSeconds = GGA_RMC_seconds(&szString[7]);

        if(speed ==0)
        {
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
        while ((nRecv >= 0) && ((TimeInSeconds-LastTimeSeconds) < i_skip ) && (LineCnt < (i_skip *50)))
        {
          nRecv =  ReadLine(szString, MAX_NMEA_LEN-1);
          LineCnt++;
          if( strncmp (szString,szRef[Port.ReplaySync], 6) ==0)
          {
            LineCnt =0;
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
    }


    if (nRecv > 0)
    {
      if (Port.RawByteData)
      {
        std::for_each(std::begin(szString), std::next(szString, nRecv), GetProcessCharHandler());
      }
      else
      {
        TCHAR nmea[MAX_NMEA_LEN];
        from_utf8(szString, nmea);

        devParseNMEA(GetPortIndex(), nmea, &GPS_INFO);
      }
      AddStatTx(nRecv);
      UpdateStatus();
    }
  }

  return 0UL;
}
