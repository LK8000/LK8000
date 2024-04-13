/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Globals.h"
#include "devPVCOM.h"
#include "Comm/device.h"
#include "devBase.h"
#include "utils/printf.h"
#include "Radio.h"

namespace {

void ReplaceNMEAControlChars(TCHAR *String)
{
  if(String != NULL)
  {
    for (unsigned int i=0; i< _tcslen(String) ; i++)
   {
     switch ((int)String[i])
     {
       case  _T('$'): String[i]  = 's'; break;
       case  _T('*'): String[i]  = '#'; break;
#if ((WINDOWSPC >0)    ||  (PNA > 0))
       case  (int)_T('ü'): String[i]  = 'u'; break;
       case  _T('ö'): String[i]  = 'o'; break;

       case  _T('î'):
       case  _T('ì'):
       case  _T('ï'): String[i]  = 'i'; break;

       case  _T('æ'):
       case  _T('ä'):
       case  _T('à'):
       case  _T('å'):
       case  _T('â'): String[i]  = 'a'; break;
       case  _T('ß'): String[i]  = 's'; break;
       case  _T('Ü'): String[i]  = 'U'; break;
       case  _T('Ö'): String[i]  = 'O'; break;

       case  _T('é'):
       case  _T('ç'):
       case  _T('ê'):
       case  _T('ë'):
       case  _T('è'): String[i]  = 'e'; break;
       case  _T('É'): String[i]  = 'E'; break;
       case  _T('Å'):
       case  _T('Ä'): String[i]  = 'A'; break;
#endif
       default:  String[i]  = String[i]; break;
     }
     if(String[i] < 32) String[i]  = _T('_');
     if(String[i] > 126) String[i]  = _T('_');
   }
 }
}


static
int PVCOMNMEAddCheckSumStrg( TCHAR szStrg[] )
{
int i,iCheckSum=0;
TCHAR  szCheck[254];
LKASSERT(szStrg != NULL);
 if(szStrg[0] != '$')
   return -1;

 iCheckSum = szStrg[1];
  for (i=2; i < (int)_tcslen(szStrg); i++)
  {
	//  if(szStrgi0] != ' ')
	    iCheckSum ^= szStrg[i];
  }
  _stprintf(szCheck,TEXT("*%02X\r\n"),iCheckSum);
  _tcscat(szStrg,szCheck);
  return iCheckSum;
}

BOOL PVCOMRadioMode(DeviceDescriptor_t* d, int mode) {
	  TCHAR  szTmp[255];
	  if(mode == 0)
	    _stprintf(szTmp, TEXT("$PVCOM,S,MOD,N"));
	  else
	    _stprintf(szTmp, TEXT("$PVCOM,S,MOD,D"));
	  PVCOMNMEAddCheckSumStrg(szTmp);
	  if(d != NULL)
		if(!d->Disabled)
	      if (d->Com)
	        d->Com->WriteString(szTmp);
	  return(TRUE);
	}

BOOL PVCOMPutVolume(DeviceDescriptor_t* d, int Volume) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,S,VOL,%d"), Volume);
#ifdef TESTBENCH
    StartupStore(_T(". RADIO Volume  %i%s"), Volume,NEWLINE);
#endif
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
	if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  return(TRUE);
}

BOOL PVCOMPutSquelch(DeviceDescriptor_t* d, int Squelch) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,S,SQL,%d"), Squelch);
#ifdef TESTBENCH
      StartupStore(_T(". RADIO Squelch  %i%s"), Squelch,NEWLINE);
#endif
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
	if(!d->Disabled)
      if (d->Com)
      {
        d->Com->WriteString(szTmp);
      }
  return(TRUE);
}



BOOL PVCOMPutFreqActive(DeviceDescriptor_t* d, unsigned Freq, const TCHAR* Station) {
  TCHAR  szTmp[255];
  TCHAR  StationName[255] =_T("???");;
  if(Station != NULL) {
     _stprintf(StationName, TEXT("%s"), Station);
  }
  ReplaceNMEAControlChars(StationName);

  double Mhz = (Freq / 1000.);

  lk::snprintf(szTmp, TEXT("$PVCOM,S,AF,%7.3f,%s"), Mhz, StationName);
  TestLog(_T(". RADIO Active Station  %7.3f %s"), Mhz, StationName);
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
     if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);

  return(TRUE);
}


BOOL PVCOMPutFreqStandby(DeviceDescriptor_t* d, unsigned Freq,  const TCHAR* Station) {
  TCHAR  szTmp[255];
  TCHAR  StationName[255] =_T("???");;
  if(Station != NULL)
     _stprintf(StationName, TEXT("%s"), Station);
  ReplaceNMEAControlChars(StationName);

  double Mhz = (Freq / 1000.);

  lk::snprintf(szTmp, TEXT("$PVCOM,S,PF,%7.3f,%s"), Mhz, StationName);
  TestLog(_T(". RADIO Stanby Station %7.3fMHz %s"), Mhz, StationName);
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);

  return(TRUE);
}


BOOL PVCOMStationSwap(DeviceDescriptor_t* d) {
  TCHAR  szTmp[255];
#ifdef TESTBENCH
   StartupStore(_T(". RADIO Station swap %s"), NEWLINE);
#endif
  _stprintf(szTmp, TEXT("$PVCOM,S,CHG"));
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  return(TRUE);
}


BOOL PVCOMRequestAllData(DeviceDescriptor_t* d) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,R,ALL"));
  PVCOMNMEAddCheckSumStrg(szTmp);

  ScopeLock Lock(CritSec_Comm);

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);

  return(TRUE);
}

bool ParToDouble(const char* sentence, unsigned int parIdx, double* value)
{
  char  temp[80];
  const char* stop;
  LKASSERT(value!=NULL);

  NMEAParser::ExtractParameter(sentence, temp, parIdx);

  stop = NULL;
  *value = StrToDouble(temp, &stop);

  if (stop == NULL || *stop != '\0')
    return(false);

  return(true);
} // ParToDouble()

static
bool PVCOM_ProcessPEYI(DeviceDescriptor_t* d, const char *sentence, NMEA_INFO *info)
{
  TSpaceInfo data = {};
  unsigned fieldIdx = 0;
  bool status = true;
  double value;

  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.eulerRoll = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.eulerPitch = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.rollRate = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.pitchRate = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.yawRate = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.accelX = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.accelY = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.accelZ = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.virosbandometer = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.trueHeading = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.magneticHeading = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.localDeclination = value;

  if(status) {

    info->GyroscopeAvailable = true;
    info->Pitch = data.eulerPitch;
    info->Roll = data.eulerRoll;


    info->MagneticHeadingAvailable = true;
    info->MagneticHeading = data.magneticHeading;

    info->AccelerationAvailable = false; //true;
    info->AccelX = data.accelX;
    info->AccelY = data.accelY;
    info->AccelZ = data.accelZ;
  }

#ifdef TEST_TRI_PAGE
static bool TRI_on = false;
   if(   CURTYPE == IM_TRI)   {
       if(     TRI_on == false) DoStatusMessage(_T("TRI_ON"));
       TRI_on = true;   }
   else   {
       if(     TRI_on == true) DoStatusMessage(_T("TRI_OFF"));
       TRI_on = false;  }
#endif
/***********************************************************
 * this is a hack, to prevent CPU overload on fast Gyro updates
 * should be solved with a own thread in case external
 * AHRS will become more common
 ***********************************************************/
   if(   CURTYPE == IM_TRI)  /* refresh if TRI is active only */
   {
     TriggerGPSUpdate();
   }
  return status;
}



BOOL PVCOMParseString(DeviceDescriptor_t* d, const char *String, NMEA_INFO *info)
{
char  device[250];
char  cmd[220];
char  dir[220];
char  para1[250];
char  para2[250];


if (!NMEAParser::NMEAChecksum(String) )
{
//	  DoStatusMessage(_T("RADIO Checksum Error!") );
  return FALSE;
}

NMEAParser::ExtractParameter(String,device,0);
    if(strncmp("$PEYI", device, 5) == 0)
        return PVCOM_ProcessPEYI(d, String + 6, info);

if ((strncmp("$PVCOM", device,5) == 0) )
{

	NMEAParser::ExtractParameter(String,dir,1);
    if(strcmp("A", dir) == 0)
	{
      RadioPara.Changed = TRUE;
      if(RadioPara.Enabled == FALSE)
      {
        DoStatusMessage(_T("RADIO Detected!") );
        for (auto& dev : DeviceList) {
          PVCOMRequestAllData(&dev);
        }
        RadioPara.Enabled = TRUE;
      }

      NMEAParser::ExtractParameter(String,cmd,2);
      if(strcmp("AF", cmd) == 0)
      {
        NMEAParser::ExtractParameter(String,para1,3);
        RadioPara.ActiveKhz = ExtractFrequency(para1);

        NMEAParser::ExtractParameter(String,para2,4);
        lk::snprintf(RadioPara.ActiveName, _T("%s"),para2);
      } else
      if(strcmp("PF", cmd) == 0)
      {
        NMEAParser::ExtractParameter(String,para1,3);
        RadioPara.PassiveKhz = ExtractFrequency(para1);

        NMEAParser::ExtractParameter(String,para2,4);
        lk::snprintf(RadioPara.PassiveName, _T("%s"),para2);
      }  else
      if(strcmp("VOL", cmd) == 0)
      {
	NMEAParser::ExtractParameter(String,para1,3);
	RadioPara.Volume = (int)StrToDouble(para1,NULL);
      }   else
	  if(strcmp("SQL", cmd) == 0)
	  {
	     NMEAParser::ExtractParameter(String,para1,3);
	     RadioPara.Squelch = (int)StrToDouble(para1,NULL);
	  }  else
	  if(strcmp("CHG", cmd) == 0)
	  {
      NMEAParser::ExtractParameter(String,para1,3);
      RadioPara.ActiveKhz = ExtractFrequency(para1);

      NMEAParser::ExtractParameter(String,para2,4);
      lk::snprintf(RadioPara.ActiveName, _T("%s"),para2);

      NMEAParser::ExtractParameter(String,para1,5);
      RadioPara.PassiveKhz = ExtractFrequency(para1);

      NMEAParser::ExtractParameter(String,para2,6);
      lk::snprintf(RadioPara.PassiveName,_T("%s"),para2);

	  } else
      if(strcmp("STA", cmd) == 0)
      {
	  NMEAParser::ExtractParameter(String,para1,3);
	  if(strncmp("DUAL_ON", para1, 7) == 0)
	    RadioPara.Dual = TRUE;
	  if(strncmp("DUAL_OFF", para1, 8) == 0)
		RadioPara.Dual = FALSE;
	  if(strncmp("8_33KHZ", para1, 8) == 0)
		RadioPara.Enabled8_33 = TRUE;
	  if(strncmp("25KHZ", para1, 8) == 0)
		RadioPara.Enabled8_33 = FALSE;

	  if(strncmp("8_33KHZ", para1, 6) == 0)
		RadioPara.Enabled8_33 = TRUE;
	  if(strncmp("25KHZ", para1, 4) == 0)
		RadioPara.Enabled8_33 = FALSE;

	  if(strncmp("BAT_LOW", para1, 7) == 0)
		RadioPara.lowBAT = TRUE;
	  if(strncmp("BAT_OK", para1, 6) == 0)
		RadioPara.lowBAT = FALSE;

	  if(strncmp("RX_ON", para1, 5) == 0)
		RadioPara.RX = TRUE;
	  if(strncmp("RX_OFF", para1, 6) == 0)
		RadioPara.RX = FALSE;


	  if(strncmp("TX_ON", para1, 5) == 0)
		RadioPara.TX = TRUE;
	  if(strncmp("RX_TX_OFF", para1, 9) == 0)
	  {
		RadioPara.RX        = FALSE;
		RadioPara.TX        = FALSE;
		RadioPara.TXtimeout = FALSE;
		RadioPara.RX_active = FALSE;
		RadioPara.RX_standy = FALSE;
	  }

	  if(strncmp("TE_ON", para1, 5) == 0)
		RadioPara.TXtimeout = TRUE;
	  if(strncmp("RX_AF", para1, 5) == 0)
		RadioPara.RX_active = TRUE;
	  if(strncmp("RX_SF", para1, 5) == 0)
		RadioPara.RX_standy = TRUE;


      }



	}
}
return  RadioPara.Changed;


}

} // namespace

void PVCOMInstall(DeviceDescriptor_t* d){

  _tcscpy(d->Name, TEXT("PVCOM"));
  d->IsRadio = true;
  d->PutVolume = PVCOMPutVolume;
  d->PutSquelch = PVCOMPutSquelch;
  d->PutFreqActive = PVCOMPutFreqActive;
  d->PutFreqStandby = PVCOMPutFreqStandby;
  d->StationSwap = PVCOMStationSwap;
  d->ParseNMEA    = PVCOMParseString;
  d->PutRadioMode    = PVCOMRadioMode;
  StartupStore(_T(". PVCOMInstall %s%s"), WhatTimeIsIt(),NEWLINE);
}
