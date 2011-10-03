/* magfield.h -- compute local magnetic variation given position,
**               altitude, and date
**
** This is an implementation of the NIMA WMM 2000
**
**    http://www.nima.mil/GandG/ngdc-wmm2000.html
**    For WMM2000 coefficients:
**    ftp://ftp.ngdc.noaa.gov/Solid_Earth/Mainfld_Mag/DoD_Model/wmm.cof
**    For IGRF/DGRF coefficients:
**    http://swdcdb.kugi.kyoto-u.ac.jp/igrf/coef/igrfall.d
**
** Copyright (C) 2000  Edward A Williams <Ed_Williams@compuserve.com>
**
** Adapted from Excel 3.0 version 3/27/94 EAW
** Recoded in C++ by Starry Chan
** WMM95 added and rearranged in ANSI-C EAW 7/9/95
** Put shell around program and made Borland & GCC compatible EAW 11/22/95
** IGRF95 added 2/96 EAW
** WMM2000 IGR2000 added 2/00 EAW
** Released under GPL 3/26/00 EAW
** Adaptions and modifications for the SimGear project  3/27/2000 CLO
** WMM2005 IGRF2005 added 01/05 EAW
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/




/* Convert date to Julian day    1950-2049 */
unsigned long int yymmdd_to_julian_days( int yy, int mm, int dd );

/* Convert degrees to radians */
double deg_to_rad( double deg );

/* Convert radians to degrees */
double rad_to_deg( double rad );

/* return variation (in radians) given geodetic latitude (radians), longitude
(radians) ,height (km), (Julian) date and field model
model=1 is IGRF90, 2 is WMM85, 3 is WMM90, 4 is WMM95, 5 is IGRF95, 
6 is WMM2000, 7 is IGRF2000, 8 is WMM2005, 9 is IGRF2005
N and E lat and long are positive, S and W negative
*/
double SGMagVar( double lat, double lon, double h, long dat);


