

#ifndef NOINSTHREAD
// this thread currently does nothing. May be used for background parallel calculations or new gauges
DWORD InstrumentThread (LPVOID lpvoid) {
	(void)lpvoid;
  #ifdef CPUSTATS
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;
  #endif

  // watch out for a deadlock here. This has to be done before waiting for DisplayRunning..
  goInstrumentThread=true; // 091119

  // wait for proper startup signal
  while (!MapWindow::IsDisplayRunning()) {
	Sleep(100);
  }

  while (!MapWindow::CLOSETHREAD) {

	#ifdef CPUSTATS
	GetThreadTimes( hInstrumentThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
	#endif
	WaitForSingleObject(varioTriggerEvent, 5000);
	ResetEvent(varioTriggerEvent);
	if (MapWindow::CLOSETHREAD) break; // drop out on exit

	// DO NOTHING BY NOW
	// if triggervario, render vario update eventually here
	Sleep(10000);
	#ifdef CPUSTATS
	if ( (GetThreadTimes( hInstrumentThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
		Cpu_Instrument=9999;
	} else {
		Cpustats(&Cpu_Instrument,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
	}
	#endif
  }
  return 0;
}
#endif


