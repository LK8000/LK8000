/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Globals.h"
#include "devPVCOM.h"
#include "device.h"
 
BOOL PVCOMInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("PVCOM"));
  d->IsRadio = PVCOMIsRadio;
  d->PutVolume = PVCOMPutVolume;
  d->PutSquelch = PVCOMPutSquelch;
  d->PutFreqActive = PVCOMPutFreqActive;
  d->PutFreqStandby = PVCOMPutFreqStandby;
  d->StationSwap = PVCOMStationSwap;
  d->ParseNMEA    = PVCOMParseString;
  d->RadioMode    = PVCOMRadioMode;
  StartupStore(_T(". PVCOMInstall %s%s"), WhatTimeIsIt(),NEWLINE);
  return(TRUE);

}

BOOL PVCOMRegister(void){
  return(devRegister(
    TEXT("PVCOM"),
    (1l << dfRadio),
    PVCOMInstall
  ));
}



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




BOOL PVCOMIsRadio(PDeviceDescriptor_t d){
  (void)d;
   PVCOMStationSwap( d) ;
  return(TRUE); 
}

BOOL PVCOMRadioMode(PDeviceDescriptor_t d, int mode) {
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

BOOL PVCOMPutVolume(PDeviceDescriptor_t d, int Volume) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,S,VOL,%d"), Volume);
    StartupStore(_T(". RADIO Volume  %i%s"), Volume,NEWLINE);
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
	if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  return(TRUE);
}

BOOL PVCOMPutSquelch(PDeviceDescriptor_t d, int Squelch) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,S,SQL,%d"), Squelch);
      StartupStore(_T(". RADIO Squelch  %i%s"), Squelch,NEWLINE);
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
	if(!d->Disabled)
      if (d->Com)
      {
        d->Com->WriteString(szTmp);
       //   DeviceList[NUMDEV];
        {
    //      DeviceList[0].Com->WriteString(szTmp);
     //     DeviceList[1].Com->WriteString(szTmp);
        }
    //  devB()>Com->WriteString(szTmp);     
      }
  return(TRUE);
}



BOOL PVCOMPutFreqActive(PDeviceDescriptor_t d, double Freq, TCHAR StationName[]) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,S,AF,%7.3f,%s"), Freq,StationName);
  StartupStore(_T(". RADIO Active Station  %7.3f %s%s"), Freq,StationName,NEWLINE);
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
     if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  return(TRUE);
}


BOOL PVCOMPutFreqStandby(PDeviceDescriptor_t d, double Freq,  TCHAR StationName[]) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,S,PF,%7.3f,%s"), Freq,StationName);
    StartupStore(_T(". RADIO Stanby Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  return(TRUE);
}


BOOL PVCOMStationSwap(PDeviceDescriptor_t d) {
  TCHAR  szTmp[255];
   StartupStore(_T(". RADIO Stanby swap %s"), NEWLINE);
  _stprintf(szTmp, TEXT("$PVCOM,S,CHG"));
  PVCOMNMEAddCheckSumStrg(szTmp);
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  return(TRUE);
}


BOOL PVCOMRequestAllData(PDeviceDescriptor_t d) {
  TCHAR  szTmp[255];
  _stprintf(szTmp, TEXT("$PVCOM,R,ALL"));
  PVCOMNMEAddCheckSumStrg(szTmp);
  LockComm();
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  UnlockComm();
  return(TRUE);
}


BOOL PVCOMParseString(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *info)
{
TCHAR  device[250];
TCHAR  cmd[220];
TCHAR  dir[220];
TCHAR  para1[250];
TCHAR  para2[250];


if (!NMEAParser::NMEAChecksum(String) )
{
	  DoStatusMessage(_T("RADIO Checksum Error!") );
//  return FALSE;
}


NMEAParser::ExtractParameter(String,device,0);
if ((_tcsncmp(_T("$PVCOM"), device,5) == 0) )
{

	NMEAParser::ExtractParameter(String,dir,1);
    if(_tcscmp(_T("A"), dir) == 0)
	{
      RadioPara.Changed = TRUE;
      if(RadioPara.Enabled == FALSE)
      {

        DoStatusMessage(_T("RADIO Detected!") );
        PVCOMRequestAllData(devA()) ;
        PVCOMRequestAllData(devB()) ;
        RadioPara.Enabled = TRUE;
      }

      NMEAParser::ExtractParameter(String,cmd,2);
      if(_tcscmp(_T("AF"), cmd) == 0)
      {
    	NMEAParser::ExtractParameter(String,para1,3);
    	NMEAParser::ExtractParameter(String,para2,4);
    	RadioPara.ActiveFrequency = StrToDouble(para1,NULL);
    	_stprintf(RadioPara.ActiveName,_T("%s"),para2);
      } else
      if(_tcscmp(_T("PF"), cmd) == 0)
      {
      	NMEAParser::ExtractParameter(String,para1,3);
      	NMEAParser::ExtractParameter(String,para2,4);
    	RadioPara.PassiveFrequency = StrToDouble(para1,NULL);
    	_stprintf(RadioPara.PassiveName,_T("%s"),para2);
      }  else
      if(_tcscmp(_T("VOL"), cmd) == 0)
      {
      	NMEAParser::ExtractParameter(String,para1,3);
    	RadioPara.Volume = (int)StrToDouble(para1,NULL);
      }   else
	  if(_tcscmp(_T("SQL"), cmd) == 0)
	  {
	     NMEAParser::ExtractParameter(String,para1,3);
	     RadioPara.Squelch = (int)StrToDouble(para1,NULL);
	  }  else
	  if(_tcscmp(_T("CHG"), cmd) == 0)
	  {

		NMEAParser::ExtractParameter(String,para1,3);
		NMEAParser::ExtractParameter(String,para2,4);
    	RadioPara.ActiveFrequency = StrToDouble(para1,NULL);
    	_stprintf(RadioPara.ActiveName,_T("%s"),para2);

		NMEAParser::ExtractParameter(String,para1,5);
		NMEAParser::ExtractParameter(String,para2,6);
    	RadioPara.PassiveFrequency = StrToDouble(para1,NULL);
    	_stprintf(RadioPara.PassiveName,_T("%s"),para2);

	  } else
      if(_tcscmp(_T("STA"), cmd) == 0)
      {
    	  NMEAParser::ExtractParameter(String,para1,3);
    	  if(_tcsncmp(_T("DUAL_ON"), para1, 7) == 0)
    	    RadioPara.Dual = TRUE;
    	  if(_tcsncmp(_T("DUAL_OFF"), para1, 8) == 0)
    		RadioPara.Dual = FALSE;
    	  if(_tcsncmp(_T("8_33KHZ"), para1, 8) == 0)
    		RadioPara.Enabled8_33 = TRUE;
    	  if(_tcsncmp(_T("25KHZ"), para1, 8) == 0)
    		RadioPara.Enabled8_33 = FALSE;

    	  if(_tcsncmp(_T("8_33KHZ"), para1, 6) == 0)
    		RadioPara.Enabled8_33 = TRUE;
    	  if(_tcsncmp(_T("25KHZ"), para1, 4) == 0)
    		RadioPara.Enabled8_33 = FALSE;

    	  if(_tcsncmp(_T("BAT_LOW"), para1, 7) == 0)
    		RadioPara.lowBAT = TRUE;
    	  if(_tcsncmp(_T("BAT_OK"), para1, 6) == 0)
    		RadioPara.lowBAT = FALSE;

    	  if(_tcsncmp(_T("RX_ON"), para1, 5) == 0)
    		RadioPara.RX = TRUE;
    	  if(_tcsncmp(_T("RX_OFF"), para1, 6) == 0)
    		RadioPara.RX = FALSE;


    	  if(_tcsncmp(_T("TX_ON"), para1, 5) == 0)
    		RadioPara.TX = TRUE;
    	  if(_tcsncmp(_T("RX_TX_OFF"), para1, 9) == 0)
    	  {
    		RadioPara.RX        = FALSE;
    		RadioPara.TX        = FALSE;
    		RadioPara.TXtimeout = FALSE;
    		RadioPara.RX_active = FALSE;
    		RadioPara.RX_standy = FALSE;
    	  }

    	  if(_tcsncmp(_T("TE_ON"), para1, 5) == 0)
    		RadioPara.TXtimeout = TRUE;
    	  if(_tcsncmp(_T("RX_AF"), para1, 5) == 0)
    		RadioPara.RX_active = TRUE;
    	  if(_tcsncmp(_T("RX_SF"), para1, 5) == 0)
    		RadioPara.RX_standy = TRUE;


      }



	}
}
return  RadioPara.Changed;


}


