/************************************************************************
*
* File:			Utm.cpp
* RCS:			$Header: /cvsroot/stelvio/stelvio/NavStar/Utm.cpp,v 1.1 2001/03/18 20:07:03 steve_l Exp $
* Author:		Steve Loughran
* Created:		2001
* Language:		C++
* Package:
* Status:		Experimental
* @doc
*
************************************************************************/

/*
	This is code to do UTM conversion.

  I took this code from Jason Bevins' GPS thing which blagged the VB algorithms
    from the Mapping Datum Transformation Software (MADTRAN) program,
    written in PowerBasic.  To get the source code for MADTRAN, go to:

      http://164.214.2.59/publications/guides/MADTRAN/index.html

	this version retains the core algorithms as static functions

*/

// I have cleaned it up on Nov 29th 2009 - Paolo Ventafridda

#include <math.h>
#include "Utm.h"
#include "LKAssert.h"

// Some constants used by these functions.


const double PI(3.14159265);
//const double FOURTHPI(PI / 4);
const double deg2rad(PI / 180.0);
const double rad2deg(180.0 / PI);

static const double fe = 500000.0;
static const double ok = 0.9996;

// An array containing each vertical UTM zone.
static char cArray[] = "CDEFGHJKLMNPQRSTUVWX";


/////////////////////////////////////////////////////////////////////////////
// Miscellaneous functions for these UTM conversion formulas.

double CalculateESquared (double a, double b)
{
	return ((a * a) - (b * b)) / (a * a);
}


double CalculateE2Squared (double a, double b)
{
	return ((a * a) - (b * b)) / (b * b);
}


double denom (double es, double sphi)
{
	double sinSphi = sin (sphi);
	return sqrt (1.0 - es * (sinSphi * sinSphi));
}


double sphsr (double a, double es, double sphi)
{
	double dn = denom (es, sphi);
	return a * (1.0 - es) / (dn * dn * dn);
}


double sphsn (double a, double es, double sphi)
{
	double sinSphi = sin (sphi);
	return a / sqrt (1.0 - es * (sinSphi * sinSphi));
}


double sphtmd (double ap, double bp, double cp, double dp, double ep,
	double sphi)
{
	return (ap * sphi) - (bp * sin (2.0 * sphi)) + (cp * sin (4.0 * sphi))
		- (dp * sin (6.0 * sphi)) + (ep * sin (8.0 * sphi));
}


// Purpose:
//  This function converts the specified lat/lon coordinate to a UTM
//  coordinate.
// Parameters:
//  double a:
//      Ellipsoid semi-major axis, in meters. (For WGS84 datum, use 6378137.0)
//  double f:
//      Ellipsoid flattening. (For WGS84 datum, use 1 / 298.257223563)
//  int& utmXZone:
//      Upon exit, this parameter will contain the hotizontal zone number of
//      the UTM coordinate.  The returned value for this parameter is a number
//      within the range 1 to 60, inclusive.
//  char& utmYZone:
//      Upon exit, this parameter will contain the zone letter of the UTM
//      coordinate.  The returned value for this parameter will be one of:
//      CDEFGHJKLMNPQRSTUVWX.
//  double& easting:
//      Upon exit, this parameter will contain the UTM easting, in meters.
//  double& northing:
//      Upon exit, this parameter will contain the UTM northing, in meters.
//  double lat, double lon:
//      The lat/lon coordinate to convert.
// Notes:
//  - The code in this function is a C conversion of some of the source code
//    from the Mapping Datum Transformation Software (MADTRAN) program,
//    written in PowerBasic.  To get the source code for MADTRAN, go to:
//
//      http://164.214.2.59/publications/guides/MADTRAN/index.html
//
//    and download MADTRAN.ZIP
//  - If the UTM zone is out of range, the y-zone character is set to the
//    asterisk character ('*').

void LatLonToUtm (double a, double f, int& utmXZone, char& utmYZone,
	double& easting, double& northing, double lat, double lon)
{
	double recf;
	double b;
	double eSquared;
	double e2Squared;
	double tn;
	double ap;
	double bp;
	double cp;
	double dp;
	double ep;
	double olam;
	double dlam;
	double s;
	double c;
	double t;
	double eta;
	double sn;
	double tmd;
	double t1, t2, t3, t6, t7;
	double nfn;

	if (lon <= 0.0) {
		utmXZone = 30 + (int)(lon / 6.0);
	} else {
		utmXZone = 31 + (int)(lon / 6.0);
	}
	if (lat < 84.0 && lat >= 72.0) {
		// Special case: zone X is 12 degrees from north to south, not 8.
		utmYZone = cArray[19];
	} else {
		utmYZone = cArray[(int)((lat + 80.0) / 8.0)];
	}
	if (lat >= 84.0 || lat < -80.0) {
		// Invalid coordinate; the vertical zone is set to the invalid
		// character.
		utmYZone = '*';
	}

	double latRad = lat * deg2rad;
	double lonRad = lon * deg2rad;
	LKASSERT(f!=0);
	recf = 1.0 / f;
	b = a * (recf - 1.0) / recf;
	eSquared = CalculateESquared (a, b);
	e2Squared = CalculateE2Squared (a, b);
	LKASSERT(a!=b);
	tn = (a - b) / (a + b);
	ap = a * (1.0 - tn + 5.0 * ((tn * tn) - (tn * tn * tn)) / 4.0 + 81.0 *
		((tn * tn * tn * tn) - (tn * tn * tn * tn * tn)) / 64.0);
	bp = 3.0 * a * (tn - (tn * tn) + 7.0 * ((tn * tn * tn)
		- (tn * tn * tn * tn)) / 8.0 + 55.0 * (tn * tn * tn * tn * tn) / 64.0)
		/ 2.0;
	cp = 15.0 * a * ((tn * tn) - (tn * tn * tn) + 3.0 * ((tn * tn * tn * tn)
		- (tn * tn * tn * tn * tn)) / 4.0) / 16.0;
	dp = 35.0 * a * ((tn * tn * tn) - (tn * tn * tn * tn) + 11.0
		* (tn * tn * tn * tn * tn) / 16.0) / 48.0;
	ep = 315.0 * a * ((tn * tn * tn * tn) - (tn * tn * tn * tn * tn)) / 512.0;
	olam = (utmXZone * 6 - 183) * deg2rad;
	dlam = lonRad - olam;
	s = sin (latRad);
	c = cos (latRad);
	LKASSERT(c!=0);
	t = s / c;
	eta = e2Squared * (c * c);
	sn = sphsn (a, eSquared, latRad);
	tmd = sphtmd (ap, bp, cp, dp, ep, latRad);
	t1 = tmd * ok;
	t2 = sn * s * c * ok / 2.0;
	t3 = sn * s * (c * c * c) * ok * (5.0 - (t * t) + 9.0 * eta + 4.0
		* (eta * eta)) / 24.0;
	if (latRad < 0.0) nfn = 10000000.0; else nfn = 0;
	northing = nfn + t1 + (dlam * dlam) * t2 + (dlam * dlam * dlam
		* dlam) * t3 + (dlam * dlam * dlam * dlam * dlam * dlam) + 0.5;
	t6 = sn * c * ok;
	t7 = sn * (c * c * c) * (1.0 - (t * t) + eta) / 6.0;
	easting = fe + dlam * t6 + (dlam * dlam * dlam) * t7 + 0.5;
	if (northing >= 9999999.0) northing = 9999999.0;
}

// Purpose:
//  This function converts the specified lat/lon coordinate to a UTM
//  coordinate in the WGS84 datum.  (See the comment block for the
//  LatLonToUtm() member function.)

void LatLonToUtmWGS84 (int& utmXZone, char& utmYZone,
	double& easting, double& northing, double lat, double lon)
{
	LatLonToUtm (6378137.0, 1 / 298.257223563, utmXZone, utmYZone,
		easting, northing, lat, lon);
}




//=======================================================================
// Purpose:
//  This function converts the specified UTM coordinate to a lat/lon
//  coordinate.
// Pre:
//  - utmXZone must be between 1 and 60, inclusive.
//  - utmYZone must be one of: CDEFGHJKLMNPQRSTUVWX
// Parameters:
//  double a:
//      Ellipsoid semi-major axis, in meters. (For WGS84 datum, use 6378137.0)
//  double f:
//      Ellipsoid flattening. (For WGS84 datum, use 1 / 298.257223563)
//  int utmXZone:
//      The horizontal zone number of the UTM coordinate.
//  char utmYZone:
//      The vertical zone letter of the UTM coordinate.
//  double easting, double northing:
//      The UTM coordinate to convert.
//  double& lat:
//      Upon exit, lat contains the latitude.
//  double& lon:
//      Upon exit, lon contains the longitude.
// Notes:
//  The code in this function is a C conversion of some of the source code
//  from the Mapping Datum Transformation Software (MADTRAN) program, written
//  in PowerBasic.  To get the source code for MADTRAN, go to:
//
//    http://164.214.2.59/publications/guides/MADTRAN/index.html
//
//  and download MADTRAN.ZIP
//=======================================================================

void UtmToLatLon (double a, double f, int utmXZone, char utmYZone,
	double easting, double northing, double& lat, double& lon)
{
	double recf;
	double b;
	double eSquared;
	double e2Squared;
	double tn;
	double ap;
	double bp;
	double cp;
	double dp;
	double ep;
	double nfn;
	double tmd;
	double sr;
	double sn;
	double ftphi;
	double s;
	double c;
	double t;
	double eta;
	double de;
	double dlam;
	double olam;

	recf = 1.0 / f;
	b = a * (recf - 1) / recf;
	eSquared = CalculateESquared (a, b);
	e2Squared = CalculateE2Squared (a, b);
	tn = (a - b) / (a + b);
	ap = a * (1.0 - tn + 5.0 * ((tn * tn) - (tn * tn * tn)) / 4.0 + 81.0 *
		((tn * tn * tn * tn) - (tn * tn * tn * tn * tn)) / 64.0);
	bp = 3.0 * a * (tn - (tn * tn) + 7.0 * ((tn * tn * tn)
		- (tn * tn * tn * tn)) / 8.0 + 55.0 * (tn * tn * tn * tn * tn) / 64.0)
		/ 2.0;
	cp = 15.0 * a * ((tn * tn) - (tn * tn * tn) + 3.0 * ((tn * tn * tn * tn)
		- (tn * tn * tn * tn * tn)) / 4.0) / 16.0;
	dp = 35.0 * a * ((tn * tn * tn) - (tn * tn * tn * tn) + 11.0
		* (tn * tn * tn * tn * tn) / 16.0) / 48.0;
	ep = 315.0 * a * ((tn * tn * tn * tn) - (tn * tn * tn * tn * tn)) / 512.0;
	if ((utmYZone <= 'M' && utmYZone >= 'C')
		|| (utmYZone <= 'm' && utmYZone >= 'c')) {
		nfn = 10000000.0;
	} else {
		nfn = 0;
	}
	LKASSERT(ok!=0);
	tmd = (northing - nfn) / ok;
	sr = sphsr (a, eSquared, 0.0);
	LKASSERT(sr!=0);
	ftphi = tmd / sr;
	double t10, t11, t14, t15;
	for (int i = 0; i < 5; i++) {
		t10 = sphtmd (ap, bp, cp, dp, ep, ftphi);
		sr = sphsr (a, eSquared, ftphi);
		LKASSERT(sr!=0);
		ftphi = ftphi + (tmd - t10) / sr;
	}
	sr = sphsr (a, eSquared, ftphi);
	sn = sphsn (a, eSquared, ftphi);
	s = sin (ftphi);
	c = cos (ftphi);
	LKASSERT(c!=0);
	t = s / c;
	eta = e2Squared * (c * c);
	de = easting - fe;
	LKASSERT(sn!=0);
	LKASSERT(sr!=0);
	t10 = t / (2.0 * sr * sn * (ok * ok));
	t11 = t * (5.0 + 3.0 * (t * t) + eta - 4.0 * (eta * eta) - 9.0 * (t * t)
		* eta) / (24.0 * sr * (sn * sn * sn) * (ok * ok * ok * ok));
	lat = ftphi - (de * de) * t10 + (de * de * de * de) * t11;
	t14 = 1.0 / (sn * c * ok);
	t15 = (1.0 + 2.0 * (t * t) + eta) / (6 * (sn * sn * sn) * c
		* (ok * ok * ok));
	dlam = de * t14 - (de * de * de) * t15;
	olam = (utmXZone * 6 - 183.0) * deg2rad;
	lon = olam + dlam;
	lon *= rad2deg;
	lat *= rad2deg;
}

//=======================================================================
// Purpose:
//  This function converts the specified UTM coordinate to a lat/lon
//  coordinate in the WGS84 datum.  (See the comment block for the
//  UtmToLatLon() member function.
//=======================================================================

void UtmToLatLonWGS84 (int utmXZone, char utmYZone, double easting,
					   double northing, double& lat, double& lon)
	{
	UtmToLatLon (6378137.0, 1 / 298.257223563, utmXZone, utmYZone,
		easting, northing, lat, lon);
	}
