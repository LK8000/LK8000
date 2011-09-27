#if !defined(NAVFUNCTIONS)
#define NAVFUNCTIONS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>

	void xXY_to_LL(double Lat_TP, double Lon_TP, double X_int, double Y_int, double *Lat, double *Lon);
	void xLL_to_XY(double Lat_TP, double Lon_TP, double Lat_Pnt, double Lon_Pnt, double *X, double *Y);
	void xXY_Brg_Rng(double X_1, double Y_1, double X_2, double Y_2, double *Bearing, double *Range);
	void xBrg_Rng_XY(double X_RefPos, double Y_RefPos, double Bearing, double Range, double *X, double *Y);
	void xCrs_Spd_to_VxVy(double Crs, double Spd, double *Vx, double *Vy);
	void xVxVy_to_Crs_Spd(double Vx, double Vy, double *Crs, double *Spd);
	void LL_to_BearRange(double Lat_TP, double Long_TP, double Lat_Pnt, double Long_Pnt, double *Bearing, double *Range);
#endif
