/* $Id: maperror.c,v 8.1 2009/06/19 20:28:25 venta Exp root $ */
/*
Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2005  

  	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@bigfoot.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

}
*/

#include "StdAfx.h"

#include "maperror.h"
#include "mapprimitive.h"


#ifdef NEED_NONBLOCKING_STDERR
#include <fcntl.h>
#endif

#include "utils/heapcheck.h"

static char *ms_errorCodes[MS_NUMERRORCODES] = {"",
						"Unable to access file.",
						"Memory allocation error.",
						"Incorrect data type.",
						"Symbol definition error.",
						"Regular expression error.",
						"TrueType Font error.",
						"DBASE file error.",
						"GD library error.",
						"Unknown identifier.",
						"Premature End-of-File.",
						"Projection library error.",
						"General error message.",
						"CGI error.",
						"Web application error.",
						"Image handling error.",
						"Hash table error.",
						"Join error.",
						"Search returned no results.",
						"Shapefile error.",
						"Expression parser error.",
						"SDE error.",
						"OGR error.",
						"Query error.",
						"WMS server error.",
						"WMS connection error.",
						"OracleSpatial error.",
						"WFS server error.",
						"WFS connection error.",
						"WMS Map Context error.",
						"HTTP request error."
};

#ifndef USE_THREAD

errorObj *msGetErrorObj()
{
    static errorObj ms_error = {MS_NOERR, "", "", NULL};

    return &ms_error;
}
#endif

#ifdef USE_THREAD

typedef struct te_info
{
    struct te_info *next;
    int             thread_id;
    errorObj        ms_error;
} te_info_t;

errorObj *msGetErrorObj()
{
    static te_info_t *error_list = NULL;
    te_info_t *link;
    int        thread_id;
    errorObj   *ret_obj;
    
    msAcquireLock( TLOCK_ERROROBJ );
    
    thread_id = msGetThreadId();

    /* find link for this thread */
    
    for( link = error_list; 
         link != NULL && link->thread_id != thread_id
             && link->next != NULL && link->next->thread_id != thread_id;
         link = link->next ) {}

    /* If the target thread link is already at the head of the list were ok */
    if( error_list != NULL && error_list->thread_id == thread_id )
    {
    }

    /* We don't have one ... initialize one. */
    else if( link == NULL || link->next == NULL )
    {
        te_info_t *new_link;
        errorObj   error_obj = { MS_NOERR, "", "", NULL };

        new_link = (te_info_t *) malloc(sizeof(te_info_t));
        new_link->next = error_list;
        new_link->thread_id = thread_id;
        new_link->ms_error = error_obj;

        error_list = new_link;
    }

    /* If the link is not already at the head of the list, promote it */
    else if( link != NULL && link->next != NULL )
    {
        te_info_t *target = link->next;

        link->next = link->next->next;
        target->next = error_list;
        error_list = target;
    }

    ret_obj = &(error_list->ms_error);

    msReleaseLock( TLOCK_ERROROBJ ); 

    return ret_obj;
}
#endif

/* msInsertErrorObj()
**
** We maintain a chained list of errorObj in which the first errorObj is
** the most recent (i.e. a stack).  msErrorReset() should be used to clear
** the list.
**
** Note that since some code in MapServer will fetch the head of the list and
** keep a handle on it for a while, the head of the chained list is static
** and never changes.
** A new errorObj is always inserted after the head, and only if the
** head of the list already contains some information.  i.e. If the static
** errorObj at the head of the list is empty then it is returned directly, 
** otherwise a new object is inserted after the head and the data that was in
** the head is moved to the new errorObj, freeing the head errorObj to receive
** the new error information.
*/
static errorObj *msInsertErrorObj()
{
  errorObj *ms_error;
  ms_error = msGetErrorObj();

  if (ms_error->code != MS_NOERR)
  {
      /* Head of the list already in use, insert a new errorObj after the head
       * and move head contents to this new errorObj, freeing the errorObj
       * for reuse.
       */
      errorObj *new_error;
      new_error = (errorObj *)malloc(sizeof(errorObj));

      /* Note: if malloc() failed then we simply do nothing and the head will
       * be overwritten by the caller... we cannot produce an error here 
       * since we are already inside a msSetError() call.
       */
      if (new_error)
      {
          new_error->next = ms_error->next;
          new_error->code = ms_error->code;
          strcpy(new_error->routine, ms_error->routine);
          strcpy(new_error->message, ms_error->message);

          ms_error->next = new_error;
          ms_error->code = MS_NOERR;
          ms_error->routine[0] = '\0';
          ms_error->message[0] = '\0';
      }
  }

  return ms_error;
}

/* msResetErrorList()
**
** Clear the list of error objects.
*/
void msResetErrorList()
{
  errorObj *ms_error, *this_error;
  ms_error = msGetErrorObj();

  this_error = ms_error->next;
  while( this_error != NULL)
  {
      errorObj *next_error;

      next_error = this_error->next;
      msFree(this_error);
      this_error = next_error;
  }

  ms_error->next = NULL;
  ms_error->code = MS_NOERR;
  ms_error->routine[0] = '\0';
  ms_error->message[0] = '\0';
}

char *msGetErrorCodeString(int code) {
  
  if(code<0 || code>MS_NUMERRORCODES-1)
    return("Invalid error code.");

  return(ms_errorCodes[code]);
}

char *msGetErrorString(char *delimiter) 
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  char  errbuf[512];
#else
  char errbuf[256];
#endif
  char *errstr=NULL;

  errorObj *error = msGetErrorObj();

  if(!delimiter || !error) return(NULL);

  if((errstr = _strdup("")) == NULL) return(NULL); // empty at first
  while(error && error->code != MS_NOERR) {
    if(error->next && error->next->code != MS_NOERR) // (peek ahead) more errors, use delimiter
#if defined(_WIN32) && !defined(__CYGWIN__)
      sprintf(errbuf,  "%s: %s %s%s", error->routine, ms_errorCodes[error->code], error->message, delimiter);
    else
      sprintf(errbuf, "%s: %s %s", error->routine, ms_errorCodes[error->code], error->message);
#else
      snprintf(errbuf, 255, "%s: %s %s%s", error->routine, ms_errorCodes[error->code], error->message, delimiter);
    else
      snprintf(errbuf, 255, "%s: %s %s", error->routine, ms_errorCodes[error->code], error->message);
#endif    

    if((errstr = (char *) realloc(errstr, sizeof(char)*(strlen(errstr)+strlen(errbuf)+1))) == NULL) return(NULL);
    strcat(errstr, errbuf);

    error = error->next;   
  }

  return(errstr);
}

void msSetError(int code, const char *message_fmt, const char *routine, ...)
{
//  char *errfile=NULL;
//  FILE *errstream;
//  time_t errtime;
  errorObj *ms_error = msInsertErrorObj();
  va_list args;

  ms_error->code = code;

  if(!routine)
    strcpy(ms_error->routine, "");
  else
    strncpy(ms_error->routine, routine, ROUTINELENGTH);

  if(!message_fmt)
    strcpy(ms_error->message, "");
  else
  {
    va_start(args, routine);
    vsprintf( ms_error->message, message_fmt, args );
    va_end(args);
  }
/*
  errfile = getenv("MS_ERRORFILE");
  if(errfile) {
    if(strcmp(errfile, "stderr") == 0)
      errstream = stderr;
    else if(strcmp(errfile, "stdout") == 0)
      errstream = stdout;
    else
      errstream = fopen(errfile, "a");
    if(!errstream) return;
    errtime = time(NULL);
    fprintf(errstream, "%s - %s: %s %s\n", chop(ctime(&errtime)), ms_error->routine, ms_errorCodes[ms_error->code], ms_error->message);
    fclose(errstream);
  }*/
}

void msWriteError(FILE *stream)
{
  errorObj *ms_error = msGetErrorObj();

  while (ms_error && ms_error->code != MS_NOERR)
  {
      fprintf(stream, "%s: %s %s <br>\n", ms_error->routine, ms_errorCodes[ms_error->code], ms_error->message);
      ms_error = ms_error->next;
  }
}


char *msGetVersion() {
  static char version[384];

  sprintf(version, "MapServer version %s", MS_VERSION);

#ifdef USE_GD_GIF
  strcat(version, " OUTPUT=GIF");
#endif
#ifdef USE_GD_PNG
  strcat(version, " OUTPUT=PNG");
#endif
#ifdef USE_GD_JPEG
  strcat(version, " OUTPUT=JPEG");
#endif
#ifdef USE_GD_WBMP
  strcat(version, " OUTPUT=WBMP");
#endif
#ifdef USE_PDF
  strcat(version, " OUTPUT=PDF");
#endif
#ifdef USE_MING_FLASH
  strcat(version, " OUTPUT=SWF");
#endif
#ifdef USE_PROJ
  strcat(version, " SUPPORTS=PROJ");
#endif
#ifdef USE_GD_FT
  strcat(version, " SUPPORTS=FREETYPE");
#endif
#ifdef USE_WMS_SVR
  strcat(version, " SUPPORTS=WMS_SERVER");
#endif
#ifdef USE_WMS_LYR
  strcat(version, " SUPPORTS=WMS_CLIENT");
#endif
#ifdef USE_WFS_SVR
  strcat(version, " SUPPORTS=WFS_SERVER");
#endif
#ifdef USE_WFS_LYR
  strcat(version, " SUPPORTS=WFS_CLIENT");
#endif
#ifdef USE_TIFF
  strcat(version, " INPUT=TIFF");
#endif
#ifdef USE_EPPL
  strcat(version, " INPUT=EPPL7");
#endif
#ifdef USE_JPEG
  strcat(version, " INPUT=JPEG");
#endif
#ifdef USE_SDE
  strcat(version, " INPUT=SDE");
#endif
#ifdef USE_POSTGIS
  strcat(version, " INPUT=POSTGIS");
#endif
#ifdef USE_ORACLESPATIAL
  strcat(version, " INPUT=ORACLESPATIAL"); 
#endif
#ifdef USE_OGR
  strcat(version, " INPUT=OGR");
#endif
#ifdef USE_GDAL
  strcat(version, " INPUT=GDAL");
#endif
  strcat(version, " INPUT=SHAPEFILE");

  return(version);
}

void msWebDebug( const char * pszFormat, ... )
{
   (void)pszFormat;
#ifndef _WIN32
    va_list args;
    struct timeval tv;

    fprintf(stdout, "Content-type: text/html%c%c",10,10);

    gettimeofday(&tv, NULL);
    fprintf(stdout, "[%s].%ld ", chop(ctime(&(tv.tv_sec))), tv.tv_usec);

    va_start(args, pszFormat);
    vfprintf(stdout, pszFormat, args);
    va_end(args);

    exit(0);
#endif
}

void msDebug( const char * pszFormat, ... )
{
	  (void)pszFormat;
#ifdef ENABLE_STDERR_DEBUG
    va_list args;
    struct timeval tv;

#ifdef NEED_NONBLOCKING_STDERR
    static char nonblocking_set = 0;
    if (!nonblocking_set)
    {
        fcntl(fileno(stderr), F_SETFL, O_NONBLOCK);
        nonblocking_set = 1;
    }
#endif

    gettimeofday(&tv, NULL);
    fprintf(stderr, "[%s].%ld ", chop(ctime(&(tv.tv_sec))), tv.tv_usec);

    va_start(args, pszFormat);
    vfprintf(stderr, pszFormat, args);
    va_end(args);
#endif
}

