/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "Fanet.h"

static void emptySlot(FANET_DATA& data) {
	data.Time_Fix = 0;
	data.ID = 0;
}

void Fanet_RefreshSlots(NMEA_INFO *pGPS) {
	for ( FANET_WEATHER& data : pGPS->FANET_Weather) {
		if (data.Time_Fix > 0) {
			double passed = pGPS->Time - data.Time_Fix;
			if ( passed < 0 || passed > (5*60)) { 
				//after 5min, timeout or if Time gone back.
				//empty slot
				emptySlot(data);
			}
			if ( pGPS->Time < pGPS->FANET_Weather[i].Time_Fix){ // Time gone back.
				//empty slot
				pGPS->FANET_Weather[i].Time_Fix = 0;
				pGPS->FANET_Weather[i].Cn[0] = 0;
				pGPS->FANET_Weather[i].Cn[1] = 0;
				pGPS->FANET_Weather[i].Cn[2] = 0;
			}
		}
	}
	for (int i=0; i<MAXFANETDEVICES; i++) {
		if (pGPS->FanetName[i].Time_Fix > 0) {
			passed= pGPS->Time-pGPS->FanetName[i].Time_Fix;
			if (passed > (5*240)) { //message should be sent all 240sec. --> timeout 5times
				//empty slot
				pGPS->FanetName[i].Time_Fix = 0;
				pGPS->FanetName[i].Cn[0] = 0;
				pGPS->FanetName[i].Cn[1] = 0;
				pGPS->FanetName[i].Cn[2] = 0;
			}
			if ( pGPS->Time < pGPS->FanetName[i].Time_Fix){ // Time gone back.
				//empty slot
				//Message::AddMessage(1500, MSG_COMMS, _T("FANET Time FIX Back in Time")); // message time 1.5s
				pGPS->FanetName[i].Time_Fix = 0;
				pGPS->FanetName[i].Cn[0] = 0;
				pGPS->FanetName[i].Cn[1] = 0;
				pGPS->FanetName[i].Cn[2] = 0;
			}
		}
	}

	for ( FANET_NAME& data : pGPS->FanetName) {
		if (data.Time_Fix > 0) {
			double passed = pGPS->Time - data.Time_Fix;
			if (passed < 0 || passed > (5*240)) { 
				//message should be sent all 240sec. --> timeout 5times
				// or if Time gone back.
				//empty slot
				emptySlot(data);
			}
		}
	}
}
