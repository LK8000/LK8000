#ifndef DEV_KRT2_H
#define DEV_KRT2_H


BOOL KRT2Register(void);

int KRT2NMEAddCheckSumStrg( TCHAR szStrg[] );
 BOOL KRT2IsRadio(PDeviceDescriptor_t d);
 BOOL KRT2PutVolume(PDeviceDescriptor_t d, int Volume) ;
 BOOL KRT2PutSquelch(PDeviceDescriptor_t d, int Squelch) ;
 BOOL KRT2PutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) ;
 BOOL KRT2PutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) ;
 BOOL KRT2StationSwap(PDeviceDescriptor_t d);
 BOOL KRT2RequestAllData(PDeviceDescriptor_t d) ;
BOOL KRT2ParseString(PDeviceDescriptor_t d, char  *String, int len, NMEA_INFO *info);
 BOOL KRT2Install(PDeviceDescriptor_t d);
 BOOL KRT2RadioMode(PDeviceDescriptor_t d, int mode);



#endif
