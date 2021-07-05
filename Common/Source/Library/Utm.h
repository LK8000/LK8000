#ifndef _library_utm_h
#define _library_utm_h

void LatLonToUtmWGS84 (int& utmXZone, char& utmYZone, double& easting, double& northing, double lat, double lon);
void UtmToLatLonWGS84 (int utmXZone, char utmYZone, double easting, double northing, double& lat, double& lon);
	
#endif // _library_utm_h
