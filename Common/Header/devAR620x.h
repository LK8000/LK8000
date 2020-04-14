#ifndef DEV_AR620x_H
#define DEV_AR620x_H


BOOL AR620xRegister(void);

int AR620xNMEAddCheckSumStrg( TCHAR szStrg[] );
 BOOL AR620xIsRadio(PDeviceDescriptor_t d);
 BOOL AR620xPutVolume(PDeviceDescriptor_t d, int Volume) ;
 BOOL AR620xPutSquelch(PDeviceDescriptor_t d, int Squelch) ;
 BOOL AR620xPutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) ;
 BOOL AR620xPutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) ;
 BOOL AR620xStationSwap(PDeviceDescriptor_t d);
 BOOL AR620xParseString(PDeviceDescriptor_t d, char  *String, int len, NMEA_INFO *info);
 BOOL AR620xInstall(PDeviceDescriptor_t d);
 BOOL AR620xRadioMode(PDeviceDescriptor_t d, int mode);



#endif
