#if !defined(NAVFUNCTIONS_H)
#define NAVFUNCTIONS_H


void xXY_Brg_Rng(double X_1, double Y_1, double X_2, double Y_2, double *Bearing, double *Range);

double StraightDistance(double lat1, double lon1, double lat2, double lon2);

void DistanceBearing(double lat1, double lon1,
                     double lat2, double lon2,
                     double *Distance, double *Bearing);

double DoubleDistance(double lat1, double lon1,
                      double lat2, double lon2,
                      double lat3, double lon3);

void FindLatitudeLongitude(double Lat, double Lon,
                           double Bearing, double Distance,
                           double *lat_out, double *lon_out);

double CrossTrackError(double lon1, double lat1,
                       double lon2, double lat2,
                       double lon3, double lat3,
                       double *lon4, double *lat4);

double ProjectedDistance(double lon1, double lat1,
                         double lon2, double lat2,
                         double lon3, double lat3,
                         double *xtd, double *crs);

void LatLon2Flat(double lon, double lat, int *scx, int *scy);

#endif
