/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Globals.h"
#include "devATR833.h"
#include "device.h"
#include "Radio.h"

#define FRAME_LEN      0x04
#define HEADER_LEN     0x03
#define COMMAND_IDX    0x02
#define STX            0x02
#define STX_BYTE       0x02
#define SYNC_BYTE      0x72
#define ACK_BYTE       0x06
#define NACK_BYTE      0x15
#define TIMEOUT        0x03
#define CHECKSUM       0x04
#define UKNWN_MSG      0x05
#define ILLEGAL_PARAM  0x05
#define ACK_NAK

int MsgCnt=0;
int DeviceTimeout = 2;
#define DEBUG_OUTPUT        /* switch for debugging output                    */
//#define RESEND_ON_NAK       /* switch for command retry on transmission fail  */
#define BIT(n) (1 << (n))

bool Send_Command(PDeviceDescriptor_t d, uint8_t Command, uint8_t Len, uint8_t *uiArg);
int ATR833_Convert_Answer(DeviceDescriptor_t *d, uint8_t *szCommand, int len);
#ifdef TESTBENCH
int  iATR833DebugLevel = 2;
#else
int  iATR833DebugLevel = 1;
#endif



BOOL ATR833_KeepAlive(PDeviceDescriptor_t d) ;

BOOL ATR833Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("f.u.n.k.e. ATR833"));
  d->IsRadio        = ATR833IsRadio;
  d->PutVolume      = ATR833PutVolume;
  d->PutSquelch     = ATR833PutSquelch;
  d->PutFreqActive  = ATR833PutFreqActive;
  d->PutFreqStandby = ATR833PutFreqStandby;
  d->StationSwap    = ATR833StationSwap;
  d->ParseStream    = ATR833ParseString;
  d->PutRadioMode   = ATR833RadioMode;
  d->HeartBeat      = ATR833_KeepAlive;  // called every 5s from UpdateMonitor to keep in contact with the ATR833
  RadioPara.Enabled8_33  = true;  

  return(TRUE);
}

BOOL ATR833Register(void){
  return(devRegister(
    TEXT("f.u.n.k.e. ATR833"),
    (1l << dfRadio),
    ATR833Install
  ));
}




BOOL ATR833IsRadio(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}



bool Send_Command(PDeviceDescriptor_t d, uint8_t Command, uint8_t Length, uint8_t *uiArg)
{
uint8_t szTmp[128] ;
int len =0;
uint8_t uiCheckSum=0;

  if(d == NULL) return false;
  if(Length > 0)
     if(uiArg == NULL)
      return false;


  szTmp[len++]= STX_BYTE ;
  szTmp[len++]= SYNC_BYTE;
  szTmp[len++]= Command  ;
  uiCheckSum = SYNC_BYTE ^ Command;
  for(int i=0; i < Length; i++)
  {
    szTmp[len]= (uint8_t) uiArg[i] ; uiCheckSum ^= szTmp[len++];
    if(uiArg[i] == STX_BYTE) // resend on STX occurance
    {
      szTmp[len]= (uint8_t) uiArg[i] ; uiCheckSum ^= szTmp[len++];
    }
  }
  szTmp[len++]= uiCheckSum;
  d->Com->Write(szTmp,len);

  if(iATR833DebugLevel==3)
  {
    StartupStore(_T("ATR833 ==== send  ====== %s"),  NEWLINE);
    for(int i = 0; i < (len-1); i++)
    {
      StartupStore(_T("ATR833 0x%02X %s"),  szTmp[i] , NEWLINE);
    }
    StartupStore(_T("ATR833 CRC 0x%02X %s"),  szTmp[len-1] , NEWLINE);
    StartupStore(_T("ATR833 ==== send end   ====== %s"),   NEWLINE);
  }
  return true;
}


bool Send_ACK(PDeviceDescriptor_t d, uint8_t Command)
{
  if(d == NULL) return false;
#ifdef ACK_NAK    
  BYTE szTmp[4] = {STX_BYTE, SYNC_BYTE, ACK_BYTE, Command};
  d->Com->Write(szTmp,4);
#endif    

  return true;
}


bool Send_NACK(PDeviceDescriptor_t d, uint8_t Command, uint8_t reason)
{  
  if(d == NULL) return false;
#ifdef ACK_NAK    
  BYTE szTmp[5] = {STX_BYTE, SYNC_BYTE, NACK_BYTE, Command, reason };
  d->Com->Write(szTmp,5);
#endif    
   return true;
}



BOOL ATR833PutVolume(PDeviceDescriptor_t d, int Volume) {
uint8_t Val = (uint8_t) Volume;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         Send_Command( d, 0x16 , 1, &Val);
         RadioPara.Volume= Volume;
         if(iATR833DebugLevel) StartupStore(_T(". ATR833 Send Volume %u %s"), Val,NEWLINE);
      }
  return(TRUE);
}




BOOL ATR833PutSquelch(PDeviceDescriptor_t d, int Squelch) {
uint8_t Val = (int8_t) Squelch;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         Send_Command( d, 0x17 , 1, &Val);
         RadioPara.Squelch = Squelch;
         if(iATR833DebugLevel) StartupStore(_T(". ATR833 Send Squelch %u %s"), Val,NEWLINE);
      }
  return(TRUE);
}



BOOL ATR833PutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) {
static double oldFreq=0;
if(oldFreq == Freq) return true;
oldFreq = Freq;
uint8_t Arg[2];

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        Arg[0] = (int) Freq;
        Arg[1] =((int)((Freq*1000.0)+0.5)- Arg[0]  *1000) /5;

        Send_Command( d, 0x13 , 2, Arg);  // Send Activ
        RadioPara.ActiveFrequency =  Freq;
        if(iATR833DebugLevel) StartupStore(_T(". ATR833 Active Station %7.3fMHz %i.%03i %s%s"), Freq, Arg[0], Arg[1]*5, StationName,NEWLINE);
      }
  return(TRUE);
}


BOOL ATR833PutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) {
uint8_t Arg[2];

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        Arg[0] = (int) Freq;
        Arg[1] =((int)((Freq*1000.0)+0.5)- Arg[0]  *1000) /5;
        Send_Command( d, 0x12 , 2, Arg);  // Send Activ      
        RadioPara.PassiveFrequency =  Freq;
        if(StationName != NULL)
          _sntprintf(RadioPara.PassiveName  ,NAME_SIZE,_T("%s"),StationName) ;
         if(iATR833DebugLevel) StartupStore(_T(". ATR833 Standby Station %7.3fMHz %i.%03i %s%s"), Freq, Arg[0] , Arg[1]*5,  StationName,NEWLINE);
      }
  return(TRUE);
}


BOOL ATR833StationSwap(PDeviceDescriptor_t d) {
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        Send_Command( d, 0x11 , 0, NULL);  // Send Activ
        if(iATR833DebugLevel) StartupStore(_T(". ATR833 send station swap %s"), NEWLINE);
      }
  return(TRUE);
}


BOOL ATR833RadioMode(PDeviceDescriptor_t d, int mode) {
 uint8_t Val= (int8_t) mode;

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        Send_Command( d, 0x19 , 1, &Val);  // Send Activ
        if(iATR833DebugLevel) StartupStore(_T(". ATR833  send Dual %u  %s"), Val, NEWLINE);
      }
  return(TRUE);
}



BOOL ATR833_KeepAlive(PDeviceDescriptor_t d) {

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         if(DeviceTimeout < 10)
           DeviceTimeout++;

         if(DeviceTimeout == 4)
         {
           StartupStore(_T("ATR833 No Response !%s"),  NEWLINE);
           if(MsgCnt++ < 10) DoStatusMessage( _T(" ATR833"), MsgToken(959)); // ATR833T OFF
         }
         Send_Command( d, 0x10 , 0, NULL);  // Send Keep alive
         if(iATR833DebugLevel==2)  StartupStore(_T("ATR833 ====== Keep Alive ====== %s"),  NEWLINE);
      }
  return(TRUE);
}


BOOL ATR833RequestAllData(PDeviceDescriptor_t d) {

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
         Send_Command( d, 0x82 , 0, NULL);  // Request all infos

  return(TRUE);
}




BOOL ATR833ParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
{
uint16_t cnt=0;
static int Recbuflen =0;
static uint8_t uiChecsum;
static bool STXMode = false;
static uint16_t CommanLength =0;
if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;


if (iATR833DebugLevel==4)
{
StartupStore(_T("ATR833 ====== receive ======%s"), NEWLINE);
for(int i=0; i < len; i++)
  StartupStore(_T("0x%02X%s"),  (uint8_t)String[i]   , NEWLINE);
}
#define REC_BUFSIZE 128
static uint8_t  converted[REC_BUFSIZE];

  while (cnt < len )
  {
    switch ((uint8_t)String[cnt])
    {
      case STX_BYTE:
        if(STXMode)  {
          Recbuflen--;   STXMode = false;
        }
        else
          STXMode = true;
      break;
      case SYNC_BYTE:
        if (STXMode)
        {
          STXMode = false;
          Recbuflen =0;
          converted[Recbuflen++] = STX;
          uiChecsum = 0;
        }
      break;
    }
    LKASSERT(Recbuflen < REC_BUFSIZE);
   
    converted[Recbuflen++] =  (uint8_t)String[cnt];

    if(Recbuflen == (COMMAND_IDX+1))
    {
      switch (converted[COMMAND_IDX])
      {
        case 0x10: CommanLength=0  ; break;         // keep ALive
        case 0x11: CommanLength=0  ; break;         // Swap
        case 0x12: CommanLength=2  ; break;         // Standby Freq
        case 0x13: CommanLength=2  ; break;         // Active Freq
        case 0x14: CommanLength=1  ; break;         // Intercom
        case 0x16: CommanLength=1  ; break;         // Volume
        case 0x17: CommanLength=1  ; break;         // Squelch
        case 0x18: CommanLength=1  ; break;         // Vox
        case 0x19: CommanLength=1  ; break;         // Daual watch
        case 0x1A: CommanLength=1  ; break;         // External
        /******************************************/
        case 0x40: CommanLength=1  ; break;         // TxRx State
        case 0x41: CommanLength=1  ; break;         // Erroe State
        case 0x42: CommanLength=12 ; break;         // All Data
        case 0x43: CommanLength=4  ; break;         // Device Info
        /*****************************************/
        case 0x80: CommanLength=0  ; break;         // Request RxTx
        case 0x81: CommanLength=0  ; break;         // Request Error
        case 0x82: CommanLength=0  ; break;         // Request All Data
        case 0x83: CommanLength=0  ; break;         // Request Device Info
        case 0x84: CommanLength=0  ; break;         // Brightness (DIM)
         /*****************************************/
        case 0x06: CommanLength=1  ; break;         // ACK Handler
        case 0x15: CommanLength=2  ; break;         // NACK Handler
        default:
        break;
      }
    }
    /***************************************************************************/
     if(Recbuflen == (CommanLength+FRAME_LEN-1) )
     {
       if(converted[COMMAND_IDX]  == ACK_BYTE)
       { // Acknolage Handler here
         Recbuflen =0;
         if (iATR833DebugLevel==2) StartupStore(_T("ATR833 ACK Command 0x%02X   %s"),  converted[3]   , NEWLINE);
         return true;
       }
       
       if(converted[COMMAND_IDX]  == NACK_BYTE)
       {   // No-Acknolage Handler here      
         if (iATR833DebugLevel==2) StartupStore(_T("ATR833 NO-ACK Command 0x%02X  cause  0x%02X !!! %s"),  converted[3]   , converted[4],  NEWLINE);
         Recbuflen =0;
         return true;
       }      
     }
     /***************************************************************************/  
    if (Recbuflen == (CommanLength+FRAME_LEN))  // Command len + Header Length + CRC Byte
    {      
      if (iATR833DebugLevel==3) StartupStore(_T("ATR833 ==== receive  ====== %s"), NEWLINE);
      if (iATR833DebugLevel==3) for (int i=0; i < Recbuflen; i++)    StartupStore(_T("ATR833 Received  0x%02X %s"),  converted[i] , NEWLINE);
      if(converted[CommanLength+3] == uiChecsum)
      {
        ATR833_Convert_Answer(d, (uint8_t*)&converted[COMMAND_IDX] , Recbuflen-FRAME_LEN +1);
        Send_ACK(d, converted[COMMAND_IDX] );
      }
      else
      {
        if (iATR833DebugLevel) StartupStore(_T("ATR833 CHECKSUM ERR  Command 0x%02X  0x%02X  %s"),  converted[CommanLength+3] , uiChecsum, NEWLINE);
        Send_NACK(d, converted[COMMAND_IDX] , CHECKSUM);
      }
      Recbuflen =0;
      if (iATR833DebugLevel==3) StartupStore(_T("ATR833 ==== end ===== %s"),   NEWLINE);
    }  
    uiChecsum  ^= String[cnt++];
  }
return RadioPara.Changed;
}



/*****************************************************************************
 * this function converts a KRT answer sting to a NMEA answer
 *
 * szAnswer       NMEA Answer
 * Answerlen      number of valid characters in the NMEA answerstring
 * szCommand      ATR833 binary code to be converted
 * len            length of the ATR833 binary code to be converted
 ****************************************************************************/
int ATR833_Convert_Answer(DeviceDescriptor_t *d, uint8_t *szCommand, int len)
{
if(d == NULL) return 0;
if(szCommand == NULL) return 0;
if(len == 0)          return 0;
TCHAR szTempStr[180] = _T("");
uint16_t processed=0;
LKASSERT(szCommand !=NULL);
LKASSERT(d !=NULL);

  switch (szCommand[0])
  {  
    case 0x10:               // keep alive
      RadioPara.Changed = true;
      if(DeviceTimeout > 2)
      {        
        ATR833RequestAllData(d);
        StartupStore(_T("ATR833 detected! %s"),  NEWLINE);
        if(MsgCnt++ < 10) DoStatusMessage( _T(" ATR833"), MsgToken(958)); // _@M947_ "found ATR833T"
      }
      DeviceTimeout = 0;
      processed  = 1;
    break;
    /*****************************************************************************************/
    case 0x11:               // Swap Frequency
      RadioPara.Changed = true;
      std::swap(RadioPara.PassiveFrequency, RadioPara.ActiveFrequency);
      std::swap(RadioPara.PassiveName, RadioPara.ActiveName);
      RadioPara.ActiveValid = false;
      RadioPara.PassiveValid = false;
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Swap %s"),    NEWLINE);
      processed  = 1;
    break;
    /*****************************************************************************************/
    case 0x12:               // Standby Frequency
      RadioPara.PassiveFrequency = (double)szCommand[1] +(((double) szCommand[2] * 5.0) / 1000.0);
      _stprintf(szTempStr,_T("ATR833 Passive: %7.3fMHz"),  RadioPara.PassiveFrequency );
      if (iATR833DebugLevel)  StartupStore(_T("%s %s"),szTempStr, NEWLINE);
      UpdateStationName(RadioPara.PassiveName, RadioPara.PassiveFrequency);

      if (iATR833DebugLevel)StartupStore(_T("%s %s %s"),szTempStr,RadioPara.PassiveName, NEWLINE);
      RadioPara.PassiveValid = true;
      RadioPara.Changed = true;
      processed  = 3;
    break;
    /*****************************************************************************************/
    case 0x13:               // Active Frequency
      RadioPara.ActiveFrequency = (double) szCommand[1] +(((double) szCommand[2] * 5.0) /1000.0);
      _stprintf(szTempStr,_T("ATR833 Active:  %7.3fMHz"),  RadioPara.ActiveFrequency );
      UpdateStationName(RadioPara.ActiveName, RadioPara.ActiveFrequency);
      if (iATR833DebugLevel)StartupStore(_T("%s %s %s"),szTempStr,RadioPara.ActiveName, NEWLINE);

      RadioPara.Changed = true;
      RadioPara.ActiveValid = true;
      processed  = 3;
    break;
    /*****************************************************************************************/
    case 0x14:               // Intercom
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Intercon %s"),    NEWLINE);
      processed  = 2;
    break;
    /*****************************************************************************************/
    case 0x16:               // Volume
      RadioPara.Volume = szCommand[1] ;
      RadioPara.Changed = true;
      RadioPara.VolValid = true;
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Volume %i %s"),   RadioPara.Volume, NEWLINE);
      processed  = 2;
    break;
    /*****************************************************************************************/
    case 0x17:               // Squelch
      RadioPara.Squelch = szCommand[1] ;
      RadioPara.Changed = true;
      RadioPara.SqValid = true;
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Squelch %i %s"),   RadioPara.Squelch, NEWLINE);
      processed  = 2;
    break;
    /*****************************************************************************************/
    case 0x18:               // Vox
      RadioPara.Vox = szCommand[1] ;
      RadioPara.Changed = true;
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Vox %i %s"),   RadioPara.Vox , NEWLINE);
      processed  = 2;
    break;
     /*****************************************************************************************/
    case 0x19:               // Dual
      RadioPara.Dual = szCommand[1] ;
      RadioPara.Changed = true;
      RadioPara.DualValid = true;
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Dual %i %s"),   RadioPara.Dual, NEWLINE);
      processed  = 2;
    break;
    /*****************************************************************************************/
    case 0x1A:               // NF
   //   RadioPara.Dual = szCommand[1] ;
      RadioPara.Changed = true;
      if (iATR833DebugLevel) StartupStore(_T("ATR833 NF %i %s"),   RadioPara.Volume, NEWLINE);
      processed  = 2;
    break;
    /*****************************************************************************************/
    case 0x40:               // TxRx
      processed  = 2;
      RadioPara.TX        = ((szCommand[1] & BIT(3)) > 0) ? true : false;
      RadioPara.RX_active = ((szCommand[1] & BIT(1)) > 0) ? true : false;
      RadioPara.RX_standy = ((szCommand[1] & BIT(2)) > 0) ? true : false;
      RadioPara.RX        = (RadioPara.RX_active ||   RadioPara.RX_standy );
      RadioPara.Changed = true;
    break;
    /*****************************************************************************************/
    case 0x41:               // ErrorStatus
      processed  = 2;
      if(szCommand[1] >0)   // request data on Error
        ATR833RequestAllData(d);
      StartupStore(_T("ATR833 Error Code 0x%x %s"),   szCommand[1], NEWLINE);

    break;
    /*****************************************************************************************/
    case 0x42:               // All Data
      RadioPara.ActiveFrequency  = (double) szCommand[1] +(((double) szCommand[2] * 5.0) /1000.0);
      RadioPara.PassiveFrequency = (double) szCommand[3] +(((double) szCommand[4] * 5.0) /1000.0);
      RadioPara.Volume  = szCommand[5] ;
      RadioPara.Squelch = szCommand[6] ;
      RadioPara.Vox = szCommand[7];
      if(szCommand[11]==1)
        RadioPara.Enabled8_33  = false;
      else
        RadioPara.Enabled8_33  = true;
      RadioPara.Dual = szCommand[12];
      RadioPara.Changed = true;
      RadioPara.ActiveValid  = true;
      RadioPara.PassiveValid = true;
      RadioPara.VolValid     = true;
      RadioPara.SqValid      = true;
      RadioPara.DualValid    = true;
      if (iATR833DebugLevel) StartupStore(_T("ATR833 received all Data %s"),  NEWLINE);

      _stprintf(szTempStr,_T("ATR833 Active: %7.3fMHz"),  RadioPara.ActiveFrequency );
      UpdateStationName(RadioPara.ActiveName, RadioPara.ActiveFrequency);

      if (iATR833DebugLevel)StartupStore(_T("%s %s %s"),szTempStr,RadioPara.ActiveName, NEWLINE);


      _stprintf(szTempStr,_T("ATR833 Passive: %7.3fMHz"),  RadioPara.PassiveFrequency );
      UpdateStationName(RadioPara.PassiveName, RadioPara.PassiveFrequency);

      if (iATR833DebugLevel)  StartupStore(_T("%s %s %s"),szTempStr,RadioPara.PassiveName, NEWLINE);

      if (iATR833DebugLevel) StartupStore(_T("ATR833 Volume %i %s"),   RadioPara.Volume, NEWLINE);
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Squelch %i %s"),   RadioPara.Squelch, NEWLINE);
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Vox %i %s"),   RadioPara.Vox , NEWLINE);
      if (iATR833DebugLevel) StartupStore(_T("ATR833 Dual %i %s"),   RadioPara.Dual, NEWLINE);
      processed  = 13;
    break;
    /*****************************************************************************************/
    case 0x43:               // Device Information
      processed  = 2;
    break;
    /*****************************************************************************************/
    default:
      if (iATR833DebugLevel) StartupStore(_T("ATR833 unknown Command %02X %s"),  szCommand[0], NEWLINE);
      processed  = 0;
    break;
  }  // case
  return processed;  /* return the number of converted characters */
}
