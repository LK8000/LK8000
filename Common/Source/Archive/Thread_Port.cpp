/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "TraceThread.h"

extern void ComPort_StatusMessage(UINT type, const TCHAR *caption, const TCHAR *fmt, ...);
extern void Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);


DWORD WINAPI ComPort::ThreadProc(LPVOID prt)
{
  ComPort *port = (ComPort *)prt;
  port->ReadThread();
  return 0;
}

DWORD ComPort::ReadThread()
{
  #if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM	// 100222
  DWORD dwCommModemStatus=0;
  #endif
  DWORD dwBytesTransferred=0; // 091117 initialized variables
  BYTE inbuf[1024];
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;

  Purge();
  #if TESTBENCH
  StartupStore(_T(". ReadThread running on port %d%s"),sportnumber+1,NEWLINE);
  #endif
  #if TRACETHREAD
  StartupStore(_T("##############  PORT=%d threadid=%d\n"),sportnumber,GetCurrentThreadId());
  if (sportnumber=0) _THREADID_PORT1=GetCurrentThreadId();
  if (sportnumber=1) _THREADID_PORT2=GetCurrentThreadId();
  if (sportnumber!=1 && sportnumber!=2) _THREADID_UNKNOWNPORT=GetCurrentThreadId();
  #endif

  
  // Specify a set of events to be monitored for the port.

  dwMask = EV_RXFLAG | EV_CTS | EV_DSR | EV_RING | EV_RXCHAR;

  // #if !defined(WINDOWSPC) || (WINDOWSPC == 0) 091206
  #if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM
  SetCommMask(hPort, dwMask);
  #endif
#if (WINDOWSPC<1)
  if (!PollingMode) SetCommMask(hPort, dwMask);
#endif

  fRxThreadTerminated = FALSE;
  DWORD dwErrors=0;
  COMSTAT comStat;
  short valid_frames=0;

  while ((hPort != INVALID_HANDLE_VALUE) && (!MapWindow::CLOSETHREAD) && (!CloseThread)) 
  {
	GetThreadTimes( hReadThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);

	ClearCommError(hPort,&dwErrors,&comStat);
	if ( dwErrors & CE_FRAME ) {
		//StartupStore(_T("... Com port %d, dwErrors=%ld FRAME (old status=%d)\n"),
		//	sportnumber,dwErrors,ComPortStatus[sportnumber]);
		ComPortStatus[sportnumber]=CPS_EFRAME;
		ComPortErrRx[sportnumber]++;
		valid_frames=0;
	} else {
		if (++valid_frames>10) { 
			valid_frames=20; 
			ComPortStatus[sportnumber]=CPS_OPENOK;
		}
	}

	#if (WINDOWSPC>0) || NEWCOMM // 091206
	// PC version does BUSY WAIT
	Sleep(50);  // ToDo rewrite the whole driver to use overlaped IO on W2K or higher
	#else
	if (PollingMode)  
		Sleep(100);
	else
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
	if (PollingMode || (dwCommModemStatus & EV_RXFLAG) || (dwCommModemStatus & EV_RXCHAR)) // Do this only for non-PC
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
				ComPortRx[sportnumber]+=dwBytesTransferred; // 100210
			} else {
				dwBytesTransferred = 0;
			}

			Sleep(50); // JMW20070515: give port some time to
			// fill... prevents ReadFile from causing the
			// thread to take up too much CPU
			if ( (GetThreadTimes( hReadThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
				if(sportnumber==0)
					Cpu_PortA=9999;
				else
					Cpu_PortB=9999;
			} else {
				Cpustats((sportnumber==0)?&Cpu_PortA:&Cpu_PortB,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
			}
		  
			if (CloseThread) {
				dwBytesTransferred = 0;
				StartupStore(_T(". ComPort %d ReadThread: CloseThread ordered%s"),sportnumber+1,NEWLINE);
			}
		} while (dwBytesTransferred != 0);
	}

	// give port some time to fill
	Sleep(5);

	// Retrieve modem control-register values.
	#if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) 

	if (!PollingMode)
	// this is causing problems on PC BT, apparently. Setting Polling will not call this, but it is a bug
	GetCommModemStatus(hPort, &dwCommModemStatus);

	#endif

  }

  Purge();

  fRxThreadTerminated = TRUE;
  StartupStore(_T(". ComPort %d ReadThread: terminated%s"),sportnumber+1,NEWLINE);

  return 0;
}


// Stop Rx Thread
// return: TRUE on success, FALSE on error
BOOL ComPort::StopRxThread()
{  
  if (hPort == INVALID_HANDLE_VALUE) return FALSE;
  if (fRxThreadTerminated) return TRUE;

  CloseThread = TRUE;

// currently NEWCOMM is NOT used
#if (WINDOWSPC>0) || NEWCOMM  // 091206
  DWORD tm = GetTickCount()+20000l;
  while (!fRxThreadTerminated && ((tm-GetTickCount()) > 0) ) {
	Sleep(10);
  }
  if (!fRxThreadTerminated) {
	TerminateThread(hReadThread, 0);
  } 
  ::WaitForSingleObject(hReadThread, 5000);
  CloseHandle(hReadThread);
#else
  Flush();

  // setting the comm event mask with the same value
  //  GetCommMask(hPort, &dwMask);
  SetCommMask(hPort, dwMask);          // will cancel any
                                        // WaitCommEvent!  this is a
                                        // documented CE trick to
                                        // cancel the WaitCommEvent

  int count=0;
  while (!fRxThreadTerminated && (count<2000)){ 
	Sleep(10);
	count++;
  }

  if (!fRxThreadTerminated) {
	#if 101122
	TerminateThread(hReadThread, 0);
	StartupStore(_T("...... ComPort StopRxThread: RX Thread forced to terminate!%s"),NEWLINE);
	#else

	// LKTOKEN  _@M540_ = "RX Thread not Terminated!" 
	ComPort_StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), sPortName, gettext(TEXT("_@M540_")));
	StartupStore(_T("...... ComPort %d StopRxThread: RX Thread not terminated!%s"),sportnumber+1,NEWLINE);

	#endif
	//#endif
  } else {
	CloseHandle(hReadThread);
	StartupStore(_T(". ComPort %d StopRxThread: RX Thread terminated%s"),sportnumber+1,NEWLINE);
  }
#endif

  return fRxThreadTerminated;
}


// Restart Rx Thread
// return: TRUE on success, FALSE on error
BOOL ComPort::StartRxThread(void)
{
  DWORD dwThreadID;

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
	//DWORD dwError = GetLastError();
	return FALSE;
  }

  return TRUE;
}


