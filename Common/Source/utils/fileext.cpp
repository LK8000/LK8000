/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "externs.h"
#include "fileext.h"
#include "stringext.h"

#include "utils/heapcheck.h"



//______________________________________________________________________________


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Initializes file.
//
Utf8File::Utf8File()
: fp(NULL), convErReported(), writeErReported()
{
  path[0] = _T('\0');
} // Utf8File()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Closes open file.
///
Utf8File::~Utf8File()
{
  Close();
} // ~Utf8File()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Closes open file.
///
void Utf8File::Close()
{
  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
  }
} // Close()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Opens existing UTF-8 encoded file.
///
/// @retval true  file open successfully
/// @retval false cannot open file
///
bool Utf8File::Open(const TCHAR* fileName, Mode ioMode)
{
  const TCHAR* fmode;

  switch (ioMode) {
    case io_read:   fmode = _T("rb"); break;
    case io_append: fmode = _T("a+t"); break;
    case io_create: fmode = _T("w+t"); break; 
    default:
      return(false);
  }

  LK_tcsncpy(path, fileName, countof(path)-1);

  fp = _tfopen(fileName, fmode);

  return(fp != NULL);
} // Open()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads one line from UTF-8 encoded file and converts it to TCHAR Unicode
/// string.
/// maxChars-1 characters are returned at most, leaving 1 char for
/// terminating '\0'.
///
/// The method strips any newline character ('\r' and '\n').
///
/// @param unicode    output buffer (will be terminated with '\0'); must be
///                   large enough to contain full line (otherwise rest
///                   of line will be discarded)
/// @param maxChars   output buffer size (nb of TCHARs)
///
/// @retval true  line has been read successfully
/// @retval false error during reading from the file
///
bool Utf8File::ReadLn(TCHAR* unicode, int maxChars)
{
  if (fp == NULL)
    return(false);

  // in worst case each char can be encoded in 4 bytes
  char cstr[4 * maxChars];

  if (fgets(cstr, countof(cstr), fp) == NULL)
    return(false);

  // strip new-line separators
  size_t len = strlen(cstr);

  while (len > 0) {
    char last = cstr[len - 1];
    if (last == '\r' || last == '\n')
      cstr[--len] = '\0';
    else
      break;
  }

  if (utf2TCHAR(cstr, unicode, maxChars) < 0 && !convErReported) {
    StartupStore(_T("Invalid UTF8-WC conversion for '%s'%s"), path, NEWLINE);
    convErReported = true;
  }

  return(true);
} // ReadLn()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Write zero terminated line into the file.
/// Newline character will be added automatically.
///
/// @param unicode    zero terminated string
///
void Utf8File::WriteLn(const TCHAR* unicode)
{
  if (fp == NULL)
    return;

  if (unicode != NULL) {
    size_t len = _tcslen(unicode);
    // in worst case each char can be encoded in 4 bytes
    char cstr[4 * len + 1];

    // (conversion and file error is ignored now, maybe in future it should
    // throw exception)
    if (TCHAR2utf(unicode, cstr, sizeof(cstr)) < 0 && !convErReported) {
      StartupStore(_T("Invalid WC-UTF8 conversion for '%s'%s"), path, NEWLINE);
      convErReported = true;
    }

    if (fputs(cstr, fp) == EOF && !writeErReported) {
      StartupStore(_T("Cannot wite to file '%s'%s"), path, NEWLINE);
      writeErReported = true;
    }
  }

  fputc('\n', fp);
} // WriteLn()


//static
bool Utf8File::Exists(const TCHAR* fileName)
{
  Utf8File file;
  return(file.Open(fileName, io_read));
} // Exists()
