/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef PROCESS_H
#define PROCESS_H
#include "externs.h"

void				NoProcessing(int UpDown);
void				WindSpeedProcessing(int UpDown);
void				WindDirectionProcessing(int UpDown);
void				MacCreadyProcessing(int UpDown);
void				NextUpDown(int UpDown);
void				SpeedProcessing(int UpDown);
void				DirectionProcessing(int UpDown);
void				AltitudeProcessing(int UpDown);
void				QFEAltitudeProcessing(int UpDown); // VENTA3
void				Alternate1Processing(int UpDown); // VENTA3
void				Alternate2Processing(int UpDown); // VENTA3
void				BestAlternateProcessing(int UpDown); // VENTA3
void				AirspeedProcessing(int UpDown);
void				TeamCodeProcessing(int UpDown);
void				ForecastTemperatureProcessing(int UpDown);
int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
int DetectCurrentTime(void);
int TimeLocal(int d);

#endif
