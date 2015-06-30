/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "ComCheck.h"

extern bool GotFirstBaroAltitude; // used by UpdateBaroSource
extern double LastRMZHB;	 // common to both devA and devB, updated in Parser
extern NMEAParser nmeaParser1;
extern NMEAParser nmeaParser2;


TCHAR ComCheckBuffer[CC_NUMBUFLINES][CC_BUFSIZE];
unsigned int ComCheck_LastLine;
short ComCheck_ActivePort=-1, ComCheck_Reset=-1;
bool ComCheck_BufferFull;

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
}

//
// We should lock this function
// 
void ComCheck_AddLine(TCHAR *tline) {

    if (ComCheck_Reset>=0) {
        ComCheck_Init();
        ComCheck_ActivePort=ComCheck_Reset;
        ComCheck_Reset=-1;
        return;
    }
    if ( _tcslen(tline)==0 ) {
        return;
    }

    int nextline;

    if (ComCheck_LastLine>=(CC_NUMBUFLINES-1)) {
        nextline=0;
        ComCheck_BufferFull=true;
    } else {
        nextline=ComCheck_LastLine+1;
    }
    _tcscpy(ComCheckBuffer[nextline],tline);
    ComCheck_LastLine=nextline;
}



