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
  #if defined(HC_DMALLOC) ||  defined(HC_DUMA)
    // when using heap checker, do not try allocate maximum size - malloc() can
    // return NULL which heap checker recognizes as an error and will terminate
    // program immediately when configured so (can be confusing for developer)
    return(0xFFFFFFFF);
  #else
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
  #endif
}


#if defined(UNDER_CE) && !defined(NDEBUG)
_CrtMemState memstate_s1;
#endif

void MemCheckPoint()
{
#if defined(UNDER_CE) && !defined(NDEBUG)
  _CrtMemCheckpoint( &memstate_s1 );
#endif
}


void MemLeakCheck() {
#if defined(UNDER_CE) && !defined(NDEBUG)

  _CrtMemState memstate_s2, memstate_s3;

   // Store a 2nd memory checkpoint in s2
   _CrtMemCheckpoint( &memstate_s2 );

   if ( _CrtMemDifference( &memstate_s3, &memstate_s1, &memstate_s2 ) ) {
     _CrtMemDumpStatistics( &memstate_s3 );
     _CrtMemDumpAllObjectsSince(&memstate_s1);
   }

  _CrtCheckMemory();
#endif
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
  #if TESTBENCH
  StartupStore(_T("... FindFreeSpace <%s> start\n"),path);
  #endif
  if (GetDiskFreeSpaceEx(path,
			 &FreeBytesAvailableToCaller,
			 &TotalNumberOfBytes,
			 &TotalNumberOfFreeBytes)) {
      #if TESTBENCH
      StartupStore(_T("... FindFreeSpace ok\n"));
      #endif

    return FreeBytesAvailableToCaller.LowPart/1024;
  } else {
      #if TESTBENCH
      StartupStore(_T("... FindFreeSpace failed, error=%ld\n"),GetLastError());
      #endif
    return 0;
  }
}
