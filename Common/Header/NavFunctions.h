#if !defined(NAVFUNCTIONS_H)
#define NAVFUNCTIONS_H


void xXY_to_LL(double Lat_TP, double Lon_TP, double X_int, double Y_int, double *Lat, double *Lon);
void xLL_to_XY(double Lat_TP, double Lon_TP, double Lat_Pnt, double Lon_Pnt, double *X, double *Y);
void xXY_Brg_Rng(double X_1, double Y_1, double X_2, double Y_2, double *Bearing, double *Range);
void xBrg_Rng_XY(double X_RefPos, double Y_RefPos, double Bearing, double Range, double *X, double *Y);
void xCrs_Spd_to_VxVy(double Crs, double Spd, double *Vx, double *Vy);
void xVxVy_to_Crs_Spd(double Vx, double Vy, double *Crs, double *Spd);
void LL_to_BearRange(double Lat_TP, double Long_TP, double Lat_Pnt, double Long_Pnt,
			double *Bearing, double *Range);
void DistanceBearing(double lat1, double lon1, double lat2, double lon2, double *Distance, double *Bearing);
double DoubleDistance(double lat1, double lon1, double lat2, double lon2, double lat3, double lon3);
void FindLatitudeLongitude(double Lat, double Lon, double Bearing, double Distance,
			double *lat_out, double *lon_out);

void IntermediatePoint(double lon1, double lat1,
                       double lon2, double lat2,
                       double dthis,
                       double dtotal,
                       double *lon3, double *lat3);

double CrossTrackError(double lon1, double lat1,
                       double lon2, double lat2,
                       double lon3, double lat3,
                       double *lon4, double *lat4);

void ScreenClosestPoint(const POINT &p1, const POINT &p2,
                        const POINT &p3, POINT *p4, int offset);

double ProjectedDistance(double lon1, double lat1,
                         double lon2, double lat2,
                         double lon3, double lat3,
                         double *xtd, double *crs);

void LatLon2Flat(double lon, double lat, int *scx, int *scy);

#endif
