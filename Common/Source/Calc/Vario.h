/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   Vario.h
 * Author: Bruno de Lacheisserie
 */

#ifndef _CALC_VARIO_H_
#define _CALC_VARIO_H_

struct DeviceDescriptor_t;
struct NMEA_INFO;
struct DERIVED_INFO;

void UpdateVarioSource(NMEA_INFO& Info, const DeviceDescriptor_t& d, double Vario);
void ResetVarioAvailable(NMEA_INFO& Info);

bool VarioAvailable(const NMEA_INFO& Info);

const DeviceDescriptor_t* getVarioDevice(const NMEA_INFO& Info);

void Vario(const NMEA_INFO& Info, DERIVED_INFO& Calculated);

#endif // _CALC_VARIO_H_
