/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"




double ReadAltitude(const TCHAR *temp)
{
  const TCHAR *Stop=temp;
  double Altitude = StrToDouble(temp, &Stop);
  if (temp == Stop)		// error at begin
	Altitude=-9999;
  else {
	if (Stop != NULL){	// number converted endpointer is set
		switch(*Stop){
			case 'M':				// meter's nothing to do
			case 'm':
			case '\0':
				break;
			case 'F':				// feet, convert to meter
			case 'f':
				Altitude = Altitude / TOFEET;
				break;
			default:				// anything else is a syntax error
				Altitude = -9999;
				break;
		}
	}
  }
  return Altitude;
}
