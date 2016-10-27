#ifndef DEV_PVCOM_H
#define DEV_PVCOM_H


struct TAtmosphericInfo {
  double pStatic;
  double pTotal;
  double pAlt;
  double qnh;
  double windDirection;
  double windSpeed;
  double tas;
  double vzp;
  double oat;
  double humidity;
  double cloudBase;
  double cloudTemp;
  double groundTemp;
};

struct TSpaceInfo {
  double eulerRoll;
  double eulerPitch;
  double rollRate;
  double pitchRate;
  double yawRate;
  double accelX;
  double accelY;
  double accelZ;
  double virosbandometer;
  double trueHeading;
  double magneticHeading;
  double localDeclination;
};

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
