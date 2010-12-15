/************************************************************************
*
* File:			OSGB.h
* RCS:			$Header: /cvsroot/stelvio/stelvio/NavStar/OSGB.h,v 1.1.1.1 2000/11/02 06:00:27 steve_l Exp $
* Author:		Steve Loughran
* Created:		1999
* Language:		C++
* Package:		
* Status:		Experimental
* @doc
*
************************************************************************/

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

//LatLong-  conversion..h
//definitions for lat/long to OSGB and OSGB to lat/lng conversions
//#include <string.h>
#pragma once
#ifndef LATLONGCONV
#define LATLONGCONV

bool LLtoOSGB(const double Lat, 
			  const double Long, 
			  long &OSGBEasting, 
			  long &OSGBNorthing, 
			  TCHAR OSGBGridSquare[3]);

bool OSGBtoLL(const double OSGBNorthing, 
			  const double OSGBEasting, 
			  LPCTSTR OSGBZone,
			  double& Lat,  
			  double& Long );

//LPCTSTR OSGBLetterDesignator(double Lat);

bool CoordsToOSGBSquare(double easting, 
						double northing,  
						TCHAR OSGBGridSquare[3], 
						long &OSGBEasting, 
						long &OSGBNorthing);

bool OSGBSquareToRefCoords(const char* OSGBGridSquare,
						   int &RefEasting, 
						   int &RefNorthing);


const double PI(3.14159265);
const double FOURTHPI(PI / 4);
const double deg2rad(PI / 180.0);
const double rad2deg(180.0 / PI);


#endif
