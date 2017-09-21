/*  module magfield.c */

/* Module to calculate magnetic variation and field given position,
**               altitude, and date
** Implements the NIMA (formerly DMA) WMM and IGRF models
**
**    http://www.nima.mil/GandG/ngdc-wmm2000.html
**    For WMM2000 coefficients:
**    ftp://ftp.ngdc.noaa.gov/Solid_Earth/Mainfld_Mag/DoD_Model/wmm.cof
**    For IGRF/DGRF coefficients:
**    http://swdcdb.kugi.kyoto-u.ac.jp/igrf/coef/igrfall.d
**
** Copyright (C) 2000  Edward A Williams <Ed_Williams@compuserve.com>
**
**  The routine uses a spherical harmonic expansion of the magnetic
** potential up to twelfth order, together with its time variation, as
** described in Chapter 4 of "Geomagnetism, Vol 1, Ed. J.A.Jacobs,
** Academic Press (London 1987)". The program first converts geodetic
** coordinates (lat/long on elliptic earth and altitude) to spherical
** geocentric (spherical lat/long and radius) coordinates. Using this,
** the spherical (B_r, B_theta, B_phi) magnetic field components are
** computed from the model. These are finally referred to surface (X, Y,
** Z) coordinates.
**
**   Fields are accurate to better than 200nT, variation and dip to
** better than 0.5 degrees, with the exception of the declination near
** the magnetic poles (where it is ill-defined) where the error may reach
** 4 degrees or more.
**
**   Variation is undefined at both the geographic and  
** magnetic poles, even though the field itself is well-behaved. To
** avoid the routine blowing up, latitude entries corresponding to
** the geographic poles are slightly offset. At the magnetic poles,
** the routine returns zero variation.
**
** HISTORY
** Adapted from EAW Excel 3.0 version 3/27/94 EAW
** Recoded in C++ by Starry Chan
** WMM95 added and rearranged in ANSI-C EAW 7/9/95
** Put shell around program and made Borland & GCC compatible EAW 11/22/95
** IGRF95 added 2/96 EAW
** WMM2000 IGR2000 added 2/00 EAW
** Released under GPL  3/26/00 EAW
** Adaptions and modifications for the SimGear project  3/27/2000 CLO
** Removed all pow() calls and made static roots[][] arrays to
** save many sqrt() calls on subsequent invocations
** 3/28/2000  Norman Vine -- nhv@yahoo.com
** Put in some bullet-proofing to handle magnetic and geographic poles.
** 3/28/2000 EAW
** Added missing comment close, the lack of which caused the altitude 
** correction to be omitted.
** 01/31/01 Jim Seymour (jseymour@LinxNet.com)
** 1/16/05 EAW  added wmm2005 igrf2005 models
** modified code for n=13 expansion for igrf2005
** 1/28/12 EAW added wmm2010
** 7/7
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

** DEEPLY CUT FOR LK8000 USAGE, NO USE OF MODEL (always using WMM2015)
*/ 

#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include        "magfield.h"
#include        "LKAssert.h"

#define	max(a,b)	(((a) > (b)) ? (a) : (b))

static const double pi = 3.14159265358979;
static const double a = 6378.16;	/* major radius (km) IAU66 ellipsoid */
static const double f = 1.0 / 298.25;	/* inverse flattening IAU66 ellipsoid */
static const double b = 6378.16 * (1.0 -1.0 / 298.25 );
	/* minor radius b=a*(1-f) */
static const double r_0 = 6371.2;	/* "mean radius" for spherical harmonic expansion */


static const double gnm_wmm2015[13][13]={
{0,0,0,0,0,0,0,0,0,0,0,0,0},
{-29438.5,-1501.1,0,0,0,0,0,0,0,0,0,0,0},
{-2445.3,3012.5,1676.6,0,0,0,0,0,0,0,0,0,0},
{1351.1,-2352.3,1225.6,581.9,0,0,0,0,0,0,0,0,0},
{907.2,813.7,120.3,-335,70.3,0,0,0,0,0,0,0,0},
{-232.6,360.1,192.4,-141,-157.4,4.3,0,0,0,0,0,0,0},
{69.5,67.4,72.8,-129.8,-29,13.2,-70.9,0,0,0,0,0,0},
{81.6,-76.1,-6.8,51.9,15,9.3,-2.8,6.7,0,0,0,0,0},
{24,8.6,-16.9,-3.2,-20.6,13.3,11.7,-16,-2,0,0,0,0},
{5.4,8.8,3.1,-3.1,0.6,-13.3,-0.1,8.7,-9.1,-10.5,0,0,0},
{-1.9,-6.5,0.2,0.6,-0.6,1.7,-0.7,2.1,2.3,-1.8,-3.6,0,0},
{3.1,-1.5,-2.3,2.1,-0.9,0.6,-0.7,0.2,1.7,-0.2,0.4,3.5,0},
{-2,-0.3,0.4,1.3,-0.9,0.9,0.1,0.5,-0.4,-0.4,0.2,-0.9,0}
};
static const double hnm_wmm2015[13][13]={
{0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,4796.2,0,0,0,0,0,0,0,0,0,0,0},
{0,-2845.6,-642,0,0,0,0,0,0,0,0,0,0},
{0,-115.3,245,-538.3,0,0,0,0,0,0,0,0,0},
{0,283.4,-188.6,180.9,-329.5,0,0,0,0,0,0,0,0},
{0,47.4,196.9,-119.4,16.1,100.1,0,0,0,0,0,0,0},
{0,-20.7,33.2,58.8,-66.5,7.3,62.5,0,0,0,0,0,0},
{0,-54.1,-19.4,5.6,24.4,3.3,-27.5,-2.3,0,0,0,0,0},
{0,10.2,-18.1,13.2,-14.6,16.2,5.7,-9.1,2.2,0,0,0,0},
{0,-21.6,10.8,11.7,-6.8,-6.9,7.8,1,-3.9,8.5,0,0,0},
{0,3.3,-0.3,4.6,4.4,-7.9,-0.6,-4.1,-2.8,-1.1,-8.7,0,0},
{0,-0.1,2.1,-0.7,-1.1,0.7,-0.2,-2.1,-1.5,-2.5,-2,-2.3,0},
{0,-1,0.5,1.8,-2.2,0.3,0.7,-0.1,0.3,0.2,-0.9,-0.2,0.7}
};
static const double gtnm_wmm2015[13][13]={
{0,0,0,0,0,0,0,0,0,0,0,0,0},
{10.7,17.9,0,0,0,0,0,0,0,0,0,0,0},
{-8.6,-3.3,2.4,0,0,0,0,0,0,0,0,0,0},
{3.1,-6.2,-0.4,-10.4,0,0,0,0,0,0,0,0,0},
{-0.4,0.8,-9.2,4,-4.2,0,0,0,0,0,0,0,0},
{-0.2,0.1,-1.4,0,1.3,3.8,0,0,0,0,0,0,0},
{-0.5,-0.2,-0.6,2.4,-1.1,0.3,1.5,0,0,0,0,0,0},
{0.2,-0.2,-0.4,1.3,0.2,-0.4,-0.9,0.3,0,0,0,0,0},
{0,0.1,-0.5,0.5,-0.2,0.4,0.2,-0.4,0.3,0,0,0,0},
{0,-0.1,-0.1,0.4,-0.5,-0.2,0.1,0,-0.2,-0.1,0,0,0},
{0,0,-0.1,0.3,-0.1,-0.1,-0.1,0,-0.2,-0.1,-0.2,0,0},
{0,0,-0.1,0.1,0,0,0,0,0,0,-0.1,-0.1,0},
{0.1,0,0,0.1,-0.1,0,0.1,0,0,0,0,0,0}
};
static const double htnm_wmm2015[13][13]={
{0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,-26.8,0,0,0,0,0,0,0,0,0,0,0},
{0,-27.1,-13.3,0,0,0,0,0,0,0,0,0,0},
{0,8.4,-0.4,2.3,0,0,0,0,0,0,0,0,0},
{0,-0.6,5.3,3,-5.3,0,0,0,0,0,0,0,0},
{0,0.4,1.6,-1.1,3.3,0.1,0,0,0,0,0,0,0},
{0,0,-2.2,-0.7,0.1,1,1.3,0,0,0,0,0,0},
{0,0.7,0.5,-0.2,-0.1,-0.7,0.1,0.1,0,0,0,0,0},
{0,-0.3,0.3,0.3,0.6,-0.1,-0.2,0.3,0,0,0,0,0},
{0,-0.2,-0.1,-0.2,0.1,0.1,0,-0.2,0.4,0.3,0,0,0},
{0,0.1,-0.1,0,0,-0.2,0.1,-0.1,-0.2,0.1,-0.1,0,0},
{0,0,0.1,0,0.1,0,0,0.1,0,-0.1,0,-0.1,0},
{0,0,0,-0.1,0,0,0,0,0,0,0,0,0}
};

static const int nmax = 13;

double P[14][14];
double DP[14][14];
double gnm[14][14];
double hnm[14][14];
double sm[14];
double cm[14];

static double root[14];
static double roots[14][14][2];

/* Convert date to Julian day    1950-2049 */
unsigned long int yymmdd_to_julian_days(int yy,int mm,int dd)
{
  unsigned long jd;
 
  yy = (yy < 50) ? (2000 + yy) : (1900 + yy);
  jd = dd - 32075L + 1461L * (yy + 4800L + (mm - 14) / 12 ) / 4;
  jd = jd + 367L * (mm - 2 - (mm - 14) / 12*12) / 12;
  jd = jd - 3 * ((yy + 4900L + (mm - 14) / 12) / 100) / 4;

  return(jd);
} 

/* Convert degrees to radians */
double deg_to_rad(double deg)
{
return deg*pi/180.;
}

/* Convert radians to degrees */
double rad_to_deg(double rad)
{
return rad*180./pi;
}
	     



/* 
 * return variation (in radians) given geodetic latitude (radians), longitude
 * (radians) ,height (km) and (Julian) date
 * model=1 is IGRF90, 2 is WMM85, 3 is WMM90, 4 is WMM95, 
 * 5 is IGRF95, 6 is WMM2000, 7 is IGRF2000, 8 is WMM2005, 9 is IGRF2005, 10 is WMM2010, 11 is IGRF2010, 12 is WMM2015, 13 is IGRF2015
 * N and E lat and long are positive, S and W negative

   2017/09/21 modified for LK8000 by paolo to adopt only WMM2015 model, and no field output.
   The source of the original C code is at www.edwilliams.org
   We are all grateful to Ed for his work!
*/

double SGMagVar( double lat, double lon, double h, long dat)
{
    /* output field B_r,B_th,B_phi,B_x,B_y,B_z */
    int n,m,nmaxl;
    double yearfrac,sr,r,theta,c,s,psi,fn,fn_0,B_r,B_theta,B_phi,X,Y;
    double sinpsi, cospsi, inv_s;

    static int been_here = 0;
	
    double sinlat = sin(lat);
    double coslat = cos(lat);

    /* convert to geocentric */ 
    sr = sqrt(a*a*coslat*coslat + b*b*sinlat*sinlat);
    /* sr is effective radius */
    theta = atan2(coslat * (h*sr + a*a), sinlat * (h*sr + b*b));
		  
    /* theta is geocentric co-latitude */
    LKASSERT((a*a - (a*a - b*b) * sinlat*sinlat) !=0);

    r = h*h + 2.0*h * sr +
	(a*a*a*a - ( a*a*a*a - b*b*b*b ) * sinlat*sinlat ) / 
	(a*a - (a*a - b*b) * sinlat*sinlat );

    r = sqrt(r);

    /* r is geocentric radial distance */
    c = cos(theta);
    s = sin(theta);
      /* protect against zero divide at geographic poles */
    inv_s =  1.0 / (s + (s == 0.)*1.0e-8); 

    /*zero out arrays */
    for ( n = 0; n <= nmax; n++ ) {
      for ( m = 0; m <= n; m++ ) {
	P[n][m] = 0;
	DP[n][m] = 0;
      }
    }

    /* diagonal elements */
    P[0][0] = 1;
    P[1][1] = s;
    DP[0][0] = 0;
    DP[1][1] = c;
    P[1][0] = c ;
    DP[1][0] = -s;

    /* these values will not change for subsequent function calls */
    if( !been_here ) {
      for ( n = 2; n <= nmax; n++ ) {
	root[n] = sqrt((2.0*n-1) / (2.0*n));
      }
	
      for ( m = 0; m <= nmax; m++ ) {
	double mm = m*m;
	for ( n = max(m + 1, 2); n <= nmax; n++ ) {
	  roots[m][n][0] = sqrt((n-1)*(n-1) - mm);
	  roots[m][n][1] = 1.0 / sqrt( n*n - mm);
	}
      }
      been_here = 1;
    }
	
    for ( n=2; n <= nmax; n++ ) {
      /*  double root = sqrt((2.0*n-1) / (2.0*n)); */
      P[n][n] = P[n-1][n-1] * s * root[n];
      DP[n][n] = (DP[n-1][n-1] * s + P[n-1][n-1] * c) * root[n];
    }

    /* lower triangle */
    for ( m = 0; m <= nmax; m++ ) {
      /*  double mm = m*m;  */
      for ( n = max(m + 1, 2); n <= nmax; n++ ) {
	/* double root1 = sqrt((n-1)*(n-1) - mm); */
	/* double root2 = 1.0 / sqrt( n*n - mm);  */
	P[n][m] = (P[n-1][m] * c * (2.0*n-1) -
		   P[n-2][m] * roots[m][n][0]) * roots[m][n][1];
	DP[n][m] = ((DP[n-1][m] * c - P[n-1][m] * s) *
		    (2.0*n-1) - DP[n-2][m] * roots[m][n][0]) * roots[m][n][1];
      }
    }

    /* compute gnm, hnm at dat */
    nmaxl = 12;  /* models except IGRF2005 */

      yearfrac = (dat - yymmdd_to_julian_days(15,1,1)) / 365.25;
      for (n=1;n<=nmaxl;n++)
	for (m = 0;m<=nmaxl;m++) {
	  gnm[n][m] = gnm_wmm2015[n][m] + yearfrac * gtnm_wmm2015[n][m];
	  hnm[n][m] = hnm_wmm2015[n][m] + yearfrac * htnm_wmm2015[n][m];
	}

    /* compute sm (sin(m lon) and cm (cos(m lon)) */
    for (m = 0;m<=nmaxl;m++) {
      sm[m] = sin(m * lon);
      cm[m] = cos(m * lon);
    }

    /* compute B fields */
    B_r = 0.0;
    B_theta = 0.0;
    B_phi = 0.0;
    fn_0 = r_0/r;
    fn = fn_0 * fn_0;
	
    for ( n = 1; n <= nmaxl; n++ ) {
      double c1_n=0;
      double c2_n=0;
      double c3_n=0;
      for ( m = 0; m <= n; m++ ) {
	double tmp = (gnm[n][m] * cm[m] + hnm[n][m] * sm[m]); 
	c1_n += tmp * P[n][m];
	c2_n += tmp * DP[n][m];
	c3_n +=  m * (gnm[n][m] * sm[m] - hnm[n][m] * cm[m]) * P[n][m];
      }
      /* fn=pow(r_0/r,n+2.0);   */
      fn *= fn_0;
      B_r += (n + 1) * c1_n * fn;
      B_theta -= c2_n * fn;
      B_phi += c3_n * fn * inv_s;
    }



    /* Find geodetic field components: */
    psi = theta - (pi / 2.0 - lat);
    sinpsi = sin(psi);
    cospsi = cos(psi);
    X = -B_theta * cospsi - B_r * sinpsi;
    Y = B_phi;

    /* find variation in radians */
    /* return zero variation at magnetic pole X=Y=0. */
    /* E is positive */
    return (X != 0. || Y != 0.) ? atan2(Y, X) : (double) 0.; 
}

