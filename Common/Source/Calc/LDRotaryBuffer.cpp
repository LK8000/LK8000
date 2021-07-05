/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LDRotaryBuffer.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"

//#define DEBUG_ROTARY 1

bool InitLDRotary(ldrotary_s *buf) {
short i, bsize;
#ifdef DEBUG_ROTARY
char ventabuffer[200];
FILE *fp;
#endif
	#if TESTBENCH
	StartupStore(_T("... Init LDRotary @%s%s"),WhatTimeIsIt(),NEWLINE);
	#endif

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
	}
	buf->totaldistance=0;
    buf->totalaltitude=0;
	buf->totalias=0;
	buf->start=-2;
	buf->size=bsize;
	buf->valid=false;
#ifdef DEBUG_ROTARY
	sprintf(ventabuffer,"InitLdRotary size=%d\r\n",buf->size);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
                    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif

	Rotary_Speed=0;
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
#ifdef DEBUG_ROTARY
char ventabuffer[200];
FILE *fp;
#endif
	if (LKSW_ResetLDRotary) {
		#if TESTBENCH
		StartupStore(_T("... LD ROTARY SWITCH RESET @%s%s"),WhatTimeIsIt(),NEWLINE);
		#endif
		LKSW_ResetLDRotary=false;
		InitLDRotary(&rotaryLD);
	}

        // For IGC Replay, we are never OnGround, see Calc/TakeoffLanding.cpp
	if (Calculated->OnGround) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"OnGround, ignore LDrotary\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		return;
	}

	if (ISCAR) {
		goto _noautoreset;
	}

	if (Calculated->Circling) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"Circling, ignore LDrotary\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
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
#ifdef DEBUG_ROTARY
			sprintf(ventabuffer,"Rotary reset after exceeding errors\r\n");
			if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
				    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
			#if TESTBENCH
			StartupStore(_T("... LDROTARY RESET, distance errors%s"),NEWLINE);
			#endif
			InitLDRotary(&rotaryLD);
			errs=10; // an no more here until errs reset with valid data
			return;

		}
		if (errs<9) errs++;  // make it up to 9
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"(errs=%d) IGNORE INVALID distance=%d altitude=%d\r\n",errs,(int)(distance),(int)(Calculated->NavAltitude));
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
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
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"*** rotary reset and VALID=TRUE ++bufstart=%d >=bufsize=%d\r\n",buf->start, buf->size);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
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
	}
	buf->totaldistance+=iround(distance*100);
	buf->distance[buf->start]=iround(distance*100);

    buf->totalaltitude+=diffAlt;
	buf->altitude[buf->start]=diffAlt;

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
#ifdef DEBUG_ROTARY
	sprintf(ventabuffer,"insert buf[%d/%d], distance=%d totdist=%d\r\n",buf->start, buf->size-1, buf->distance[buf->start], buf->totaldistance);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
}

//#define DEBUG_EQMC 1
/*
 * returns 0 if invalid, 999 if too high
 * EqMc is negative when no value is available, because recalculated and buffer still not usable
 */
double CalculateLDRotary(ldrotary_s *buf, NMEA_INFO *Basic, DERIVED_INFO *Calculated ) {

	double eff;
#ifdef DEBUG_ROTARY
	short bcold;
	char ventabuffer[200];
	FILE *fp;
#endif
	double averias;
	double avertas;

	if ( Calculated->Circling || Calculated->OnGround || !Calculated->Flying ) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"Not Calculating, on ground or circling, or not flying\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		#if DEBUG_EQMC
		StartupStore(_T("... Circling, grounded or not flying, EqMc -1 (---)\n"));
		#endif
		Calculated->EqMc = -1;
		return(0);
	}

	if ( buf->start <0) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"Calculate: invalid buf start<0\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
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

		if (ISCAR) {
			Rotary_Speed=averias;
		}

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
		Calculated->EqMc, Calculated->EqMc, averias*TOKPH, avertas*TOKPH,-1*GlidePolar::SinkRateFast(0,avertas),
		GlidePolar::SinkRateBestLd(), GlidePolar::Vbestld()*TOKPH, GlidePolar::Vminsink()*TOKPH);
		#endif

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

#ifdef DEBUG_ROTARY
	if (bc.valid && bc.start < (bc.size-1))
		bcold=bc.start+1;
	else
		bcold=0;

	sprintf(ventabuffer,"bcstart=%d bcold=%d altnew=%d altold=%d altdiff=%d totaldistance=%d eff=%f\r\n",
		bc.start, bcold,
		bc.altitude[bc.start], bc.altitude[bcold], bc.totalaltitude, bc.totaldistance, eff);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
                    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif

	if (eff>MAXEFFICIENCYSHOW) eff=INVALID_GR;

	return(eff);

}

#if 0
// Example code used for rotary buffering
bool InitFilterBuffer(ifilter_s *buf, short bsize) {
short i;
	if (bsize <3 || bsize>RASIZE) return false;
	for (i=0; i<RASIZE; i++) buf->array[i]=0;
	buf->start=-1;
	buf->size=bsize;
	return true;
}

void InsertRotaryBuffer(ifilter_s *buf, int value) {
	if (++buf->start >=buf->size) {
		buf->start=0;
	}
	buf->array[buf->start]=value;
}

int FilterFast(ifilter_s *buf, int minvalue, int maxvalue) {

  ifilter_s bc;
  memcpy(&bc, buf, sizeof(ifilter_s));

  short i,nc,iter;
  int s, *val;
  float aver=0.0, oldaver, low=minvalue, high=maxvalue, cutoff;

  for (iter=0; iter<MAXITERFILTER; iter++) {
	 for (i=0, nc=0, s=0; i<bc.size; i++) {
		val=&bc.array[i];
		if (*val >=low && *val <=high) { s+=*val; nc++; }
	  }
	  if (nc==0) { aver=0.0; break; }
	  oldaver=aver; aver=((float)s/nc);
	  //printf("Sum=%d count=%d Aver=%0.3f (old=%0.3f)\n",s,nc,aver,oldaver);
	  if (oldaver==aver) break;
	  cutoff=aver/50; // 2%
	  low=aver-cutoff;
	  high=aver+cutoff;
  }
  //printf("Found: aver=%d (%0.3f) after %d iterations\n",(int)aver, aver, iter);
  return ((int)aver);

}

int FilterRotary(ifilter_s *buf, int minvalue, int maxvalue) {

  ifilter_s bc;
  memcpy(&bc, buf, sizeof(ifilter_s));

  short i,curs,nc,iter;
  int s, val;
  float aver, low, high, cutoff;

  low  = minvalue;
  high = maxvalue;

  for (iter=0; iter<MAXITERFILTER; iter++) {
	 for (i=0, nc=0, s=0,curs=bc.start; i<bc.size; i++) {

		val=bc.array[curs];
		if (val >=low && val <=high) {
			s+=val;
			nc++;
		}
		if (++curs >= bc.size ) curs=0;
	  }
	  if (nc==0) {
		aver=0.0;
		break;
	  }
	  aver=((float)s/nc);
	  //printf("Sum=%d count=%d Aver=%0.3f\n",s,nc,aver);

	  cutoff=aver/50; // 2%
	  low=aver-cutoff;
	  high=aver+cutoff;
  }

  //printf("final: aver=%d\n",(int)aver);
  return ((int)aver);

}
#endif
/*
main(int argc, char *argv[])
{
  short i;
  ifilter_s buf;
  InitFilterBuffer(&buf,20);
  int values[20] = { 140,121,134,119,116,118,121,122,120,124,119,117,116,130,122,119,110,118,120,121 };
  for (i=0; i<20; i++) InsertRotaryBuffer(&buf, values[i]);
  FilterFast(&buf, 70,200 );
  buf.start=10;
  for (i=0; i<20; i++) InsertRotaryBuffer(&buf, values[i]);
  FilterFast(&buf, 70,200 );
}
*/
