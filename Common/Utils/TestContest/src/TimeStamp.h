/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include <sys/time.h>


/** 
 * @brief Simple timestamping class
 */
class CTimeStamp {
  timeval _tv;
public:
  CTimeStamp() { gettimeofday(&_tv, 0); }
  int operator-(const CTimeStamp &ref) const;
};


#endif /* __TIMESTAMP_H__ */
