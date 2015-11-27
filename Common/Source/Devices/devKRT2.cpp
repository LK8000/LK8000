/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Globals.h"
#include "devKRT2.h"
#include "device.h"

#ifdef RADIO_ACTIVE     

 #define min(X,Y) ((X) < (Y) ? : (X) : (Y))


#define ACTIVE_STATION  1
#define PASSIVE_STATION 0

#define ACTIVE          1   /* index for active radio data set                */
#define SHADDOW         0   /* index for shaddowed radio data set             */
#define STX             0x02  /* STX Command prefix hex code                  */
#define ACK             0x6   /* acknolage hex code                           */
#define NAK             0x15  /* not acknolage hex code                       */
//#define MAX_ANSWER_LEN 255  /* max. answer string dimentsion constants        */
//#define MAX_COMMAND_LEN 255 /* max. command string dimentsion constants       */
//#define DEBUG_OUTPUT        /* switch for debugging output                    */
//#define RESEND_ON_NAK       /* switch for command retry on transmission fail  */



int KRT2_Convert_Answer(DeviceDescriptor_t *d, char *szCommand, int len);




BOOL KRT2Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("Dittel KRT2"));
  d->IsRadio = KRT2IsRadio;
  d->PutVolume = KRT2PutVolume;
  d->PutSquelch = KRT2PutSquelch;
  d->PutFreqActive = KRT2PutFreqActive;
  d->PutFreqStandby = KRT2PutFreqStandby;
  d->StationSwap = KRT2StationSwap;
  d->ParseNMEA    = NULL;
  d->ParseStream   = KRT2ParseString;
  d->RadioMode    = KRT2RadioMode;
  return(TRUE);

}

BOOL KRT2Register(void){
  return(devRegister(
    TEXT("Dittel KRT2"),
    (1l << dfRadio),
    KRT2Install
  ));
}





BOOL KRT2IsRadio(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}



/*****************************************************************************
 * this function set the station name and frequency on the KRT2
 *
 * ComPort        index of the comport the KRT2 is attached
 * Active_Passive Active or passive station switch
 * fFrequency     station frequency
 * Station        station Name string
 *
 *****************************************************************************/
int  SetKRT2Station(TCHAR *Command ,int Active_Passive, double fFrequency, TCHAR* Station)
{
TCHAR cActivePassive  = 'U';
unsigned int len = 8;
unsigned int i;
int MHz= (int) fFrequency;
int kHz= (int) (fFrequency *1000.0 - MHz *1000  + 0.5);
int Chan = kHz/5;
//char Command[MAX_ANSWER_LEN];
char Airfield[10]={"   ---   "};
LKASSERT(Station !=NULL)
LKASSERT(Command !=NULL)
if(Command == NULL )
    return false;
if(Station != NULL)
{
  if( len > _tcslen(Station))
    len =_tcslen(Station);

    for (i= 0; i < len ; i++) {
      Airfield[i] = Station[i];
    }
 }
  switch (Active_Passive)
  {
      case ACTIVE_STATION:
        cActivePassive = 'U';
      break;
      default:
      case PASSIVE_STATION:
        cActivePassive = 'R';
      break;
  }

  _stprintf(Command, _T("%c%c%c%c%c%c%c%c%c%c%c%c%c"),STX,cActivePassive,  (unsigned char)MHz,  (unsigned char)Chan,
			  Airfield[0],
                                                        Airfield[1],
			  Airfield[2],
			  Airfield[3],
			  Airfield[4],
			  Airfield[5],
			  Airfield[6],
			  Airfield[7],
			(unsigned char)MHz ^ (unsigned char)Chan );
//  SendDataBufferISR(ComPort, Command, 13);
//  sprintf(szLastCommandSend, Command); lastComPort = ComPort; // remember for 2nd try in case it fails
  return 13;
}





/*****************************************************************************
 * this function set the KRT2 audio settings
 *
 * ComPort        index of the comport the KRT2 is attached
 * Vol            new volume on the KRT2 radio
 * Squelch        new Squelch on the KRT2 radio
 * Vox            new Intercom VOX value on the KRT2 radio
 *
 *****************************************************************************/
int  SetKRT2Audio(TCHAR *Command ,int Vol, int Squelch, int Vox)
{ 
LKASSERT(Command !=NULL)
if(Command == NULL )
    return false;

unsigned char Checksum = (unsigned char)Squelch + (unsigned char)Vox;
  _stprintf(Command, _T("%cA%c%c%c%c"),STX,
                         (unsigned char)Vol,
                         (unsigned char)Squelch,
                         (unsigned char)Vox,
	       (unsigned char)Checksum);

//  SendDataBufferISR(ComPort, Command, 6);
//  sprintf(szLastCommandSend, Command); lastComPort = ComPort; // remember for 2nd try in case it fails
  return 6;
}


BOOL KRT2PutVolume(PDeviceDescriptor_t d, int Volume) {
  TCHAR  szTmp[255];
  int i, len;    
 
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
          if(( len = SetKRT2Audio(szTmp ,Volume ,  RadioPara.Squelch, RadioPara.Vox) ) >0)
          {
           for (i=0; i < len; i++)
              d->Com->PutChar(szTmp[i]);
            StartupStore(_T(". KRT2 Volume  %i%s"), Volume,NEWLINE);
            RadioPara.Volume = Volume;
          }
      }
  return(TRUE);
}




BOOL KRT2PutSquelch(PDeviceDescriptor_t d, int Squelch) {
  TCHAR  szTmp[255];
  int i, len;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
          if(( len = SetKRT2Audio(szTmp ,RadioPara.Volume ,  Squelch, RadioPara.Vox) ) >0)
          {
           for (i=0; i < len; i++)
              d->Com->PutChar(szTmp[i]);
            StartupStore(_T(". KRT2 Squelch  %i%s"), Squelch,NEWLINE);
            RadioPara.Squelch = Squelch;
          }
      }
  return(TRUE);
}



BOOL KRT2PutFreqActive(PDeviceDescriptor_t d, double Freq, TCHAR StationName[]) {
  TCHAR  szTmp[255];
    int i, len;
 
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {        
         len = SetKRT2Station(szTmp ,ACTIVE_STATION, Freq, StationName);
         for (i=0; i < len; i++)
           d->Com->PutChar(szTmp[i]);
         
        RadioPara.ActiveFrequency=  Freq;
        if(StationName != NULL)        
          _stprintf(RadioPara.ActiveName,_T("%s"),StationName) ;       
   
         StartupStore(_T(". KRT2 Active Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);
      }
  return(TRUE);
}


BOOL KRT2PutFreqStandby(PDeviceDescriptor_t d, double Freq,  TCHAR StationName[]) {
  TCHAR  szTmp[255];
  int i, len;
  
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {        
         len = SetKRT2Station(szTmp ,PASSIVE_STATION, Freq, StationName);
         for (i=0; i < len; i++)
           d->Com->PutChar(szTmp[i]);        
         
         
        RadioPara.PassiveFrequency =  Freq;
        if(StationName != NULL)
          _stprintf(RadioPara.PassiveName  ,_T("%s"),StationName) ;       
            
         StartupStore(_T(". KRT2 Standby Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);
      }
  return(TRUE);
}


BOOL KRT2StationSwap(PDeviceDescriptor_t d) {
  TCHAR  szTmp[255];
  int i;

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
          _stprintf(szTmp, _T("%cC"), STX);
          for (i=0; i < 2; i++)
            d->Com->PutChar(szTmp[i]);
           StartupStore(_T(". KRT2  station swap %s"), NEWLINE);
      }
  return(TRUE);
}


BOOL KRT2RadioMode(PDeviceDescriptor_t d, int mode) {
  TCHAR  szTmp[255];
  int i;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         if( mode > 0  )
         {     
            _stprintf(szTmp, _T("%cO"), STX);    // turn Dual Mode On
            StartupStore(_T(". KRT2  Dual on %s"), NEWLINE);
         }
         else
         {
            _stprintf(szTmp, _T("%co"), STX);     // turn Dual Mode Off
            StartupStore(_T(". KRT2  Dual off %s"), NEWLINE);
         }    
          for (i=0; i < 2; i++)
            d->Com->PutChar(szTmp[i]);
      }
  return(TRUE);
}


BOOL KRT2RequestAllData(PDeviceDescriptor_t d) {
  TCHAR  szTmp[255];

  LockComm();
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
        d->Com->WriteString(szTmp);
  UnlockComm();
  return(TRUE);
}


BOOL KRT2ParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
//(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *info)
{
int i;
int tmp =0;
int processed =0;
static int Recbuflen =0;

if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;

#define REC_BUFSIZE 180
 RadioPara.Changed = false;
 static char  converted[REC_BUFSIZE];
 for (i=0; i < len; i++)
 {
    LKASSERT(Recbuflen < REC_BUFSIZE);        
     converted[Recbuflen++] =  String[i];     
     converted[Recbuflen] =0;
 }
processed = KRT2_Convert_Answer(d, converted , Recbuflen);

if(processed >0)
{
  RadioPara.Changed = true;

  tmp  = Recbuflen-processed;
  LKASSERT(tmp< REC_BUFSIZE);        
  LKASSERT(tmp>= 0);          
  for (i=0; i < tmp; i++)
     converted[i] =   converted[processed+i];
  Recbuflen = tmp;
}

  //    StartupStore(_T(". KRT2 %s %s"), String, NEWLINE);

        
return  RadioPara.Changed;


}




/*****************************************************************************
 * this function converts a KRT answer sting to a NMEA answer
 *
 * szAnswer       NMEA Answer
 * Answerlen      number of valid characters in the NMEA answerstring
 * szCommand      KRT2 binary code to be converted
 * len            length of the KRT2 binary code to be converted
 ****************************************************************************/
int KRT2_Convert_Answer(DeviceDescriptor_t *d, char *szCommand, int len)
{   
if(d == NULL) return 0;
if(szCommand == NULL) return 0;
if(len == 0)          return 0;
//char szAnswer[180];
TCHAR szTempStr[180] = _T("");
TCHAR szMessage[180] = _T("");
double  fTmp  =0.0;
int processed=0;
static int iDetected = 0;
static bool bFound = false;
static int iInvalidCount =0;


int i;
LKASSERT(szCommand !=NULL);   
LKASSERT(d !=NULL);   

    if(szCommand[0] == 'S')
    {
        d->Com->WriteString(_T("x"));        
        if(bFound == false)
        {
          bFound = true;
          iDetected++;
          if(iDetected < 10)
            DoStatusMessage(gettext(TEXT("RADIO DETECTED"))); // RADIO DETECTED
          else
              if(iDetected == 10)
                 DoStatusMessage(gettext(TEXT("Radio Message disabled"))); 
   
        }
        else
        {            
         //   bFound = false;
        }
      processed++;
    }
    else
    if(szCommand[0] == ACK)
    {
      processed++;
    }
    else
    if(szCommand[0] == NAK)
    {
#ifdef RESEND_ON_NAK
      if(  iReSendCount++ < 5)
      {
       /****************************************************************
        * here we try a second transmission of the last command
        ****************************************************************/
        #ifdef DEBUG_OUTPUT

        #endif
        iReSendCount++;
//        SendDataBufferISR(lastComPort, szLastCommandSend, strlen(szLastCommandSend));
      }
#else
      _stprintf(szTempStr,_T("$PVCOM,A,ERR,FAILED"));
#endif
      processed++;
    }
    else
    if(szCommand[0] == STX)
    {
      processed++;
      if(len > 1)
      {
      processed++;
      iInvalidCount =0;
      switch (szCommand[1])
      {

          case 'U':
            if(len >= 13)
            {
              if(szCommand[12] != (szCommand[2] ^ szCommand[3]))
                  DoStatusMessage(_T("Checksum Fail"));               
              {               
                RadioPara.ActiveFrequency=  ((double)(unsigned char)szCommand[2]) + ((double)(unsigned char)szCommand[3])/ 200.0;                
                for(i=0; i < 8; i++)
                  RadioPara.ActiveName[i] =   szCommand[4+i]; 
                RadioPara.ActiveName[8] =0;
                   _stprintf(szTempStr,_T("Active: %s %7.3fMHz"),  RadioPara.ActiveName,RadioPara.ActiveFrequency );
                processed = 13;
              }
            } else processed=0;
          break;

          case 'R':
            if(len >= 13)
            {
              if(szCommand[12] != (szCommand[2] ^ szCommand[3]))
                  DoStatusMessage(_T("Checksum Fail"));               
              {
                RadioPara.PassiveFrequency =  ((double)(unsigned char)szCommand[2]) + ((double)(unsigned char)szCommand[3])/ 200.0;
                for(i=0; i < 8; i++)
                  RadioPara.PassiveName[i] =   szCommand[4+i]; 
                RadioPara.PassiveName[8] =0;                 
                _stprintf(szTempStr,_T("Passive: %s %7.3fMHz"),  RadioPara.PassiveName,RadioPara.PassiveFrequency );
                processed = 13;
              }
            } else processed=0;
          break;

          case 'A':
            if(len >= 6)
            {
              if( szCommand[5] != (szCommand[3]+ szCommand[4]))
                  DoStatusMessage(_T("Checksum Fail"));               
              {
                if(RadioPara.Volume != (int)szCommand[2])
                {
                  RadioPara.Volume = (int)szCommand[2];
                  _stprintf(szTempStr,_T("%s %i "),  gettext(TEXT("_@M2310_")), RadioPara.Volume );  // _@M2310_ Vol
                }
                if( RadioPara.Squelch != (int)szCommand[3])
                {
                  RadioPara.Squelch  = (int)szCommand[3];
                  _stprintf(szTempStr,_T("%s %i "),  gettext(TEXT("_@M2311_")), RadioPara.Squelch );  //_@M2311_ Sqw
                }
                if(RadioPara.Vox != (int)szCommand[4])
                {
                  RadioPara.Vox= (int)szCommand[4];
                  _stprintf(szTempStr,_T("Vox %i "),   RadioPara.Vox );

                }
                processed = 6;
              }
            } else processed =0;
          break;
          case 'C':
             if(len >= 2)
            {
                fTmp =   RadioPara.ActiveFrequency;
                RadioPara.ActiveFrequency = RadioPara.PassiveFrequency;
                RadioPara.PassiveFrequency=  fTmp;
                _tcscpy( szTempStr,  RadioPara.ActiveName);
                _tcscpy(  RadioPara.ActiveName, RadioPara.PassiveName);
                _tcscpy(  RadioPara.PassiveName, szTempStr);
                 _stprintf(szTempStr,_T("Swap "));

            }
          break;
          case 'O':             
             if(len >= 2)        
             {
               RadioPara.Dual = true;             
               _stprintf(szTempStr,_T("Dual ON "));
             }
          break;
          case 'o':
             if(len >= 2)            
             {
               RadioPara.Dual = false;             
               _stprintf(szTempStr,_T("Dual OFF "));

             }
          break;      
          case '8':  _stprintf(szTempStr,_T("STA,8_33KHZ"));
          break;
          case '6':  _stprintf(szTempStr,_T("STA,25KHZ"));
          break;
          case '1': _stprintf(szTempStr,_T("SIDETONE"));
          break;
          case '2': _stprintf(szTempStr,_T("STA,ARBIT"));
          break;
          case '3': _stprintf(szTempStr,_T("INTERCOM"));
          break;
          case '4': _stprintf(szTempStr,_T("EXT_AUD")); 
          break;

          /**********************************************************************************/
          
          case 'B':  _stprintf(szTempStr,_T("BAT_LOW"));
          break;
          case 'D':  _stprintf(szTempStr,_T("STA,BAT_OK")); 
          break;          
          case 'J':  _stprintf(szTempStr,_T("STA,RX_ON"));  
          break;
          case 'V':  _stprintf(szTempStr,_T("RX_OFF"));  
          break;
          case 'K':  _stprintf(szTempStr,_T("TX_ON"));  
          break;
          case 'L': _stprintf(szTempStr,_T("TE_ON"));  
          break;

          case 'Y': _stprintf(szTempStr,_T("RX_TX_OFF"));   
          break;
          case 'M':  _stprintf(szTempStr,_T("RX_AF")); 
          break;
          case 'm': _stprintf(szTempStr,_T("RX_SF"));  
          break;
          case 'E': _stprintf(szTempStr,_T("STX_E"));  
          break;
          case 'H': _stprintf(szTempStr,_T("STX_H"));  
          break;

          /*************************************************************************************/
          case 'e':  _stprintf(szTempStr,_T("ERR,PLL"));   
          break;
          case 'F': _stprintf(szTempStr,_T("ERR,RELEASE")); 
          break;
          case 'a': _stprintf(szTempStr,_T("ERR,ADC"));  
          break;
          case 'b': _stprintf(szTempStr,_T("ERR,ANT"));   
          break;
          case 'c': _stprintf(szTempStr,_T("ERR,FPA"));   
          break;
          case 'd': _stprintf(szTempStr,_T("ERR,FUSE"));  
          break;
          case 'f': _stprintf(szTempStr,_T("ERR,KEYBLOCK"));   
          break;
          case 'g':_stprintf(szTempStr,_T("ERR,I2C"));   
          break;
          case 'h': _stprintf(szTempStr,_T("ERR,ID10"));    
          break;
          default:
            _stprintf(szTempStr,_T("ERR,UNKNOWN COMMAND 0x%X "),(int)szCommand[1]);    

          break;          
      }
      }
      else
      {             /* try up to 30 times this is getting a valid command */

 // processed=0;        
        if( iInvalidCount++ < 30)
          processed=0;
        else        /* no! remove the  fragment in the  message queue */
        {
          #ifdef  DEBUG_OUTPUT
              _stprintf(szTempStr,_T("ERR, PIPE CLEARED 0x%2X%2X "),(int)szCommand[0], (int)szCommand[1]);
          #endif
        }
      }
    }
    else
    {
      processed++; // skip invalid char
      #ifdef  DEBUG_OUTPUT
         _stprintf(szTempStr,_T("ERR,UNKNOWN CHARACTER 0x%X"),(int)szCommand[0]);
      #endif
    }


if(processed> 0)
{    
        _stprintf(szMessage,_T("%s:%s "),gettext(TEXT("_@M2309_")),szTempStr);
        StartupStore(_T(" %s %s%s"), szMessage,WhatTimeIsIt(),NEWLINE);
#ifdef TESTBENCH            
        DoStatusMessage(szMessage); 
#endif        
}

 //   if(processed > 0) /* if a valid Answer:  */

    return processed;  /* return the number of converted characters */
}
#endif  // RADIO_ACTIVE        