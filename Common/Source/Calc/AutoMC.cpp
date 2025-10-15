/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "McReady.h"

double PirkerAnalysis(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_bearing, const double GlideSlope);
bool ActiveIsFinalWaypoint();

// Helper function to handle AutoMC in equivalent mode
static bool TryHandleEquivalentAutoMC(DERIVED_INFO* Calculated) {
    if (AutoMcMode != amcEquivalent) {
        return false;
    }

    if (!Calculated->Circling && !Calculated->OnGround) {
        CheckSetMACCREADY(Calculated->EqMc, nullptr);
    }
    return true;
}

// Helper function to calculate the average thermal strength
static double CalculateAverageThermal(const DERIVED_INFO* Calculated) {
    if (flightstats.ThermalAverage.y_ave > 0) {
        if (Calculated->Circling && (Calculated->AverageThermal > 0)) {
            BUGSTOP_LKASSERT((flightstats.ThermalAverage.sum_n + 1) != 0);
            if (flightstats.ThermalAverage.sum_n == -1) {
                flightstats.ThermalAverage.sum_n = 0;
            }
            return (flightstats.ThermalAverage.y_ave * flightstats.ThermalAverage.sum_n +
                    Calculated->AverageThermal) /
                   (flightstats.ThermalAverage.sum_n + 1);
        }
        return flightstats.ThermalAverage.y_ave;
    }
    
    if (Calculated->Circling && (Calculated->AverageThermal > 0)) {
        // insufficient stats, so use this/last thermal's average
        return Calculated->AverageThermal;
    }

    return -1.0;
}

// Helper function to calculate MacCready value using Pirker analysis
static double CalculatePirkerMc(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
    BUGSTOP_LKASSERT((Calculated->WaypointDistance + 1) != 0);

    if (Calculated->WaypointDistance < 0) {
        Calculated->WaypointDistance = 0; // temporary but ok
    }
    const double slope = (Calculated->NavAltitude + Calculated->EnergyHeight -
                          FAIFinishHeight(Basic, Calculated, ActiveTaskPoint)) /
                         (Calculated->WaypointDistance + 1);

    const double mc_pirker = PirkerAnalysis(Basic, Calculated, Calculated->WaypointBearing, slope);
    return max(0.0, mc_pirker);
}

// Helper function to handle AutoMC during final glide
static void HandleFinalGlideAutoMC(double& mc_new, bool& first_mc, double av_thermal, NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
    if (Calculated->TaskAltitudeDifference0 > 0) {
        // only change if above final glide with zero Mc
        const double mc_pirker = CalculatePirkerMc(Basic, Calculated);
        if (first_mc) {
            // don't allow Mc to wind down to zero when first achieving final glide
            if (mc_pirker >= mc_new) {
                mc_new = mc_pirker;
                first_mc = false;
            } else if (AutoMcMode == amcFinalAndClimb && av_thermal > 0) {
                // revert to averager based auto Mc
                mc_new = av_thermal;
            }
        } else {
            mc_new = mc_pirker;
        }
    } else { // below final glide at zero Mc, never achieved final glide
        if (first_mc && AutoMcMode == amcFinalAndClimb && av_thermal > 0) {
            // revert to averager based auto Mc
            mc_new = av_thermal;
        }
    }
}


void DoAutoMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
    if (!Calculated->AutoMacCready) {
        return;
    }

    if (TryHandleEquivalentAutoMC(Calculated)) {
        return;
    }

    LockTaskData();

    static bool first_mc = true;
    bool is_final_glide = Calculated->FinalGlide &&
                          ValidTaskPoint(ActiveTaskPoint) &&
                          ActiveIsFinalWaypoint();

    if (!is_final_glide) {
        first_mc = true;
    }

    double mc_new = MACCREADY;
    const double av_thermal = CalculateAverageThermal(Calculated);

    if (!ValidTaskPoint(ActiveTaskPoint)) {
        mc_new = (av_thermal > 0) ? av_thermal : 0;
    } else if (((AutoMcMode == amcFinalGlide) || (AutoMcMode == amcFinalAndClimb)) && is_final_glide) {
        HandleFinalGlideAutoMC(mc_new, first_mc, av_thermal, Basic, Calculated);
    } else if ((AutoMcMode == amcAverageClimb) || ((AutoMcMode == amcFinalAndClimb) && !is_final_glide)) {
        if (av_thermal > 0) {
            mc_new = av_thermal;
        }
    }

    UnlockTaskData();

    CheckSetMACCREADY(LowPassFilter(MACCREADY, mc_new, 0.6), nullptr);
}
