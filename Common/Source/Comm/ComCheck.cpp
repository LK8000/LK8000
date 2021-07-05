/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "ComCheck.h"

TCHAR ComCheckBuffer[CC_NUMBUFLINES][CC_BUFSIZE];
unsigned int ComCheck_LastLine;
short ComCheck_ActivePort=-1, ComCheck_Reset=-1;
bool ComCheck_BufferFull;
TCHAR lastChar='\0';
// Better to leave the buffers always allocated even if unused in SIM mode
// This is why we dont use malloc.

// These functions are called by device thread only

void ComCheck_Init(void)
{
    StartupStore(_T(". ComCheck Init%s"),NEWLINE);

    for (short i=0; i<CC_NUMBUFLINES; i++)
        memset(ComCheckBuffer[i],0, CC_BUFSIZE*sizeof(TCHAR));

    ComCheck_LastLine=0;
    ComCheck_BufferFull=false;
    ComCheck_ActivePort=-1;
    lastChar='\0';
}

void ComCheck_NewLine() {
    if (ComCheck_LastLine>=(CC_NUMBUFLINES-1)) {
        ComCheck_LastLine=0;
        ComCheck_BufferFull=true;
    } else {
        ComCheck_LastLine++;
    }
    ComCheckBuffer[ComCheck_LastLine][0]=_T('\0');
}

//
// We should lock this function
// 
void ComCheck_AddChar(TCHAR c) {

#ifndef UNICODE
    if(!::isascii(c)) {
        c = '?';
    }
#endif

    const unsigned MaxLineSize = std::size(ComCheckBuffer[ComCheck_LastLine])-1;
            
    if (ComCheck_Reset>=0) {
        ComCheck_Init();
        ComCheck_ActivePort=ComCheck_Reset;
        ComCheck_Reset=-1;
    }
    
    if(lastChar != '\r' || c != '\n') {
        size_t nSize = _tcslen(ComCheckBuffer[ComCheck_LastLine]);
        if( (c == '\n') || (c == '\r') || (nSize >= MaxLineSize) ) {
            ComCheck_NewLine();
            nSize = 0;
        } else {
            ComCheckBuffer[ComCheck_LastLine][nSize+1] = ('\0');
            ComCheckBuffer[ComCheck_LastLine][nSize] = c;
        }
    }
    lastChar = c;
}


void ComCheck_AddText(const TCHAR* Text)
{
  if(Text != NULL)
  {
    for(int i =0; i < min((int)_tcslen(Text),(CC_BUFSIZE-2)); i++)
    {
	  ComCheck_AddChar(Text[i]);
    }
  }
}

