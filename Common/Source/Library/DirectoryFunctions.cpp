/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


void ExtractDirectory(TCHAR *Dest, TCHAR *Source) {
  int len = _tcslen(Source);
  int found = -1;
  int i;
  if (len==0) {
    Dest[0]= 0;
    return;
  }
  for (i=0; i<len; i++) {
    if ((Source[i]=='/')||(Source[i]=='\\')) {
      found = i;
    }
  }
  for (i=0; i<=found; i++) {
    Dest[i]= Source[i];
  }
  Dest[i]= 0;
}


void CreateDirectoryIfAbsent(const TCHAR *filename) {
  TCHAR fullname[MAX_PATH];

  LocalPath(fullname, filename);

  DWORD fattr = GetFileAttributes(fullname);

  if ((fattr != 0xFFFFFFFF) &&
      (fattr & FILE_ATTRIBUTE_DIRECTORY)) {
    // directory exists
  } else {
    CreateDirectory(fullname, NULL);
  }

}

bool FileExists(const TCHAR *FileName){

  HANDLE hFile = CreateFile(FileName, GENERIC_READ, 0, NULL,
                 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if( hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  CloseHandle(hFile);

  return(TRUE);
  
  /*
  FILE *file = _tfopen(FileName, _T("r"));
  if (file != NULL) {
    fclose(file);
    return(TRUE);
  }
  return FALSE;
  */
}



#ifdef PNA
void CreateRecursiveDirectory(TCHAR *fullpath)
{
  TCHAR tmpbuf[MAX_PATH];
  TCHAR *p;
  TCHAR *lastslash;
  bool found;
  
  if ( _tcslen(fullpath) <10 || _tcslen(fullpath)>=MAX_PATH) {
	StartupStore(_T("... FontPath too short or too long, cannot create folders%s"),NEWLINE);
	return;
  }

  if (*fullpath != '\\' ) {
	StartupStore(TEXT("... FontPath <%s> has no leading backslash, cannot create folders on a relative path.%s"),fullpath,NEWLINE);
	return;
  }

  lastslash=tmpbuf;

  do {
	// we copy the full path in tmpbuf as a working copy 
	_tcscpy(tmpbuf,fullpath);
	found=false;
	// we are looking after a slash. like in /Disk/
	// special case: a simple / remaining which we ignore, because either it is the first and only (like in \)
	// or it is a trailing slash with a null following
	if (*(lastslash+1)=='\0') {
		break;
	}
	
	// no eol, so lets look for another slash, starting from the char after last
	for (p=lastslash+1; *p != '\0'; p++) {
		if ( *p == '\\' ) {
			*p='\0';
			found=true;
			lastslash=p;
			break;
		}
	}
	if (_tcscmp(tmpbuf,_T("\\Windows"))==0) {
		continue;
	}
	CreateDirectory(tmpbuf, NULL);
  } while (found);
			
  return;
}
#endif

