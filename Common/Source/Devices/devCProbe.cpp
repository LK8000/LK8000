/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "devCProbe.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "Baro.h"
#include "Calc/Vario.h"


PDeviceDescriptor_t CDevCProbe::m_pDevice=NULL;
BOOL CDevCProbe::m_bCompassCalOn=FALSE;
Mutex* CDevCProbe::m_pCritSec_DeviceData=NULL;
double CDevCProbe::m_abs_press=0.0;
double CDevCProbe::m_delta_press=0.0;

TCHAR CDevCProbe::m_szVersion[15]={0};


void CDevCProbe::Install( PDeviceDescriptor_t d ) {
	_tcscpy(d->Name, GetName());
	d->ParseNMEA = ParseNMEA;
	d->Open = Open;
	d->Close = Close;

	d->Config = Config;
}

BOOL CDevCProbe::Open( PDeviceDescriptor_t d) {
	m_pDevice = d;

	m_pCritSec_DeviceData = new Mutex();

	return TRUE;
}

BOOL CDevCProbe::Close (PDeviceDescriptor_t d) {
	m_pDevice = NULL;

	delete m_pCritSec_DeviceData;
	m_pCritSec_DeviceData = NULL;

	return TRUE;
}


inline double int16toDouble(int v) {
	return (double)(int16_t)v;
};

inline double int24toDouble(int v) {
	if(v > (1<<23)){
		v = -(v - (1<<24)+1);
	}
	return v;
};

void CDevCProbe::LockDeviceData(){
	if(m_pCritSec_DeviceData) {
		m_pCritSec_DeviceData->lock();
	}
}

void CDevCProbe::UnlockDeviceData(){
	if(m_pCritSec_DeviceData) {
		m_pCritSec_DeviceData->unlock();
	}
}

BOOL CDevCProbe::ParseNMEA( DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *pINFO ) {
	tnmeastring wiss(String);
	TCHAR* strToken = wiss.GetNextString();

 	if(_tcscmp(strToken,TEXT("$PCPROBE"))==0) {

  		strToken = wiss.GetNextString();

  		// this sentence must handled first, also we can't detect end of Compass Calibration.
		if (_tcscmp(strToken,TEXT("COMPASSCALIBRATION"))==0) {
			// $PCPROBE,COMPASSCALIBRATION
			//  The calibration of the accelerometers and of the magnetometers is being performed

			// no other thread modify m_bCompassCal Flag : no lock needed for read in this thread
			if(!m_bCompassCalOn){
				LockDeviceData();
				m_bCompassCalOn=TRUE;
				UnlockDeviceData();
			}

			return TRUE;
		}

		// if we receive sentence other than compass calibration -> compass calibration is finish.
		// no other thread modify m_bCompassCal Flag : no lock needed for read in this thread
		if(m_bCompassCalOn) {
			LockDeviceData();
			m_bCompassCalOn=FALSE;
			UnlockDeviceData();
		}

  		if (_tcscmp(strToken,TEXT("T"))==0) {
			BOOL bOk = ParseData(d, wiss, pINFO);
			if(!BaroAltitudeAvailable(*pINFO)) {
				SetBaroOn(d);
			}
			return bOk;
		}
		if (_tcscmp(strToken,TEXT("GYROCALIBRATION"))==0) {

			LockDeviceData();
	 		m_bCompassCalOn=FALSE;
			UnlockDeviceData();

			return ParseGyro(wiss, pINFO);
		}
		if (_tcscmp(strToken,TEXT("FW"))==0) {

			LockDeviceData();
	 		m_bCompassCalOn=FALSE;
			UnlockDeviceData();

			return ParseFW(wiss, pINFO);
		}
		if(_tcscmp(strToken,TEXT("NAME"))==0) {

			LockDeviceData();
	 		m_bCompassCalOn=FALSE;
			UnlockDeviceData();

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
BOOL CDevCProbe::ParseData(DeviceDescriptor_t *d, tnmeastring& wiss, NMEA_INFO *pINFO ) {

	double q0 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	double q1 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	double q2 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	double q3 = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;

	double sin_pitch = -2 * (q0 * q2 - q3 * q1); // if sin_pitch > 1 or sin_pitch < -1, discard the data

	if(sin_pitch < 1.0 && sin_pitch > -1.0){
		pINFO->MagneticHeadingAvailable=TRUE;
		pINFO->MagneticHeading = (PI + atan2(2*(q1 * q2 + q3 * q0), q3 * q3 - q0 * q0 - q1 * q1 + q2 * q2))*RAD_TO_DEG;

		pINFO->GyroscopeAvailable=TRUE;
		pINFO->Pitch = asin(sin_pitch)*RAD_TO_DEG;
		pINFO->Roll = atan2( 2 * (q0 * q1 + q3 * q2), q3 * q3 + q0 * q0 - q1 * q1 - q2 * q2)*RAD_TO_DEG;
	}else{
		pINFO->MagneticHeadingAvailable=FALSE;
		pINFO->GyroscopeAvailable=FALSE;
	}

	pINFO->AccelerationAvailable=TRUE;
	pINFO->AccelX = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	pINFO->AccelY = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;
	pINFO->AccelZ = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.001;

	pINFO->TemperatureAvailable=TRUE;
	pINFO->OutsideAirTemperature = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.1;

	pINFO->HumidityAvailable=TRUE;
	pINFO->RelativeHumidity = int16toDouble(HexStrToInt(wiss.GetNextString())) * 0.1;

	pINFO->ExtBatt1_Voltage = int16toDouble(HexStrToInt(wiss.GetNextString()))+1000;

	double delta_press = int16toDouble(HexStrToInt(wiss.GetNextString())) * 1.0/10.0 ;
	double abs_press = int24toDouble(HexStrToInt(wiss.GetNextString())) * (1.0/4.0);

    LockDeviceData();
	m_delta_press = delta_press;
	m_abs_press = abs_press;
	UnlockDeviceData();
    
    // the highest sea level air pressure ever recorded was 1084 mb (32.01 in.) 
    //  at Siberia associated with an extremely cold air mass.
	if(abs_press > 0.0 && abs_press < 115000.0) {
		UpdateBaroSource(pINFO, d, StaticPressureToQNHAltitude(abs_press));
	}
	else {
		if (BaroAltitudeAvailable(*pINFO)) {
			abs_press = QNHAltitudeToStaticPressure(pINFO->BaroAltitude);
		}
		else {
			abs_press = QNHAltitudeToStaticPressure(pINFO->Altitude);
		}
	}

	if(delta_press>0.0){
		pINFO->AirspeedAvailable = TRUE;
		pINFO->IndicatedAirspeed = sqrt(2 * delta_press / 1.225);
		pINFO->TrueAirspeed = TrueAirSpeed(delta_press,	pINFO->RelativeHumidity, pINFO->OutsideAirTemperature, abs_press>0.0?abs_press:101325.0);
	}

	if(*(wiss.GetNextString()) == L'C'){
		pINFO->ExtBatt1_Voltage *= -1;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// $PCPROBE, GYROCALIBRATION,n,m
//  The calibration of the gyroscopes is being performed. "m" is the number of steps required. "n" is the
//  current step. The percentage of work performed is 100 * n / m.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDevCProbe::ParseGyro( tnmeastring& wiss, NMEA_INFO *pINFO ) {
	unsigned int n = HexStrToInt(wiss.GetNextString());
	unsigned int m = HexStrToInt(wiss.GetNextString());

	StartupStore(TEXT("Gyro Calibration : %03ud/%03ud\n"), n, m);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// $PCPROBE, FW,f
//  Firmware version. f = 0xNNMM, where NN is the major version number and MM the minor number.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDevCProbe::ParseFW( tnmeastring& wiss, NMEA_INFO *pINFO ) {
	unsigned int Version = HexStrToInt(wiss.GetNextString());

	LockDeviceData();
	_stprintf(m_szVersion, TEXT("%u.%.02u"), ((Version&0xFF00) >> 8), (Version&0x00FF));
	UnlockDeviceData();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// $PCPROBE, NAME,s
//  "s" is the name of the C-Probe. When searched for as a BlueTooth device, it is found as C-Probe-"s".
//  For example: $PCPROBE, NAME,Vinc means that the C-Probe will be detected as C-Probe-Vinc.
//  The name can be set by the user (see below).
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDevCProbe::ParseName( tnmeastring& wiss, NMEA_INFO *pINFO ) {
	TCHAR* szName = wiss.GetNextString();

	StartupStore(TEXT("C-Probe Name : %s"), szName);
	return TRUE;
}

BOOL CDevCProbe::SetBaroOn( PDeviceDescriptor_t d ){
    if (d && d->Com) {
    	d->Com->WriteString(TEXT("$PCPILOT,C,BAROON\r\n"));
    }
	return TRUE;
}

BOOL CDevCProbe::SetBaroOff( PDeviceDescriptor_t d ){
    if (d && d->Com) {
    	d->Com->WriteString(TEXT("$PCPILOT,C,BAROOFF\r\n"));
    }
	return TRUE;
}

BOOL CDevCProbe::GetDeviceName( PDeviceDescriptor_t d ){
    if (d && d->Com) {
    	d->Com->WriteString(TEXT("$PCPILOT,C,GETNAME\r\n"));
    }
	return TRUE;
}

BOOL CDevCProbe::SetDeviceName( PDeviceDescriptor_t d, const tstring& strName ){
	if (d && d->Com && strName.size() <= 15) {
		d->Com->WriteString(TEXT("$PCPILOT,C,SET,"));
		d->Com->WriteString(strName.c_str());
		d->Com->WriteString(TEXT("\r\n"));
		return GetDeviceName(d);
	}
	return FALSE;
}

BOOL CDevCProbe::GetFirmwareVersion(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,GETFW\r\n"));
    }
	return TRUE;
}

BOOL CDevCProbe::SetZeroDeltaPressure(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,CALZERO\r\n"));
    }
	return TRUE;
}

BOOL CDevCProbe::SetCompassCalOn(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,COMPCALON\r\n"));
    }
	return TRUE;
}

BOOL CDevCProbe::SetCompassCalOff(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,COMPCALOFF\r\n"));
    }
	return TRUE;
}

BOOL CDevCProbe::SetCalGyro(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,CALGYRO\r\n"));
    }
	return TRUE;
}

CallBackTableEntry_t CDevCProbe::CallBackTable[]={
  EndCallBackEntry()
};

BOOL CDevCProbe::Config(PDeviceDescriptor_t d){
	if(m_pDevice != d) {
		StartupStore(_T("C-Probe Config : Invalide device descriptor%s"), NEWLINE);
		return FALSE;
	}

	WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_DEVCPROBE);
	if(wf) {

    	WndButton *wBt = NULL;

    	wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
    	if(wBt){
        	wBt->SetOnClickNotify(OnCloseClicked);
    	}
    	wBt = (WndButton *)wf->FindByName(TEXT("cmdSetCompassCal"));
    	if(wBt){
        	wBt->SetOnClickNotify(OnCompassCalClicked);
    	}
    	wBt = (WndButton *)wf->FindByName(TEXT("cmdSetCalGyro"));
    	if(wBt){
        	wBt->SetOnClickNotify(OnCalGyroClicked);
    	}
    	wBt = (WndButton *)wf->FindByName(TEXT("cmdZeroDeltaPress"));
    	if(wBt){
        	wBt->SetOnClickNotify(OnZeroDeltaPressClicked);
    	}

    	GetFirmwareVersion(m_pDevice);

		wf->SetTimerNotify(1000, OnTimer);
		wf->ShowModal();

		delete wf;
		wf=NULL;
	}
	return TRUE;
}

bool CDevCProbe::OnTimer(WndForm* pWnd){
  Update(pWnd);
  return true;
}

void CDevCProbe::OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

void CDevCProbe::OnCompassCalClicked(WndButton* pWnd){
	(void)pWnd;
	if(m_pDevice) {
		if(m_bCompassCalOn) {
			SetCompassCalOff(m_pDevice);
		} else {
			SetCompassCalOn(m_pDevice);

			MessageBoxX(MsgToken(2136), TEXT("C-Probe"), mbOk, false);

			SetCompassCalOff(m_pDevice);
		}
	}
}

void CDevCProbe::OnCalGyroClicked(WndButton* pWnd) {
	(void)pWnd;
	if(m_pDevice) {
		SetCalGyro(m_pDevice);
	}
}

void CDevCProbe::OnZeroDeltaPressClicked(WndButton* pWnd) {
	(void)pWnd;
	if(m_pDevice) {
		SetZeroDeltaPressure(m_pDevice);
	}
}

void CDevCProbe::Update(WndForm* pWnd) {
	TCHAR Temp[50] = {0};

	LockFlightData();
	NMEA_INFO _INFO = GPS_INFO;
	UnlockFlightData();

	LockDeviceData();
	_stprintf(Temp, TEXT("C-Probe - Version: %s"), m_szVersion);
	UnlockDeviceData();

	pWnd->SetCaption(Temp);

	WndProperty* wp;
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpPitch"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f%s"), _INFO.Pitch, MsgToken(2179));
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpHeading"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f%s"), _INFO.MagneticHeading, MsgToken(2179));
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpRoll"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f%s"), _INFO.Roll, MsgToken(2179));
		wp->SetText(Temp);
	}

	wp = (WndProperty*)pWnd->FindByName(TEXT("prpGx"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f"), _INFO.AccelX);
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpGy"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f"), _INFO.AccelY);
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpGz"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f"), _INFO.AccelZ);
		wp->SetText(Temp);
	}

	wp = (WndProperty*)pWnd->FindByName(TEXT("prpTemp"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f %sC"), _INFO.OutsideAirTemperature, MsgToken(2179));
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpRh"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f %%"), _INFO.RelativeHumidity);
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpDeltaPress"));
	if(wp){
		LockDeviceData();
		_stprintf(Temp, TEXT("%.2f Pa"), m_delta_press);
		UnlockDeviceData();

		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpAbsPress"));
	if(wp){
		LockDeviceData();
		_stprintf(Temp, TEXT("%.2f hPa"), m_abs_press/100.);
		UnlockDeviceData();

		wp->SetText(Temp);
	}
}
