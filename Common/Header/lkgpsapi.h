#ifndef lkgpsapi_h__
#define lkgpsapi_h__

#define GPS_MAX_SATELLITES      12
#define GPS_MAX_PREFIX_NAME     16
#define GPS_MAX_FRIENDLY_NAME   64

#define GPS_VERSION_1           1

typedef enum {
	GPS_FIX_UNKNOWN = 0,
	GPS_FIX_2D,
	GPS_FIX_3D
}
GPS_FIX_TYPE;

typedef enum {
	GPS_FIX_SELECTION_UNKNOWN = 0,
	GPS_FIX_SELECTION_AUTO,
	GPS_FIX_SELECTION_MANUAL
}
GPS_FIX_SELECTION;

typedef enum {
	GPS_FIX_QUALITY_UNKNOWN = 0,
	GPS_FIX_QUALITY_GPS,
	GPS_FIX_QUALITY_DGPS
}
GPS_FIX_QUALITY;

//
// GPS_VALID_XXX bit flags in GPS_POSITION structure are valid.
//
#define GPS_VALID_UTC_TIME                                 0x00000001
#define GPS_VALID_LATITUDE                                 0x00000002
#define GPS_VALID_LONGITUDE                                0x00000004
#define GPS_VALID_SPEED                                    0x00000008
#define GPS_VALID_HEADING                                  0x00000010
#define GPS_VALID_MAGNETIC_VARIATION                       0x00000020
#define GPS_VALID_ALTITUDE_WRT_SEA_LEVEL                   0x00000040
#define GPS_VALID_ALTITUDE_WRT_ELLIPSOID                   0x00000080
#define GPS_VALID_POSITION_DILUTION_OF_PRECISION           0x00000100
#define GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION         0x00000200
#define GPS_VALID_VERTICAL_DILUTION_OF_PRECISION           0x00000400
#define GPS_VALID_SATELLITE_COUNT                          0x00000800
#define GPS_VALID_SATELLITES_USED_PRNS                     0x00001000
#define GPS_VALID_SATELLITES_IN_VIEW                       0x00002000
#define GPS_VALID_SATELLITES_IN_VIEW_PRNS                  0x00004000
#define GPS_VALID_SATELLITES_IN_VIEW_ELEVATION             0x00008000
#define GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH               0x00010000
#define GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO 0x00020000


//
// GPS_DATA_FLAGS_XXX bit flags set in GPS_POSITION dwFlags field
// provide additional information about the state of the query.
//

// Set when GPS hardware is not connected to GPSID and we
// are returning cached data.
#define GPS_DATA_FLAGS_HARDWARE_OFF                        0x00000001

//
// GPS_POSITION contains our latest physical coordinates, the time,
// and satellites used in determining these coordinates.
//
typedef struct _GPS_POSITION {
	DWORD dwVersion;             // Current version of GPSID client is using.
	DWORD dwSize;                // sizeof(_GPS_POSITION)

	// Not all fields in the structure below are guaranteed to be valid.
	// Which fields are valid depend on GPS device being used, how stale the API allows
	// the data to be, and current signal.
	// Valid fields are specified in dwValidFields, based on GPS_VALID_XXX flags.
	DWORD dwValidFields;

	// Additional information about this location structure (GPS_DATA_FLAGS_XXX)
	DWORD dwFlags;

	//** Time related
	SYSTEMTIME stUTCTime; 	//  UTC according to GPS clock.

	//** Position + heading related
	double dblLatitude;            // Degrees latitude.  North is positive
	double dblLongitude;           // Degrees longitude.  East is positive
	float  flSpeed;                // Speed in knots
	float  flHeading;              // Degrees heading (course made good).  True North=0
	double dblMagneticVariation;   // Magnetic variation.  East is positive
	float  flAltitudeWRTSeaLevel;  // Altitute with regards to sea level, in meters
	float  flAltitudeWRTEllipsoid; // Altitude with regards to ellipsoid, in meters

	//** Quality of this fix
	GPS_FIX_QUALITY     FixQuality;        // Where did we get fix from?
	GPS_FIX_TYPE        FixType;           // Is this 2d or 3d fix?
	GPS_FIX_SELECTION   SelectionType;     // Auto or manual selection between 2d or 3d mode
	float flPositionDilutionOfPrecision;   // Position Dilution Of Precision
	float flHorizontalDilutionOfPrecision; // Horizontal Dilution Of Precision
	float flVerticalDilutionOfPrecision;   // Vertical Dilution Of Precision

	//** Satellite information
	DWORD dwSatelliteCount;                                            // Number of satellites used in solution
	DWORD rgdwSatellitesUsedPRNs[GPS_MAX_SATELLITES];                  // PRN numbers of satellites used in the solution

	DWORD dwSatellitesInView;                      	                   // Number of satellites in view.  From 0-GPS_MAX_SATELLITES
	DWORD rgdwSatellitesInViewPRNs[GPS_MAX_SATELLITES];                // PRN numbers of satellites in view
	DWORD rgdwSatellitesInViewElevation[GPS_MAX_SATELLITES];           // Elevation of each satellite in view
	DWORD rgdwSatellitesInViewAzimuth[GPS_MAX_SATELLITES];             // Azimuth of each satellite in view
	DWORD rgdwSatellitesInViewSignalToNoiseRatio[GPS_MAX_SATELLITES];  // Signal to noise ratio of each satellite in view
} GPS_POSITION, *PGPS_POSITION;


//
// GPS_DEVICE contains information about the device driver and the
// service itself and is returned on a call to GPSGetDeviceState().
// States are indicated with SERVICE_STATE_XXX flags defined in service.h
//
typedef struct _GPS_DEVICE {
	DWORD    dwVersion;                                 // Current version of GPSID client is using.
	DWORD    dwSize;                                    // sizeof this structure
	DWORD    dwServiceState;                            // State of the GPS Intermediate Driver service.
	DWORD    dwDeviceState;                             // Status of the actual GPS device driver.
	FILETIME ftLastDataReceived;                        // Last time that the actual GPS device sent information to the intermediate driver.
	WCHAR    szGPSDriverPrefix[GPS_MAX_PREFIX_NAME];    // Prefix name we are using to communicate to the base GPS driver
	WCHAR    szGPSMultiplexPrefix[GPS_MAX_PREFIX_NAME]; // Prefix name that GPS Intermediate Driver Multiplexer is running on
	WCHAR    szGPSFriendlyName[GPS_MAX_FRIENDLY_NAME];  // Friendly name real GPS device we are currently using
} *PGPS_DEVICE, GPS_DEVICE;

HANDLE GPSOpenDevice(HANDLE hNewLocationData, HANDLE hDeviceStateChange, const WCHAR *szDeviceName, DWORD dwFlags);
DWORD  GPSCloseDevice(HANDLE hGPSDevice);
DWORD  GPSGetPosition(HANDLE hGPSDevice, GPS_POSITION *pGPSPosition, DWORD dwMaximumAge, DWORD dwFlags);
DWORD  GPSGetDeviceState(GPS_DEVICE *pGPSDevice);

#endif // lkgpsapi_h__
