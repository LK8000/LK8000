/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

void CreateDirectoryIfAbsent(const TCHAR *filename) {
  TCHAR fullname[MAX_PATH];
  LocalPath(fullname, filename);
  lk::filesystem::createDirectory(fullname);
}
