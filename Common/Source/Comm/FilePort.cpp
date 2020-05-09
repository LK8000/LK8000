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

}

FilePort::~FilePort() {


}

bool FilePort::Initialize() {

TCHAR szReplayFileName [255];
  LocalPath(szReplayFileName,TEXT(LKD_LOGS),Replay_FileName[GetPortIndex()]);

	FileStream = _tfopen(szReplayFileName, TEXT("rt"));

	if((!FileStream) || (!ComPort::Initialize()))
	{
		 StartupStore(_T(". FilePort  %u failed to open file %s Port <%s> %s"), (unsigned)GetPortIndex() + 1, Replay_FileName[GetPortIndex()], GetPortName(), NEWLINE);
		 StatusMessage(mbOk, NULL, TEXT("%s %s"), MsgToken(762), GetPortName());
		 return false;
	}

	StartupStore(_T(". FilePort  %u open file %s Port <%s> OK%s"), (unsigned)GetPortIndex() + 1, Replay_FileName[GetPortIndex()], GetPortName(), NEWLINE);
	return true;
}


int FilePort::SetRxTimeout(int TimeOut) {
// not yet needed
return 0;
}

size_t FilePort::Read(void *pString, size_t size) {
char* szString = (char*)pString;

	if(!FileStream) {
			return false; // socket not connect,that can happen with TCPServer Port.
	}

	if (fgets((char*) szString, size, FileStream)==NULL) {
		szString[0] = '\0';
		return false;
	}

	return strlen((char*)szString);;
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

unsigned FilePort::RxThread() {
unsigned long	dwWaitTime = 5;
unsigned LineCnt =0;
_Buff_t szString;
size_t nRecv;

        
	while (!StopEvt.tryWait(dwWaitTime) && FileStream )
	{

		nRecv =  Read(szString, 1023);
		UpdateStatus();

		if((strncmp (szString,"$GPRMC", 6) ==0) || (LineCnt++ > 50)) // wait until next GPS fix or at least every 30 sentences
		{
			LineCnt =0;
			if(ReplaySpeed[GetPortIndex()] >0)
			  dwWaitTime = 1000/ReplaySpeed[GetPortIndex()]; // x1 = 1Hz
			else
				dwWaitTime = 10000;  //  ( 0 = 0.1Hz GPS  )
		}
		else
		{
			dwWaitTime = 2;
		}

    if(dwWaitTime < 2)
    	dwWaitTime = 2; // avoid cpu overhead;

		if (nRecv > 0) {
				std::for_each(std::begin(szString), std::begin(szString) + nRecv, std::bind(&FilePort::ProcessChar, this, _1));
		}
	}

	return 0UL;
}
