/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "Compiler.h"
#include "options.h"
#include "fileext.h"
#include "Util/tstring.hpp"
#include "utils/unique_file_ptr.h"
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
    fp = nullptr;
} // Close()

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Opens existing UTF-8 encoded file.
///
/// @retval true  file open successfully
/// @retval false cannot open file
///
bool Utf8File::Open(const TCHAR* fileName, Mode ioMode) {
  const TCHAR* fmode;

  switch (ioMode) {
    case io_read:   fmode = _T("rb"); break;
    case io_append: fmode = _T("a+t"); break;
    case io_create: fmode = _T("w+t"); break;
    default:
      return(false);
  }

  fp = make_unique_file_ptr(fileName, fmode);
  if (fp) {
    return true;
  }

  //
  // Windows has case-insensitive file system. We try alternatives only for unix
  //
#ifdef __linux__
  namespace fs = std::filesystem;

  fs::path file_path = fileName;
  std::string_view name = file_path.filename().c_str();

  ci_equal<std::string_view> comp;
  for (auto const& dir_entry : std::filesystem::directory_iterator{file_path.parent_path()}) {
    if (comp(name, dir_entry.path().filename().c_str())) {
      fp = make_unique_file_ptr(dir_entry.path().c_str(), fmode);
      if (fp) {
        return true;
      }
    }
  }

#endif
  return false;
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
  
    fp.fputs(cstr);
  }
  fp.fputc('\n');
  fp.fflush();
} // WriteLn()


bool Utf8File::Empty() const {
  return fp.ftell() == 0;
}
