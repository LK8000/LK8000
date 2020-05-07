#ifndef DEV_ATR833_H
#define DEV_ATR833_H


BOOL ATR833Register(void);

int ATR833NMEAddCheckSumStrg( TCHAR szStrg[] );
 BOOL ATR833IsRadio(PDeviceDescriptor_t d);
 BOOL ATR833PutVolume(PDeviceDescriptor_t d, int Volume) ;
 BOOL ATR833PutSquelch(PDeviceDescriptor_t d, int Squelch) ;
 BOOL ATR833PutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) ;
 BOOL ATR833PutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) ;
 BOOL ATR833StationSwap(PDeviceDescriptor_t d);
 BOOL ATR833RequestAllData(PDeviceDescriptor_t d) ;
BOOL ATR833ParseString(PDeviceDescriptor_t d, char  *String, int len, NMEA_INFO *info);
 BOOL ATR833Install(PDeviceDescriptor_t d);
 BOOL ATR833RadioMode(PDeviceDescriptor_t d, int mode);



#endif
