/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "McReady.h"


extern double CRUISE_EFFICIENCY;

//
// Sollfarh / Dolphin Speed calculator
//
void SpeedToFly(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {


    if (((AutoMcMode == amcFinalGlide) || (AutoMcMode == amcFinalAndClimb)) 
            && DoOptimizeRoute() && Calculated->NextAltitude > 0.) {

        // Special case for Conical end of Speed section
        int Type = -1;
        double ConeSlope = 0.0;
        LockTaskData();
        if (ValidTaskPoint(ActiveWayPoint)) {
            GetTaskSectorParameter(ActiveWayPoint, &Type, NULL);
            ConeSlope = Task[ActiveWayPoint].PGConeSlope;
        }
        UnlockTaskData();
        if (Type == CONE && ConeSlope > 0.0) {
            double VOpt = GlidePolar::FindSpeedForSlope(ConeSlope);
            double eqMC = GlidePolar::EquMC(VOpt);
            if(eqMC <= MACCREADY ) {
                Calculated->VOpt = VOpt;
                return;
            }
        }
    }

    double HeadWind = 0;
    if (Calculated->FinalGlide && ValidTaskPoint(ActiveWayPoint)) {
        // according to MC theory STF take account of wind only if on final Glide
        // TODO : for the future add config parameter for always use wind.
        if (Calculated->HeadWind != -999) {
            HeadWind = Calculated->HeadWind;
        }
    }

    // this is IAS for best Ground Glide ratio acounting current air mass ( wind / Netto vario )
    double VOptnew = GlidePolar::STF(MACCREADY, Calculated->NettoVario, HeadWind);

    // apply cruises efficiency factor.
    VOptnew *= CRUISE_EFFICIENCY;
    
    if (Calculated->NettoVario > MACCREADY) {
        // this air mass is better than maccready, so don't fly at speed less than minimum sink speed adjusted for load factor
        double n = fabs((Basic->AccelerationAvailable) ? Basic->AccelZ : Calculated->Gload);
        VOptnew = max(VOptnew, GlidePolar::Vminsink() * sqrt(n));
    } else {
        // never fly at speed less than min sink speed
        VOptnew = max(VOptnew, GlidePolar::Vminsink());
    }

    // use low pass filter for avoid big jump of value.
    Calculated->VOpt = LowPassFilter(Calculated->VOpt, VOptnew, 0.6);
}

