/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKLanguage.cpp,v 1.4 2010/12/20 23:35:24 root Exp root $
 */

#include "StdAfx.h"
#include <stdio.h>
#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#endif
#include "options.h"
#include "externs.h"
#include "XCSoar.h"
#include "InfoBoxLayout.h"
#include "Utils2.h"
#include "Cpustats.h"
#include "device.h"
#include "Logger.h"
#include "Parser.h"
#include "WaveThread.h"
#include "GaugeFLARM.h"
#include "LKUtils.h"
#include "Message.h"
#include "McReady.h"
#include "InputEvents.h"

#include <ctype.h>


// #define DEBUG_GETTEXT	1
#define LKD_LANGUAGE	"_Language"
#define MAX_HELP	1024	// complete help including several lines, and also for each single line

#define MAX_MESSAGES		950 // dynamically allocated
#define MAX_MESSAGE_SIZE	150 // just for setting a limit

bool LKLoadMessages(void);

// _@Hnnnn@
// minimal: _@H1_  maximal: _@H1234_

TCHAR *LKgethelptext(const TCHAR *TextIn) {

  static TCHAR sFile[MAX_PATH];
  static TCHAR sPath[MAX_PATH];
  static TCHAR sTmp[MAX_HELP+1];
  static TCHAR sHelp[MAX_HELP+1];

  bool foundnotfound=false;

  if (TextIn == NULL) return (TCHAR *)TextIn;
  short tlen=_tcslen(TextIn);
  if (tlen<5 || tlen>8) return (TCHAR *)TextIn;
  if ( (TextIn[0]!='_') || (TextIn[1]!='@') || (TextIn[tlen-1]!='_') ) return (TCHAR *)TextIn;


  // get the item index number, quick conversion from unicode
  char snum[6];
  // char *pnum=(char *)&TextIn[3]; 

  short i;
  for (i=0; i<tlen-4; i++) {
	/*
	snum[i++]=*pnum; // advance to unicode 0
	pnum++; // advance to next char
	pnum++; // advance to next char
	*/
	snum[i] = (char)TextIn[3+i];
  }
  snum[i]='\0';
  
  unsigned short inumber=atoi(snum);
  #if DEBUG_GETTEXT
  StartupStore(_T(".... Help item TextIn=<%s> snum= <%S> number=%d \n"),TextIn, snum, inumber);
  #endif

  if (inumber>9999) {
	#if DEBUG_GETTEXT
	StartupStore(_T(".... Help item snum= <%S> number=%d \n"),snum, inumber);
	#endif
	_stprintf(sHelp,_T("ERROR, wrong index number <%d> from XML: <%s>\r\n"),inumber,TextIn);
	return (sHelp);
  }


  // get the type character
  char ttype = TextIn[2];
  TCHAR suffix[20];

  if (ttype=='H') {
	_tcscpy(suffix,_T("_HELP.TXT"));
	LocalPath(sPath,_T(LKD_LANGUAGE));
	_stprintf(sFile,_T("%s\\%s%s"),sPath,LKLangSuffix,suffix);

	// Help File, dynamically handled
	#if DEBUG_GETTEXT
  	StartupStore(_T("... Open Language file type <%C>: <%s>%s"),ttype,sFile,NEWLINE);
	#endif

	TCHAR sNum[10];
	_stprintf(sNum,_T("%d"),inumber);

	HANDLE hHelpFile;
	hHelpFile = INVALID_HANDLE_VALUE;
	hHelpFile = CreateFile(sFile, GENERIC_READ,0,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
	if( hHelpFile == INVALID_HANDLE_VALUE) {
		#if ALPHADEBUG
		StartupStore(_T("... Missing HELP FILE <%s>%s"),sFile,NEWLINE);
		#endif
		// we can only have one Help call at a time, from the user interface. Ok static sHelp.
		_stprintf(sHelp,_T("ERROR, help file not found:\r\n%s\r\nCheck configuration!"),sFile);
		return (sHelp);
	}

	short filetype=FileIsUTF16(hHelpFile);

	// search for beginning of code index   @000
	bool found=false;
	while ( ReadUString(hHelpFile,READLINE_LENGTH-1,sTmp,filetype) ) {
		int slen=_tcslen(sTmp); // includes cr or lf or both
		if (slen<3|| slen>8) {
			#if DEBUG_GETTEXT
			StartupStore(_T("... skip line <3||>8 : %s\n"),sTmp);
			#endif
			continue;
		}
		
		if (sTmp[0]=='#') {
			#if DEBUG_GETTEXT
			StartupStore(_T("... skip remark: %s\n"),sTmp);
			#endif
			continue;
		}
		if (sTmp[0]=='@') {
			for (i=1; i<slen; i++) {
				if ( sTmp[i] < '0' || sTmp[i] > '9' )  {
					sTmp[i] = '\0';
					break;
				}
			}
			// sTmp[slen-1]='\0'; // remove cr
			if ( _tcscmp(&sTmp[1],sNum) == 0 ) {
				#if DEBUG_GETTEXT
				StartupStore(_T("... found correct index: %s\n"),sTmp);
				#endif
				found=true;
				break;
			} else {
				// this one should be the very last line in the help file
				if ( _tcscmp(&sTmp[1],_T("9999")) == 0 ) {
					foundnotfound=true;
					#if DEBUG_GETTEXT
					StartupStore(_T("... found NOTFOUND index: %s\n"),&sTmp[1]);
					#endif
					// warning this means that placing 9999 not at the end of HELP file will
					// make all other messages ignored! always check HELP file to have 9999 at the end
					break;
				} else {
					#if DEBUG_GETTEXT
					StartupStore(_T("... found wrong index: %s not %s\n"),&sTmp[1],sNum);
					#endif
				}
			}
		}
	}
	if (!found && !foundnotfound) {
		#if DEBUG_GETTEXT
		StartupStore(_T("... index <%s> not found in help file <%s>\n"),sNum,sFile);
		#endif
		_stprintf(sHelp,_T("ERROR: index <%s> not found in language help file:\r\n%s\r\n"),sNum,sFile);
		CloseHandle(hHelpFile);
		return (sHelp);
	}

	// now load the help text for this index
	_tcscpy(sHelp,_T(""));
	int hlen=0;
	while ( ReadUString(hHelpFile,READLINE_LENGTH-1,sTmp,filetype) ) {

		int slen=_tcslen(sTmp); // including cr or lf or both
		if (slen==0 || sTmp[0]=='#') continue;
		if (slen>2 && slen<9) {
			// is it another index marker?
			if (sTmp[0]=='@') {
				if (hlen==0)
					continue; // multihelp section, continue loading help
				else 
					break; // another marker, close the help
			}
		}
		if ( sTmp[slen-1]=='\r' ) {
			sTmp[slen-1]='\0';
			slen--;
		}

		// add it to the help, if possible
		if ( (hlen+slen+2) > MAX_HELP) {
			#if DEBUG_GETTEXT
			StartupStore(_T("... help too long: truncating line <%s>\n"),sTmp);
			#endif
			_tcsncat(sHelp,sTmp,MAX_HELP-hlen-1);
			break;
		}

		hlen+=slen;
		#if DEBUG_GETTEXT
		//StartupStore(_T(".. adding line <%s>\n"),sTmp);
		#endif
		_tcscat(sHelp,sTmp);
		_tcscat(sHelp,_T("\r\n"));
	}

	CloseHandle(hHelpFile);
	return (sHelp);

  } // end ttype == H

  #if ALPHADEBUG
  StartupStore(_T(".... Unknown Text type <%c> in <%s>%s"),ttype,TextIn,NEWLINE);
  #endif
  return (TCHAR *)TextIn;

}



static TCHAR * LKMessages[MAX_MESSAGES+1];
static short LKMessagesIndex[MAX_MESSAGES+1];

//  Tokenized Language support for LK8000
//  gettext is now a definition for LKGetText
//  101208 
TCHAR *LKGetText(const TCHAR *TextIn) {

  // quick preliminar checks
  if (TextIn == NULL) return (TCHAR *)TextIn;
  short tlen=_tcslen(TextIn);
  if (tlen<5 || tlen>8) return (TCHAR *)TextIn;
  if ( (TextIn[0]!='_') || (TextIn[1]!='@') || (TextIn[tlen-1]!='_') ) return (TCHAR *)TextIn;

  // get the item index number, quick conversion from unicode
  char snum[6];
  short i;
  for (i=0; i<tlen-4; i++) {
	snum[i] = (char)TextIn[3+i];
  }
  snum[i]='\0';
  unsigned short inumber=atoi(snum);


  #if DEBUG_GETTEXT
  //StartupStore(_T(".... LKGettext item TextIn=<%s> snum= <%S> number=%d \n"),TextIn, snum, inumber);
  #endif

  // get the type character
  char ttype = TextIn[2];

  // Text messages inside C code
  if (ttype=='M') { // Message
	if (inumber> MAX_MESSAGES) return (TCHAR *)TextIn; // safe check
	int pointer= LKMessagesIndex[inumber];
        if (pointer<0 || pointer>MAX_MESSAGES) return (TCHAR *)TextIn;
	return (TCHAR *)LKMessages[pointer];
  }
  return (TCHAR *)TextIn;


}


void LKReadLanguageFile() {
  static bool doinit=true;
  static TCHAR oldLang[4];

  if (doinit) {
	_tcscpy(LKLangSuffix,_T(""));
	_tcscpy(oldLang,_T("XXX"));
	doinit=false;
  }

  bool english=false;
  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  _tcscpy(LKLangSuffix,_T(""));
  GetRegistryString(szRegistryLanguageFile, szFile1, MAX_PATH);
  tryeng:
  if (_tcslen(szFile1)==0) {
	_tcscpy(szFile1,_T("%LOCAL_PATH%\\\\_Language\\ENGLISH.LNG"));
	english=true;
  }
  ExpandLocalPath(szFile1);
  // SetRegistryString(szRegistryLanguageFile, TEXT("\0")); // ?

  HANDLE hLangFile;
  hLangFile = INVALID_HANDLE_VALUE;
  hLangFile = CreateFile(szFile1, GENERIC_READ,0,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
  if( hLangFile == INVALID_HANDLE_VALUE) {
	if (english) {
		StartupStore(_T("--- CRITIC, NO ENGLISH LANGUAGE FILES!%s"),NEWLINE);
		// critic point, no default language! BIG PROBLEM here!
		for (unsigned short i=0; i<MAX_MESSAGES; i++) {
			LKMessages[i]=NULL;
			LKMessagesIndex[i]=-1;
		}
		return;
		
	} else {
		StartupStore(_T("--- NO LANGUAGE FILE FOUND <%s>, retrying with ENGlish!%s"),szFile1,NEWLINE);
		_tcscpy(szFile1,_T(""));
		goto tryeng;
	}
	return;
  }
  short filetype=FileIsUTF16(hLangFile);

  bool found=false;
  TCHAR sTmp[200];
  TCHAR mylang[30];
  while ( ReadUString(hLangFile,199,sTmp,filetype) ) {
	if (_tcslen(sTmp)<3) continue;
	if ((sTmp[0]=='L')&&(sTmp[1]=='=')) {
		_tcscpy(mylang,&sTmp[2]);

		for (unsigned short i=0; i<_tcslen(mylang); i++) {
			if (mylang[i]=='\r' || mylang[i]=='\n') {
				mylang[i]='\0';
				break;
			}
		}
		found=true;
		break;
	}
  }

  if (found) {
	if (_tcslen(mylang)>3) mylang[3]='\0';
	_tcscpy(LKLangSuffix,mylang);
  }

  if (_tcscmp(oldLang,LKLangSuffix)!=0) { 

	if ( !LKLoadMessages() ) {
		// force reload of english
		if (_tcscmp(_T("ENG"),LKLangSuffix) == 0 ) {
			StartupStore(_T("... CRITICAL, no english langauge available!%s"),NEWLINE);
		} else {
			StartupStore(_T("... LoadText failed, fallback to english language\n"));
			_tcscpy(szFile1,_T("%LOCAL_PATH%\\\\_Language\\ENGLISH.LNG"));
			SetRegistryString(szRegistryLanguageFile, szFile1); 
			_tcscpy(LKLangSuffix,_T("ENG"));
			LKLoadMessages();
		}
	} else 
	  	_tcscpy(oldLang,mylang);
  }
  CloseHandle(hLangFile);
  return;
}


// Load or reload language messages
bool LKLoadMessages(void) {
  TCHAR sFile[MAX_PATH];
  TCHAR sPath[MAX_PATH];
  TCHAR suffix[20];
  unsigned int i, j;
  #if DEBUG_GETTEXT
  int maxsize=0;
  #endif

  static bool doinit=true;

  if (doinit) {
	for (i=0; i<MAX_MESSAGES; i++) {
		LKMessages[i]=NULL;
		LKMessagesIndex[i]=-1;
	}
	doinit=false;
  } else {
	// init data when reloading language files or changing it
	for (i=0; i<MAX_MESSAGES; i++) {
		if (LKMessages[i] != NULL) free(LKMessages[i]);
		LKMessages[i]=NULL;
		LKMessagesIndex[i]=-1;
	}
  }

  LocalPath(sPath,_T(LKD_LANGUAGE));
  _tcscpy(suffix,_T("_MSG.TXT"));
  _stprintf(sFile,_T("%s\\%s%s"),sPath,LKLangSuffix,suffix);

  HANDLE hFile;
  hFile = INVALID_HANDLE_VALUE;
  hFile = CreateFile(sFile, GENERIC_READ,0,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE) {
	StartupStore(_T("... LoadText Missing Language File: <%s>%s"),sFile,NEWLINE);
	return false;
  } else {
	StartupStore(_T(". Language Load file: <%s>%s"),sFile,NEWLINE);
  }

  short filetype=FileIsUTF16(hFile);

  // search for beginning of code index   @000
  TCHAR sTmp[300];
  char snum[5];
  TCHAR scapt[MAX_MESSAGE_SIZE+1];
  TCHAR scaptraw[MAX_MESSAGE_SIZE+1];

  short mnumber=0;
  while ( ReadUString(hFile,299,sTmp,filetype) ) {

	unsigned int slen=_tcslen(sTmp); // includes cr or lf or both
	if (slen<9) continue;
	if ( (sTmp[0]!='_') || (sTmp[1]!='@') || (sTmp[2]!='M') ) {
		#if DEBUG_GETTEXT
		StartupStore(_T(".... MSG_ENG missing _@M line <%s>\n"),sTmp);
		#endif
		continue;
	}

	snum[0]=(char)sTmp[3];
	snum[1]=(char)sTmp[4];
	snum[2]=(char)sTmp[5];
	snum[3]=(char)sTmp[6];
	if (snum[3]=='_') snum[3]='\0';
	if (snum[2]=='_') snum[2]='\0';
	if (snum[1]=='_') snum[1]='\0';

	unsigned short inumber;
	inumber=atoi(snum);

	int start=0;
	for (i=3; i<slen; i++) {
		if (sTmp[i]=='\"') {
			start=i;
			break;
		}
	}

	int end=0;
	if (start==0) {
		#if DEBUG_GETTEXT
		StartupStore(_T(".... MSG_ENG no start\n"));
		#endif
		continue;
	}
	for (i=start+1; i<slen; i++) {
		if (sTmp[i]=='\"') {
			sTmp[i]='\0';
			end=i;
			break;
		}
	}
	if (end==0) {
		#if DEBUG_GETTEXT
		StartupStore(_T(".... MSG_ENG no end <%s> start=%d\n"),sTmp,start);
		#endif
		continue;
	}
	int newlen;
	newlen=_tcslen(&sTmp[start+1]);
	if (newlen>MAX_MESSAGE_SIZE) {
		#if DEBUG_GETTEXT
		StartupStore(_T(".... MSG_ENG caption too big, len=%d\n"),newlen);
		#endif
		continue;
	}
        #if DEBUG_GETTEXT
	if (newlen>maxsize) maxsize=newlen;
        #endif

	// transcode special charcaters while loading from file
	TCHAR tcode;
	bool donetcode;
	_tcscpy(scaptraw,&sTmp[start+1]);
	for (i=0, j=0; i<_tcslen(scaptraw); i++) {
		donetcode=false;
		if (scaptraw[i] == '\\') {
			if ( (i+1) <_tcslen(scaptraw)) {
				switch(scaptraw[i+1]) {
					case 'n':
						tcode='\n';
						break;
					case 'r':
						tcode='\r';
						break;
					default:
						tcode=' ';
						break;
				}
				scapt[j++]=tcode;
				i++;
				donetcode=true;
			}
		}
		if (!donetcode) {
			scapt[j++]=scaptraw[i];
		}
	}
	scapt[j]='\0';

	if (LKMessagesIndex[inumber]!= -1) {
		StartupStore(_T("... INVALID LANGUAGE MESSAGE INDEX <%d> duplicated!\n"),inumber);
		continue;
	}
	LKMessagesIndex[inumber]=mnumber;
	LKMessages[mnumber] = (TCHAR *)malloc((wcslen(scapt)+1)*sizeof(TCHAR));
	_tcscpy(LKMessages[mnumber],scapt);
	mnumber++;
	if (mnumber>=MAX_MESSAGES) {
		#if ALPHADEBUG
		StartupStore(_T("... TOO MANY MESSAGES, MAX %d%s"), MAX_MESSAGES, NEWLINE);
		#endif
		break;
	}
	
  }

  #if DEBUG_GETTEXT
  StartupStore(_T("... LOADED %d MESSAGES, max size = %d\n"),mnumber-1,maxsize);
  #endif

  CloseHandle(hFile);
  return true;


}



// return -1  if file is UTF16-LE
// return  0  if file is not UTF16
// return  1  if file is UTF16-BE
// If error, 0 is returned assuming the calling function will perform its own checks in any case
// UTF-32 is not even considered, and BOM is required for UTF16. 

// #define DEBUG_UTF16	1
short FileIsUTF16(HANDLE hFile) {	// 101221

  DWORD dwNumBytesRead=0;
  char buffer[10];

  if (hFile == INVALID_HANDLE_VALUE) {
	#if DEBUG_UTF16
	StartupStore(_T("... Invalid hFile, No UTF16\n"));
	#endif
	return(0);
  }
  SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

  // file is empty?
  if (ReadFile(hFile, buffer, sizeof(buffer)-1, &dwNumBytesRead, (OVERLAPPED *)NULL) == 0) {
	#if DEBUG_UTF16
	StartupStore(_T("... Readfile 0, empty? No UTF16\n"));
	#endif
	return(0);
  }
  SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

  #if DEBUG_UTF16
  buffer[dwNumBytesRead-1]='\0';
  StartupStore(_T("UTFcheck <%S>\n"),buffer);
  #endif


  // 0xff 0xfe 0x30 0x00 0x00 0x00  the minimum UTF16 begin string!
  if (dwNumBytesRead<6) {
	#if DEBUG_UTF16
	StartupStore(_T("... Short (%d) header line, no UTF16\n"),dwNumBytesRead);
	#endif
	return(0);
  }

  if ( (buffer[0]==(char)0xff) && (buffer[1]==(char)0xfe) ) {
	#if DEBUG_UTF16
	StartupStore(_T("... UTF-16 LE\n"));
	#endif
	return(-1);
  }
  if ( (buffer[0]==(char)0xfe) && (buffer[1]==(char)0xff) ) {
	#if DEBUG_UTF16
	StartupStore(_T("... UTF-16 BE\n"));
	#endif
	return(1);
  }

  #if DEBUG_UTF16
  StartupStore(_T("... No encoding , no UTF16\n"));
  #endif
  return(0);
}


