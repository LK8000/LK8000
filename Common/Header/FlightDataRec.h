/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef	FLIGHT_DATA_REC_H
#define	FLIGHT_DATA_REC_H

void InitFlightDataRecorder();
void UpdateFlightDataRecorder(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);
void CloseFlightDataRecorder();

#endif
