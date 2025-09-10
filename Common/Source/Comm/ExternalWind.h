/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ExternalWind.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 18 Avril 2024
 */
#ifndef _COMM_EXTERNALWIND_H_
#define _COMM_EXTERNALWIND_H_

struct DeviceDescriptor_t;
struct NMEA_INFO;
struct DERIVED_INFO;

/**
 * @Speed : in systemm Unit, use "Units::From(...)" to set it in right unit
 * @Direction : from the wind is blowing in degrees 0-360°
 */
void UpdateExternalWind(NMEA_INFO& Info, const DeviceDescriptor_t& d, double Speed, double Direction);

void ResetExternalWindAvailable(NMEA_INFO& Info);

bool ExternalWindAvailable(const NMEA_INFO& Info);

#endif // _COMM_EXTERNALWIND_H_
