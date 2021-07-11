/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "TimeStamp.h"


int CTimeStamp::operator-(const CTimeStamp &ref) const
{
  int result = (_tv.tv_sec * 1000) + (_tv.tv_usec / 1000) - ((ref._tv.tv_sec * 1000) + (ref._tv.tv_usec / 1000));
  return result;
}
