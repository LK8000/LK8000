/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Port.cpp,v 8.5 2010/12/12 16:18:34 root Exp root $
*/

#include "StdAfx.h"
#include "Cpustats.h"
#include "Port.h"
#include "externs.h"
#include "XCSoar.h"
#include "device.h"
#include "Utils2.h"

#include <windows.h>
#include <tchar.h>

#define COMMDEBUG 0

static void ComPort_StatusMessage(UINT type, const TCHAR *caption, const TCHAR *fmt, ...)
{
  TCHAR tmp[127];
  va_list ap;

  va_start(ap, fmt);
  _vsntprintf(tmp, 127, fmt, ap);
  va_end(ap);

  tmp[126] = _T('\0');

  if (caption)
    MessageBoxX(hWndMainWindow, tmp, gettext(caption), type);
  else
    DoStatusMessage(tmp);
}

ComPort::ComPort(int the_dev_idx)
{
  hReadThread = NULL;
  CloseThread = 0;
  fRxThreadTerminated = TRUE;
  dwMask = 0;
  hPort = INVALID_HANDLE_VALUE;
  BuildingString[0] = 0;
  bi = 0;
  devIdx = the_dev_idx;
}

// Initialize is called from device  devInit, and dwPortNumber is passed along new threads
#ifdef COMDIAG
BOOL ComPort::Initialize(LPCTSTR lpszPortName, DWORD dwPortSpeed, DWORD dwPortBit, DWORD dwPortNumber)
#else
BOOL ComPort::Initialize(LPCTSTR lpszPortName, DWORD dwPortSpeed)
#endif
{
  DWORD dwError;
  DCB PortDCB;
  TCHAR lkbuf[100];
  TCHAR lkPortName[10]; // 9 should be enough
#if NOSIM
  if (SIMMODE) return FALSE;
#else
#ifdef _SIM_
  return FALSE;
#endif
#endif

#if (WINDOWSPC>0)
  // Do not use anymore COMn: , use \\.\COMnn  on PC version
  if (lpszPortName) {
	_tcscpy(sPortName,_T("\\\\.\\")); // 091117
	_tcscat(sPortName, lpszPortName);
	_tcscpy(lkPortName, lpszPortName);
  }
#else
  if (lpszPortName) {
	_tcscpy(sPortName, lpszPortName);
	_tcscpy(lkPortName, lpszPortName);
  }
#endif

#ifdef COMDIAG
  StartupStore(_T(". ComPort %d Initialize <%s> speed=%d bit=%d %s"),dwPortNumber+1,lkPortName,dwPortSpeed,8-dwPortBit,NEWLINE);
#else
  StartupStore(_T(". ComPort Initialize <%s>%s"),lkPortName,NEWLINE);
#endif

  hPort = CreateFile(sPortName, // Pointer to the name of the port
                      GENERIC_READ | GENERIC_WRITE,
                                    // Access (read-write) mode
                      0,            // Share mode
                      NULL,         // Pointer to the security attribute
                      OPEN_EXISTING,// How to open the serial port
                      FILE_ATTRIBUTE_NORMAL,            // Port attributes
                      NULL);        // Handle to port with attribute
                                    // to copy

  if (hPort == INVALID_HANDLE_VALUE) {
	dwError = GetLastError();
#ifdef COMDIAG
	_stprintf(lkbuf,_T("... ComPort %d Init failed, error=%d%s"),dwPortNumber+1,dwError,NEWLINE); // 091117
#else
	_stprintf(lkbuf,_T("... ComPort Init failed, error=%d%s"),dwError,NEWLINE); // 091117
#endif
	StartupStore(lkbuf);
	// LKTOKEN  _@M762_ = "Unable to open port" 
	ComPort_StatusMessage(MB_OK|MB_ICONINFORMATION, NULL, TEXT("%s %s"), gettext(TEXT("_@M762_")), lkPortName);
	return FALSE;
  }
#ifdef COMDIAG
  StartupStore(_T(". ComPort %d  <%s> is now open%s"),dwPortNumber+1,lkPortName,NEWLINE);
#else
  StartupStore(_T(". ComPort <%s> is now open%s"),lkPortName,NEWLINE); 
#endif

  PortDCB.DCBlength = sizeof(DCB);     

  // Get the default port setting information.
  GetCommState(hPort, &PortDCB);

  // Change the DCB structure settings.
  PortDCB.BaudRate = dwPortSpeed;       // Current baud 
  PortDCB.fBinary = TRUE;               // Binary mode; no EOF check 
  PortDCB.fParity = TRUE;               // Enable parity checking  
  PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control 
  PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control 
  PortDCB.fDtrControl = DTR_CONTROL_ENABLE; 
                                        // DTR flow control type 
  PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity 
  PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx 
  PortDCB.fOutX = FALSE;                // No XON/XOFF out flow control 
  PortDCB.fInX = FALSE;                 // No XON/XOFF in flow control 
  PortDCB.fErrorChar = FALSE;           // Disable error replacement 
  PortDCB.fNull = FALSE;                // Disable null removal
  PortDCB.fRtsControl = RTS_CONTROL_ENABLE; 
                                        // RTS flow control 

  PortDCB.fAbortOnError = TRUE;         // FALSE need something else to work
  switch(dwPortBit) {
	case bit7E1:
			PortDCB.ByteSize = 7;
			PortDCB.Parity = EVENPARITY; 
			break;
	case bit8N1:
	default:
			PortDCB.ByteSize = 8;
			PortDCB.Parity = NOPARITY; 
			break;
  }

  PortDCB.StopBits = ONESTOPBIT;

  PortDCB.EvtChar = '\n'; // wait for end of line, RXFLAG should detect it

  if (!SetCommState(hPort, &PortDCB)) {
	// Could not create the read thread.
	CloseHandle(hPort);
	hPort = INVALID_HANDLE_VALUE;
#if (WINDOWSPC>0) || NEWCOMM // 091206
	Sleep(2000); // needed for windows bug 101116 not verified
#endif
#if POLLINGMODE && !(WINDOWSPC>0)
	if (PollingMode) Sleep(2000);
#endif
	// LKTOKEN  _@M759_ = "Unable to Change Settings on Port" 
	ComPort_StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M759_")), lkPortName);
	dwError = GetLastError();
#ifdef COMDIAG
	_stprintf(lkbuf,_T("... ComPort %d Init <%s> change setting FAILED, error=%d%s"),dwPortNumber+1,lkPortName,dwError,NEWLINE); // 091117
#else
	_stprintf(lkbuf,_T("... ComPort Init <%s> change setting FAILED, error=%d%s"),lkPortName,dwError,NEWLINE); // 091117
#endif
	StartupStore(lkbuf);
	return FALSE;
  }

  SetRxTimeout(RXTIMEOUT); 

  SetupComm(hPort, 1024, 1024);

  // Direct the port to perform extended functions SETDTR and SETRTS
  // SETDTR: Sends the DTR (data-terminal-ready) signal.
  // SETRTS: Sends the RTS (request-to-send) signal. 
  EscapeCommFunction(hPort, SETDTR);
  EscapeCommFunction(hPort, SETRTS);
#ifdef COMDIAG
  sportnumber=dwPortNumber; // 100210
  if (!StartRxThread()){
	_stprintf(lkbuf,_T("... ComPort %d Init <%s> StartRxThread failed%s"),dwPortNumber+1,lkPortName,NEWLINE);
#else
  if (!StartRxThread()){
	_stprintf(lkbuf,_T("... ComPort Init <%s> StartRxThread failed%s"),lkPortName,NEWLINE);
#endif
	StartupStore(lkbuf);
	if (!CloseHandle(hPort)) {
		dwError = GetLastError();
		#ifdef COMDIAG
		_stprintf(lkbuf,_T("... ComPort %d Init <%s> close failed, error=%d%s"),dwPortNumber+1,lkPortName,dwError,NEWLINE);
		#else
		_stprintf(lkbuf,_T("... ComPort Init <%s> close failed, error=%d%s"),lkPortName,dwError,NEWLINE);
		#endif
		StartupStore(lkbuf);
	} else {
		#ifdef COMDIAG
		_stprintf(lkbuf,_T("... ComPort %d Init <%s> closed%s"),dwPortNumber+1,lkPortName,NEWLINE);
		#else
		_stprintf(lkbuf,_T("... ComPort Init <%s> closed%s"),lkPortName,NEWLINE);
		#endif
		StartupStore(lkbuf);
	}
	hPort = INVALID_HANDLE_VALUE;
#if (WINDOWSPC>0) || NEWCOMM // 091206
	Sleep(2000); // needed for windows bug
#endif
#if POLLINGMODE && !(WINDOWSPC>0)
	if (PollingMode) Sleep(2000);
#endif

	return FALSE;
  }

  #ifdef COMDIAG
  _stprintf(lkbuf,_T(". ComPort %d Init <%s> end OK%s"),dwPortNumber+1,lkPortName,NEWLINE);
  #else
  _stprintf(lkbuf,_T(". ComPort Init <%s> end OK%s"),lkPortName,NEWLINE);
  #endif
  StartupStore(lkbuf);
  return TRUE;
}


/***********************************************************************

  PortWrite (BYTE Byte)

***********************************************************************/
// TODO CHECK PutChar, no error handling!
// This is used only by Volkslogger 
void ComPort::PutChar(BYTE Byte)
{
  if (hPort == INVALID_HANDLE_VALUE)
    return;

  DWORD dwError, dwNumBytesWritten;

  if (!WriteFile(hPort,              // Port handle
                 &Byte,               // Pointer to the data to write 
                 1,                   // Number of bytes to write
                 &dwNumBytesWritten,  // Pointer to the number of bytes 
                                      // written
                 (OVERLAPPED *)NULL)) // Must be NULL for Windows CE
  {
    // WriteFile failed. Report error.
    dwError = GetLastError();
  }
}


void ComPort::Flush(void)
{
  PurgeComm(hPort, 
            PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

DWORD WINAPI ComPort::ThreadProc(LPVOID prt)
{
  ComPort *port = (ComPort *)prt;
  port->ReadThread();
  return 0;
}

/***********************************************************************

  PortReadThread (LPVOID lpvoid)

***********************************************************************/
DWORD ComPort::ReadThread()
{
  #if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM	// 100222
  DWORD dwCommModemStatus=0;
  #endif
  DWORD dwBytesTransferred=0; // 091117 initialized variables
  BYTE inbuf[1024];
  #ifdef CPUSTATS
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;
  #endif

  // JMW added purging of port on open to prevent overflow
  Flush();
  #ifdef COMDIAG
  StartupStore(_T(". ReadThread running on port %d%s"),sportnumber+1,NEWLINE);
  #endif
  
  // Specify a set of events to be monitored for the port.

  dwMask = EV_RXFLAG | EV_CTS | EV_DSR | EV_RING | EV_RXCHAR;

  // #if !defined(WINDOWSPC) || (WINDOWSPC == 0) 091206
  #if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM
  SetCommMask(hPort, dwMask);
  #endif
#if POLLINGMODE && (WINDOWSPC<1)
  if (!PollingMode) SetCommMask(hPort, dwMask);
#endif

  fRxThreadTerminated = FALSE;
  
  while ((hPort != INVALID_HANDLE_VALUE) && (!MapWindow::CLOSETHREAD) && (!CloseThread)) 
  {
	#ifdef CPUSTATS
	GetThreadTimes( hReadThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
	#endif
	#if (WINDOWSPC>0) || NEWCOMM // 091206
	// PC version does BUSY WAIT
	Sleep(50);  // ToDo rewrite the whole driver to use overlaped IO on W2K or higher
	#else
#if POLLINGMODE
	if (PollingMode)  
		Sleep(100);
	else
#endif
	// Wait for an event to occur for the port.
	if (!WaitCommEvent(hPort, &dwCommModemStatus, 0)) {
		// error reading from port
		Sleep(100);
	}
	#endif

	// Re-specify the set of events to be monitored for the port.
	//    SetCommMask(hPort, dwMask1);

	// #if !defined(WINDOWSPC) || (WINDOWSPC == 0) 091206
	#if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM
#if POLLINGMODE
	if (PollingMode || (dwCommModemStatus & EV_RXFLAG) || (dwCommModemStatus & EV_RXCHAR)) // Do this only for non-PC
#else
	if ((dwCommModemStatus & EV_RXFLAG) || (dwCommModemStatus & EV_RXCHAR)) // Do this only for non-PC
#endif
	#endif
	{

		// Loop for waiting for the data.
		do {
			dwBytesTransferred = 0;
			// Read the data from the serial port.
			if (ReadFile(hPort, inbuf, 1024, &dwBytesTransferred, (OVERLAPPED *)NULL)) {
				if (ProgramStarted >= psNormalOp) {  // ignore everything until started
					for (unsigned int j = 0; j < dwBytesTransferred; j++) {
						ProcessChar(inbuf[j]);
					}
				}
				#ifdef COMDIAG
				ComPortRx[sportnumber]+=dwBytesTransferred; // 100210
				#endif
			} else {
				dwBytesTransferred = 0;
			}

			Sleep(50); // JMW20070515: give port some time to
			// fill... prevents ReadFile from causing the
			// thread to take up too much CPU
			#ifdef CPUSTATS
			if ( (GetThreadTimes( hReadThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
				Cpu_Port=9999;
			} else {
				Cpustats(&Cpu_Port,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
			}
			#endif
		  
			if (CloseThread) {
				dwBytesTransferred = 0;
				#ifdef COMDIAG
				StartupStore(_T(". ComPort %d ReadThread: CloseThread ordered%s"),sportnumber+1,NEWLINE);
				#else
				StartupStore(_T(". ComPort ReadThread: CloseThread ordered%s"),NEWLINE);
				#endif
			}
		} while (dwBytesTransferred != 0);
	}

	// give port some time to fill
	Sleep(5);

	// Retrieve modem control-register values.
	#if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) 

#if POLLINGMODE 
	if (!PollingMode)
#endif
	// this is causing problems on PC BT, apparently. Setting Polling will not call this, but it is a bug
	GetCommModemStatus(hPort, &dwCommModemStatus);

	#endif

  }

  Flush();      

  fRxThreadTerminated = TRUE;
  #ifdef COMDIAG
  StartupStore(_T(". ComPort %d ReadThread: terminated%s"),sportnumber+1,NEWLINE);
  #else
  StartupStore(_T(". ComPort ReadThread: terminated%s"),NEWLINE);
  #endif

  return 0;
}


/***********************************************************************

  PortClose()

***********************************************************************/
BOOL ComPort::Close()
{
  DWORD dwError;
  TCHAR lkbuf[100];

  if (hPort != INVALID_HANDLE_VALUE) {
	StopRxThread();
	Sleep(100);  // todo ...
    
	dwError = 0;

	// Close the communication port.
	if (!CloseHandle(hPort)) {
		dwError = GetLastError();
		#ifdef COMDIAG
		_stprintf(lkbuf,_T("... ComPort %d close failed, error=%d%s"),sportnumber+1,dwError,NEWLINE);
		#else
		_stprintf(lkbuf,_T("... ComPort close failed, error=%d%s"),dwError,NEWLINE);
		#endif
		StartupStore(lkbuf);
		return FALSE;
	} else {
		#if (WINDOWSPC>0) || NEWCOMM // 091206
		Sleep(2000); // needed for windows bug
		#endif
		hPort = INVALID_HANDLE_VALUE; 
		#ifdef COMDIAG
		_stprintf(lkbuf,_T(". ComPort %d closed Ok.%s"),sportnumber+1,NEWLINE);
		#else
		_stprintf(lkbuf,_T(". ComPort closed Ok%s"),NEWLINE);
		#endif
		StartupStore(lkbuf); // 100210 BUGFIX missing
		return TRUE;
	}
  }
  #ifdef COMDIAG
  StartupStore(_T("... ComPort %d Close failed, invalid handle%s"),sportnumber+1,NEWLINE);
  #else
  StartupStore(_T("... ComPort Close failed, invalid handle%s"),NEWLINE);
  #endif
  return FALSE;
}


// this is used by all functions to send data out
// it is called internally from thread for each device
void ComPort::WriteString(const TCHAR *Text)
{
   char tmp[512];
   DWORD written, error;

   if (hPort == INVALID_HANDLE_VALUE)
     return;
    
   int len = _tcslen(Text);
 
   len = WideCharToMultiByte(CP_ACP, 0, Text, len + 1, tmp, sizeof(tmp), NULL, NULL);
 
   // don't write trailing '\0' to device
#ifdef COMDIAG
   if (--len<=0 || !WriteFile(hPort, tmp, len, &written, NULL)) {
	// WriteFile failed, report error
	error = GetLastError();
	ComPortErrTx[sportnumber]++;
	ComPortErrors[sportnumber]++;
    } else
	ComPortTx[sportnumber]+=written;
#else
   if (--len<=0 || !WriteFile(hPort, tmp, len, &written, NULL)) 
     // WriteFile failed, report error
     error = GetLastError();
#endif
}


// Stop Rx Thread
// return: TRUE on success, FALSE on error
BOOL ComPort::StopRxThread()
{  
  if (hPort == INVALID_HANDLE_VALUE) return FALSE;
  if (fRxThreadTerminated) return TRUE;

  CloseThread = TRUE;

#ifndef GTCFIX // 100510
  DWORD tm = GetTickCount()+20000l;
#endif

// currently NEWCOMM is NOT used
#if (WINDOWSPC>0) || NEWCOMM  // 091206
  #ifdef GTCFIX
  DWORD tm = GetTickCount()+20000l;
  while (!fRxThreadTerminated && ((tm-GetTickCount()) > 0) ) {
	Sleep(10);
  }
  #else
  while (!fRxThreadTerminated && (long)(tm-GetTickCount()) > 0) {
	Sleep(10);
  }
  #endif
  if (!fRxThreadTerminated) {
	TerminateThread(hReadThread, 0);
  } else {
	CloseHandle(hReadThread);
  }
#else
  Flush();

  // setting the comm event mask with the same value
  //  GetCommMask(hPort, &dwMask);
  SetCommMask(hPort, dwMask);          // will cancel any
                                        // WaitCommEvent!  this is a
                                        // documented CE trick to
                                        // cancel the WaitCommEvent

  #ifdef GTCFIX
  int count=0;
  while (!fRxThreadTerminated && (count<2000)){ 
	Sleep(10);
	count++;
  }

  #else
  while (!fRxThreadTerminated && (long)(GetTickCount()-tm) < 1000){
	Sleep(10);
  }
  #endif

  if (!fRxThreadTerminated) {
	//#if COMMDEBUG > 0
	#if 101122
	TerminateThread(hReadThread, 0);
	StartupStore(_T("...... ComPort StopRxThread: RX Thread forced to terminate!%s"),NEWLINE);
	#else

	// LKTOKEN  _@M540_ = "RX Thread not Terminated!" 
	ComPort_StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), sPortName, gettext(TEXT("_@M540_")));
        #ifdef COMDIAG
	StartupStore(_T("...... ComPort %d StopRxThread: RX Thread not terminated!%s"),sportnumber+1,NEWLINE);
	#else
	StartupStore(_T("...... ComPort StopRxThread: RX Thread not terminated!%s"),NEWLINE);
	#endif

	#endif
	//#endif
  } else {
	CloseHandle(hReadThread);
	#ifdef COMDIAG
	StartupStore(_T(". ComPort %d StopRxThread: RX Thread terminated%s"),sportnumber+1,NEWLINE);
	#else
	StartupStore(_T(". ComPort StopRxThread: RX Thread terminated%s"),NEWLINE);
	#endif
  }
#endif

  return fRxThreadTerminated;
}


// Restart Rx Thread
// return: TRUE on success, FALSE on error
BOOL ComPort::StartRxThread(void)
{
  DWORD dwThreadID, dwError;

  if (hPort == INVALID_HANDLE_VALUE) return FALSE;

  CloseThread = FALSE;

  // Create a read thread for reading data from the communication port.
  if ((hReadThread = CreateThread (NULL, 0, ThreadProc, this, 0, &dwThreadID)) != NULL) {

	SetThreadPriority(hReadThread, THREAD_PRIORITY_NORMAL); //THREAD_PRIORITY_ABOVE_NORMAL

	//???? JMW Why close it here?    CloseHandle(hReadThread);
  } else {
	// Could not create the read thread.
	// LKTOKEN  _@M761_ = "Unable to Start RX Thread on Port" 
	ComPort_StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M761_")), sPortName);
	dwError = GetLastError();
	return FALSE;
  }

  return TRUE;
}

                                        // Get a single Byte
                                        // return: char readed or EOF on error
int ComPort::GetChar(void)
{
  BYTE  inbuf[2];
  DWORD dwBytesTransferred;

  if (hPort == INVALID_HANDLE_VALUE || !CloseThread)
    return EOF;
  
  if (ReadFile(hPort, inbuf, 1, &dwBytesTransferred, (OVERLAPPED *)NULL)) {
    if (dwBytesTransferred == 1)
      return inbuf[0];
  }

  return EOF;
}

// Set Rx Timeout in ms
// Timeout: Rx receive timeout in ms
// return: last set Rx timeout or -1 on error
int ComPort::SetRxTimeout(int Timeout)
{
  COMMTIMEOUTS CommTimeouts;
  int result;
  DWORD dwError;

  if (hPort == INVALID_HANDLE_VALUE)
    return -1;

  GetCommTimeouts(hPort, &CommTimeouts);

  result = CommTimeouts.ReadTotalTimeoutConstant;
  
  // Change the COMMTIMEOUTS structure settings.
  CommTimeouts.ReadIntervalTimeout = MAXDWORD;

  // JMW 20070515
  if (Timeout == 0) {
    // no total timeouts used
    CommTimeouts.ReadTotalTimeoutMultiplier = 0; 
    CommTimeouts.ReadTotalTimeoutConstant = 0;
  } else {
    // only total timeout used
    CommTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutConstant = Timeout;
  }

  CommTimeouts.WriteTotalTimeoutMultiplier = 10;  
  CommTimeouts.WriteTotalTimeoutConstant = 1000;    

                                        // Set the time-out parameters
                                        // for all read and write
                                        // operations on the port.
  if (!SetCommTimeouts(hPort, &CommTimeouts)) {
     // Could not create the read thread.
    CloseHandle(hPort);
    hPort = INVALID_HANDLE_VALUE;
#if (WINDOWSPC>0) || NEWCOMM // 091206
    Sleep(2000); // needed for windows bug
#endif
    ComPort_StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"),
	// LKTOKEN  _@M760_ = "Unable to Set Serial Port Timers" 
                 gettext(TEXT("_@M760_")), sPortName);
    dwError = GetLastError();
    return -1;
  }

  return result;
}

unsigned long ComPort::SetBaudrate(unsigned long BaudRate)
{
  COMSTAT ComStat;
  DCB     PortDCB;
  DWORD   dwErrors;
  unsigned long result = 0;

  if (hPort == INVALID_HANDLE_VALUE)
    return result;

  do {
    ClearCommError(hPort, &dwErrors, &ComStat);
  } while (ComStat.cbOutQue > 0);

  Sleep(10);

  GetCommState(hPort, &PortDCB);

  result = PortDCB.BaudRate;
  PortDCB.BaudRate = BaudRate;
  
  if (!SetCommState(hPort, &PortDCB))
    return 0;

  return result;
}

int ComPort::Read(void *Buffer, size_t Size)
{
  DWORD dwBytesTransferred;

  if (hPort == INVALID_HANDLE_VALUE)
    return -1;

  if (ReadFile(hPort, Buffer, Size, &dwBytesTransferred, 
                (OVERLAPPED *)NULL)) {
    return dwBytesTransferred;
  }

  return -1;
}


void ComPort::ProcessChar(char c) {
  if (bi<(NMEA_BUF_SIZE-1)) {

	BuildingString[bi++] = c;

#if 100430
	if(c=='\n' || c=='\r') {
		// abcd\n , now closing the line also with \r
		BuildingString[bi] = '\0';

		if (c == '\r') BuildingString[bi-1]='\n';

		// process only meaningful sentences, avoid processing a single \n \r etc.
		if (bi>5) { // 100430
			LockFlightData();
			devParseNMEA(devIdx, BuildingString, &GPS_INFO);
			UnlockFlightData();
		}
	} else {
		return;
	}
#else
	if(c=='\n') {
		// abcd\n 
		BuildingString[bi] = '\0';

		// do not consider CR in ProcessChar inside Port
		// bi-2==\r bi-1==\n bi=\0
		if (bi>1 && (BuildingString[bi-2]==0x0d) ) {
			BuildingString[bi-2] = '\n';
			BuildingString[bi-1] = '\0';
		}
		LockFlightData();
		devParseNMEA(devIdx, BuildingString, &GPS_INFO);
		UnlockFlightData();
	} else {
		return;
	}
#endif
  }
  // overflow, so reset buffer
  bi = 0;
}

