/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "magfield.h"


void InitWindRotary(windrotary_s *buf) {
short i;

	for (i=0; i<WCALC_ROTARYSIZE; i++) {
		buf->speed[i]=0;
		buf->ias[i]=0;
		buf->track[i]=0;
		buf->compass[i]=0;
		buf->altitude[i]=0;
	}
	buf->start=-1;
	// only last x seconds of data are used
	buf->size=WCALC_MAXSIZE;
	return;
}

//#define DEBUG_ROTARY

void InsertWindRotary(windrotary_s *buf, double speed, double track, double altitude ) {
static short errs=0;

#ifdef DEBUG_ROTARY
char ventabuffer[200];
FILE *fp;
#endif
	if (CALCULATED_INFO.OnGround) {
		return;
	}

	if (CALCULATED_INFO.Circling) {
		return;
	}


	// speed is in m/s
	if (speed<1 || speed>100 || altitude <1 || altitude >10000) {
		if (errs>2) {
			#ifdef DEBUG_ROTARY
			sprintf(ventabuffer,"Wind Rotary reset after exceeding errors\n");
			if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
				    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
			#endif
			InitWindRotary(&rotaryWind);
			errs=0;
			return;

		}
		errs++;
		#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"(errs=%d) IGNORE INVALID speed=%f track=%f alt=%f\n",errs,speed, track, altitude);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		return;
	}
	errs=0;

	if (++buf->start >=buf->size) {
		#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"*** rotary reset ++bufstart=%d >=bufsize=%d\n",buf->start, buf->size);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		buf->start=0;
	}


	// We store GS in kmh, simply.
	buf->speed[buf->start] = (int)Units::To(unKiloMeterPerHour,speed);
	// we use track+180 in order to be always over 0 in range calculations
	// we want track north to be 0, not 360
	if (track==360) track=0;
	buf->track[buf->start]=(int)track+180;
	buf->altitude[buf->start]=(int)altitude;
	// ias is in m/s
	if (GPS_INFO.AirspeedAvailable && GPS_INFO.IndicatedAirspeed>0) {
		buf->ias[buf->start] = (int)GPS_INFO.IndicatedAirspeed;
	}

	#ifdef DEBUG_ROTARY
	sprintf(ventabuffer,"insert buf[%d/%d], track=%d alt=%d GS=%d",
		buf->start, buf->size-1,
		buf->track[buf->start]-180,
		buf->altitude[buf->start] ,
		buf->speed[buf->start]);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
}

// The Magic TrueWind returns: (see Utils2.h WCALC return codes) <=0 error values
//
// iaspeed is Indicated Air Speed in m/s. Target track is automatic. (TODO: use digital compass if available)
// IF IAS IS AVAILABLE, we use average IAS, in m/s and we ignore iaspeed
//

#if (WINDOWSPC>0)
//#define DEBUG_WCALC  // REMOVE BEFORE FLIGHT!
//#define STORE_WCALC  // REMOVE BEFORE FLIGHT!
#endif

// wmode  is 0 for NESW , 1 for 3,12,21,30    2 for 6 15 24 33
// For digital compass, wmode is heading degrees + 180 (that is, it starts from 180 degrees..)

#define WCALCTHRESHOLD 0.60
int CalculateWindRotary(windrotary_s *buf, double iaspeed , double *wfrom, double *wspeed, int windcalctime, int wmode) {

  #ifdef DEBUG_WCALC
  char ventabuffer[200];
  FILE *fp;
  #endif

  if (windcalctime<3) {
	StartupStore(_T("------ TrueWind: calctime=%d too low!%s"),windcalctime,NEWLINE);
	return WCALC_INVALID_DATA;
  }

  // make a copy of the working area, and work offline
  windrotary_s bc;
  memcpy(&bc, buf, sizeof(windrotary_s));

  short i,curs,nc,iter;
  int s, alt;
  int low, high;
  float averspeed, averias, avertrack, cutoff;
  double averaltitude=0;
  bool haveias=false;
  short gsquality=0, trackquality=0, iasquality=0;

  averspeed=0; averias=0; avertrack=0; cutoff=0;

  if (iaspeed<2||iaspeed>180) {
	StartupStore(_T("------ TrueWind: Invalid target speed=%f in Wind Calculation%s"),iaspeed,NEWLINE);
	return WCALC_INVALID_DATA;
  } else {
	// kmh range for GS
	low = 10;
	high = 300;
  }

  #ifdef DEBUG_WCALC
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {
	fprintf(fp,"-------------------------------------------\n");
	if (GPS_INFO.AirspeedAvailable) {
		fprintf(fp,"TAS/IAS are available, preconfig iaspeed=%.3f not used \n",iaspeed);
		fprintf(fp,"*** DUMP BUFFER, rotary size=%d (max buffer:%d) start=%d\n",bc.size, WCALC_ROTARYSIZE,bc.start);
		for (i=0; i< bc.size; i+=5) {
			sprintf(ventabuffer,"  [%02d] sp=%03d trk=%03d ias=%03d  ",i,bc.speed[i],bc.track[i]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d  ",i+1,bc.speed[i+1],bc.track[i+1]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d  ",i+2,bc.speed[i+2],bc.track[i+2]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d  ",i+3,bc.speed[i+3],bc.track[i+3]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d\n",i+4,bc.speed[i+4],bc.track[i+4]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
		}
		fclose(fp);

	} else {
		fprintf(fp,"Preconfigured iaspeed=%.3f \n",iaspeed);
		fprintf(fp,"*** DUMP BUFFER, rotary size=%d (max buffer:%d) start=%d\n",bc.size, WCALC_ROTARYSIZE,bc.start);
		for (i=0; i< bc.size; i+=5) {
			sprintf(ventabuffer,"  [%02d] sp=%03d trk=%03d  ",i,bc.speed[i],bc.track[i]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d  ",i+1,bc.speed[i+1],bc.track[i+1]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d  ",i+2,bc.speed[i+2],bc.track[i+2]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d  ",i+3,bc.speed[i+3],bc.track[i+3]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d\n",i+4,bc.speed[i+4],bc.track[i+4]-180);
			fprintf(fp,ventabuffer);
		}
		fclose(fp);

	}
  }
  #endif


  // search for a GS convergent value between these two margins.

  for (iter=0; iter<MAXITERFILTER; iter++) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(speed iter=%d) low=%d high=%d",iter,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	// skip last 2 seconds and go backwards to last 40
	if (bc.start<WCALC_TIMESKIP) curs=bc.size-1; else curs=bc.start-WCALC_TIMESKIP;

	//for (i=0, nc=0, s=0; i<WCALC_TIMEBACK; i++) {
	for (i=0, nc=0, s=0; i<windcalctime; i++) {
		if (bc.speed[curs] >=low && bc.speed[curs] <=high) {
			s+=bc.speed[curs];
			nc++;
		}
		if (--curs <0 ) curs=bc.size-1;
	}
	// do not accept a valid result if we don't have at least (TIMEBACK/2)+1 valid samples
	// do not accept a valid result if we don't have at least 75% +1 valid samples
	//if (nc<((WCALC_TIMEBACK/2)+1)) {
	if (nc<((windcalctime*WCALCTHRESHOLD)+1)) {
		#ifdef DEBUG_WCALC
		sprintf(ventabuffer,"(speed iter=%d) nc=%d <%d : no valid GS averspeed",iter,nc,(int)(windcalctime*WCALCTHRESHOLD)+1);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		averspeed=0.0;
		break;
	}
	// ground speed quality is an absolute value: the valid fixes accounted
	gsquality=nc;
	averspeed=((float)s/nc);
	cutoff=averspeed/(float)(10.0*(iter+1)); // era 50
	if (cutoff<1) cutoff=1;
	low=(int)(averspeed-cutoff);
	high=(int)(averspeed+cutoff);
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(speed iter=%d) nc=%d  averspeed=%.3f >> new: cutoff=%.3f >> low=%d high=%d",
		iter,nc,averspeed,cutoff,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif

  }

  #ifdef DEBUG_WCALC
  if (averspeed>0) {
	sprintf(ventabuffer,"GPS GS averspeed is %d kmh", (int)averspeed);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif

  // track search, and also altitude average
  // Track is stored + 180, so 359 is now 539. There is NO 360, it is 0 + 180

  low=145;
  high=540;
  for (iter=0; iter<MAXITERFILTER; iter++) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(track iter=%d) low=%d high=%d",iter,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	// skip last seconds and go backwards to last 40 or whatever
	if (bc.start<WCALC_TIMESKIP) curs=bc.size-1; else curs=bc.start-WCALC_TIMESKIP;

	for (i=0,nc=0,s=0,alt=0; i<windcalctime; i++) {

		// Track over 325 (505) should appear as x-180 (145)  100316 ?? it was a bug
		int track=bc.track[curs];
		if (track >=low && track <=high) {
			//s+=bc.track[curs];
			s+=track;
			alt+=bc.altitude[curs];
			nc++;
		}
		if (--curs <0 ) curs=bc.size-1;
	}
	// do not accept a valid result if we don't have at least (TIMEBACK/2)+1 valid samples
	// do not accept a valid result if we don't have at least 75%+1 valid samples
	if (nc<((windcalctime*WCALCTHRESHOLD)+1)) {
		#ifdef DEBUG_WCALC
		sprintf(ventabuffer,"(track iter=%d) nc=%d <%d : no valid avertrack",iter,nc,(int)(windcalctime*WCALCTHRESHOLD)+1);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		avertrack=-1; // negative avertrack means invalid
		break;
	}
	// track quality is number of valid fixes found
	trackquality=nc;
	LKASSERT(nc!=0);
	avertrack=((float)s/nc);
	averaltitude=alt/nc;

	cutoff=avertrack/(float)(10.0*(iter+1));
	if (cutoff<1) cutoff=1;
	low=(int)(avertrack-cutoff);
	high=(int)(avertrack+cutoff);

	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(track iter=%d) nc=%d  avertrack=%.3f >> new: cutoff=%.3f >> low=%d high=%d",
		iter,nc,avertrack,cutoff,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
  }

  #ifdef DEBUG_WCALC
  if (avertrack>-1) {
	sprintf(ventabuffer,"GPS TRACK avertrack is %d", (int)avertrack-180);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  } else {
	sprintf(ventabuffer,"GPS TRACK avertrack is -1 : NO average track found)");
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif

  // If Airspeed is available, use it  (ias is in m/s) but not if using Condor
  #if TESTIASWITHCONDOR
  if (!GPS_INFO.AirspeedAvailable ) goto goto_NoAirspeed;
  #else
  if (!GPS_INFO.AirspeedAvailable || DevIsCondor ) goto goto_NoAirspeed;
  #endif
  low=2;
  high=60;
  for (iter=0; iter<MAXITERFILTER; iter++) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(IAS iter=%d) low=%d high=%d",iter,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	// skip last 2 seconds and go backwards to last 40
	if (bc.start<WCALC_TIMESKIP) curs=bc.size-1; else curs=bc.start-WCALC_TIMESKIP;

	for (i=0, nc=0, s=0; i<windcalctime; i++) {
		if (bc.ias[curs] >=low && bc.ias[curs] <=high) {
			s+=bc.ias[curs];
			nc++;
		}
		if (--curs <0 ) curs=bc.size-1;
	}
	// do not accept a valid result if we don't have at least 75% +1 valid samples
	if (nc<((windcalctime*WCALCTHRESHOLD)+1)) {
		#ifdef DEBUG_WCALC
		sprintf(ventabuffer,"(IAS iter=%d) nc=%d <%d : no valid average IAS",iter,nc,(int)(windcalctime*WCALCTHRESHOLD)+1);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		averias=0.0;
		if (nc>0) return WCALC_INVALID_IAS;
		else return WCALC_INVALID_NOIAS;
		// otherwise, try to use eIAS
	}
	// iasquality is number of valid fixes found
	iasquality=nc;
	haveias=true;
	averias=((float)s/nc);
	cutoff=averias/(float)(10.0*(iter+1));
	if (cutoff<1) cutoff=1;
	low=(int)(averias-cutoff);
	high=(int)(averias+cutoff);
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(IAS iter=%d) nc=%d  averias=%.3f >> new: cutoff=%.3f >> low=%d high=%d",
		iter,nc,averias,cutoff,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif

  }

  // Assuming 10 seconds, GS is calculated on ground moving in this time. We want in 10 seconds at least 50m made
  // averias is >0 , but must be >18kmh, >5ms ?
  // NO, by now we keep 2ms minimum, for paragliders mainly
  #ifdef DEBUG_WCALC
  sprintf(ventabuffer,"REAL IAS averias is %d", (int)averias);
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  #endif
  if (averias<=2) {
	#ifdef STORE_WCALC
	StartupStore(_T("... TrueWind: REAL average IAS =%d TOO LOW%s"),(int)averias, NEWLINE);
	#endif
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"REAL IAS averias is %d TOO LOW", (int)averias);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_NOIAS;
  }

  goto_NoAirspeed:
  // track is inserted with +180
  // a negative heading to 325 is 145, so 145-180 would be -35 . 360-35=325 correct
  if (avertrack>-1) { // if valid avertrack found
	avertrack-=180;
	if (avertrack<0) avertrack=360-avertrack;
  }
  #ifdef DEBUG_WCALC
  if (averspeed>0 && avertrack>=0) {
	#if TESTIASWITHCONDOR
	if (GPS_INFO.AirspeedAvailable ) {
	#else
	if (GPS_INFO.AirspeedAvailable && !DevIsCondor ) {
	#endif
		if (averias>0)
			sprintf(ventabuffer,"*** Averages found (AIRSPEED AVAILABLE): averIAS=%d speed=%d track=%d altitude=%d\n",
				(int)(averias*TOKPH), (int)averspeed, (int)avertrack, (int)averaltitude);
		else
			sprintf(ventabuffer,"*** Averages found (AIRSPEED AVAILABLE): INVALID averIAS=%dm/s speed=%d track=%d altitude=%d\n",
				(int)averias, (int)averspeed, (int)avertrack, (int)averaltitude);
	} else {
		sprintf(ventabuffer,"*** Averages found: speed=%d track=%d altitude=%d\n",
			(int)averspeed, (int)avertrack, (int)averaltitude);
	}

	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif
  #ifdef STORE_WCALC
  #if TESTIASWITHCONDOR
  if (GPS_INFO.AirspeedAvailable && averias>0 )
  #else
  if (GPS_INFO.AirspeedAvailable && averias>0 && !DevIsCondor )
  #endif
	StartupStore(_T("... TrueWind: REAL average IAS =%d Average GS=%d Track=%d Altitude=%d%s"),
		(int)(averias*TOKPH),(int)averspeed,(int)avertrack,(int)averaltitude, NEWLINE);
  else
	StartupStore(_T("... TrueWind: IAS =%d Average GS=%d Track=%d Altitude=%d%s"),
		(int)(iaspeed*TOKPH),(int)averspeed,(int)avertrack,(int)averaltitude, NEWLINE);
  #endif

  if (averspeed<=0 && avertrack<0){
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS averspeed<=0 && avertrack<0 INVALID ALL \n");
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_ALL;
  }
  if (averspeed<=0) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS averspeed<=0 INVALID SPEED \n");
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_SPEED;
  }
  if (avertrack<0) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS avertrack<0 INVALID TRACK \n");
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_TRACK;
  }


  // p_heading is presumed heading , NSEW .. in degrees (understood from avertrack)
  // iaspeed is presumed IAS , as configured . in km/h (passed as parameter)

  // TODO If we have a digital compass, use wmode for it
#if 100215
  double p_heading=999;
  switch(wmode) {
	case 0:
		// wmode==0:  0 90  180  270   (N E S W)
		if (avertrack >=325.0 || avertrack<=35.0 ) p_heading=0;
		if (avertrack >=55.0 && avertrack<=125.0 ) p_heading=90;
		if (avertrack >=145.0 && avertrack<=215.0 ) p_heading=180;
		if (avertrack >=235.0 && avertrack<=305.0 ) p_heading=270;
		break;
	case 1:
		// wmode==1:  30 120 210 300
		if (avertrack >=355.0 || avertrack<=65.0 ) p_heading=30;
		if (avertrack >=85.0 && avertrack<=155.0 ) p_heading=120;
		if (avertrack >=175.0 && avertrack<=245.0 ) p_heading=210;
		if (avertrack >=265.0 && avertrack<=335.0 ) p_heading=300;
		break;
	case 2:
		// wmode==2:  60 150 240 330
		if (avertrack >=25.0 && avertrack<=95.0 ) p_heading=60;
		if (avertrack >=115.0 && avertrack<=185.0 ) p_heading=150;
		if (avertrack >=205.0 && avertrack<=275.0 ) p_heading=240;
		if (avertrack >=295.0 || avertrack<=5.0 ) p_heading=330;
		break;
	default:
		LKASSERT(0);
		StartupStore(_T("... INVALID WMODE WINDCALC: %d%s"),wmode,NEWLINE);
		return WCALC_INVALID_DATA;
		break;
   }


#else
  double p_heading=999;
  if (avertrack >=325.0 || avertrack<=35.0 ) p_heading=0;
  if (avertrack >=55.0 && avertrack<=125.0 ) p_heading=90;
  if (avertrack >=145.0 && avertrack<=215.0 ) p_heading=180;
  if (avertrack >=235.0 && avertrack<=305.0 ) p_heading=270;
#endif

  if (p_heading==999) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS avertrack=%d INVALID HEADING \n",(int)avertrack);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_HEADING;
  }
  #ifdef DEBUG_WCALC
  else {
	sprintf(ventabuffer,"*** avertrack=%d presumed HEADING is %d \n",(int)avertrack,(int)p_heading);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif
  #ifdef STORE_WCALC
  StartupStore(_T("... TrueWind: average track=%d , presumed heading to %d%s"),(int)avertrack,(int)p_heading, NEWLINE);
  #endif

  // Condor has its own wind precalculated: TrueWind will not cheat using read IAS
  #if TESTIASWITHCONDOR
  if (GPS_INFO.AirspeedAvailable && averias>0 ) iaspeed=averias;
  #else
  if (GPS_INFO.AirspeedAvailable && averias>0 && !DevIsCondor ) iaspeed=averias;
  #endif

  double tas = Units::To(unKiloMeterPerHour, iaspeed * AirDensityRatio(QNHAltitudeToQNEAltitude(averaltitude)));

  double magvar=CalculateMagneticVariation();

  // use true heading instead of magnetic heading
  double true_heading=p_heading-magvar; // magvar positive for E, negative for W
  if (true_heading >360) true_heading-=360;
  if (true_heading == 360) true_heading=0;
  if (true_heading <0 ) true_heading+=360;

  double tasns=tas*cos( true_heading / RAD_TO_DEG );
  double tasew=tas*sin( true_heading / RAD_TO_DEG );

  double gsns=averspeed * cos(avertrack/ RAD_TO_DEG);
  double gsew=averspeed * sin(avertrack/ RAD_TO_DEG);

  double windns=gsns-tasns;
  double windew=gsew-tasew;

  double windsp=sqrt(pow(windns,2)+pow(windew,2));

  double windtmp=0;

  if (windns==0) windtmp=M_PI/2; else windtmp=atan(windew/windns);

  double q1=windtmp*RAD_TO_DEG;
  double q2=180+q1;
  double q3=q2;
  double q4=360+q1;
  double windto=0;
  if (windns>=0) {
	if ( windew>=0 )
		windto=q1;
	else
		windto=q4;
  }
  if (windns<0) {
	if ( windew>=0 )
		windto=q2;
	else
		windto=q3;
  }

  double windfrom=(windto-180)>0?windto-180:windto+180;
  if (windfrom==360) windfrom=0;

  #ifdef DEBUG_WCALC
  sprintf(ventabuffer,"ias=%.3f tas=%.3f tasns=%.3f tasew=%.3f gsns=%.3f gsew=%.3f\n windns=%.3f\
	windew=%.3f windsp=%.3f tmp=%.3f q1=%.3f q2=%.3f q3=%.3f q4=%.3f windto=%.3f windfrom=%.3f\n",
	iaspeed*TOKPH,tas, tasns, tasew, gsns, gsew, windns, windew, windsp, windtmp, q1, q2, q3,q4,windto, windfrom);
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  #endif

  *wfrom=windfrom;
  *wspeed=windsp;

  #ifdef DEBUG_WCALC
  sprintf(ventabuffer,"*** Wind from %.0f deg/ %.0f kmh\n", windfrom, windsp);
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  #endif
  #ifdef STORE_WCALC
  StartupStore(_T("... TrueWind: calculated wind from %.0f deg, %.0f km/h%s"), windfrom, windsp,NEWLINE);
  #endif

  // StartupStore(_T(".......gsq=%d tq=%d wcalc=%d  qual=%d\n"),gsquality,trackquality, windcalctime,
  // 						((gsquality+trackquality)*100) / (windcalctime*2)  );
  // return quality percentage: part/total *100
  if (haveias)
	return ( ((gsquality+trackquality+iasquality)*100) / (windcalctime*3) );
  else
	return ( ((gsquality+trackquality)*100) / (windcalctime*2) );


}
