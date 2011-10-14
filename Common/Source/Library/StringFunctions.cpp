/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#else
#include "wcecompat/ts_string.h"
#endif

#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "lk8000.h"
#include "Utils.h"
#include "Utils2.h"

#include "utils/heapcheck.h"


#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif




void PExtractParameter(TCHAR *Source, TCHAR *Destination, int DesiredFieldNumber)
{
  int index = 0;
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength        = 0;

  StringLength = _tcslen(Source);

  while( (CurrentFieldNumber < DesiredFieldNumber) && (index < StringLength) )
    {
      if ( Source[ index ] == ',' )
	{
	  CurrentFieldNumber++;
	}
      index++;
    }

  if ( CurrentFieldNumber == DesiredFieldNumber )
    {
      while( (index < StringLength)    &&
	     (Source[ index ] != ',') &&
	     (Source[ index ] != 0x00) )
	{
	  Destination[dest_index] = Source[ index ];
	  index++; dest_index++;
	}
      Destination[dest_index] = '\0';
    }
}



BOOL ReadString(ZZIP_FILE *zFile, int Max, TCHAR *String)
{
  char sTmp[READLINE_LENGTH+1];
  char FileBuffer[READLINE_LENGTH+1];
  long dwNumBytesRead=0;
  long dwTotalNumBytesRead=0;
  long dwFilePos;

  String[0] = '\0';
  sTmp[0] = 0;

  ASSERT(Max<sizeof(sTmp));

  if (Max >= (int)(sizeof(sTmp))) // 100207 fixed signed unsigned (int)
    return(FALSE);
  if (!zFile)
    return(FALSE);

  dwFilePos = zzip_tell(zFile);

  dwNumBytesRead = zzip_fread(FileBuffer, 1, Max, zFile);
  if (dwNumBytesRead <= 0)
    return(FALSE);

  int i = 0;
  int j = 0;
  while((i<Max) && (j<(int)dwNumBytesRead)) {

    char c = FileBuffer[j];
    j++;
    dwTotalNumBytesRead++;

    if((c == '\n')){
      break;
    }

    sTmp[i] = c;
    i++;
  }

  sTmp[i] = 0;
  zzip_seek(zFile, dwFilePos+j, SEEK_SET);
  sTmp[Max-1] = '\0';
  mbstowcs(String, sTmp, strlen(sTmp)+1);
  return (dwTotalNumBytesRead>0);
}


// read string from file
// support national codepage
// hFile:  file handle
// Max:    max chars to fit in Buffer
// String: pointer to string buffer
// return: True if at least one byte was read from file
//         False Max > MAX_PATH or EOF or read error
BOOL ReadString(HANDLE hFile, int Max, TCHAR *String)
{
  char sTmp[READLINE_LENGTH+1];
  DWORD dwNumBytesRead=0;
  DWORD dwTotalNumBytesRead=0;
  char  FileBuffer[READLINE_LENGTH+1];
  DWORD dwFilePos;

  String[0] = '\0';
  sTmp[0] = 0;

  ASSERT(Max<sizeof(sTmp));

  if (Max >= (int)(sizeof(sTmp)))  
    return(FALSE);

  dwFilePos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

  if (hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  if (ReadFile(hFile, FileBuffer, sizeof(FileBuffer), 
	       &dwNumBytesRead, (OVERLAPPED *)NULL) == 0)
    return(FALSE);

  int i = 0;
  int j = 0;
  while(i<Max && j<(int)dwNumBytesRead){

    char c = FileBuffer[j];
    j++;
    dwTotalNumBytesRead++;

    if((c == '\n')){
      break;
    }

    sTmp[i] = c;
    i++;
    continue;
  }

  sTmp[i] = 0;
  SetFilePointer(hFile, dwFilePos+j, NULL, FILE_BEGIN);
  sTmp[Max-1] = '\0';
  mbstowcs(String, sTmp, strlen(sTmp)+1);
  return (dwTotalNumBytesRead>0);

}

BOOL ReadStringX(FILE *fp, int Max, TCHAR *String){
  if (fp == NULL || Max < 1 || String == NULL) {
    if (String) {
      String[0]= '\0';
    }
    return (0);
  }

  if (_fgetts(String, Max, fp) != NULL){     // 20060512/sgi change 200 to max

    String[Max-1] = '\0';                    // 20060512/sgi added make shure the  string is terminated
    TCHAR *pWC = &String[max(0,_tcslen(String)-1)]; 
    // 20060512/sgi change add -1 to set pWC at the end of the string

    while (pWC > String && (*pWC == '\r' || *pWC == '\n')){
      *pWC = '\0';
      pWC--;
    }

    return (1);
  }

  return (0);

}


/*
 * WARNING: if you are using Stop pointer, remember that you MUST set it equal to *Source before
   calling this function, because in case of null result, it will not be set!!
 */
double StrToDouble(TCHAR *Source, TCHAR **Stop)
{
  int index = 0;
  int StringLength        = 0;
  double Sum = 0;
  double Divisor = 10;
  int neg = 0;

  StringLength = _tcslen(Source);

  while(((Source[index] == ' ')||(Source[index]=='+')||(Source[index]==9)) 
        && (index<StringLength))
    // JMW added skip for tab stop
    // JMW added skip for "+"
    {
      index ++;
    }
  if (index>= StringLength) {
	// WARNING> we are not setting Stop here!!
    return 0.0; // Set 0.0 as error, probably not the best thing to do. TOFIX 110307
  }
  if (Source[index]=='-') {
    neg=1;
    index++;
  }

  while( (index < StringLength)
	 &&
	 (
	  (Source[index]>= '0') && (Source [index] <= '9')
          )
	 )
    {
      Sum = (Sum*10) + (Source[ index ] - '0');
      index ++;
    }
  if(Source[index] == '.')
    {
      index ++;
      while( (index < StringLength)
	     &&
	     (
	      (Source[index]>= '0') && (Source [index] <= '9')
	      )
	     )
	{
	  Sum = (Sum) + (double)(Source[ index ] - '0')/Divisor;
	  index ++;Divisor = Divisor * 10;
	}
    }
  if(Stop != NULL)
    *Stop = &Source[index];

  if (neg) {
    return -Sum;
  } else {
    return Sum;
  }
}


// RMN: Volkslogger outputs data in hex-strings.  Function copied from StrToDouble
// Note: Decimal-point and decimals disregarded.  Assuming integer amounts only.
double HexStrToDouble(TCHAR *Source, TCHAR **Stop) 
{
  int index = 0;
  int StringLength        = 0;
  double Sum = 0;
  int neg = 0;

  StringLength = _tcslen(Source);

  while((Source[index] == ' ')||(Source[index]==9))
    // JMW added skip for tab stop
    {
      index ++;
    }
  if (Source[index]=='-') {
    neg=1;
    index++;
  }

  while( 
  (index < StringLength)	 &&	 
	(	( (Source[index]>= '0') && (Source [index] <= '9')  ) || 
		( (Source[index]>= 'A') && (Source [index] <= 'F')  ) || 
		( (Source[index]>= 'a') && (Source [index] <= 'f')  )
		)	 
	)
    {
      if((Source[index]>= '0') && (Source [index] <= '9'))	  {
		Sum = (Sum*16) + (Source[ index ] - '0');
		index ++;
	  }
	  if((Source[index]>= 'A') && (Source [index] <= 'F'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'A' + 10);
		index ++;
	  }
	  if((Source[index]>= 'a') && (Source [index] <= 'f'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'a' + 10);
		index ++;
	  }
    }
  
  if(Stop != NULL)
    *Stop = &Source[index];

  if (neg) {
    return -Sum;
  } else {
    return Sum;
  }
}




TCHAR *strtok_r(TCHAR *s, TCHAR *delim, TCHAR **lasts){
// "s" MUST be a pointer to an array, not to a string!!! 
// (ARM92, Win emulator cause access violation if not)

  TCHAR *spanp;
	int   c, sc;
	TCHAR *tok;


	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */

cont:
	c = *s++;
	for (spanp = (TCHAR *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (TCHAR *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;  // causes access violation in some configs if s is a pointer instead of an array
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

// Same As strtok_r but not skeep leading delimiter ...
TCHAR *strsep_r(TCHAR *s, TCHAR *delim, TCHAR **lasts){
// "s" MUST be a pointer to an array, not to a string!!!
// (ARM92, Win emulator cause access violation if not)

	TCHAR *spanp;
	int   c, sc;
	TCHAR *tok = NULL;


	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	// return empty string if s start with delimiter
	c = *s++;
	for (spanp = delim; (sc = *spanp++) != 0;) {
		if (c == sc) {
			*lasts = s;
			s[-1] = 0;
			return (s - 1);
		}
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (TCHAR *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;  // causes access violation in some configs if s is a pointer instead of an array
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}



TCHAR* StringMallocParse(TCHAR* old_string) {
  TCHAR buffer[2048];	// Note - max size of any string we cope with here !
  TCHAR* new_string;
  unsigned int used = 0;
  unsigned int i;

  // Transcode special characters 
  for (i = 0; i < _tcslen(old_string); i++) {
    if (used < 2045) {
      if (old_string[i] == '\\' ) {
        if (old_string[i + 1] == 'r') {
          buffer[used++] = '\r';
          i++;
        } else if (old_string[i + 1] == 'n') { 
          buffer[used++] = '\n';
          i++;
        } else if (old_string[i + 1] == '\\') {
          buffer[used++] = '\\';
          i++;
        } else {
          buffer[used++] = old_string[i];
        }
      } else {
	buffer[used++] = old_string[i];
      }
    }
  };
  buffer[used++] =_T('\0');

  TCHAR *pstart;
  pstart = _tcsstr(buffer, _T("_@M"));
  if (pstart==NULL) {
	goto _notoken;
  }

  TCHAR *pend, *ptmp;

  ptmp=pstart;
  pend=NULL;
  while (++ptmp != NULL) {
	if (*ptmp != _T('_')) continue;
	pend=ptmp;
	break;
  }
  if (pend==NULL) {
	StartupStore(_T("...... Menu Label incorrect, no pend: <%s>%s"),buffer,NEWLINE);
	goto _notoken;
  }
  if ((pend-pstart)>9) {
	StartupStore(_T("...... Menu Label incorrect, token too big: <%s>%s"),buffer,NEWLINE);
	goto _notoken;
  }

  TCHAR lktoken[10];
  ptmp=pstart;
  i=0;
  while (ptmp<=pend) {
	lktoken[i++]=*ptmp++;
  }
  lktoken[i]='\0';
  *pstart='\0';
  TCHAR newbuffer[2048];
  wsprintf(newbuffer,_T("%s%s%s"), buffer, gettext(lktoken), pend+1);

  new_string = (TCHAR *)malloc((_tcslen(newbuffer)+1)*sizeof(TCHAR));
  _tcscpy(new_string, newbuffer);
  return new_string;

_notoken:
  new_string = (TCHAR *)malloc((_tcslen(buffer)+1)*sizeof(TCHAR));
  _tcscpy(new_string, buffer);
  return new_string;
}




int TextToLineOffsets(TCHAR* text, int* LineOffsets, int maxLines) {
  int nTextLines=0;
  LineOffsets[0]= 0;
  if (text) {
    if (_tcslen(text)>0) {

      int delta = 0;
      int cumul = 0;
      TCHAR* vind = text;

      while (nTextLines<maxLines) {
	delta = _tcscspn(vind+cumul, TEXT("\n"));
	if (!delta) {
	  break;
	}
	if (_tcslen(vind+cumul+delta)>0) {
	  delta++;
	} else {
	  break;
	}
	cumul += delta;
	nTextLines++;
	LineOffsets[nTextLines]= cumul;
      }
      nTextLines++;

    }
  }
  return nTextLines;
}



bool MatchesExtension(const TCHAR *filename, const TCHAR* extension) {
  TCHAR *ptr;
  ptr = _tcsstr((TCHAR*)filename, extension);
  if (ptr != filename+_tcslen(filename)-_tcslen(extension)) {
    return false;
  } else {
    return true;
  }
}



/* 
 * Implementation of the _splitpath runtime library function with wide character strings
 * Copyright 2000, 2004 Martin Fuchs -- GPL licensed - WINE project
 */
void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext)
{
	const WCHAR* end; /* end of processed string */
	const WCHAR* p;	  /* search pointer */
	const WCHAR* s;	  /* copy pointer */

	/* extract drive name */
	if (path[0] && path[1]==':') {
		if (drv) {
			*drv++ = *path++;
			*drv++ = *path++;
			*drv = '\0';
		}
	} else if (drv)
		*drv = '\0';

	/* search for end of string or stream separator */
	for(end=path; *end && *end!=':'; )
		end++;

	/* search for begin of file extension */
	for(p=end; p>path && *--p!='\\' && *p!='/'; )
		if (*p == '.') {
			end = p;
			break;
		}

	if (ext)
		for(s=end; (*ext=*s++); )
			ext++;

	/* search for end of directory name */
	for(p=end; p>path; )
		if (*--p=='\\' || *p=='/') {
			p++;
			break;
		}

	if (name) {
		for(s=p; s<end; )
			*name++ = *s++;

		*name = '\0';
	}

	if (dir) {
		for(s=path; s<p; )
			*dir++ = *s++;

		*dir = '\0';
	}
}



// Reads line from UTF-8 encoded text file.
// File must be open in binary read mode.
bool ReadULine(ZZIP_FILE* fp, TCHAR *unicode, int maxChars)
{
  unsigned char buf[READLINE_LENGTH * 2];

  long startPos = zzip_tell(fp);

  if (startPos < 0) {
    StartupStore(_T(". ftell() error = %d%s"), errno, NEWLINE);
    return(false);
  }

  size_t nbRead = zzip_fread(buf, 1, sizeof(buf) - 1, fp);
  
  if (nbRead == 0)
    return(false);

  buf[nbRead] = '\0';

  // find new line (CR/LF/CRLF) in the string and terminate string at that position
  size_t i;
  for (i = 0; i < nbRead; i++) {
    if (buf[i] == '\n')
    {
      buf[i++] = '\0';
      if (buf[i] == '\r')
        i++;
      break;
    }

    if (buf[i] == '\r')
    {
      buf[i++] = '\0';
      if (buf[i] == '\n')
        i++;
      break;
    }
  }

  // next reading will continue after new line
  zzip_seek(fp, startPos + i, SEEK_SET);

  // skip leading BOM
  char* begin = (char*) buf;
  if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
    begin += 3;

  return(utf2unicode(begin, unicode, maxChars) >= 0);
}



/*
 * Convert to uppercase a TCHAR array
 */
void ConvToUpper( TCHAR *str )
{
	if ( str )
	{
		for ( ; *str; ++str )
		*str = towupper(*str);

	}

	return ;
}

// ////////////////////////////////////////////////////////////////
//            Unicode <> Ascii conversions
//
void ConvertTToC(CHAR* pszDest, const TCHAR* pszSrc)
{
	for(unsigned int i = 0; i < _tcslen(pszSrc); i++)
		pszDest[i] = (CHAR) pszSrc[i];
}

void ConvertCToT(TCHAR* pszDest, const CHAR* pszSrc)
{
	for(unsigned int i = 0; i < strlen(pszSrc); i++)
		pszDest[i] = (TCHAR) pszSrc[i];
}

void unicodetoascii(TCHAR *utext, int utextsize, char *atext) {

	int i,j;
	if (utextsize==0) {
		atext[0]=0;
		return;
	}

	for (i=0,j=0; i<utextsize; i++) {
		if (utext[i]==0) continue;
		atext[j++]=utext[i];
	}
	atext[j]=0;

}
// ////////////////////////////////////////////////////////////////


