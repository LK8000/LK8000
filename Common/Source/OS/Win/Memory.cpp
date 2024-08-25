/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


size_t CheckFreeRam(void) {
  MEMORYSTATUS    memInfo;
  // Program memory
  memInfo.dwLength = sizeof(memInfo);
  GlobalMemoryStatus(&memInfo);

  //	   memInfo.dwTotalPhys,
  //	   memInfo.dwAvailPhys,
  //	   memInfo.dwTotalPhys- memInfo.dwAvailPhys);

  return memInfo.dwAvailPhys;
}

// check maximum allocatable heap block
unsigned long CheckMaxHeapBlock(void) {

    // try allocate maximum block (of course on PC with disk swapping, we will not
    // try maximum block, function just returns something near to initial top value)
    size_t top = 100*1024*1024; // start with 100MB/2
    size_t btm = 0;

    void*  addr;
    size_t size;

    while ((size = (btm + top) / 2) != 0) { // ~ btm + (top - btm) / 2
      addr = malloc(size);
      if (addr == NULL)
        top = size;
      else {
        free(addr);
        if ((top - btm) < 1024) // 1 KB accuracy
          return(size);
        btm = size;
      }
    }

    return(0);
}

// This is necessary to be called periodically to get rid of
// memory defragmentation, since on pocket pc platforms there is no
// automatic defragmentation.
void MyCompactHeaps() {
#ifndef UNDER_CE
  HeapCompact(GetProcessHeap(),0);
#else
  typedef DWORD (_stdcall *CompactAllHeapsFn) (void);
  static CompactAllHeapsFn CompactAllHeaps = NULL;
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    CompactAllHeaps = (CompactAllHeapsFn)
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("CompactAllHeaps"));
    init=true;
  }
  if (CompactAllHeaps) {
    CompactAllHeaps();
  }
#endif
}


size_t FindFreeSpace(const TCHAR *path) {
  // returns number of kb free on destination drive

  ULARGE_INTEGER FreeBytesAvailableToCaller;
  ULARGE_INTEGER TotalNumberOfBytes;
  ULARGE_INTEGER TotalNumberOfFreeBytes;
  TestLog(_T("... FindFreeSpace <%s> start"),path);
  if (GetDiskFreeSpaceEx(path,
			 &FreeBytesAvailableToCaller,
			 &TotalNumberOfBytes,
			 &TotalNumberOfFreeBytes)) {
      TestLog(_T("... FindFreeSpace ok"));

    return FreeBytesAvailableToCaller.LowPart/1024;
  } else {
    TestLog(_T("... FindFreeSpace failed, error=%ld"), GetLastError());
    return 0;
  }
}
