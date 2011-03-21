/* -*-C++-*-
*******************************************************************
*
* File:			$Workfile: OSGB.cpp $ 
* RCS:          $Header: /cvsroot/stelvio/stelvio/NavStar/OSGB.cpp,v 1.1.1.1 2000/11/02 06:00:27 steve_l Exp $
* Author:       Steve Loughran,
* Created:      Jan 2000
* Modified:		$Modtime: 00-08-23 22:41 $
* Revision		$Revision: 1.1.1.1 $  
* Language:     C++
* Package:      Navstar
* Status:       Work In Progress
* Homepage:		www.iseran.com/gps
* @doc
*********************************************************************/

/*
	Lat Long - OSGB, OSGB - Lat Long conversions
	These all came from some code by
	Chuck Gantz- chuck.gantz@globalstar.com
	The math is complex, so do not mess with it with caution.
	One area to look at more rigourously is how well it handles
	out of bound grid squares. 

	Chuck's original code wasnt too strict and could generate references
	which were invalid if you were outside the UK. Now it certainly doesnt
	do that in the US, but I have encountered the problem in the Netherlands
	*/
/*
	Copyright:	Copyright (c) 2000 Steve Loughran. 

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "stdafx.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#include "OSGB.h"

#include "utils/heapcheck.h"



//=======================================================================
/**
@func	
@parm	
@rdesc	
//converts lat/long to OSGB coords.  Equations from USGS Bulletin 1532 
//East Longitudes are positive, West longitudes are negative. 
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees
	//Written by Chuck Gantz- chuck.gantz@globalstar.com
*/
//=======================================================================

bool LLtoOSGB(const double Lat, const double Long, 
			  long &OSGBEasting, long &OSGBNorthing, TCHAR OSGBGridSquare[3])
	{
	
	double a;
	double eccSquared;
	double k0 = 0.9996012717;
	
	double LongOrigin = -2;
	double LongOriginRad = LongOrigin * deg2rad;
	double LatOrigin = 49;
	double LatOriginRad = LatOrigin * deg2rad;
	double eccPrimeSquared;
	double N, T, C, A, M;
	
	double LatRad = Lat*deg2rad;
	double LongRad = Long*deg2rad;
	double easting, northing;
	
	double majoraxis = a = 6377563.396;//Airy
	double minoraxis = 6356256.91;//Airy
	
	eccSquared = (majoraxis * majoraxis - minoraxis * minoraxis) 
		/ (majoraxis * majoraxis);
	
	//only calculate M0 once since it is based on the origin 
	//of the OSGB projection, which is fixed
	static double 	M0 = a*((1		- eccSquared/4 - 3*eccSquared*eccSquared/64	
		- 5*eccSquared*eccSquared*eccSquared/256)*LatOriginRad 
		- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	
		+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatOriginRad)
		+ (15*eccSquared*eccSquared/256 
		+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatOriginRad) 
		- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatOriginRad));
	
	eccPrimeSquared = (eccSquared)/(1-eccSquared);
	
	N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
	T = tan(LatRad)*tan(LatRad);
	C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
	A = cos(LatRad)*(LongRad-LongOriginRad);
	
	
	
	M = a*((1	- eccSquared/4		- 3*eccSquared*eccSquared/64	
		- 5*eccSquared*eccSquared*eccSquared/256)*LatRad 
		- (3*eccSquared/8	
		+ 3*eccSquared*eccSquared/32	
		+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
		+ (15*eccSquared*eccSquared/256 
		+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad) 
		- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
	
	easting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
		+ (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120));
	easting += 400000.0; //false easting
	
	northing = (double)(k0*(M-M0+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
		+ (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
	northing -= 100000.0;//false northing
	
	return CoordsToOSGBSquare(easting, northing, OSGBGridSquare, OSGBEasting, OSGBNorthing);
	
	}

//=======================================================================
//=======================================================================

bool CoordsToOSGBSquare(double easting, double northing,  
				  TCHAR OSGBGridSquare[3], long &OSGBEasting, long &OSGBNorthing)
	{
	TCHAR GridSquare[] = _T("VWXYZ"
		"QRSTU"
		"LMNOP"
		"FGHJK"
		"ABCDE");
	int posx, posy; //positions in grid
	
	OSGBEasting = long(easting + 0.5); //round to nearest int
	OSGBNorthing = long(northing + 0.5); //round to nearest int
	
	//find correct 500km square
	posx = OSGBEasting / 500000L;
	posy = OSGBNorthing / 500000L;
	if(posx<0 || posx>4 || posy<0 || posy>4)
		return false;
	int offset=posx + posy * 5 + 7;
	OSGBGridSquare[0] = GridSquare[offset];
	
	//find correct 100km square
	posx = OSGBEasting % 500000L;//remove 500 km square
	posy = OSGBNorthing % 500000L;//remove 500 km square
	posx = posx / 100000L;//find 100 km square
	posy = posy / 100000L;//find 100 km square
	if(posx<0 || posx>4 || posy<0 || posy>4)
		return false;
	offset=posx + posy * 5;
	OSGBGridSquare[1] = GridSquare[offset];
	
	OSGBGridSquare[2] = _T('\0');//terminate grid ref string with null
	
	//remainder is northing and easting
	OSGBNorthing = OSGBNorthing % 500000L; 
	OSGBNorthing = OSGBNorthing % 100000L;
	
	OSGBEasting = OSGBEasting % 500000L;
	OSGBEasting = OSGBEasting % 100000L;
	return true;
	}


//=======================================================================
//=======================================================================

bool OSGBSquareToRefCoords(LPCTSTR OSGBGridSquare,
						   int &RefEasting, 
						   int &RefNorthing)
{
	bool b=true;
	int pos, x_multiplier, y_multiplier;
	TCHAR GridSquare[] = _T("VWXYZQRSTULMNOPFGHJKABCDE");

	//find 500km offset
	TCHAR ch = _totupper( OSGBGridSquare[0]);
	switch(ch)
	{
		case _T('S'): x_multiplier = 0; y_multiplier = 0; break;
		case _T('T'): x_multiplier = 1; y_multiplier = 0; break;
		case _T('N'): x_multiplier = 0; y_multiplier = 1; break;
		case _T('O'): x_multiplier = 1; y_multiplier = 1; break;
		case _T('H'): x_multiplier = 0; y_multiplier = 2; break;
		case _T('J'): x_multiplier = 1; y_multiplier = 2; break;
		default:
			b=false; break;
	}
	if(!b)
		return false;
	RefEasting = x_multiplier * 500000L;
	RefNorthing = y_multiplier * 500000L;

//find 100km offset and add to 500km offset to get coordinate of 
//square point is in
	ch=_totupper(OSGBGridSquare[1]);
	pos = _tcschr(GridSquare, ch)-GridSquare;
	if(pos<0)
		return false;

	RefEasting += ((pos % 5) * 100000L);
	RefNorthing += ((pos / 5) * 100000L);
	return true;
}

//=======================================================================
//converts OSGB coords to lat/long.  Equations from USGS Bulletin 1532 
//East Longitudes are positive, West longitudes are negative. 
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees. 
//Written by Chuck Gantz- chuck.gantz@globalstar.com
//=======================================================================

bool OSGBtoLL(const double OSGBNorthing, 
			  const double OSGBEasting, 
			  LPCTSTR OSGBZone,
			  double& Lat,  double& Long )
	{
	
	double k0 = 0.9996012717;
	double a;
	double eccPrimeSquared;
	double N1, T1, C1, R1, D, M;
	double LongOrigin = -2;
	double LatOrigin = 49;
	double LatOriginRad = LatOrigin * deg2rad;
	double mu, phi1, phi1Rad;
	double x, y;
	int RefEasting, RefNorthing;
	
	
	double majoraxis = a = 6377563.396;//Airy
	double minoraxis = 6356256.91;//Airy
	
	double eccSquared = (majoraxis * majoraxis - minoraxis * minoraxis) 
			/ (majoraxis * majoraxis);
	double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
	
	//only calculate M0 once since it is based on the origin of the OSGB projection, which is fixed
	static double 	M0 = a*((1	- eccSquared/4		
		- 3*eccSquared*eccSquared/64	
		- 5*eccSquared*eccSquared*eccSquared/256)*LatOriginRad 
		- (3*eccSquared/8	
		+ 3*eccSquared*eccSquared/32	
		+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatOriginRad)
		+ (15*eccSquared*eccSquared/256 
		+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatOriginRad) 
		- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatOriginRad));
	
	bool b=OSGBSquareToRefCoords(OSGBZone, RefEasting, RefNorthing);
	if(!b)
		return false;
	x = OSGBEasting - 400000.0 + RefEasting; //remove 400,000 meter false easing for longitude
	y = OSGBNorthing + 100000.0 + RefNorthing; //remove 100,000 meter false easing for longitude
	
	eccPrimeSquared = (eccSquared)/(1-eccSquared);
	
	M = M0 + y / k0;
	mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-
		5*eccSquared*eccSquared*eccSquared/256));
	
	phi1Rad = mu	+ (3*e1/2-27*e1*e1*e1/32)*sin(2*mu) 
		+ (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
		+(151*e1*e1*e1/96)*sin(6*mu);
	phi1 = phi1Rad*rad2deg;
	
	N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
	T1 = tan(phi1Rad)*tan(phi1Rad);
	C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
	R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
	D = x/(N1*k0);
	
	Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*
		(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
		+(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
	Lat = Lat * rad2deg;
	
	Long = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
		*D*D*D*D*D/120)/cos(phi1Rad);
	Long = LongOrigin + Long * rad2deg;
	return true;
	}
