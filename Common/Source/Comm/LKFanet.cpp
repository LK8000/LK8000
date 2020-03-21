/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "Fanet.h"

void Fanet_RefreshSlots(NMEA_INFO *pGPS) {
	double passed;
	for (int i=0; i<MAXFANETWEATHER; i++) {
		if (pGPS->FANET_Weather[i].Time_Fix > 0) {
			passed= pGPS->Time-pGPS->FANET_Weather[i].Time_Fix;
			if (passed > (5*60)) { //after 5min, timeout !!
				//empty slot
				pGPS->FANET_Weather[i].Time_Fix = 0;
				pGPS->FANET_Weather[i].Cn[0] = 0;
				pGPS->FANET_Weather[i].Cn[1] = 0;
				pGPS->FANET_Weather[i].Cn[2] = 0;
			}
		}
	}
}

