/* 
 * This is not under GPL
 */
#ifndef _FILEHANDLERS_H_
#define _FILEHANDLERS_H_


#include <stdio.h>
#include <unistd.h>

#include <list>
#include <string>

typedef std::list<std::string> LineList;

// Read input file into memory buffer
int ReadInputFile(LineList &lines, const TCHAR *filename);

// Write file from memory buffer
int WriteOutputFile(LineList &lines, const TCHAR *filename);

#endif
