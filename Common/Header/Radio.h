/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Radio.h,v 1.1 2020/22/12 
*/

#ifndef __RADIO_H__
#define __RADIO_H__

BOOL ValidFrequency(double Freq);
double ExtractFrequency(const TCHAR *text);
int SearchNearestStationWithFreqency(double Freq);
int SearchNearestStation(void);
int SearchBestStation(void);

BOOL CopyActiveStationNameByIndex( int Idx);
BOOL CopyPassiveStationNameByIndex( int Idx);
#endif