/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/tokenizer.h"


extern int globalFileNum;



bool ParseOZIWayPointString(TCHAR *String,WAYPOINT *Temp){

	Temp->Visible = true; // default all waypoints visible at start
	Temp->FarVisible = true;
	Temp->Format = LKW_OZI;
	Temp->Number = WayPointList.size();
	Temp->FileNum = globalFileNum;
	Temp->Flags = TURNPOINT;

	memset(Temp->Name, 0, sizeof(Temp->Name)); // clear Name

	TCHAR TempString[READLINE_LENGTH];
	memset(TempString, 0, sizeof(TempString)); // clear TempString

	_tcscpy(TempString, String);

	TCHAR *pToken = NULL;

	lk::tokenizer<TCHAR> tok(TempString);


	//	Field 1 : Number - this is the location in the array (max 1000), must be unique, usually start at 1 and increment. Can be set to -1 (minus 1) and the number will be auto generated.
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 2 : Name - the waypoint name, use the correct length name to suit the GPS type.
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	// guard against overrun
	if (_tcslen(pToken)>NAME_SIZE) {
		pToken[NAME_SIZE-1]= _T('\0');
	}

	// remove trailing spaces
	for (int i=_tcslen(pToken)-1; i>1; i--) if (pToken[i]==' ') pToken[i]=0; else break;

	_tcscpy(Temp->Name, pToken);

	//	Field 3 : Latitude - decimal degrees.
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	Temp->Latitude = (double)StrToDouble(pToken, nullptr);

	if((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
		return false;
	}

	//	Field 4 : Longitude - decimal degrees.
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	Temp->Longitude  = (double)StrToDouble(pToken, nullptr);
	if((Temp->Longitude  > 180) || (Temp->Longitude  < -180)) {
		return false;
	}
	//	Field 5 : Date - see Date Format below, if blank a preset date will be used
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 6 : Symbol - 0 to number of symbols in GPS
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 7 : Status - always set to 1
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 8 : Map Display Format
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 9 : Foreground Color (RGB value)
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 10 : Background Color (RGB value)
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 11 : Description (max 40), no commas
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

    if (_tcslen(pToken) >0 ) {
		// remove trailing spaces
		for (int i=_tcslen(pToken)-1; i>1; i--) if (pToken[i]==' ') pToken[i]=0; else break;
		SetWaypointComment(*Temp, pToken);
    }
    else {
		SetWaypointComment(*Temp, _T(""));
    }

	//	Field 12 : Pointer Direction
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 13 : Garmin Display Format
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 14 : Proximity Distance - 0 is off any other number is valid
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	//	Field 15 : Altitude - in feet (-777 if not valid)
	if ((pToken = tok.Next({_T(',')})) == NULL)
		return false;

	Temp->Altitude = (double)StrToDouble(pToken, nullptr)/TOFEET;
	if(Temp->Altitude <= 0) {
		WaypointAltitudeFromTerrain(Temp);
	}

	//	Field 16 : Font Size - in points
	//	Field 17 : Font Style - 0 is normal, 1 is bold.
	//	Field 18 : Symbol Size - 17 is normal size
	//	Field 19 : Proximity Symbol Position
	//	Field 20 : Proximity Time
	//	Field 21 : Proximity or Route or Both
	//	Field 22 : File Attachment Name
	//	Field 23 : Proximity File Attachment Name
	//	Field 24 : Proximity Symbol Name

	return true;
}
