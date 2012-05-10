/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include <Windows.h>
#include "Units.h"
#include "Calculations.h"
#include "Parser.h"
#include "Defines.h"
#include "device.h"
#include "devCProbe.h"

#include "Enums.h"
#include "lk8000.h"
#include "Utils.h"

#define BARO__CPROBE		7
extern bool UpdateBaroSource( NMEA_INFO* GPS_INFO, const short parserid, const PDeviceDescriptor_t d, const double fAlt);


bool CDevCProbe::Register(){
	return devRegister(GetName(), cap_baro_alt|cap_vario, &Install);
}

BOOL CDevCProbe::Install( PDeviceDescriptor_t d ) {
	_tcscpy(d->Name, GetName());
	d->ParseNMEA = ParseNMEA;
	d->PutMacCready = NULL;
	d->PutBugs = NULL;
	d->PutBallast = NULL;
	d->Open = Open;
	d->Close = NULL;
	d->Init = NULL;
	d->LinkTimeout = NULL;
	d->Declare = NULL;
	d->IsGPSSource = GetFalse;
	d->IsBaroSource = GetTrue;

	return(TRUE);
}

BOOL CDevCProbe::Open( PDeviceDescriptor_t d, int Port ) {
	return TRUE;
}

double int16toDouble(int v) {
	if(v > (1<<15)){
		v = -((1<<16)-v-1);
	}
	return v;
};

double int24toDouble(int v) {
	if(v > (1<<23)){
		v = -(v - (1<<24)+1);
	}
	return v;
};

BOOL CDevCProbe::ParseNMEA( DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *pINFO ) {
	wnmeastring wiss(String);
	TCHAR* strToken = wiss.GetNextString();

 	if(_tcscmp(strToken,TEXT("$PCPROBE"))==0) {
 		strToken = wiss.GetNextString();
		if (_tcscmp(strToken,TEXT("T"))==0) {
			BOOL bOk = ParseData(wiss, pINFO);
			if(!pINFO->BaroAltitudeAvailable) {
				SetBaroOn(d);
			}
			return bOk;
		}
		if (_tcscmp(strToken,TEXT("COMPASSCALIBRATION"))==0) {
			// $PCPROBE,COMPASSCALIBRATION
			//  The calibration of the accelerometers and of the magnetometers is being performed
			return TRUE;
		}
		if (_tcscmp(strToken,TEXT("GYROCALIBRATION"))==0) {
			return ParseGyro(wiss, pINFO);
		}
		if (_tcscmp(strToken,TEXT("FW"))==0) {
			return ParseFW(wiss, pINFO);
		}
		if(_tcscmp(strToken,TEXT("NAME"))==0) {
			return ParseName(wiss, pINFO);
		}
 	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// $PCPROBE,T,Q0,Q1,Q2,Q3,ax,ay,az,temp,rh,batt,delta_press,abs_press,C,
// - "T" after "$PCPROBE" indicates that the string contains data. Data are represented as signed,
//  16-bit hexadecimal integers. The only exception is abs_press which is in signed 24-bits hex
//  format.
// - Q0, Q1, Q2, Q3: 3D orientation of the C-Probe in quaternion format. Heading, pitch, and roll can
//    be calculated as follows:
//      q0 = Q0 * 0.001;
//      q1 = Q1 * 0.001;
//      q2 = Q2 * 0.001;
//      q3 = Q3 * 0.001;
//      sin_pitch = -2 * (q0 * q2 - q3 * q1); // if sin_pitch > 1 or sin_pitch < -1, discard the data
//      pitch = asin(sin_pitch);
//      heading = M_PI + atan2(2*(q1 * q2 + q3 * q0), q3 * q3 - q0 * q0 - q1 * q1 + q2 * q2);
//      roll = atan2( 2 * (q0 * q1 + q3 * q2), q3 * q3 + q0 * q0 - q1 * q1 - q2 * q2);
// - ax, ay, az: x, y, z components of the acceleration in units of 0.001 g.
// - temp: temperature in units of 0.1°C.
// - rh: relative humidity in units of 0.1%.
// - batt: battery level from 0 to 100%.
// - delta_press: differential pressure (dynamic - static) in units of 0.1 Pa.
// - abs_press: absolute pressure in units of 1/400 Pa
// - C: is transmitted only if the C-Probe is being charged. In this case, heat produced by the charging
//    process is likely to affect the readings of the temperature and humidity sensors.
////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDevCProbe::ParseData( wnmeastring& wiss, NMEA_INFO *pINFO ) {

	double q0 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	double q1 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	double q2 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	double q3 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;

	double sin_pitch = -2 * (q0 * q2 - q3 * q1); // if sin_pitch > 1 or sin_pitch < -1, discard the data

	if(sin_pitch < 1.0 && sin_pitch > -1.0){
		pINFO->MagneticCompassAvailable=TRUE;
		pINFO->Heading = PI + atan2(2*(q1 * q2 + q3 * q0), q3 * q3 - q0 * q0 - q1 * q1 + q2 * q2)*RAD_TO_DEG;

		pINFO->GyroscopeAvailable=TRUE;
		pINFO->Pitch = asin(sin_pitch)*RAD_TO_DEG;
		pINFO->Roll = atan2( 2 * (q0 * q1 + q3 * q2), q3 * q3 + q0 * q0 - q1 * q1 - q2 * q2)*RAD_TO_DEG;
	}else{
		pINFO->MagneticCompassAvailable=FALSE;
		pINFO->GyroscopeAvailable=FALSE;
	}

	pINFO->AccelerationAvailable=TRUE;
	pINFO->AccelX = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	pINFO->AccelY = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	pINFO->AccelZ = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;

	pINFO->Gload = sqrt(pow(pINFO->AccelX,2.0)+pow(pINFO->AccelY,2.0)+pow(pINFO->AccelZ,2.0));

	pINFO->TemperatureAvailable=TRUE;
	pINFO->OutsideAirTemperature = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.1;

	pINFO->HumidityAvailable=TRUE;
	pINFO->RelativeHumidity = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.1;

	pINFO->ExtBatt1_Voltage = int16toDouble(HexStrToInt(wiss.GetNextString()))+1000;
	
	double delta_press = int16toDouble(HexStrToInt(wiss.GetNextString())) / 10.0 ;
	double abs_press = int24toDouble(HexStrToInt(wiss.GetNextString())) / 400.0;

	if(abs_press > 0.0) {
		UpdateBaroSource(pINFO, BARO__CPROBE, NULL, StaticPressureToAltitude(abs_press*100));
	}
	else {
		if(pINFO->BaroAltitudeAvailable) {
			abs_press = QNHAltitudeToStaticPressure(pINFO->BaroAltitude);
		}
		else {
			abs_press = QNHAltitudeToStaticPressure(pINFO->Altitude);
		}
	}

	if(delta_press>=0){
		pINFO->AirspeedAvailable = TRUE;
		pINFO->IndicatedAirspeed = sqrt(2*delta_press);
		pINFO->TrueAirspeed = TrueAirSpeed(delta_press,	pINFO->RelativeHumidity, pINFO->OutsideAirTemperature, abs_press>0.0?abs_press*100:101325.0);
	}

	if(*(wiss.GetNextString()) == L'C'){
		pINFO->ExtBatt1_Voltage *= -1;
	}

	TriggerVarioUpdate();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// $PCPROBE, GYROCALIBRATION,n,m
//  The calibration of the gyroscopes is being performed. "m" is the number of steps required. "n" is the
//  current step. The percentage of work performed is 100 * n / m.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDevCProbe::ParseGyro( wnmeastring& wiss, NMEA_INFO *pINFO ) {
	unsigned int n = HexStrToInt(wiss.GetNextString());
	unsigned int m = HexStrToInt(wiss.GetNextString());

	StartupStore(TEXT("Gyro Calibration : %03ud/%03ud"), n, m);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// $PCPROBE, FW,f
//  Firmware version. f = 0xNNMM, where NN is the major version number and MM the minor number.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDevCProbe::ParseFW( wnmeastring& wiss, NMEA_INFO *pINFO ) {
	unsigned int Version = HexStrToInt(wiss.GetNextString());

	unsigned int minor = (Version&0x00FF);
	unsigned int major = (Version&0xFF00) >> 8;

	StartupStore(TEXT("C-Probe Firmware Version : %02ud.%02ud"), major, minor);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// $PCPROBE, NAME,s
//  "s" is the name of the C-Probe. When searched for as a BlueTooth device, it is found as C-Probe-"s".
//  For example: $PCPROBE, NAME,Vinc means that the C-Probe will be detected as C-Probe-Vinc.
//  The name can be set by the user (see below).
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDevCProbe::ParseName( wnmeastring& wiss, NMEA_INFO *pINFO ) {
	wchar_t* szName = wiss.GetNextString();

	StartupStore(TEXT("C-Probe Name : %s"), szName);
	return TRUE;
}

BOOL CDevCProbe::SetBaroOn( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,BAROON\r\n"));
	return TRUE;
}

BOOL CDevCProbe::SetBaroOff( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,BAROOFF\r\n"));
	return TRUE;
}

BOOL CDevCProbe::GetDeviceName( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,GETNAME\r\n"));
	return TRUE;
}

BOOL CDevCProbe::SetDeviceName( PDeviceDescriptor_t d, const std::wstring& strName ){
	if (strName.size() <= 15) {
		d->Com->WriteString(TEXT("$PCPILOT,C,SET,"));
		d->Com->WriteString(strName.c_str());
		d->Com->WriteString(TEXT("\r\n"));
		return GetDeviceName(d);
	}
	return FALSE;
}

BOOL CDevCProbe::GetFirmwareVersion( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,GETFW\r\n"));
	return TRUE;
}

BOOL CDevCProbe::SetZeroDeltaPressure( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,CALZERO\r\n"));
	return TRUE;
}

BOOL CDevCProbe::SetCompassCalOn( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,COMPCALON\r\n"));
	return TRUE;
}

BOOL CDevCProbe::SetCompassCalOff( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,COMPCALOFF\r\n"));
	return TRUE;
}

BOOL CDevCProbe::SetCalGyro( PDeviceDescriptor_t d ){
	d->Com->WriteString(TEXT("$PCPILOT,C,CALGYRO\r\n"));
	return TRUE;
}
