/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Process.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef PROCESS_H
#define PROCESS_H

void	WindSpeedProcessing(int UpDown);
void	WindDirectionProcessing(int UpDown);
void	MacCreadyProcessing(int UpDown);
void	NextUpDown(int UpDown);

int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

int LocalTime();
int LocalTime(int utc_time);

#endif
