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

#ifdef RADIO_ACTIVE



#define FRAME_LEN        0x04
#define HEADER_LEN     0x03
#define COMMAND_IDX  0x02
#define STX                         0x02
#define STX_BYTE             0x02
#define SYNC_BYTE           0x72
#define ACK_BYTE             0x06
#define NACK_BYTE           0x15
#define TIMEOUT               0x03
#define CHECKSUM           0x04
#define UKNWN_MSG       0x05
#define ILLEGAL_PARAM 0x05
#define ACK_NAK


#define DEBUG_OUTPUT        /* switch for debugging output                    */
//#define RESEND_ON_NAK       /* switch for command retry on transmission fail  */


bool Send_Command(PDeviceDescriptor_t d, uint8_t Command, uint8_t Len, uint8_t *uiArg);
int ATR833_Convert_Answer(DeviceDescriptor_t *d, uint8_t *szCommand, int len);

int  iDebugLevel = 1;


BOOL ATR833Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("f.u.n.k.e. ATR833"));
  d->IsRadio = ATR833IsRadio;
  d->PutVolume = ATR833PutVolume;
  d->PutSquelch = ATR833PutSquelch;
  d->PutFreqActive = ATR833PutFreqActive;
  d->PutFreqStandby = ATR833PutFreqStandby;
  d->StationSwap = ATR833StationSwap;
  d->ParseNMEA    = NULL;
  d->ParseStream   = ATR833ParseString;
  d->RadioMode    = ATR833RadioMode;
  RadioPara.Enabled8_33  = true;  
  StartupStore(_T("ATR833 Install & Data Request%s"),  NEWLINE);  
  ATR833RequestAllData(d);

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



bool Send_Command(PDeviceDescriptor_t d, uint8_t Command, uint8_t Len, uint8_t *uiArg)
{
    if(d == NULL) return false;
    if(Len > 0)
       if(uiArg == NULL)
        return false;

     if(iDebugLevel==2)  StartupStore(_T("ATR833 ==== send  ====== %s"),  NEWLINE);      
     if(iDebugLevel==2)   StartupStore(_T("ATR833 0x%02X %s"),  STX_BYTE , NEWLINE);    
     if(iDebugLevel==2)   StartupStore(_T("ATR833 0x%02X %s"),  SYNC_BYTE , NEWLINE);      
     if(iDebugLevel==2)   StartupStore(_T("ATR833 0x%02X %s"),  Command , NEWLINE);      
    d->Com->PutChar(STX_BYTE);
    d->Com->PutChar(SYNC_BYTE);
    d->Com->PutChar(Command);
    uint8_t uiCheckSum =SYNC_BYTE^Command;
    for(int i=0; i < Len; i++)
    {
       if(iDebugLevel) StartupStore(_T("ATR833 0x%02X %s"),  uiArg[i] , NEWLINE);           
       d->Com->PutChar( uiArg[i]); 
       uiCheckSum^= uiArg[i];
       if(uiArg[i] == STX_BYTE) // resend on STX occurance
       {
          d->Com->PutChar( uiArg[i]); 
          uiCheckSum ^= uiArg[i]; 
       }
       
    }
    d->Com->PutChar(uiCheckSum);

    if(iDebugLevel==2) StartupStore(_T("ATR833 0x%02X %s"),  uiCheckSum , NEWLINE);     
    if(iDebugLevel==2)  StartupStore(_T("ATR833 ==== send end   ====== %s"),   NEWLINE);     
    return true;
}

bool Send_ACK(PDeviceDescriptor_t d, uint8_t Command)
{
    if(d == NULL) return false;
#ifdef ACK_NAK    
    d->Com->PutChar(STX_BYTE);
    d->Com->PutChar(SYNC_BYTE);
    d->Com->PutChar(ACK_BYTE);
    d->Com->PutChar(Command);
#endif    
    return true;
}


bool Send_NACK(PDeviceDescriptor_t d, uint8_t Command, int reason)
{  
    if(d == NULL) return false; 
#ifdef ACK_NAK    
    d->Com->PutChar(STX_BYTE);    // STX
    d->Com->PutChar(SYNC_BYTE);   // Sync
    d->Com->PutChar(NACK_BYTE);    //NAK
    d->Com->PutChar(Command);
    d->Com->PutChar(reason);
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
         if(iDebugLevel) StartupStore(_T(". ATR833 Send Volume %ui %s"), Val,NEWLINE);
      }
  return(TRUE);
}




BOOL ATR833PutSquelch(PDeviceDescriptor_t d, int Squelch) {
uint8_t Val = (uint8_t) Squelch;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         Send_Command( d, 0x16 , 1, &Val);
         RadioPara.Squelch = Squelch;
         if(iDebugLevel) StartupStore(_T(". ATR833 Send Squelch %ui %s"), Val,NEWLINE);
      }
  return(TRUE);
}



BOOL ATR833PutFreqActive(PDeviceDescriptor_t d, double Freq, TCHAR StationName[]) {
uint8_t Arg[2];

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        Arg[0] = (int) Freq;
        Arg[1] =((Freq- (double)Arg[0] ) *1000.0) /5;
        Send_Command( d, 0x13 , 2, Arg);  // Send Activ
        RadioPara.ActiveFrequency =  Freq;
        if(iDebugLevel) StartupStore(_T(". ATR833 Active Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);
      }
  return(TRUE);
}


BOOL ATR833PutFreqStandby(PDeviceDescriptor_t d, double Freq,  TCHAR StationName[]) {
uint8_t Arg[2];


  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        Arg[0] = (int) Freq;
        Arg[1] =((Freq- (double)Arg[0] ) *1000.0) /5;
        Send_Command( d, 0x12 , 2, Arg);  // Send Activ      
        RadioPara.PassiveFrequency =  Freq;
        if(StationName != NULL)
          _stprintf(RadioPara.PassiveName  ,_T("%s"),StationName) ;
         if(iDebugLevel) StartupStore(_T(". ATR833 Standby Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);

      }
  return(TRUE);
}


BOOL ATR833StationSwap(PDeviceDescriptor_t d) {
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
           Send_Command( d, 0x11 , 0, NULL);  // Send Activ      
           if(iDebugLevel) StartupStore(_T(". ATR833  station swap %s"), NEWLINE);
      }
  return(TRUE);
}


BOOL ATR833RadioMode(PDeviceDescriptor_t d, int mode) {
    uint8_t Val = (uint8_t) mode;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
         Send_Command( d, 0x19 , 1, &Val);  // Send Activ        
           if(iDebugLevel) StartupStore(_T(". ATR833  Dual %ui  %s"), Val, NEWLINE);
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
//(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *info)
//(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *info)
{
uint16_t cnt=0;
static int Recbuflen =0;
static uint8_t uiChecsum;
static bool STXMode = false;
static uint16_t CommanLength =0;
if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;


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
            case 0x12: CommanLength=2  ; break;         // Standba Freq
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
            case 0x42: CommanLength=12; break;          // All Data
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
           if (iDebugLevel==2) StartupStore(_T("ATR833 ACK Command 0x%02X   %s"),  converted[3]   , NEWLINE);  
           return true;
       }
       
       if(converted[COMMAND_IDX]  == NACK_BYTE)
       {   // No-Acknolage Handler here      
           if (iDebugLevel==2) StartupStore(_T("ATR833 NO-ACK Command 0x%02X  cause  0x%02X !!! %s"),  converted[3]   , converted[4],  NEWLINE);  
           Recbuflen =0;
           return true;
       }      
     }
     /***************************************************************************/  
    if (Recbuflen == (CommanLength+FRAME_LEN))  // Command len + Header Length + CRC Byte
    {      
      if (iDebugLevel==2) StartupStore(_T("ATR833 ==== receive  ====== %s"), NEWLINE);      
      if (iDebugLevel==2) for (int i=0; i < Recbuflen; i++)    StartupStore(_T("ATR833 Received  0x%02X %s"),  converted[i] , NEWLINE);      
      if(converted[CommanLength+3] == uiChecsum)
      {
         ATR833_Convert_Answer(d, (uint8_t*)&converted[COMMAND_IDX] , Recbuflen-FRAME_LEN +1);
         Send_ACK(d, converted[COMMAND_IDX] );
      }
      else
      {
          if (iDebugLevel) StartupStore(_T("ATR833 CHECKSUM ERR  Command 0x%02X  0x%02X  %s"),  converted[CommanLength+3] , uiChecsum, NEWLINE);      
         Send_NACK(d, converted[COMMAND_IDX] , CHECKSUM); 
      }
      Recbuflen =0;
       if (iDebugLevel==2) StartupStore(_T("ATR833 ==== end ===== %s"),   NEWLINE);      
    }  
    uiChecsum  ^= String[cnt++];
 }
return true;
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
 double fTmp;

  switch (szCommand[0])
  {  
      /*****************************************************************************************/
      case 0x11:               // Swap Frequency           
          RadioPara.Changed = true;     
           fTmp =  RadioPara.PassiveFrequency;
          RadioPara.PassiveFrequency =  RadioPara.ActiveFrequency;          
          RadioPara.ActiveFrequency = fTmp;           
          _stprintf(RadioPara.PassiveName,_T("        ")) ;   
          _stprintf(RadioPara.ActiveName,_T("        ")) ;   
         if (iDebugLevel) StartupStore(_T("ATR833 Swap %s"),    NEWLINE);
         processed  = 2;                             
      break;      
      /*****************************************************************************************/
      case 0x12:               // Standby Frequency                      
          RadioPara.PassiveFrequency = (double)szCommand[1] +((double) szCommand[2] * 5.0 / 1000.0);                      
          _stprintf(szTempStr,_T("ATR833 Passive: %7.3fMHz"),  RadioPara.PassiveFrequency );
          if (iDebugLevel)  StartupStore(_T(" %s %s"),szTempStr, NEWLINE);                       
          _stprintf(RadioPara.PassiveName,_T("        ")) ;   
          RadioPara.Changed = true;     
         processed  = 3;                             
      break;
      /*****************************************************************************************/
      case 0x13:               // Active Frequency
         RadioPara.ActiveFrequency = (double) szCommand[1] +((double) szCommand[2] * 5.0 /1000.0);
         _stprintf(szTempStr,_T("ATR833 Active:  %7.3fMHz"),  RadioPara.ActiveFrequency );
          if (iDebugLevel)StartupStore(_T(" %s %s"),szTempStr, NEWLINE);
         _stprintf(RadioPara.ActiveName  ,_T("        ")) ;     
         RadioPara.Changed = true;                     
         processed  = 3;                         
      break;                         
      /*****************************************************************************************/
      case 0x14:               // Intercom        
          if (iDebugLevel) StartupStore(_T("ATR833 Intercon %s"),    NEWLINE);
          processed  = 2;
      break; 
      /*****************************************************************************************/
      case 0x16:               // Volume                                
         RadioPara.Volume = szCommand[1] ;
         RadioPara.Changed = true;
         if (iDebugLevel) StartupStore(_T("ATR833 Volume %i %s"),   RadioPara.Volume, NEWLINE);
         processed  = 2;                   
      break; 
      /*****************************************************************************************/    
      case 0x17:               // Squelch                          
         RadioPara.Squelch = szCommand[1] ;
         RadioPara.Changed = true;
         if (iDebugLevel) StartupStore(_T("ATR833 Squelch %i %s"),   RadioPara.Squelch, NEWLINE);      
         processed  = 2;                   
      break; 
      /*****************************************************************************************/    
      case 0x18:               // Vox            
         RadioPara.Vox = szCommand[1] ;
         RadioPara.Changed = true;          
         if (iDebugLevel) StartupStore(_T("ATR833 Vox %i %s"),   RadioPara.Vox , NEWLINE);
         processed  = 2;                   
      break; 
       /*****************************************************************************************/        
       case 0x19:               // Dual          
         RadioPara.Dual = szCommand[1] ;
         if (iDebugLevel) StartupStore(_T("ATR833 Dual %i %s"),   RadioPara.Dual, NEWLINE);
         processed  = 2;                   
      break; 
      /*****************************************************************************************/        
        case 0x1A:               // NF          
         RadioPara.Dual = szCommand[1] ;
          RadioPara.Changed = true;
         if (iDebugLevel) StartupStore(_T("ATR833 NF %i %s"),   RadioPara.Volume, NEWLINE);
         processed  = 2;                   
      break; 
      /*****************************************************************************************/       
        case 0x40:               // TxRx  
         processed  = 2;                   
      break; 
      /*****************************************************************************************/      
        case 0x41:               // ErrorStatus   
         processed  = 2;                
      break; 
      /*****************************************************************************************/      
        case 0x42:               // All Data  
             RadioPara.ActiveFrequency  = (double) szCommand[1] +((double) szCommand[2] * 5.0 /1000.0);
             RadioPara.PassiveFrequency = (double)szCommand[3] +((double) szCommand[4] * 5.0 / 1000.0);    
             RadioPara.Volume  = szCommand[5] ;
             RadioPara.Squelch = szCommand[6] ;       
             RadioPara.Vox = szCommand[7];    
             if(szCommand[11]==1)     
               RadioPara.Enabled8_33  = false;  
             else
               RadioPara.Enabled8_33  = true;  
             RadioPara.Dual = szCommand[12];       
             RadioPara.Changed = true;           
             if (iDebugLevel) StartupStore(_T("All Data %s"),  NEWLINE);   
         processed  = 13;                   
      break; 
      /*****************************************************************************************/      
        case 0x43:               // Device Information  
         processed  = 2;                   
      break; 
      /*****************************************************************************************/      
      
      default:
          if (iDebugLevel) StartupStore(_T("ATR833 unknown Command %02X %s"),  szCommand[0], NEWLINE);
          processed  = 0;
      break;
   }  // case
   return processed;  /* return the number of converted characters */
}
#endif  // RADIO_ACTIVE
