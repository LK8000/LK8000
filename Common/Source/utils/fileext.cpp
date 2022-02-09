/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "externs.h"
#include "fileext.h"
#include "stringext.h"

#include <memory>
#ifdef __linux__
#include <filesystem>
#endif


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

  path = fileName;


  fp = _tfopen(fileName, fmode);
  if (fp) return true;

  //
  // Windows has case-insensitive file system. We try alternatives only for unix
  //
#ifdef __linux__
  namespace fs = std::filesystem;

  fs::path file_path = fileName;
  std::string name = file_path.filename().string();

  ci_equal<std::string> comp;
  for (auto const& dir_entry : std::filesystem::directory_iterator{file_path.parent_path()}) {
    if (comp(name, dir_entry.path().filename().string())) {
      fp = _tfopen(dir_entry.path().c_str(), fmode);
      if (fp) {
        return true;
      }
    }
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
    to_utf8(unicode, cstr, max_char);
#else
    const char* cstr = unicode;
#endif
  
    if (fputs(cstr, fp) == EOF && !writeErReported) {
      StartupStore(_T("Cannot wite to file '%s'\n"), path.c_str());
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
