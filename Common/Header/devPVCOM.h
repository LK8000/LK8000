#ifndef DEV_PVCOM_H
#define DEV_PVCOM_H


BOOL PVCOMRegister(void);

int PVCOMNMEAddCheckSumStrg( TCHAR szStrg[] );
 BOOL PVCOMIsRadio(PDeviceDescriptor_t d);
 BOOL PVCOMPutVolume(PDeviceDescriptor_t d, int Volume) ;
 BOOL PVCOMPutSquelch(PDeviceDescriptor_t d, int Squelch) ;
 BOOL PVCOMPutFreqActive(PDeviceDescriptor_t d, double Freq, TCHAR StationName[]) ;
 BOOL PVCOMPutFreqStandby(PDeviceDescriptor_t d, double Freq,  TCHAR StationName[]) ;
 BOOL PVCOMStationSwap(PDeviceDescriptor_t d);
 BOOL PVCOMRequestAllData(PDeviceDescriptor_t d) ;
BOOL PVCOMParseString(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *info);
 BOOL PVCOMInstall(PDeviceDescriptor_t d);
 BOOL PVCOMRadioMode(PDeviceDescriptor_t d, int mode);



#endif

