/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LDRotaryBuffer.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "utils/unique_file_ptr.h"

#define DEBUG_ROTARY

template<typename... Args>
static void Log(const char* fmt, Args&&... args) {
#ifdef DEBUG_ROTARY
	auto fp = make_unique_file_ptr("Rotary_Debug.TXT", "a");
	if (fp) {
		fprintf(fp.get(), fmt, std::forward<Args>(args)...);
	}
#endif
}


bool InitLDRotary(ldrotary_s *buf) {
	short i, bsize;
	TestLog(_T("... Init LDRotary @%s"), WhatTimeIsIt());

	switch (AverEffTime) {
		case ae3seconds:
			bsize=3;
			break;
		case ae5seconds:
			bsize=5;
			break;
		case ae10seconds:
			bsize=10;
			break;
		case ae15seconds:
			bsize=15;	// useless, LDinst already there
			break;
		case ae30seconds:
			bsize=30;	// limited useful
			break;
		case ae45seconds:
			bsize=45;	// limited useful
			break;
		case ae60seconds:
			bsize=60;	// starting to be valuable
			break;
		case ae90seconds:
			bsize=90;	// good interval
			break;
		case ae2minutes:
			bsize=120;	// other software's interval
			break;
		case ae3minutes:
			bsize=180;	// probably too long interval
			break;
		default:
			bsize=3; // make it evident
			break;
	}
	//if (bsize <3 || bsize>MAXLDROTARYSIZE) return false;
	for (i=0; i<MAXLDROTARYSIZE; i++) {
		buf->distance[i]=0;
		buf->altitude[i]=0;
		buf->ias[i]=0;
		buf->gs[i]=0;
	}
	buf->totaldistance=0;
    buf->totalaltitude=0;
	buf->totalias=0;
    buf->totalgs=0;
	buf->start=-2;
	buf->size=bsize;
	buf->valid=false;

	Log("InitLdRotary size=%d\r\n", buf->size);

	Rotary_Distance=0;

	return false;
}


//
// Called by LD  in Calc thread
//
void InsertLDRotary(ldrotary_s *buf, double distance, NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
	static unsigned short errs=0;
#ifdef TESTBENCH
	static bool zeroerrs=true;
#endif

	if (LKSW_ResetLDRotary) {
		TestLog(_T("... LD ROTARY SWITCH RESET @%s"), WhatTimeIsIt());
		LKSW_ResetLDRotary=false;
		InitLDRotary(&rotaryLD);
	}

        // For IGC Replay, we are never OnGround, see Calc/TakeoffLanding.cpp
	if (Calculated->OnGround) {
		Log("OnGround, ignore LDrotary\r\n");
		return;
	}

	if (ISCAR) {
		goto _noautoreset;
	}

	if (Calculated->Circling) {
		Log("Circling, ignore LDrotary\r\n");
		return;
	}

	if (distance<3 || distance>150) { // just ignore, no need to reset rotary
#ifdef TESTBENCH
		if (distance==0 && zeroerrs) {
			StartupStore(_T("... InsertLDRotary distance error=%f @%s%s"),distance,WhatTimeIsIt(),NEWLINE);
			zeroerrs=false;
		}
#endif
		if (errs==9) {
			Log("Rotary reset after exceeding errors\r\n");
			TestLog(_T("... LDROTARY RESET, distance errors"));
			InitLDRotary(&rotaryLD);
			errs=10; // an no more here until errs reset with valid data
			return;

		}
		if (errs<9) errs++;  // make it up to 9

		Log("(errs=%d) IGNORE INVALID distance=%d altitude=%d\r\n",errs,(int)(distance),(int)(Calculated->NavAltitude));
		return;
	}
	errs=0;
        #ifdef TESTBENCH
        zeroerrs=true;
        #endif

_noautoreset:

    if((buf->start) < -1) {
        BUGSTOP_LKASSERT(buf->start==-2);
        // this is the first run after reset,
        // save NavAltitude for calculate in AltDiff next run
        // and return;
        buf->start=-1;
        buf->prevaltitude = iround(Calculated->NavAltitude*100);
        return;
    }

    int diffAlt = buf->prevaltitude - iround(Calculated->NavAltitude*100);
    buf->prevaltitude = iround(Calculated->NavAltitude*100);

	if (++buf->start >=buf->size) {
		Log("*** rotary reset and VALID=TRUE ++bufstart=%d >=bufsize=%d\r\n", buf->start, buf->size);
		buf->start=0;
		buf->valid=true; // flag for a full usable buffer
	}
        LKASSERT(buf->start>=0 && buf->start<MAXLDROTARYSIZE);
        if (buf->start<0 ||buf->start>=MAXLDROTARYSIZE) buf->start=0; // UNMANAGED RECOVERY!
	// need to fill up buffer before starting to empty it
	if ( buf->valid == true) {
		buf->totaldistance-=buf->distance[buf->start];
		buf->totalaltitude-=buf->altitude[buf->start];

		buf->totalias-=buf->ias[buf->start];
		buf->totalgs-=buf->gs[buf->start];
	}
    buf->totaldistance+=iround(distance*100);
    buf->distance[buf->start]=iround(distance*100);

    buf->totalaltitude+=diffAlt;
    buf->altitude[buf->start]=diffAlt;

    buf->totalgs+=Basic->Speed;
    buf->gs[buf->start] = Basic->Speed;

    // insert IAS in the rotary buffer, either real or estimated
    if (Basic->AirspeedAvailable) {
        buf->totalias += (int)(Basic->IndicatedAirspeed*100);
                buf->ias[buf->start] = (int)(Basic->IndicatedAirspeed*100);
	} else {
		if (ISCAR) {
			buf->totalias += (int)(Basic->Speed*100);
			buf->ias[buf->start] = (int)(Basic->Speed*100);
		} else {
			buf->totalias += (int)(Calculated->IndicatedAirspeedEstimated*100);
			buf->ias[buf->start] = (int)(Calculated->IndicatedAirspeedEstimated*100);
		}
	}
	Log("insert buf[%d/%d], distance=%d totdist=%d\r\n",
			buf->start, buf->size-1, buf->distance[buf->start], buf->totaldistance);
}

//#define DEBUG_EQMC 1
/*
 * returns 0 if invalid, 999 if too high
 * EqMc is negative when no value is available, because recalculated and buffer still not usable
 */
double CalculateLDRotary(ldrotary_s *buf, NMEA_INFO *Basic, DERIVED_INFO *Calculated ) {

	double eff;
	double averias;
	double avertas;

	if ( Calculated->Circling || Calculated->OnGround || !Calculated->Flying ) {
		Log("Not Calculating, on ground or circling, or not flying\r\n");
		#if DEBUG_EQMC
		StartupStore(_T("... Circling, grounded or not flying, EqMc -1 (---)\n"));
		#endif
		Calculated->EqMc = -1;
		return(0);
	}

	if ( buf->start <0) {
		Log("Calculate: invalid buf start<0\r\n");
		#if DEBUG_EQMC
		StartupStore(_T("... Invalid buf start <0, EqMc -1 (---)\n"));
		#endif
		Calculated->EqMc = -1;
		return(0);
	}

	ldrotary_s bc;
	memcpy(&bc, buf, sizeof(ldrotary_s));

	if (bc.valid == false ) {
		if (bc.start==0) {
			#if DEBUG_EQMC
			StartupStore(_T("... bc.valid is false, bc.start is 0, EqMc -1 (---)\n"));
			#endif
			Calculated->EqMc = -1;
			return(0); // unavailable
		}
    }
	// if ( bc.valid == true ) {
	// bcsize<=0  should NOT happen, but we check it for safety
	if ( (bc.valid == true) && bc.size>0 ) {
		averias = bc.totalias/bc.size;
		averias/=100;

		// We use GPS altitude to be sure that the tas is correct, we dont know in fact
		// if qnh is correct, while gps is generally accurate for the purpose.
		avertas = TrueAirSpeed(averias, QNHAltitudeToQNEAltitude(Basic->Altitude));
		// This is just to be sure we are not using an impossible part of the polar
		if (avertas>(GlidePolar::Vminsink()-8.3) && (avertas>0)) { // minsink - 30km/h

            Calculated->EqMc = GlidePolar::EquMC(averias);

			// Do not consider impossible MC values as Equivalent
			if (Calculated->EqMc>20) Calculated->EqMc=-1;
		} else  {
			Calculated->EqMc=-1;
			#if DEBUG_EQMC
			StartupStore(_T(".......too slow for eqmc\n"));
			#endif
		}
		#if DEBUG_EQMC
		StartupStore(_T(".. eMC=%.2f (=%.1f)  Averias=%f Avertas=%f kmh, sinktas=%.1f ms  sinkmc0=%.1f ms Vbestld=%.1f Vminsink=%.1f\n"),
                        Calculated->EqMc, 
                        Calculated->EqMc, 
                        Units::To(unKiloMeterPerHour, averias), 
                        Units::To(unKiloMeterPerHour, avertas),
                        -1*GlidePolar::SinkRateFast(0,avertas),
                        GlidePolar::SinkRateBestLd(), 
                        Units::To(unKiloMeterPerHour, GlidePolar::Vbestld()), 
                        Units::To(unKiloMeterPerHour, GlidePolar::Vminsink()));
        #endif

        //Calculating AverageGS
         Calculated->AverageGS=bc.totalgs/bc.size;;
        //End calculating AverageGS


	} else {
		Calculated->EqMc=-1;
		#if DEBUG_EQMC
		StartupStore(_T(".... bc.valid=%d bc.size=%d <=0, no eqMc\n"), bc.valid,bc.start);
		#endif
	}

	Rotary_Distance=bc.totaldistance;
	if (bc.totalaltitude == 0 ) {
		return(INVALID_GR); // infinitum
	}
	eff= ((double)bc.totaldistance) / ((double)bc.totalaltitude);

	int bcold = ((bc.valid && bc.start < (bc.size-1)) ?  bc.start + 1 : 0);
	Log("bcstart=%d bcold=%d altnew=%d altold=%d altdiff=%d totaldistance=%d eff=%f\r\n",
			bc.start, bcold, bc.altitude[bc.start], bc.altitude[bcold], 
			bc.totalaltitude, bc.totaldistance, eff);

	if (eff>MAXEFFICIENCYSHOW) eff=INVALID_GR;

	return(eff);

}
