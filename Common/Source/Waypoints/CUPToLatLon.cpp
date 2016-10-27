/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"



double CUPToLat(TCHAR *temp)
{
  TCHAR *dot, td;
  TCHAR tdeg[4], tmin[4], tsec[5];
  double degrees, mins, secs;
  unsigned int slen;
  bool north=false;

  // lat is 4555.0X minimum
  if (_tcslen(temp)<7||_tcslen(temp)>9) return -9999;
  // check there is a dot, to be sure
  dot = _tcschr(temp,'.');
  if(!dot) return -9999;
  *dot = _T('\0');
  dot++;

  _tcscpy(tsec,dot);
  slen=_tcslen(tsec);

  // seconds are 0X minimum including the letter
  if (slen<2 || slen>4) return -9999;
  td= tsec[slen-1];

  if ( (td != _T('N')) && ( td != _T('S')) ) return -9999;
  if ( td == _T('N') ) north=true;

  td='\0';

  tdeg[0]=temp[0];
  tdeg[1]=temp[1];
  tdeg[2]=_T('\0');

  tmin[0]=temp[2];
  tmin[1]=temp[3];
  tmin[2]=_T('\0');

  degrees = (double)_tcstol(tdeg, NULL, 10);
  mins     = (double)_tcstol(tmin, NULL, 10);
  secs     = (double)_tcstol(tsec, NULL, 10);

  // if seconds are only a decimal, for example 3 , they really are 300
  switch (slen) {
	case 2:
		// 3X
		secs*=100;
		break;
	case 3:
		// 33X
		secs*=10;
		break;
	default:
		break;
  }

  mins += secs / 1000.0;
  degrees += mins / 60.0;

  if (!north) degrees *= -1;

  return degrees;
}

double CUPToLon(TCHAR *temp)
{
  TCHAR *dot, td;
  TCHAR tdeg[4], tmin[4], tsec[5];
  double degrees, mins, secs;
  unsigned int slen;
  bool east=false;

  // longit can be 01234.5X
  if (_tcslen(temp)<8 || _tcslen(temp)>10) return -9999;

  // check there is a dot, to be sure
  dot = _tcschr(temp,'.');
  if(!dot) return -9999;
  *dot = _T('\0');
  dot++;

  _tcscpy(tsec,dot);
  slen=_tcslen(tsec);
  // seconds are 0X minimum including the letter
  if (slen<2 || slen>4) return -9999;
  td= tsec[slen-1];

  if ( (td != _T('E')) && ( td != _T('W')) ) return -9999;
  if ( td == _T('E') ) east=true;

  td='\0';

  tdeg[0]=temp[0];
  tdeg[1]=temp[1];
  tdeg[2]=temp[2];
  tdeg[3]=_T('\0');

  tmin[0]=temp[3];
  tmin[1]=temp[4];
  tmin[2]=_T('\0');

  degrees = (double)_tcstol(tdeg, NULL, 10);
  mins     = (double)_tcstol(tmin, NULL, 10);
  secs     = (double)_tcstol(tsec, NULL, 10);

  // if seconds are only a decimal, for example 3 , they really are 300
  switch (slen) {
	case 2:
		// 3X
		secs*=100;
		break;
	case 3:
		// 33X
		secs*=10;
		break;
	default:
		break;
  }

  mins += secs / 1000.0;
  degrees += mins / 60.0;

  if (!east) degrees *= -1;

  return degrees;
}
