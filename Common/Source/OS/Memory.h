/* 
 * File:   Memory.h
 * Author: blc
 *
 * Created on January 13, 2015, 3:50 PM
 */

#ifndef MEMORY_H
#define	MEMORY_H

size_t CheckFreeRam(void);
size_t FindFreeSpace(const TCHAR *path);

void MyCompactHeaps();

#ifdef HC_DMALLOC
// check maximum allocatable heap block
size_t CheckMaxHeapBlock(void);
#endif

#ifdef WIN32
// Unused
void MemCheckPoint();
void MemLeakCheck();
#endif

#endif	/* MEMORY_H */

