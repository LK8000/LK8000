/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devFlymasterF1.h"

static BOOL VARIO(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

static BOOL FlymasterF1ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){

  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  if(_tcsncmp(TEXT("$VARIO"), String, 6)==0)
    {
      return VARIO(d, &String[7], pGPS);
    }

  return FALSE;

}

static BOOL FlymasterF1IsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL FlymasterF1IsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}



/*
 * http://luftigduk.mamutweb.com/Resource/File/0/FLYMASTER%20F1%20KOMMANDOER%20V%206.0.PDF
 *
 * Switching navigation mode on and off
 *  During normal operation F1 will continuously send data to the port, this is navigation and altimeter
 *  data which is used by the navigation application (NAV+ etc).To stop F1 from sending navigation data,
 *  send the following command:
 *  $PFMDNL,*1D
 *
 *  On receiving this command the F1 stop sending NMEA navigation sentences.
 *  To return to navigation mode, i.e. tell F1 to start sending navigation data again send:
 *  $PFMNAV,*02
 */
static BOOL Open(PDeviceDescriptor_t d) {
  if(d && d->Com) {
    const char szNmeaOn[] = "$PFMNAV,*2E\r\n";
    return d->Com->Write(szNmeaOn, strlen(szNmeaOn));
  }
  return FALSE;
}

static BOOL Close(PDeviceDescriptor_t d) {
  if(d && d->Com) {
    const char szNmeaOff[] = "$PFMSNP,*3A\r\n";
    return d->Com->Write(szNmeaOff, strlen(szNmeaOff));
  }
  return FALSE;
}

void flymasterf1Install(PDeviceDescriptor_t d) {

  StartupStore(_T(". FlymasterF1 device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("FlymasterF1"));
  d->ParseNMEA = FlymasterF1ParseNMEA;
  d->IsGPSSource = FlymasterF1IsGPSSource;
  d->IsBaroSource = FlymasterF1IsBaroSource;
}

void flymasterInstall(PDeviceDescriptor_t d) {

  StartupStore(_T(". Flymaster GPS device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("Flymaster GPS"));
  d->ParseNMEA = FlymasterF1ParseNMEA;
  d->Open = Open;
  d->Close = Close;
  d->IsGPSSource = FlymasterF1IsGPSSource;
  d->IsBaroSource = FlymasterF1IsBaroSource;
}


// *****************************************************************************
// local stuff

static BOOL VARIO(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{
  // $VARIO,fPressure,fVario,Bat1Volts,Bat2Volts,BatBank,TempSensor1,TempSensor2*CS

  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,0);
  double ps = StrToDouble(ctemp,NULL);
  UpdateBaroSource( pGPS, 0,d,  	 (1 - pow(fabs(ps / QNH), 0.190284)) * 44307.69);

  NMEAParser::ExtractParameter(String,ctemp,1);
  double Vario = StrToDouble(ctemp,NULL)/10.0;
  UpdateVarioSource(*pGPS, *d, Vario);
  // JMW vario is in dm/s

  NMEAParser::ExtractParameter(String,ctemp,2);
  pGPS->ExtBatt1_Voltage = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,3);
  pGPS->ExtBatt2_Voltage = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,4);
  pGPS->ExtBatt_Bank = (int)StrToDouble(ctemp,NULL);

  return TRUE;
}
