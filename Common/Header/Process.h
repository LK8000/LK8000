/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef PROCESS_H
#define PROCESS_H
#include "externs.h"

void	WindSpeedProcessing(int UpDown);
void	WindDirectionProcessing(int UpDown);
void	MacCreadyProcessing(int UpDown);
void	NextUpDown(int UpDown);

int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
int DetectCurrentTime(void);
int LocalTime(void);
int TimeLocal(int d);

#endif
