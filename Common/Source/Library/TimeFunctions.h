/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#ifndef _LIBRARY_TIMEFUNCTIONS_H_
#define _LIBRARY_TIMEFUNCTIONS_H_

struct NMEA_INFO;

int LocalTime();
int LocalTime(int utc_time);

time_t to_time_t(const NMEA_INFO& info);

#endif // _LIBRARY_TIMEFUNCTIONS_H_
