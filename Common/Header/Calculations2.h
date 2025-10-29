#ifndef CALCULATIONS2_H
#define CALCULATIONS2_H

void DoLogging(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool DetectFreeFlying(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

void AddSnailPoint(void);

double PirkerAnalysis(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
		      const double bearing,
		      const double GlideSlope);

double MultiLegPirkerAnalysis(NMEA_INFO* Basic, DERIVED_INFO* Calculated);

double EffectiveMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
double EffectiveCruiseEfficiency(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

double MacCreadyTimeLimit(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
			  const double bearing,
			  const double timeremaining,
			  const double hfinal);

void CalculateOwnTeamCode(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void CalculateTeammateBearingRange(NMEA_INFO *Basic, DERIVED_INFO *Calculated) ;
void CalculateOptimizedTargetPos(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void ClearOptimizedTargetPos();

#endif
