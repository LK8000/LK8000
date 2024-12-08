/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   DoCalculationsGLoad.h
 * Author: Bruno de Lacheisserie
 */
#ifndef _CALC_DOCALCULATIONGLOAD_H_
#define _CALC_DOCALCULATIONGLOAD_H_

struct NMEA_INFO;
struct DERIVED_INFO;

void DoCalculationsGLoad(const NMEA_INFO& Basic, DERIVED_INFO& Calculated);

#endif // _CALC_DOCALCULATIONGLOAD_H_
