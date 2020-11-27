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

#include <memory>

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

#ifndef countof
    #define countof(array) (sizeof(array)/sizeof(array[0]))
#endif
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
  if (fp) return true;

  //
  // Windows has case-insensitive file system. We try alternatives only for unix
  //
  #ifdef __linux__
  TCHAR stmp[MAX_PATH+1], tdrive[255], tdir[255], tname[255], text[255];

  _tcscpy(stmp,fileName);
  LK_tsplitpath(stmp, tdrive, tdir, tname, text);

  // Try ???.EXT
  CharUpper(text);
  _stprintf(stmp,_T("%s%s%s%s"),tdrive,tdir,tname,text);
  fp = _tfopen(stmp, fmode);
  if (fp) {
      LK_tcsncpy(path, stmp, countof(path)-1);
      return(true);
  }

  // Try ???.ext
  CharLower(text);
  _stprintf(stmp,_T("%s%s%s%s"),tdrive,tdir,tname,text);
  fp = _tfopen(stmp, fmode);
  if (fp) {
      LK_tcsncpy(path, stmp, countof(path)-1);
      return(true);
  }

  // Try name.ext
  CharLower(tname);
  _stprintf(stmp,_T("%s%s%s%s"),tdrive,tdir,tname,text);
  fp = _tfopen(stmp, fmode);
  if (fp) {
      LK_tcsncpy(path, stmp, countof(path)-1);
      return(true);
  }

  // Try NAME.EXT
  CharUpper(tname);
  CharUpper(text);
  _stprintf(stmp,_T("%s%s%s%s"),tdrive,tdir,tname,text);
  fp = _tfopen(stmp, fmode);
  if (fp) {
      LK_tcsncpy(path, stmp, countof(path)-1);
      return(true);
  }
#endif

  return(false);
} // Open()

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Write zero terminated line into the file.
/// Newline character will be added automatically.
///
/// @param unicode    zero terminated string
///
void Utf8File::WriteLn(const TCHAR* unicode) {
  if (!fp) {
    return;
  }
  if(unicode) {

#ifdef UNICODE
    
    // in worst case each characters can be encoded in 4 bytes
    const size_t max_char = _tcslen(unicode) * 4 + 1;
    std::unique_ptr<char[]> utf = std::make_unique<char[]>(max_char);
    char* cstr = utf.get();
    
    // (conversion and file error is ignored now, maybe in future it should
    // throw exception)
    if (TCHAR2utf(unicode, cstr, max_char) < 0 && !convErReported) {
      StartupStore(_T("Invalid WC-UTF8 conversion for '%s'\n"), path);
      convErReported = true;
    }
    
#else
    const char* cstr = unicode;
#endif
  
    if (fputs(cstr, fp) == EOF && !writeErReported) {
      StartupStore(_T("Cannot wite to file '%s'\n"), path);
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
