#if !defined(AFX_PORT_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_PORT_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>

class ComPort {
 public:
	ComPort(int the_devIdx);
	~ComPort() { };

	void PutChar(BYTE);
	bool Write(const void *data, size_t length);
	void WriteString(const TCHAR *);
	void Flush();

	BOOL Initialize(LPCTSTR, DWORD, DWORD,DWORD);
	BOOL Close();

	int SetRxTimeout(int);
	unsigned long SetBaudrate(unsigned long);
	unsigned long GetBaudrate();

	BOOL StopRxThread();
	BOOL StartRxThread();
	void ProcessChar(char);

	int GetChar();
	int Read(void *Buffer, size_t Size);

 private:
	static DWORD WINAPI ThreadProc(LPVOID);
	DWORD ReadThread();

	HANDLE hPort;
	HANDLE hReadThread;
	DWORD dwMask;
	// For Comm ports, a prefix of  \\.\ is needed, and thus \\.\COM99 need 9 chars
	// but we want also long device names
	TCHAR sPortName[30]; 
	BOOL CloseThread;
	BOOL fRxThreadTerminated;

	TCHAR BuildingString[MAX_NMEA_LEN+1];
	int bi;
	int devIdx;
	int sportnumber;
};

#endif
