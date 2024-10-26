/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#ifndef _CALC_LD_H_
#define _CALC_LD_H_

struct NMEA_INFO;
struct DERIVED_INFO;

void LD(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void CruiseLD(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

#endif // _CALC_LD_H_
