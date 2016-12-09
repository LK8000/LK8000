
#ifndef	FLIGHT_DATA_REC_H
#define	FLIGHT_DATA_REC_H
 
#ifndef NO_DATARECORDER

void InitFlightDataRecorder(void);
void UpdateFlightDataRecorder(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void CloseFlightDataRecorder(void);

#endif
#endif
