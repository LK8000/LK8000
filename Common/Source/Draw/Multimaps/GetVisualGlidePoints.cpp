/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Multimap.h"
#include "Sideview.h"
#include "LKObjects.h"
#include "RGB.h"
#include "LKStyle.h"
#include "McReady.h"

extern void ResetVisualGlideGlobals(void);

int slotWpIndex[MAXBSLOT + 1];


//#define DEBUG_GVG	1
//#define DEBUGSORT	1



// This is filling up the slotWpIndex[MAXBSLOT] list.
// DoNearest is automatically updating its data every 5 seconds.
// We are returning the number of items filled, or -1. In this case
// we should not print anything at all because there are no valid
// wpts, or they were not calculated , etc.
// The difference between 0 and -1:
//  0 means no waypoints found!
// -1 means data not yet ready, wait please.
//
// This is also called by DrawMultimap_Asp when a EVENT_NEWRUN is detected for visualglide mode.
// We are resetting from there.
//

short MapWindow::GetVisualGlidePoints(unsigned short numslots) {

    BUGSTOP_LKASSERT(numslots <= MAXBSLOT);
    if (numslots > MAXBSLOT) numslots = MAXBSLOT;

    static short currentFilledNumber = -1;
    static double tmpSlotBrgDiff[MAXBSLOT + 1];

    int i;

    // RESET COMMAND by passing 0, normally by EVENT_NEWRUN
    if (numslots == 0) {
#if DEBUG_GVG
        StartupStore(_T("...... GVGP: RESET\n"));
#endif
        for (i = 0; i < MAXBSLOT; i++) {
            slotWpIndex[i] = INVALID_VALUE;
        }
        currentFilledNumber = INVALID_VALUE;
        ResetVisualGlideGlobals();

        return INVALID_VALUE;
    }

    bool ndr = NearestDataReady;
    NearestDataReady = false;

    // No data ready..
    // if cfn is -1 we did not ever calculate it yet
    // otherwise 0 or >0  means use what we have already in the list
    if (!ndr) {
#if DEBUG_GVG
        StartupStore(_T("...... GVGP: no data ready, currentFN=%d\n"), currentFilledNumber);
#endif
        return currentFilledNumber;
    }

    if (SortedNumber <= 0) {
#if DEBUG_GVG
        StartupStore(_T("...... GVGP: SortedNumber is 0, no available wpts in this range!\n"));
#endif
        return 0;
    }

    int *pindex;
    int wpindex = 0;
    pindex = SortedTurnpointIndex;
    //
    // Reset  content
    //
    currentFilledNumber = 0;
    for (i = 0; i < MAXBSLOT; i++) {
        slotWpIndex[i] = INVALID_VALUE;
        tmpSlotBrgDiff[i] = -999;
    }

    //
    // set up fine tuned parameters for this run
    //

    int maxgratio = 1, mingratio = 1;
    double maxdistance = 300; // in METERS, not in KM!
    if (ISPARAGLIDER) {
        maxgratio = (int) (GlidePolar::bestld / 2);
        mingratio = (int) (GlidePolar::bestld * 1.5);
        maxdistance = 100;
    }
    if (ISGLIDER) {
        maxgratio = (int) (GlidePolar::bestld / 2);
        mingratio = (int) (GlidePolar::bestld * 1.5);
        maxdistance = 300;
    }
    if (ISGAAIRCRAFT) {
        maxgratio = (int) (GlidePolar::bestld / 2);
        mingratio = (int) (GlidePolar::bestld * 2);
        maxdistance = 300;
    }
    if (ISCAR) {
        maxgratio = 1;
        mingratio = 999;
        maxdistance = 100;
    }

    //
    // WE USE THE 2.3 PAGE (Nearest turnpoints) sorted by DIRECTION
    //

    // We do this in several passes.
#define MAXPHASES	4
    unsigned short phase = 1;

#if DEBUG_GVG
    StartupStore(_T("GVGP: USING  %d Sorted Items available\n"), SortedNumber);
    int count = 0;
#endif
_tryagain:

    for (i = 0; i < numslots; i++) {
        LKASSERT(phase <= MAXPHASES);
#if DEBUG_GVG
        if (i >= SortedNumber) {
            StartupStore(_T("...... GVGP: PHASE %d warning not enough SortedNumber (%d) for i=%d\n"), phase, SortedNumber, i);
        }
#endif

        // Did we found at least one valid in current phase? Otherwise we skip the rest of numslots.
        // And we pass directly to the next phase.
        bool found = false;

        // look up for an empty slot, needed if running after phase 1
        if (slotWpIndex[i] != INVALID_VALUE) continue;

        // browse results for the best usable items
        for (int k = 0; k < SortedNumber; k++) {
            wpindex = *(pindex + k);
            if (!ValidWayPoint(wpindex)) {
                // since we are not synced with DoNearest update cycle, we might fall here while it
                // has reset the sorted list. No worry, in the worst case we miss a waypoint printed
                // for a second, and the list might be inaccurate for the current second.
                // But in the next run it will be ok, because at the end of its run, the DoNearest
                // will be setting DataReady flag on and we shall update correctly.
                continue;
            }

#if DEBUG_GVG
            count++;
#endif

            // did we already use it?
            bool alreadyused = false;
            for (int j = 0; j < numslots; j++) {
                if (slotWpIndex[j] == INVALID_VALUE) break;
                if (slotWpIndex[j] == wpindex) {
                    alreadyused = true;
                    break;
                }
            }
            if (alreadyused) continue;

            // unused one, decide if good or not
            // We do this in 3 phases..
            double distance = WayPointCalc[wpindex].Distance;
            double brgdiff = WayPointCalc[wpindex].Bearing - DrawInfo.TrackBearing;
            if (brgdiff < -180.0) {
                brgdiff += 360.0;
            } else {
                if (brgdiff > 180.0) brgdiff -= 360.0;
            }
            double abrgdiff = brgdiff;
            if (abrgdiff < 0) abrgdiff *= -1;

#if 0
            // do we already have a wp with same bearing?
            for (int j = 0; j < numslots; j++) {
                if ((int) tmpSlotBrgDiff[j] == (int) brgdiff) {
                    //StartupStore(_T("%s with bdiff=%d already used\n"),WayPointList[wpindex].Name,(int)brgdiff);
                    alreadyused = true;
                    break;
                }
            }
            if (alreadyused) continue;
#endif

            // Careful: we are selecting abrgdiff from a list that is tuned to max 60, so ok.
            // DIRECTIONRANGE definition in DoNearest

            // Then we make a selective insertion, in several steps

            // First, we pick mountain passes and task points, generally priority #1 points
            // accepted from +-45 deg, but only if they are not absolutely unreachable
            if (phase == 1) {
                if (abrgdiff > 45) continue;
                // if we are practically over the waypoint, skip it
                if (distance < maxdistance) continue;

                // Consider only Mt.Passes and task points not below us...
                if ((WayPointList[wpindex].Style == STYLE_MTPASS) ||
                        ((WayPointList[wpindex].InTask) && (distance > 100))) {

                    // ... that are not within an obvious glide ratio
                    // (considering even strong headwind, we want a very positive arrival)
                    if (WayPointCalc[wpindex].AltArriv[AltArrivMode] > 150) {
                        if (WayPointCalc[wpindex].GR <= (maxgratio / 1.5)) continue;
                    }
                    if (WayPointCalc[wpindex].AltArriv[AltArrivMode]<-100) {
                        if (WayPointCalc[wpindex].GR >= mingratio) continue;
                    }
                    goto _takeit;
                }
                continue;
            }

            // Second we take anything not obviously reachable
            if (phase == 2) {
                if (abrgdiff > 45) continue;
                if (distance < maxdistance) continue;
                if (WayPointCalc[wpindex].AltArriv[AltArrivMode] > 150) {
                    if (WayPointCalc[wpindex].GR <= (maxgratio)) continue;
                }
                goto _takeit;
            }

            if (phase == 3) {
                if (abrgdiff > 45) continue;
                if (distance < maxdistance) continue;
                goto _takeit;
            }

            // else we accept anything, in the original sort order


_takeit:

            // ok good, use it
            slotWpIndex[i] = wpindex;
            tmpSlotBrgDiff[i] = brgdiff;
            found = true;

#if DEBUG_GVG
            StartupStore(_T("PHASE %d  slot [%d] of %d : wp=%d <%s> brg=%f\n"),
                    phase, i, numslots, wpindex, WayPointList[wpindex].Name, brgdiff);
#endif
            currentFilledNumber++;
            break;
        } // for all sorted wps

        if (!found) {
#if DEBUG_GVG
            StartupStore(_T("PHASE %d  nothing found during search (stop at slot %d), advance to next phase\n"), phase, i >= numslots ? numslots - 1 : i);
#endif
            break;
        }
        if (currentFilledNumber >= numslots) {
#if DEBUG_GVG
            StartupStore(_T("PHASE %d  stop search, all slots taken\n"), phase);
#endif
            break;
        }
    } // for all slots to be filled

    if ((currentFilledNumber < numslots) && (phase < MAXPHASES)) {
#if DEBUG_GVG
        StartupStore(_T("PHASE %d  filled %d of %d slots, going phase %d\n"), phase, currentFilledNumber, numslots, phase + 1);
#endif
        phase++;
        goto _tryagain;
    }
#if DEBUG_GVG
    else {
        StartupStore(_T("PHASE %d  filled %d of %d slots\n"), phase, currentFilledNumber, numslots);
    }
    StartupStore(_T("TOTAL COUNTS=%d\n"), count);
#endif


    //
    // All wpts in the array are shuffled, unsorted by direction.
    // We must reposition them horizontally, using their bearing
    //
    int tmpSlotWpIndex[MAXBSLOT + 1];
    for (i = 0; i < MAXBSLOT; i++) tmpSlotWpIndex[i] = INVALID_VALUE;

#if DEBUG_GVG
    for (i = 0; i < numslots; i++) {
        StartupStore(_T(">>> [%d] %f  (wp=%d)\n"), i, tmpSlotBrgDiff[i], slotWpIndex[i]);
    }
#endif

    bool invalid = true, valid = false;
    unsigned short g;

    for (unsigned short nslot = 0; nslot < numslots; nslot++) {
        g = 0;
        double minim = 999;
        valid = false;
#if DEBUGSORT
        StartupStore(_T(".... Slot [%d]:\n"), nslot);
#endif
        for (unsigned short k = 0; k < numslots; k++) {
            if (tmpSlotBrgDiff[k] <= minim) {
                // is this already used?
                invalid = false;
                if (slotWpIndex[k] == -1) {
#if DEBUGSORT
                    StartupStore(_T(".... not using g=%d, it is an invalid waypoint\n"), k);
#endif
                    continue;
                }
                for (unsigned short n = 0; n < nslot; n++) {
                    if (tmpSlotWpIndex[n] == slotWpIndex[k]) {
#if DEBUGSORT
                        StartupStore(_T(".... not using g=%d, it is already used in newslot=%d\n"), k, n);
#endif
                        invalid = true;
                        continue;
                    }
                }

                if (invalid || !ValidWayPoint(slotWpIndex[k])) continue;

                // We do have a valid choice
                g = k;
                minim = tmpSlotBrgDiff[k];
#if DEBUGSORT
                StartupStore(_T(".... minim=%f g=%d\n"), minim, g);
#endif
                valid = true;
            }
        }

        if (valid) {
            tmpSlotWpIndex[nslot] = slotWpIndex[g];
#if DEBUGSORT
            StartupStore(_T(".... FINAL for SLOT %d:  minim=%f g=%d\n"), nslot, minim, g);
#endif
        }
#if DEBUGSORT
        else {
            StartupStore(_T(".... FINAL for SLOT %d:  no valid point\n"), nslot);
        }
#endif
    }


    for (i = 0; i < numslots; i++) {
        slotWpIndex[i] = tmpSlotWpIndex[i];
    }

    return currentFilledNumber;
}
