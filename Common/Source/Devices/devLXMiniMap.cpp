/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

//
// LK V5 WARNING - the function devWriteNMEA  is used only by LXMiniMap and it is 
// creating a possible deadlock situation
//
//_____________________________________________________________________includes_

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devLXMiniMap.h"
#include "McReady.h"
#include "InputEvents.h"
#include "Comm/UpdateQNH.h"
#include "devLXNano.h"

//____________________________________________________________class_definitions_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Registers device into device subsystem.
///
/// @retval true  when device has been registered successfully
/// @retval false device cannot be registered
///
//static
PeriodClock		  TICKER;
PeriodClock		  TICKER_PFLX4;
PeriodClock		  AlttimeOutTicker;
bool AltTimeoutWait = false;


short McReadyTimeout = 0;
short BugsTimeout = 0;
short BallastTimeout = 0;

double AltOffset = 0;
double FT2M = 0.3048;
double M2FT = 1.0/FT2M;

double LXNettoVario= 0;
double	 LXDistanceVario= 0;
double		 LXFinalGlide= 0;
double LXLegSpeed= 0;
bool FirstCheckBaroAlt = true;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
void DevLXMiniMap::Install(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = LXMiniMapPutMacCready;
  d->PutBugs      = LXMiniMapPutBugs;
  d->PutBallast   = LXMiniMapPutBallast;
  d->Open         = Open;
  d->Declare      = DeclareTaskMinimap;

  d->PutQNH = LXMiniMapPutQNH;
} // Install()

BOOL DevLXMiniMap::LXMiniMapPutBallast(PDeviceDescriptor_t	d, double	Ballast)
{

		BallastTimeout = 2;
		TCHAR mcbuf[100];

		double newBallastFactor = CalculateBalastFactor(Ballast) ;

		_stprintf(mcbuf, TEXT("PFLX2,,%.2f,,,,"), newBallastFactor);
		devWriteNMEAString(d, mcbuf);

		return (TRUE);
}
BOOL DevLXMiniMap::LXMiniMapPutBugs(PDeviceDescriptor_t	d, double	Bugs)
{
		BugsTimeout = 2;
		TCHAR mcbuf[100];

		int TransformedBugsValue = 100 - (int)(Bugs*100.0);

		_stprintf(mcbuf, TEXT("PFLX2,,,%d,,,"), TransformedBugsValue);
		devWriteNMEAString(d, mcbuf);

		return (TRUE);
}
BOOL DevLXMiniMap::LXMiniMapPutQNH(DeviceDescriptor_t *d, double NewQNH){



	double xy = QNHAltitudeToQNEAltitude(1000.0);
	AltOffset = 1000.0 - xy;
	AltTimeoutWait = true;
	AlttimeOutTicker.Update();



	TCHAR mcbuf[100];
	_stprintf(mcbuf, TEXT("PFLX3,%.2f,,,,,,,,,,,,"),AltOffset * M2FT );
	devWriteNMEAString(d, mcbuf);

  return(TRUE);
}

BOOL DevLXMiniMap::LXMiniMapPutMacCready(PDeviceDescriptor_t d, double MacCready) {

	McReadyTimeout = 2;
	TCHAR mcbuf[100];
	_stprintf(mcbuf, TEXT("PFLX2,%.2f,,,,,"), MacCready);
	devWriteNMEAString(d, mcbuf);

	return (TRUE);

}

BOOL DevLXMiniMap::SendPFLX4(DeviceDescriptor_t *d)
{
	/*PFLX4 Sc, Netto, Relativ, gl.dif, leg speed, leg time, integrator, flight time, battery voltage *CS<CR><LF>
	· Sc float (m/s)
	· Netto float (m/s)
	· Relativ float (m/s)
	· Distance float (m)
	· gl.dif int (ft)
	· leg speed (km/h)
	· leg time (km/h)
	· integrator float (m/s)
	· flight time unsigned in seconds
	· battery voltage float (V)*/




			double finalGlide = 0;
		//	double distance = 0;
			if ( ValidTaskPoint(ActiveTaskPoint) != false )
			{
				int index = Task[ActiveTaskPoint].Index;
				if (index>=0)
				{
						// don't use current MC...
						double value=WayPointCalc[index].AltArriv[AltArrivMode];
						if ( value > -3000 )
						{
							finalGlide =value;

						}
				//		distance = DerivedDrawInfo.WaypointDistance*DISTANCEMODIF;
				}
			}



			TCHAR mcbuf[100];

		    _stprintf(mcbuf, TEXT("PFLX4,,,,%.2f,%d,,,,,"),
		//	 CALCULATED_INFO.NettoVario,
			 CALCULATED_INFO.WaypointDistance,
			 (int)(finalGlide  * M2FT)
		//	 CALCULATED_INFO.LegSpeed
			);
			devWriteNMEAString(d, mcbuf);

	 return(TRUE);

}


BOOL DevLXMiniMap::LXMiniMapOnSysTicker(DeviceDescriptor_t *d) {
    if (TICKER_PFLX4.CheckUpdate(2*1000)) {
        SendPFLX4(d);

        if (TICKER.CheckUpdate(20*1000)) {
            TCHAR mcbuf[100];
            _stprintf(mcbuf, TEXT("PFLX0,LXWP0,1,LXWP2,3,LXWP3,%d"), 4);
            devWriteNMEAString(d, mcbuf);
        }
    }

    return (TRUE);
}

BOOL DevLXMiniMap::Open(PDeviceDescriptor_t d){

  devWriteNMEAString(d, TEXT("PFLX0,LXWP0,1,LXWP2,3,LXWP3,3"));

  return(TRUE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWPn sentences.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevLXMiniMap::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{

  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }

  if (_tcsncmp(_T("$GPGGA"), sentence, 6) == 0)
	   LXMiniMapOnSysTicker(d);
  else if (_tcsncmp(_T("$LXWP0"), sentence, 6) == 0)
      return LXWP0(d, sentence + 7, info);
  else if (_tcsncmp(_T("$LXWP1"), sentence, 6) == 0)
      return LXWP1(d, sentence + 7, info);
  else if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0)
      return LXWP2(d, sentence + 7, info);
  else if (_tcsncmp(_T("$LXWP3"), sentence, 6) == 0)
      return LXWP3(d, sentence + 7, info);

  return(false);
} // ParseNMEA()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP0 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevLXMiniMap::LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
  // $LXWP0,logger_stored, airspeed, airaltitude,
  //   v1[0],v1[1],v1[2],v1[3],v1[4],v1[5], hdg, windspeed*CS<CR><LF>
  //
  // 0 loger_stored : [Y|N] (not used in LX1600)
  // 1 IAS [km/h] ----> Condor uses TAS!
  // 2 baroaltitude [m]
  // 3-8 vario values [m/s] (last 6 measurements in last second)
  // 9 heading of plane (not used in LX1600)
  // 10 windcourse [deg] (not used in LX1600)
  // 11 windspeed [km/h] (not used in LX1600)
  //
  // e.g.:
  // $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

  TICKER.Update();

  double alt=0, airspeed=0;

  if (ParToDouble(sentence, 1, &airspeed))
  {
    airspeed /= TOKPH;
    info->TrueAirspeed = airspeed;
    info->AirspeedAvailable = TRUE;
  }

  if (ParToDouble(sentence, 2, &alt))
  {
    if (airspeed>0) {
      info->IndicatedAirspeed = IndicatedAirSpeed(airspeed, alt);
    }

    if (static_cast<unsigned>(d->PortNumber) == info->BaroSourceIdx.device_index) {
      UpdateQNH(CalculateQNH(alt, alt + AltOffset));
      UpdateBaroSource(info, d, alt + AltOffset);
    }
  }

  double Vario = 0;
  if (ParToDouble(sentence, 3, &Vario)) {
    UpdateVarioSource(*info, *d, Vario);
  }

  if (ParToDouble(sentence, 10, &info->ExternalWindDirection) &&
      ParToDouble(sentence, 11, &info->ExternalWindSpeed))
    info->ExternalWindAvailable = TRUE;

  return(true);
} // LXWP0()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP1 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevLXMiniMap::LXWP1(PDeviceDescriptor_t, const TCHAR*, NMEA_INFO*)
{
  // $LXWP1,serial number,instrument ID, software version, hardware
  //   version,license string,NU*SC<CR><LF>
  //
  // instrument ID ID of LX1600
  // serial number unsigned serial number
  // software version float sw version
  // hardware version float hw version
  // license string (option to store a license of PDA SW into LX1600)

  // nothing to do
  return(true);
} // LXWP1()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP2 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
double DevLXMiniMap::CalculateBalastFactor(double Ballast)
{
	double CurrentWeight = WEIGHTS[0] +WEIGHTS[1] + (WEIGHTS[2]*Ballast) +  GlidePolar::WeightOffset;
	double WithoutBallastWeight =  WEIGHTS[0] +WEIGHTS[1] +  GlidePolar::WeightOffset;

	if(WithoutBallastWeight == 0)
		WithoutBallastWeight = 1;

	return   CurrentWeight/WithoutBallastWeight;


}
double DevLXMiniMap::CalculateBalast(double Factor)
{
	double TotalAvailableBallast  = WEIGHTS[2];
	if(TotalAvailableBallast == 0)
		TotalAvailableBallast = 1;

	return ((Factor-1) * (WEIGHTS[0] +WEIGHTS[1] +  GlidePolar::WeightOffset))/ TotalAvailableBallast;
}



bool DevLXMiniMap::LXWP2(PDeviceDescriptor_t, const TCHAR* sentence, NMEA_INFO* info)
{
  // $LXWP2,mccready,ballast,bugs,polar_a,polar_b,polar_c, audio volume
  //   *CS<CR><LF>
  //
  // Mccready: float in m/s
  // Ballast: float 1.0 ... 1.5
  // Bugs: 0 - 100%
  // polar_a: float polar_a=a/10000 w=a*v2+b*v+c
  // polar_b: float polar_b=b/100 v=(km/h/100) w=(m/s)
  // polar_c: float polar_c=c
  // audio volume 0 - 100%

	if(McReadyTimeout>0)
	{
		McReadyTimeout--;
	}
	else
	{
		ParToDouble(sentence, 0, &info->MacReady);
		CheckSetMACCREADY(info->MacReady);
	}

	if(BallastTimeout>0)
	{
		BallastTimeout--;
	}
	else
	{
		double tempBallastFactor;

		ParToDouble(sentence, 1, &tempBallastFactor);

		double newBallast = CalculateBalast(tempBallastFactor);

		if(fabs(newBallast- BALLAST) > 0.01 )
		{
			CheckSetBallast(newBallast);
		}

	}


	if(BugsTimeout>0)
	{
		BugsTimeout--;
	}
	else
	{

		double tempBugs;
		ParToDouble(sentence, 2, &tempBugs);

		tempBugs = (100.0 - tempBugs)/100;

		if(fabs(tempBugs -BUGS) > 0.01)
		{
			CheckSetBugs(tempBugs);
		}

	}



  return(true);
} // LXWP2()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP3 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevLXMiniMap::LXWP3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{


	if(AltTimeoutWait)
	{
        if (AlttimeOutTicker.Check(4*1000)) {
			AltTimeoutWait = false;
		}
	}
	else
	{
		double offsettmp = 0.0;
		ParToDouble(sentence, 0, &offsettmp);
		AltOffset = offsettmp * FT2M;
	}

  // $LXWP3,altioffset, scmode, variofil, tefilter, televel, varioavg,
  //   variorange, sctab, sclow, scspeed, SmartDiff,
  //   GliderName, time offset*CS<CR><LF>
  //
  // altioffset //offset necessary to set QNE in ft default=0
  // scmode // methods for automatic SC switch index 0=EXTERNAL, 1=ON CIRCLING
  //   2=auto IAS default=1
  // variofil // filtering of vario in seconds (float) default=1
  // tefilter // filtering of TE compensation in seconds (float) 0 = no
  //   filtering (default=0)
  // televel // level of TE compensation from 0 to 250 default=0 (%) default=0
  // varioavg // averaging time in seconds for integrator default=25
  // variorange // 2.5 5 or 10 (m/s or kts) (float) default=5.0
  // sctab // area of silence in SC mode (float) 0-5.0 1.0= silence between
  //   +1m/s and -1m/s default=1
  // sclow // external switch/taster function 0=NORMAL 1=INVERTED 2=TASTER
  //   default=1
  // scspeed // speed of automatic switch from vario to sc mode if SCMODE==2 in
  //   (km/h) default=110
  // SmartDiff float (m/s/s) (Smart VARIO filtering)
  // GliderName // Glider name string max. 14 characters
  // time offset int in hours

  // nothing to do
  return(true);
} // LXWP3()

double CalculateQNH(double alt_qne, double alt_qnh)
{

	  const double k1=0.190263;
	  const double k2=8.417286e-5;

	  // step 1, find static pressure from device assuming it's QNH adjusted
	  double psraw = QNEAltitudeToStaticPressure(alt_qne);
	  // step 2, calculate QNH so that reported alt will be known alt
	  return pow(pow(psraw/100.0,k1) + k2*alt_qnh,1/k1);
}


////////////////////////////////   TASK    /////////////////////////////////////
BOOL DevLXMiniMap::DeclareTaskMinimap(PDeviceDescriptor_t d,
  const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[])
{
  Decl  decl;
  Class lxClass;



  if (!FillFlight(*lkDecl, decl, errBufSize, errBuf))
    return(false);

  if (!FillTask(*lkDecl, decl, errBufSize, errBuf))
    return(false);

  // we will use text-defined class
  decl.flight.cmp_cls = Decl::cls_textdef;
  lxClass.SetName(lkDecl->CompetitionClass);

  // stop RX thread
  {
    ScopeUnlock unlock(CritSec_Comm); // required to avoid deadlock In StopRxThread
    if (!StopRxThread(d, errBufSize, errBuf))
      return(false);
  }

  // set new Rx timeout
  int  orgRxTimeout;
  bool status = SetRxTimeout(d, 2000, orgRxTimeout, errBufSize, errBuf);

  if (status)
  {
	  devWriteNMEAString(d,TEXT("PFLX0,COLIBRI") );

	 Poco::Thread::sleep(100);


	 d->Com->SetBaudrate(4800);
	 d->Com->SetRxTimeout(2000);

    ShowProgress(decl_enable);
    status = StartCMDMode(d, errBufSize, errBuf);

    if(status == false)
    {
   	  d->Com->SetBaudrate(9600);
   	  d->Com->SetRxTimeout(2000);
      status = StartCMDMode(d, errBufSize, errBuf);
    }
    if(status == false)
    {
   	  d->Com->SetBaudrate(19200);
   	  d->Com->SetRxTimeout(2000);
      status = StartCMDMode(d, errBufSize, errBuf);
    }
    if(status == false)
    {
   	  d->Com->SetBaudrate(38400);
   	  d->Com->SetRxTimeout(2000);
      status = StartCMDMode(d, errBufSize, errBuf);
    }



    if (status)
    {
      ShowProgress(decl_send);
      status = status && WriteDecl(d, decl, errBufSize, errBuf);
      status = status && WriteClass(d, lxClass, errBufSize, errBuf);
    }

    ShowProgress(decl_disable);
    // always do the following step otherwise NMEA will not be sent
    // (don't overwrite error descr)
    status = StartNMEAMode(d, status ? errBufSize : 0, errBuf) && status;


	 d->Com->SetBaudrate(4800);
	 devWriteNMEAString(d,TEXT("PFLX0,LX160") );

	 d->Com->SetBaudrate(38400);


    // restore Rx timeout (we must try that always; don't overwrite error descr)
    status = SetRxTimeout(d, orgRxTimeout,
      orgRxTimeout, status ? errBufSize : 0, errBuf) && status;
  }

  // restart RX thread (we must try that always; don't overwrite error descr)
  status = StartRxThread(d, status ? errBufSize : 0, errBuf) && status;

  return(status);
}
