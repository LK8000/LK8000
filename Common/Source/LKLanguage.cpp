/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKLanguage.cpp,v 1.4 2010/12/20 23:35:24 root Exp root $
 */

#include "externs.h"
#include "DoInits.h"
#include "utils/stl_utils.h"
#include "utils/openzip.h"
#include "Util/UTF8.hpp"

//#define DEBUG_GETTEXT	1
#define MAX_HELP	1500	// complete help including several lines, and also for each single line
				// Remember there is a limit in ReadULine, always careful with large strings

#define MAX_MESSAGES		2500 //2250 // Max number of MSG items
#define MAX_MESSAGE_SIZE	150 // just for setting a limit

TCHAR LKLangSuffix[4];

bool LKLoadMessages(bool fillup);

extern void FillDataOptions(void);

// _@Hnnnn@
// minimal: _@H1_  maximal: _@H1234_
// this function is not thread safe ...
const TCHAR *LKgethelptext(const TCHAR *TextIn) {

  static TCHAR sFile[MAX_PATH];
  static TCHAR sPath[MAX_PATH];
  static TCHAR sTmp[MAX_HELP+1];
  static TCHAR sHelp[MAX_HELP+1];

  bool foundnotfound=false;

  if (TextIn == NULL) return TextIn;
  short tlen=_tcslen(TextIn);
  if ( (tlen<5) || (TextIn[0]!='_') || (TextIn[1]!='@') || (TextIn[tlen-1]!='_') ) return TextIn;

    // get the item index number, quick conversion from unicode
    unsigned short inumber = 0;
    for (short i = 0; i < tlen - 4; i++) {
        inumber = (inumber * 10) + ((char)TextIn[3 + i] - '0');
    }

  // get the item index number, quick conversion from unicode

  #if DEBUG_GETTEXT
  StartupStore(_T(".... Help item TextIn=<%s> number=%d \n"),TextIn, inumber);
  #endif

  // get the type character
  char ttype = TextIn[2];
  TCHAR suffix[20];

  if (ttype=='H') {
	_tcscpy(suffix,_T("_HELP.TXT"));
	LocalPath(sPath,_T(LKD_LANGUAGE));
	_stprintf(sFile,_T("%s%s%s%s"),sPath, _T(DIRSEP), LKLangSuffix, suffix);

	// Help File, dynamically handled
	#if DEBUG_GETTEXT
	StartupStore(_T("... Open Language file type <%C>: <%s>%s"),ttype,sFile,NEWLINE);
	#endif

	TCHAR sNum[11];
	_stprintf(sNum,_T("%u"),inumber);

    ZZIP_FILE *helpFile = openzip(sFile, "rb");
	if (!helpFile) {
#ifdef LKD_SYS_LANGUAGE
		SystemPath(sPath, _T(LKD_SYS_LANGUAGE));
		_stprintf(sFile,_T("%s%s%s%s"), sPath, _T(DIRSEP), LKLangSuffix, suffix);
		helpFile = openzip(sFile, "rt");
#endif

		if(!helpFile) {
			StartupStore(_T("... Missing HELP FILE <%s>%s"), sFile, NEWLINE);
			// we can only have one Help call at a time, from the user interface. Ok static sHelp.
			_stprintf(sHelp, _T("ERROR, help file not found:\r\n%s\r\nCheck configuration!"),
					  sFile);
			return (sHelp);
		}
	}

	// search for beginning of code index   @000
	bool found=false;
	while (ReadULine(helpFile, sTmp, array_size(sTmp))) {
		unsigned slen=_tcslen(sTmp); // includes cr or lf or both
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
			for (unsigned i=1; i<slen; i++) {
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
		zzip_fclose(helpFile);
		return (sHelp);
	}

	// now load the help text for this index
	_tcscpy(sHelp,_T(""));
	int hlen=0;
	while (ReadULine(helpFile, sTmp, array_size(sTmp))) {

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
            size_t  freespace = array_size(sHelp) - _tcslen(sHelp) - 1;
			_tcsncat(sHelp, sTmp, freespace);
			break;
		}

		hlen+=slen;
		#if DEBUG_GETTEXT
		//StartupStore(_T(".. adding line <%s>\n"),sTmp);
		#endif
		_tcscat(sHelp,sTmp);
		_tcscat(sHelp,_T("\r\n"));
	}

	zzip_fclose(helpFile);
	return (sHelp);

  } // end ttype == H

  StartupStore(_T(".... Unknown Text type <%c> in <%s>%s"),ttype,TextIn,NEWLINE);
  return TextIn;

}



static TCHAR * LKMessages[MAX_MESSAGES+1];
#ifdef TESTBENCH
static TCHAR * COPYLKMessages[MAX_MESSAGES+1];
#endif

//  Tokenized Language support for LK8000
//  gettext is now a definition for LKGetText
//  101208

const TCHAR *LKGetText(const TCHAR *TextIn) {
    // quick preliminar checks
    if (TextIn == NULL) return TextIn;
    short tlen = _tcslen(TextIn);
    if ((tlen < 5) || (TextIn[0] != '_') || (TextIn[1] != '@') || (TextIn[2] != 'M') || (TextIn[tlen - 1] != '_')) return TextIn;

    // get the item index number, quick conversion from unicode
    unsigned short inumber = 0;
    for (short i = 0; i < tlen - 4; i++) {
        inumber = (inumber * 10) + ((char)TextIn[3 + i] - '0');
    }

    if (inumber >= MAX_MESSAGES) return TextIn; // safe check
    if(LKMessages[inumber]) {
        return LKMessages[inumber];
    }
    return _T("");
}

//
// Direct token access, with range check, faster than LKGetText of course
//
const TCHAR *MsgToken(unsigned inumber) {
  if (inumber<=MAX_MESSAGES && LKMessages[inumber]) {
    return LKMessages[inumber];
  }
  return _T("");
}



void LKReadLanguageFile(const TCHAR* szFileName) {
  static TCHAR oldLang[4];

  if (DoInit[MDI_READLANGUAGEFILE]) {
	_tcscpy(LKLangSuffix,_T(""));
	_tcscpy(oldLang,_T("XXX"));
	DoInit[MDI_READLANGUAGEFILE]=false;
  }

  bool english=false;
  TCHAR szFile1[MAX_PATH] = _T("\0");
  _tcscpy(LKLangSuffix,_T(""));


  _tcscpy(szFile1,szFileName);
  tryeng:
  if (_tcslen(szFile1)==0) {
	_tcscpy(szFile1, _T(LKD_DEFAULT_LANGUAGE));
	english=true;
  }

  TCHAR szFilePath[MAX_PATH] = _T("\0");
  _tcscpy(szFilePath,szFile1);
  ZZIP_FILE* langFile = openzip(szFilePath, "rt");
  if(!langFile) {
	  // failed to open absolute. try LocalPath
	  LocalPath(szFilePath, _T(LKD_LANGUAGE), szFile1);
	  langFile = openzip(szFilePath, "rt");
  }
  if(!langFile) {
	// failed to open lOCAL. try SystemPath
    SystemPath(szFilePath, _T(LKD_SYS_LANGUAGE), szFile1);
    langFile = openzip(szFilePath, "rt");
  }

  if (!langFile) {
	if (english) {
		StartupStore(_T("--- CRITIC, NO ENGLISH LANGUAGE FILES!%s"),NEWLINE);
		// critic point, no default language! BIG PROBLEM here!
		return;
	} else {
		StartupStore(_T("--- NO LANGUAGE FILE FOUND <%s>, retrying with ENGlish!%s"),szFile1,NEWLINE);
		_tcscpy(szFile1,_T(""));
		goto tryeng;
	}
  }

  bool found=false;
  TCHAR sTmp[200];
  TCHAR mylang[30];
  while (ReadULine(langFile, sTmp, array_size(sTmp))) {
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

	if ( !LKLoadMessages(false) ) {
		// force reload of english
		if (_tcscmp(_T("ENG"),LKLangSuffix) == 0 ) {
			StartupStore(_T("... CRITICAL, no english langauge available!%s"),NEWLINE);
		} else {
			StartupStore(_T("... LoadText failed, fallback to english language\n"));
			_tcscpy(LKLangSuffix,_T("ENG"));
			LKLoadMessages(false);
		}
	} else  {
		_tcscpy(oldLang,mylang);
		// Now overload english messages filling gaps in translations
		// only if current lang is not english of course: no reason to load it twice
		if (_tcscmp(_T("ENG"),LKLangSuffix) != 0 ) {
			_tcscpy(LKLangSuffix,_T("ENG"));
			LKLoadMessages(true);
			_tcscpy(LKLangSuffix,oldLang);
		}
	}
  }
  zzip_fclose(langFile);

  FillDataOptions(); // Load infobox list
  return;
}


/**
 * @brief Load language MSG file into memory
 *
 * @param fillup Switch value:
 *	- false - load from scratch removing anything existing
 *	-  true - load over existing messages, adding only missing items filling up gaps
 *
 * @return @c false if language file problem, in this case english is reloaded from calling function
 */
bool LKLoadMessages(bool fillup) {
  TCHAR sFile[MAX_PATH];
  TCHAR sPath[MAX_PATH];
  TCHAR suffix[20];
  #if DEBUG_GETTEXT
  int maxsize=0;
  #endif

  static bool doinit=true;

  if (doinit) {
    std::fill(std::begin(LKMessages), std::end(LKMessages), (TCHAR*)NULL);
    doinit=false;
  } else {
     if (!fillup) {
        // init data when reloading language files or changing it
        // but not in fillup mode of course
        LKUnloadMessage();
     }
  }

  LocalPath(sPath,_T(LKD_LANGUAGE));
  _tcscpy(suffix,_T("_MSG.TXT"));
  _stprintf(sFile,_T("%s%s%s%s"), sPath, _T(DIRSEP), LKLangSuffix, suffix);

  ZZIP_FILE *hFile = openzip(sFile, "rt");
  if (hFile == NULL) {
     #ifdef LKD_SYS_LANGUAGE
     SystemPath(sPath, _T(LKD_SYS_LANGUAGE));
     _stprintf(sFile,_T("%s%s%s%s"), sPath, _T(DIRSEP), LKLangSuffix, suffix);
     hFile = openzip(sFile, "rt");
     #endif
     if(!hFile) {
        StartupStore(_T("... LoadText Missing Language File: <%s>%s"), sFile, NEWLINE);
        return false;
     }
  } else {
     if (fillup)
        StartupStore(_T(". Language fillup load file: <%s>%s"),sFile,NEWLINE);
     else
        StartupStore(_T(". Language load file: <%s>%s"),sFile,NEWLINE);
  }

  // search for beginning of code index, in the range _@M1_  _@M9999_
  TCHAR sTmp[300];
  TCHAR scapt[MAX_MESSAGE_SIZE+1];
  TCHAR scaptraw[MAX_MESSAGE_SIZE+1];

  bool havewarned=false;
  while (ReadULine(hFile, sTmp, array_size(sTmp))) {
     unsigned int slen=_tcslen(sTmp); // includes cr or lf or both
     if ( (slen<9) || (sTmp[0]!='_') || (sTmp[1]!='@') || (sTmp[2]!='M') ) {
        #if DEBUG_GETTEXT
        if(slen>0 && sTmp[0]!='#') {
           StartupStore(_T(".... MSG_ENG missing _@M line <%s>\n"),sTmp);
        }
        #endif
        continue;
     }

     // get the item index number, quick conversion from unicode
     unsigned short inumber = 0;
     for (unsigned int i = 0; i < slen - 4 && isdigit(sTmp[3 + i]); i++) {
        inumber = (inumber * 10) + ((char)sTmp[3 + i] - '0');
     }

     if (inumber >=MAX_MESSAGES) {
        if (!havewarned) {
           StartupStore(_T("...... ERROR LOADING NON-COMPATIBLE MSG FILE!%s"),NEWLINE);
           havewarned=true;
        }
        StartupStore(_T("...... MSG token <%d> over limit! <%s>%s"),inumber,sTmp,NEWLINE);
        continue;
     }

     int start=0;
     for (unsigned i=3; i<slen; i++) {
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
     for (unsigned i=start+1; i<slen; i++) {
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
     if (newlen==0) {
        #if DEBUG_GETTEXT
        StartupStore(_T(".... MSG_ENG TOKEN # %d : caption is empty, null text.\n"),inumber);
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
     unsigned j=0;
     for (unsigned i=0; i<_tcslen(scaptraw); i++) {
        donetcode=false;
        if (scaptraw[i] == '\\') {
           if ( (i+1) <_tcslen(scaptraw)) {
              switch(scaptraw[i+1]) {
                 case 'n':
                    tcode='\n';
                    i++;
                    break;
                 case 'r':
                    tcode='\r';
                    i++;
                    break;
                 default:
                    tcode='\\';
                    break;
              }
              scapt[j++]=tcode;
              donetcode=true;
           }
        }
        if (!donetcode) {
           scapt[j++]=scaptraw[i];
        }
     }
     scapt[j]='\0';

     if (LKMessages[inumber]) {
        // only for debugging translations
        #if TESTBENCH
        if (!fillup) StartupStore(_T("... INVALID LANGUAGE MESSAGE INDEX <%d> duplicated!\n"),inumber);
        #endif
        continue;
     }
     #if TESTBENCH
     #if (WINDOWSPC>0)
     // CAUTION, on a PNA this would freeze the device if language file is not updated!
     // StartupStore is locking and unlocking threads at each run!!
     if (fillup) StartupStore(_T("... Fillup: message index %d is missing from translation\n"),inumber);
     #endif
     #endif

     #ifndef UNICODE
     LKASSERT(ValidateUTF8(scapt));
     #endif

     LKMessages[inumber] = (TCHAR *)malloc((_tcslen(scapt)+1)*sizeof(TCHAR));
     LKASSERT(LKMessages[inumber]!=NULL);
     _tcscpy(LKMessages[inumber],scapt);

     #ifdef TESTBENCH
     for (int i=0; i<MAX_MESSAGES; i++) {
        COPYLKMessages[i]=LKMessages[i];
     }
     #endif

  }
  zzip_fclose(hFile);
  return true;
}

void LKUnloadMessage(){

  #ifdef TESTBENCH
  bool have_error=false;
  for (int i=0; i<MAX_MESSAGES; i++) {
     if (LKMessages[i] != COPYLKMessages[i]) {
        StartupStore(_T("***** CRITICAL LKUnloadMessage, unmatched copy[%d]%s"),i,NEWLINE);
        have_error=true;
     }
  }
  if (have_error) return;
  #endif

  for (int i=0; i<MAX_MESSAGES; i++) {
     if (LKMessages[i]) {
        free(LKMessages[i]);
     }
  }
  std::fill(std::begin(LKMessages), std::end(LKMessages), (TCHAR*)NULL);
}

