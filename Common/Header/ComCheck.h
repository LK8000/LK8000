/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.ORG
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#ifndef COMCHECK_H
#define COMCHECK_H


// Up to 15 nmea sentences per second, and we need at least 3 seconds
#define CC_NUMBUFLINES   50
#define CC_BUFSIZE  (MAX_NMEA_LEN+1)

extern TCHAR ComCheckBuffer[CC_NUMBUFLINES][CC_BUFSIZE];
extern unsigned int ComCheck_LastLine;
extern short ComCheck_ActivePort;
extern short ComCheck_Reset;
extern bool ComCheck_BufferFull;

extern void ComCheck_Init(void);
extern void ComCheck_AddChar(TCHAR c);
void ComCheck_AddText(const TCHAR* Text);



#endif
