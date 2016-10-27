#ifndef DEVLK8EX1_H
#define DEVLK8EX1_H



BOOL LK8EX1Register(void);

BOOL LK8EX1ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);
BOOL LK8EX1IsBaroSource(PDeviceDescriptor_t d);
BOOL LK8EX1LinkTimeout(PDeviceDescriptor_t d);
#endif
