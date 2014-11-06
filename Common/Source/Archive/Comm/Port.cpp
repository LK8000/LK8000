/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Port.cpp,v 8.5 2010/12/12 16:18:34 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"


void ComPort_StatusMessage(UINT type, const TCHAR *caption, const TCHAR *fmt, ...)
{
  TCHAR tmp[127];
  va_list ap;

  va_start(ap, fmt);
  _vsntprintf(tmp, 127, fmt, ap);
  va_end(ap);

  tmp[126] = _T('\0');

  if (caption)
    MessageBoxX(tmp, gettext(caption), type);
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
BOOL ComPort::Initialize(LPCTSTR lpszPortName, DWORD dwPortSpeed, DWORD dwPortBit, DWORD dwPortNumber)
{
  DWORD dwError;
  DCB PortDCB;
  TCHAR lkbuf[100];
  TCHAR lkPortName[10]; // 9 should be enough
  if (SIMMODE) return FALSE;

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

  StartupStore(_T(". ComPort %u Initialize <%s> speed=%u bit=%u %s"),dwPortNumber+1,lkPortName,dwPortSpeed,8-dwPortBit,NEWLINE);

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
	_stprintf(lkbuf,_T("... ComPort %u Init failed, error=%u%s"),dwPortNumber+1,dwError,NEWLINE); // 091117
	StartupStore(lkbuf);
	// LKTOKEN  _@M762_ = "Unable to open port" 
	ComPort_StatusMessage(MB_OK|MB_ICONINFORMATION, NULL, TEXT("%s %s"), gettext(TEXT("_@M762_")), lkPortName);
	return FALSE;
  }
  StartupStore(_T(". ComPort %u  <%s> is now open%s"),dwPortNumber+1,lkPortName,NEWLINE);

  PortDCB.DCBlength = sizeof(DCB);     

  // Get the default port setting information.
  if (GetCommState(hPort, &PortDCB)==0) {
	dwError = GetLastError();
	_stprintf(lkbuf,_T("... ComPort %u GetCommState failed, error=%u%s"),dwPortNumber+1,dwError,NEWLINE);
	StartupStore(lkbuf);
	// cannot set serial port timers. good anyway
	ComPort_StatusMessage(MB_OK|MB_ICONINFORMATION, NULL, TEXT("%s %s"), gettext(TEXT("_@M760_")), lkPortName);
	return FALSE;
  }

  // Change the DCB structure settings.
  PortDCB.BaudRate = dwPortSpeed;       // Current baud 
  PortDCB.fBinary = TRUE;               // Binary mode; no EOF check 
  PortDCB.fParity = TRUE;               // Enable parity checking  
  PortDCB.fOutxCtsFlow = FALSE;         // CTS output flow control: when TRUE, and CTS off, output suspended
  PortDCB.fOutxDsrFlow = FALSE;         // DSR output flow control 
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
#if !(WINDOWSPC>0)
	if (PollingMode) Sleep(2000);
#endif
	// LKTOKEN  _@M759_ = "Unable to Change Settings on Port" 
	ComPort_StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M759_")), lkPortName);
	dwError = GetLastError();
	_stprintf(lkbuf,_T("... ComPort %u Init <%s> change setting FAILED, error=%u%s"),dwPortNumber+1,lkPortName,dwError,NEWLINE); // 091117
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
  sportnumber=dwPortNumber; // 100210
  if (!StartRxThread()){
	_stprintf(lkbuf,_T("... ComPort %u Init <%s> StartRxThread failed%s"),dwPortNumber+1,lkPortName,NEWLINE);
	StartupStore(lkbuf);
	if (!CloseHandle(hPort)) {
		dwError = GetLastError();
		_stprintf(lkbuf,_T("... ComPort %u Init <%s> close failed, error=%u%s"),dwPortNumber+1,lkPortName,dwError,NEWLINE);
		StartupStore(lkbuf);
	} else {
		_stprintf(lkbuf,_T("... ComPort %u Init <%s> closed%s"),dwPortNumber+1,lkPortName,NEWLINE);
		StartupStore(lkbuf);
	}
	hPort = INVALID_HANDLE_VALUE;
#if (WINDOWSPC>0) || NEWCOMM // 091206
	Sleep(2000); // needed for windows bug
#endif
#if !(WINDOWSPC>0)
	if (PollingMode) Sleep(2000);
#endif

	return FALSE;
  }

  _stprintf(lkbuf,_T(". ComPort %u Init <%s> end OK%s"),dwPortNumber+1,lkPortName,NEWLINE);
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

  DWORD dwNumBytesWritten;

  if (!WriteFile(hPort,              // Port handle
                 &Byte,               // Pointer to the data to write 
                 1,                   // Number of bytes to write
                 &dwNumBytesWritten,  // Pointer to the number of bytes 
                                      // written
                 (OVERLAPPED *)NULL)) // Must be NULL for Windows CE
  {
    // WriteFile failed. Report error.
    // DWORD dwError = GetLastError();
  }
}

void ComPort::Flush(void)
{
  FlushFileBuffers(hPort);
}

void ComPort::Purge(void)
{
  PurgeComm(hPort, 
            PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
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
		_stprintf(lkbuf,_T("... ComPort %u close failed, error=%u%s"),sportnumber+1,dwError,NEWLINE);
		StartupStore(lkbuf);
		return FALSE;
	} else {
		#if (WINDOWSPC>0) || NEWCOMM // 091206
		Sleep(2000); // needed for windows bug
		#endif
		hPort = INVALID_HANDLE_VALUE; 
		_stprintf(lkbuf,_T(". ComPort %u closed Ok.%s"),sportnumber+1,NEWLINE);
		StartupStore(lkbuf); // 100210 BUGFIX missing
		return TRUE;
	}
  }
  StartupStore(_T("... ComPort %u Close failed, invalid handle%s"),sportnumber+1,NEWLINE);
  return FALSE;
}

// writes unmodified data to the port
bool ComPort::Write(const void *data, size_t length)
{
  DWORD written;
  
  if (hPort == INVALID_HANDLE_VALUE)
    return(false);
   
  if (!WriteFile(hPort, data, length, &written, NULL) || written != length) {
    // WriteFile failed, report error
      ComPortErrTx[sportnumber]++;
      ComPortErrors[sportnumber]++;
    return(false);
  }
  
    ComPortTx[sportnumber] += written;
  
  return(true);
}

// this is used by all functions to send data out
// it is called internally from thread for each device
void ComPort::WriteString(const TCHAR *Text)
{
  char tmp[512];
  int len = _tcslen(Text);
  
  len = WideCharToMultiByte(CP_ACP, 0, Text, len + 1, tmp, sizeof(tmp), NULL, NULL);
  
  // don't write trailing '\0' to device
  if (--len <= 0)
    return;
  
  Write(tmp, len);
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
    //DWORD dwError = GetLastError();
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

unsigned long ComPort::GetBaudrate()
{
  COMSTAT ComStat;
  DCB     PortDCB;
  DWORD   dwErrors;
  
  if (hPort == INVALID_HANDLE_VALUE)
    return 0;
  
  do {
    ClearCommError(hPort, &dwErrors, &ComStat);
  } while (ComStat.cbOutQue > 0);
  
  Sleep(10);
  
  GetCommState(hPort, &PortDCB);
  
  return PortDCB.BaudRate;
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
  if (bi<(MAX_NMEA_LEN-1)) {

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

