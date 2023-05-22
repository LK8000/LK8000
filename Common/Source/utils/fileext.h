/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
/*____________________________________________________________________________*/

#ifndef __fileext_h__
#define __fileext_h__

/*____________________________________________________________________________*/

#include <tchar.h>
#include <stdio.h>
#include "utils/unique_file_ptr.h"

/*____________________________________________________________________________*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// UTF-8 encoded text file manipulation.
/// Has methods for line reading and writing. Client should not write any
/// any of new line characters ('\n' '\r'), they are handled internally.
/// Class is able to read files with both CR+LF and LF line endings.
///
class Utf8File final {
  public:
    enum Mode
    {
      io_read,    ///< open for reading only
      io_append,  ///< open for appending to file end
      io_create // orverride existing file if exist
    };

    Utf8File() = default;
    Utf8File(const Utf8File&) = delete;
    Utf8File(Utf8File&&) = delete;

    ~Utf8File();

    bool Open(const TCHAR* fileName, Mode ioMode);
    void Close();
    void WriteLn(const TCHAR* unicode = NULL);
    bool Empty() const;

  protected:
    unique_file_ptr fp;
}; // Utf8File

#endif /* __fileext_h__ */
