
#ifndef	FLIGHT_DATA_REC_H
#define	FLIGHT_DATA_REC_H
 
#ifndef NO_DATARECORDER

typedef struct{
	 int   abLog;
	 float fMin;
	 float fMax;
	 TCHAR szName[30];

	 int aiCheckInterval;
	 int aiMaxWarnings;
	 int aiWarningCnt;
	 int iWarningDelay;
}sFlightDataRec;

extern bool FlightDataRecorderActive;

void InitFlightDataRecorder(void);
void UpdateFlightDataRecorder(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void CloseFlightDataRecorder(void);

#endif
#endif
