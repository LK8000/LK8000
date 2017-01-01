/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Globals.h"
#include "devAR620x.h"
#include "device.h"

#ifdef RADIO_ACTIVE
#define RoundFreq(a) ((int)((a)*1000.0+0.5)/1000.0)
#define Freq2Idx(a)  (int)(((a)-118.0) * 3040/(137.00-118.0)+0.5)
#define min(X,Y) ((X) < (Y) ? : (X) : (Y))


#define ACTIVE_STATION  1
#define PASSIVE_STATION 0


//#define RESEND_ON_NAK       /* switch for command retry on transmission fail  */
#define RADIO_VOLTAGE       /* read Radio Supply voltage and store it in BATT2 indicator */

#define HEADER_ID       0xA5
#define PROTOKOL_ID     0x14
#define BIT7            0x80
#define BIT8            0x100
#define QUERY           BIT7         
#define DAUL            BIT8
#define SQUELCH         BIT7

#ifdef TESTBENCH
int iDebugLevel =1;
#else
int iDebugLevel =0;
#endif
typedef union
{
    uint16_t intVal16;
    uint8_t  intVal8[2];
} IntConvertStruct;

IntConvertStruct CRC;
IntConvertStruct sFrequency;
IntConvertStruct sStatus;
uint8_t  szTmp[128];


int AR620x_Convert_Answer(DeviceDescriptor_t *d, uint8_t  *szCommand, int len, uint16_t CRC);

double Idx2Freq(uint16_t uFreqIdx)
{
double fFreq= 118.000 + (uFreqIdx& 0xFFF0) * (137.000-118.000)/3040.0;
switch(uFreqIdx & 0xF)
{
    case 0:  fFreq += 0.000; break;
    case 1:  fFreq += 0.005; break;
    case 2:  fFreq += 0.010; break;
    case 3:  fFreq += 0.015; break;
    case 4:  fFreq += 0.025; break;
    case 5:  fFreq += 0.030; break;
    case 6:  fFreq += 0.035; break;
    case 7:  fFreq += 0.040; break;
    case 8:  fFreq += 0.050; break;
    case 9:  fFreq += 0.055; break;
    case 10: fFreq += 0.060; break;
    case 11: fFreq += 0.065; break;
    case 12: fFreq += 0.075; break;
    case 13: fFreq += 0.080; break;
    case 14: fFreq += 0.085; break;
    case 15: fFreq += 0.090; break;   
}
return (fFreq);
}


uint16_t Frq2Idx(double fFreq)
{
uint16_t  uFreIdx= Freq2Idx(fFreq);
uFreIdx &= 0xFFF0;
uint8_t uiFrac = ((int)(fFreq*1000.0+0.5)) - (((int)(fFreq *10.0))*100);
    
switch(uiFrac )
{
    case 0:   uFreIdx += 0;  break;
    case 5:   uFreIdx += 1;  break;
    case 10:  uFreIdx += 2;  break;
    case 15:  uFreIdx += 3;  break;
    case 25:  uFreIdx += 4;  break;
    case 30:  uFreIdx += 5;  break;
    case 35:  uFreIdx += 6;  break;
    case 40:  uFreIdx += 7;  break;
    case 50:  uFreIdx += 8;  break;
    case 55:  uFreIdx += 9;  break;
    case 60:  uFreIdx += 10; break;
    case 65:  uFreIdx += 11; break;
    case 75:  uFreIdx += 12; break;
    case 80:  uFreIdx += 13; break;
    case 85:  uFreIdx += 14; break;
    case 90:  uFreIdx += 15; break;
    case 100: uFreIdx += 0;  break;
    default:   StartupStore(_T("undefined Frequency!  %u -> %u %s"),uiFrac, uFreIdx,NEWLINE);     break;
}
 
return (uFreIdx);
}


static uint16_t CRCBitwise(uint8_t *data, size_t len)
{
  uint16_t crc = 0x0000;
  size_t j;
  int i;
  for (j=len; j>0; j--) {
    crc ^= (uint16_t)(*data++) << 8;
    for (i=0; i<8; i++) {
      if (crc & 0x8000) crc = (crc<<1) ^ 0x8005;
      else crc <<= 1;
    }
  }
  return (crc);
}



BOOL AR620xInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("Dittel AR620x"));
  d->IsRadio = AR620xIsRadio;
  d->PutVolume = AR620xPutVolume;
  d->PutSquelch = AR620xPutSquelch;
  d->PutFreqActive = AR620xPutFreqActive;
  d->PutFreqStandby = AR620xPutFreqStandby;
  d->StationSwap = AR620xStationSwap;
  d->ParseNMEA    = NULL;
  d->ParseStream   = AR620xParseString;
  d->RadioMode    = AR620xRadioMode;
  RadioPara.Enabled8_33 = TRUE;
  sStatus.intVal16 = SQUELCH; // BIT7 for Squelch enabled
  return(TRUE);

}

BOOL AR620xRegister(void){
  return(devRegister(
    TEXT("Becker AR620x"),
    (1l << dfRadio),
    AR620xInstall
  ));
}





BOOL AR620xIsRadio(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}



/*****************************************************************************
 * this function set the station name and frequency on the AR620x
 *
 * ComPort        index of the comport the AR620x is attached
 * Active_Passive Active or passive station switch
 * fFrequency     station frequency
 * Station        station Name string
 *
 *****************************************************************************/
int  SetAR620xStation(uint8_t *Command ,int Active_Passive, double fFrequency, TCHAR* Station)
{

unsigned int len = 8;
LKASSERT(Station !=NULL)
LKASSERT(Command !=NULL)
if(Command == NULL )
    return false;

len = 10;
     IntConvertStruct   ActiveFreqIdx; ActiveFreqIdx.intVal16 = Frq2Idx(RadioPara.ActiveFrequency);
     IntConvertStruct   PassiveFreqIdx;  PassiveFreqIdx.intVal16 = Frq2Idx(RadioPara.PassiveFrequency);
    Command [0] = HEADER_ID ;
    Command [1] = PROTOKOL_ID ;
    Command [2] = 5;

    switch (Active_Passive)
    {
      case ACTIVE_STATION:
           ActiveFreqIdx.intVal16 = Frq2Idx(fFrequency);
            if(iDebugLevel) StartupStore(_T(">AF:%u  %7.3f%s"), ActiveFreqIdx.intVal16, fFrequency, NEWLINE);    
      break;
      default:
      case PASSIVE_STATION:
          PassiveFreqIdx.intVal16 =  Frq2Idx(fFrequency);
          if(iDebugLevel) StartupStore(_T(">PF:%u  %7.3f%s"), PassiveFreqIdx.intVal16, fFrequency,NEWLINE);    
      break;
    }  
     Command [3] = 22;
    Command [4] = ActiveFreqIdx.intVal8[1];
    Command [5] = ActiveFreqIdx.intVal8[0];
    Command [6] = PassiveFreqIdx.intVal8[1];
    Command [7] = PassiveFreqIdx.intVal8[0];
    CRC.intVal16 =  CRCBitwise(Command, 8);
    Command [8] = CRC.intVal8[1];
    Command [9] = CRC.intVal8[0];
  
//  SendDataBufferISR(ComPort, Command, 13);
//  sprintf(szLastCommandSend, Command); lastComPort = ComPort; // remember for 2nd try in case it fails
  return len;
}





BOOL AR620xPutVolume(PDeviceDescriptor_t d, int Volume) {
 // #define  AR620x_HAS_VOLUME_CONTROL
#ifdef AR620x_HAS_VOLUME_CONTROL
  int i, len;

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {         
            len = 0;
            szTmp [len++] = HEADER_ID ;
            szTmp [len++] = PROTOKOL_ID ;
            szTmp [len++] =3;
            szTmp [len++] =3;
            szTmp [len++]  =  50-Volume*5;   
            CRC.intVal16 =  CRCBitwise(szTmp, len);
            szTmp [len++] = CRC.intVal8[1];
            szTmp [len++] = CRC.intVal8[0];
            for (i=0; i < len; i++)
               d->Com->PutChar(szTmp[i]);

            if(iDebugLevel) StartupStore(_T(". AR620x Volume  %i%s"), Volume,NEWLINE);
     //       RadioPara.Volume = Volume         
      }
  return(TRUE);
#else
  return false;
#endif
}




BOOL AR620xPutSquelch(PDeviceDescriptor_t d, int Squelch) {
// #define	SET_SQUELCH
#ifdef SET_SQUELCH
  uint8_t i, len;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
            len = 0;
            szTmp [len++] = HEADER_ID ;
            szTmp [len++] = PROTOKOL_ID ;
            szTmp [len++] =3;
            szTmp [len++] =4;
            szTmp [len++] = 6 + (Squelch-1)*2;   
            CRC.intVal16 =  CRCBitwise(szTmp, len);
            szTmp [len++] = CRC.intVal8[1];
            szTmp [len++] = CRC.intVal8[0];
            for (i=0; i < len; i++)
               d->Com->PutChar(szTmp[i]);

            if(iDebugLevel) StartupStore(_T(". AR620x Squelch  %i%s"), Squelch,NEWLINE);

            RadioPara.Squelch = Squelch;
          
      }
#endif
  return(TRUE);
}


BOOL AR620xPutFreqActive(PDeviceDescriptor_t d, double Freq, TCHAR StationName[]) {

    int i, len;

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         len = SetAR620xStation(szTmp ,ACTIVE_STATION, Freq, StationName);
          
         for (i=0; i < len; i++)
           d->Com->PutChar(szTmp[i]);

        RadioPara.ActiveFrequency=  Freq;
        if(StationName != NULL)
          _stprintf(RadioPara.ActiveName,_T("%s"),StationName) ;

        if(iDebugLevel) StartupStore(_T(". AR620x Active Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);
      }
  return(TRUE);
}


BOOL AR620xPutFreqStandby(PDeviceDescriptor_t d, double Freq,  TCHAR StationName[]) {

  int i, len;

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         len = SetAR620xStation(szTmp ,PASSIVE_STATION, Freq, StationName);
         for (i=0; i < len; i++)
           d->Com->PutChar(szTmp[i]);


        RadioPara.PassiveFrequency =  Freq;
        if(StationName != NULL)
          _stprintf(RadioPara.PassiveName  ,_T("%s"),StationName) ;
        if(iDebugLevel) StartupStore(_T(". AR620x Standby Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);

      }
  return(TRUE);
}


BOOL AR620xStationSwap(PDeviceDescriptor_t d) {

  uint8_t i, len;

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         len = 0;
         IntConvertStruct   ActiveFreqIdx; ActiveFreqIdx.intVal16 = Frq2Idx(RadioPara.ActiveFrequency);
         IntConvertStruct   PassiveFreqIdx;  PassiveFreqIdx.intVal16 = Frq2Idx(RadioPara.PassiveFrequency);
        szTmp [len++] = HEADER_ID ;
        szTmp [len++] = PROTOKOL_ID ;
        szTmp [len++] = 5;
        szTmp [len++]  = 22;
        szTmp [len++] = PassiveFreqIdx.intVal8[1];
        szTmp [len++] = PassiveFreqIdx.intVal8[0];
        szTmp [len++] = ActiveFreqIdx.intVal8[1];
        szTmp [len++] = ActiveFreqIdx.intVal8[0];
        CRC.intVal16 =  CRCBitwise(szTmp, len);
        szTmp [len++] = CRC.intVal8[1];
        szTmp [len++] = CRC.intVal8[0];
        for (i=0; i < len ; i++)
            d->Com->PutChar(szTmp[i]);
      }
  return(TRUE);
}


BOOL AR620xRadioMode(PDeviceDescriptor_t d, int mode) {

  uint8_t  i,len;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {

         if( mode > 0  )
         {
            sStatus.intVal16 |= DAUL;  // turn Dual Mode On
            if(iDebugLevel) StartupStore(_T(". AR620x  Dual on %s"), NEWLINE);
         }
         else
         {
            sStatus.intVal16 &= ~DAUL;   // turn Dual Mode Off
            if(iDebugLevel)StartupStore(_T(". AR620x  Dual off %s"), NEWLINE);
         }
         len = 0;
         szTmp [len++] = HEADER_ID ;
         szTmp [len++] = PROTOKOL_ID ;
         szTmp [len++] =3;
         szTmp [len++] =12;
         szTmp [len++]  = sStatus.intVal8[1];   
         szTmp [len++]  = sStatus.intVal8[0];   
         CRC.intVal16 =  CRCBitwise(szTmp, len);
         szTmp [len++] = CRC.intVal8[1];
         szTmp [len++] = CRC.intVal8[0];         
         for (i=0; i < len; i++)
           d->Com->PutChar(szTmp[i]);
      }
  return(TRUE);
}




BOOL AR620xParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
//(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *info)
{
int cnt=0;
uint16_t CalCRC=0;
static  uint16_t Recbuflen=0;

 int CommandLength =0;
#define REC_BUFSIZE 127
static uint8_t  Command[REC_BUFSIZE];

if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;


while (cnt < len)
{   
  if(String[cnt] ==HEADER_ID)
    Recbuflen =0;
  
  if(Recbuflen == 2)
      if(Command[Recbuflen-1] != PROTOKOL_ID)
          Recbuflen =0;
          
  if(Recbuflen >= 3)
  {
     CommandLength = Command[2];
     if(Recbuflen >= (CommandLength+5) ) // all received
     {
       if(iDebugLevel ==2) for(int i=0; i < (CommandLength+5);i++)   StartupStore(_T("AR620x  Cmd: 0x%02X  %s") ,Command[i] ,NEWLINE);
       CRC.intVal8[1] =  Command[CommandLength+3];
       CRC.intVal8[0] =  Command[CommandLength+4];
       if(iDebugLevel ==2) StartupStore(_T("AR620x  CRC 0x%04X %s") ,CRC.intVal16,NEWLINE); 
       CalCRC =CRCBitwise(Command, CommandLength+3);
       if(CalCRC  == CRC.intVal16)
       {
           if(iDebugLevel ==2)  StartupStore(_T("AR620x  Process Command %u  %s") ,CommandLength ,NEWLINE);
           AR620x_Convert_Answer(d, Command, CommandLength+5,CalCRC);
       }
       else
       {
    	   if(iDebugLevel)StartupStore(_T("AR620x  CRC check fail! Command 0x%04X  0x%04X %s") ,CRC.intVal16,CalCRC ,NEWLINE);
       }
       Recbuflen = 0;
     }
  }
  if(Recbuflen >= REC_BUFSIZE)
     Recbuflen =0;
  LKASSERT(Recbuflen < REC_BUFSIZE);
  Command[Recbuflen++] =(uint8_t) String[cnt++];

}
return  RadioPara.Changed;
}



/*****************************************************************************
 * this function converts a KRT answer sting to a NMEA answer
 *
 * szAnswer       NMEA Answer
 * Answerlen      number of valid characters in the NMEA answerstring
 * szCommand      AR620x binary code to be converted
 * len            length of the AR620x binary code to be converted
 ****************************************************************************/
int AR620x_Convert_Answer(DeviceDescriptor_t *d, uint8_t  *szCommand, int len, uint16_t CRC)
{
if(d == NULL) return 0;
if(szCommand == NULL) return 0;
if(len == 0)          return 0;

static uint16_t uiLastChannelCRC =0;
static uint16_t uiVolumeCRC      =0;
static uint16_t uiVersionCRC     =0;
static uint16_t uiStatusCRC      =0;
static uint16_t uiSquelchCRC     =0;
#ifdef RADIO_VOLTAGE
static uint16_t uiVoltageCRC     =0;
#endif
int processed=0;


LKASSERT(szCommand !=NULL);
LKASSERT(d !=NULL);



      switch ((unsigned char)(szCommand[3] & 0x7F))
      {
        case 0:
          if(uiVersionCRC!= CRC)  {
             uiVersionCRC = CRC;
           }
        break;                  

        case 3:
          if(uiVolumeCRC != CRC){                                     
             uiVolumeCRC = CRC;
             RadioPara.Changed = true;
             RadioPara.Volume = (50-(int)szCommand[4])/5;
             if(iDebugLevel )  StartupStore(_T("AR620x  Volume %u  %s") , RadioPara.Volume  ,NEWLINE);
           }
        break;

        case 4:
          if(uiSquelchCRC!= CRC){
              uiSquelchCRC = CRC;                    
              RadioPara.Changed = true;
              RadioPara.Squelch = (int)(szCommand[4]-5)/2;
              if(iDebugLevel )  StartupStore(_T("AR620x  Volume %u  %s") , RadioPara.Squelch  ,NEWLINE);
           }
        break;

        case 12:
          if(uiStatusCRC != CRC){
             uiStatusCRC = CRC;
             RadioPara.Changed = true;
             sStatus.intVal8[1] = szCommand[4] ;
             sStatus.intVal8[0] = szCommand[5] ;    
            if(sStatus.intVal16 & DAUL) 
               RadioPara.Dual = true;
             else
               RadioPara.Dual = false;
             if(iDebugLevel )  StartupStore(_T("AR620x  Volume %u  %s") , RadioPara.Dual  ,NEWLINE);
           }
        break;
#ifdef RADIO_VOLTAGE
        case 21:
            if(uiVoltageCRC != CRC) {
                  uiVoltageCRC = CRC;
                  GPS_INFO.ExtBatt2_Voltage =   8.5 + szCommand[4] *0.1;
                  RadioPara.Changed = true;                                              
                  if(iDebugLevel  == 2)   StartupStore(_T("AR620x Supply Voltage: %4.1fV %s"),  GPS_INFO.ExtBatt2_Voltage  ,NEWLINE);                                              
                }            
           break;
#endif           
        case 22:
            if(uiLastChannelCRC != CRC) {
                 uiLastChannelCRC = CRC;
                  RadioPara.Changed = true;
                  sFrequency.intVal8[1] = szCommand[4] ;
                  sFrequency.intVal8[0] = szCommand[5] ;              
                 if(   RadioPara.ActiveFrequency  !=  Idx2Freq(sFrequency.intVal16))
                 {
                    RadioPara.ActiveFrequency =  Idx2Freq(sFrequency.intVal16);
                    _stprintf(RadioPara.ActiveName,_T("        ")) ;                        
                    if(iDebugLevel )  StartupStore(_T("AR620x <AF %u  %7.3f%s"), sFrequency.intVal16, RadioPara.ActiveFrequency ,NEWLINE);            
                }

                sFrequency.intVal8[1] = szCommand[6];
                sFrequency.intVal8[0] = szCommand[7] ;     
                if(   RadioPara.PassiveFrequency  !=  Idx2Freq(sFrequency.intVal16))
               {
                  RadioPara.PassiveFrequency =  Idx2Freq(sFrequency.intVal16);
                   _stprintf(RadioPara.PassiveName,_T("        ")) ;       
                   if(iDebugLevel )  StartupStore(_T("AR620x <PF: %u %7.3f%s"), sFrequency.intVal16, RadioPara.PassiveFrequency ,NEWLINE);

                }
             } 
        break;

        default:
        break;
      }



    return processed;  /* return the number of converted characters */
}
#endif  // RADIO_ACTIVE
