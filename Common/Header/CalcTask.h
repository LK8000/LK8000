
extern void AddAATPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int taskwaypoint);
extern double AATCloseBearing(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void AATStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void AlertGateOpen(int gate);
extern void AnnounceWayPointSwitch(DERIVED_INFO *Calculated, bool do_advance);
extern bool IsFinalWaypoint(void);
extern bool InFinishSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const int i);
extern bool InStartSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int &index, BOOL *CrossedStart);
extern bool InTurnSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const int the_turnpoint);
extern void CheckFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void CheckForceFinalGlide(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void CheckGlideThroughTerrain(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void CheckInSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern BOOL CheckRestart(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int *LastStartSector);
extern void CheckStart(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int *LastStartSector);
extern void CheckTransitionFinalGlide(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern bool ReadyToAdvance(DERIVED_INFO *Calculated, bool reset=true, bool restart=false);
extern bool ReadyToStart(DERIVED_INFO *Calculated);
extern double SpeedHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern bool TaskAltitudeRequired(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                                 double this_maccready, double *Vfinal,
                                 double *TotalTime, double *TotalDistance,
                                 int *ifinal);
extern void TaskSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_maccready);
extern void TaskStatistics(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_maccready);
extern bool ValidFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated);


