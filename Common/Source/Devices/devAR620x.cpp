/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Globals.h"
#include "devAR620x.h"
#include "Comm/device.h"
#include "Radio.h"

namespace {

#define BIT(n) (1 << (n))

#define ACTIVE_STATION  1
#define PASSIVE_STATION 0


//#define RESEND_ON_NAK       /* switch for command retry on transmission fail  */
#define RADIO_VOLTAGE       /* read Radio Supply voltage and store it in BATT2 indicator */

constexpr uint8_t HEADER_ID = 0xA5;
#define PROTOKOL_ID     0x14
#define QUERY           BIT(7)
#define DAUL            BIT(8)
#define SQUELCH         BIT(7)

#ifdef TESTBENCH
int iAR620DebugLevel =1;
#else
int iAR620DebugLevel =0;
#endif
typedef union
{
  uint16_t intVal16;
  uint8_t  intVal8[2];
} IntConvertStruct;

IntConvertStruct CRC;
IntConvertStruct sFrequency;
IntConvertStruct sStatus;

#define MAX_CMD_LEN 128


#define  RESEND_DELAY 100

volatile BOOL bSending = false;
//volatile BOOL bReceiving = false;


BOOL AR620xIsRadio(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}

int SendCommand(PDeviceDescriptor_t d, uint8_t szTmp[], uint16_t len)
{

  bSending = true;
  d->Com->Write(szTmp,len);
  Poco::Thread::sleep(100);

  d->Com->Write(szTmp,len);
  Poco::Thread::sleep(100); // don't listen to old status in the pipe
  bSending = false;
  return true;
}

double Idx2Freq(uint16_t uFreqIdx)
{
  unsigned khz = (118.000 + (uFreqIdx& 0xFFF0) * (137.000-118.000)/3040.0) * 1000.;
  switch(uFreqIdx & 0xF) {
      case 0:  khz += 00; break;
      case 1:  khz += 05; break;
      case 2:  khz += 10; break;
      case 3:  khz += 15; break;
      case 4:  khz += 25; break;
      case 5:  khz += 30; break;
      case 6:  khz += 35; break;
      case 7:  khz += 40; break;
      case 8:  khz += 50; break;
      case 9:  khz += 55; break;
      case 10: khz += 60; break;
      case 11: khz += 65; break;
      case 12: khz += 75; break;
      case 13: khz += 80; break;
      case 14: khz += 85; break;
      case 15: khz += 90; break;   
  }
  return khz;
}

unsigned Freq2Idx( unsigned khz) {
  double Mhz = khz / 1000.;
  return ((Mhz - 118.0) * 3040. / (137.00 - 118.0) + 0.5);
}

uint16_t Frq2Idx(unsigned khz) {
  uint16_t  uFreIdx = Freq2Idx(khz) & 0xFFF0;
  uint8_t uiFrac = khz - ((khz / 100) * 100);
  switch(uiFrac) {
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
    default:
      StartupStore(_T("undefined Frequency!  %u -> %u"),uiFrac, uFreIdx);
      break;
  }
  return uFreIdx;
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







/*****************************************************************************
 * this function set the station name and frequency on the AR620x
 *
 * ComPort        index of the comport the AR620x is attached
 * Active_Passive Active or passive station switch
 * fFrequency     station frequency
 * Station        station Name string
 *
 *****************************************************************************/
static int  SetAR620xStation(uint8_t *Command ,int Active_Passive, unsigned khz, const TCHAR* Station)
{
  unsigned int len = 0;
  LKASSERT(Station !=NULL)
  LKASSERT(Command !=NULL)
  if(Command == NULL ) {
    return false;
  }

  IntConvertStruct ActiveFreqIdx;  ActiveFreqIdx.intVal16  = Frq2Idx(RadioPara.ActiveKhz );
  IntConvertStruct PassiveFreqIdx; PassiveFreqIdx.intVal16 = Frq2Idx(RadioPara.PassiveKhz);
  Command [len++] = HEADER_ID ;
  Command [len++] = PROTOKOL_ID ;
  Command [len++] = 5;

  switch (Active_Passive)
  {
    case ACTIVE_STATION:
         RadioPara.ActiveValid = false;
         ActiveFreqIdx.intVal16 = Frq2Idx(khz);
         if(iAR620DebugLevel) StartupStore(_T(">AF:%u  %7.3f"), ActiveFreqIdx.intVal16, (khz / 1000.));
    break;
    default:
    case PASSIVE_STATION:
        RadioPara.PassiveValid = false;
        PassiveFreqIdx.intVal16 =  Frq2Idx(khz);
        if(iAR620DebugLevel) StartupStore(_T(">PF:%u  %7.3f"), PassiveFreqIdx.intVal16, (khz / 1000.));
    break;
  }
  Command [len++] = 22;
  Command [len++] = ActiveFreqIdx.intVal8[1];
  Command [len++] = ActiveFreqIdx.intVal8[0];
  Command [len++] = PassiveFreqIdx.intVal8[1];
  Command [len++] = PassiveFreqIdx.intVal8[0];
  CRC.intVal16 =  CRCBitwise(Command, len);
  Command [len++] = CRC.intVal8[1];
  Command [len++] = CRC.intVal8[0];
  return len;
}





BOOL AR620xPutVolume(PDeviceDescriptor_t d, int Volume) {
#define  AR620x_HAS_VOLUME_CONTROL
#ifdef AR620x_HAS_VOLUME_CONTROL
uint8_t  szTmp[MAX_CMD_LEN];
int len;

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {         
        len = 0;
        szTmp [len++] = HEADER_ID ;
        szTmp [len++] = PROTOKOL_ID ;
        szTmp [len++] =2;
        szTmp [len++] =3;
        szTmp [len++]  =  50-Volume*5;
        CRC.intVal16 =  CRCBitwise(szTmp, len);
        szTmp [len++] = CRC.intVal8[1];
        szTmp [len++] = CRC.intVal8[0];
        SendCommand(d, szTmp,len);
        if(iAR620DebugLevel) StartupStore(_T(". AR620x Volume  %i%s"), Volume,NEWLINE);
        RadioPara.Volume = Volume;
      }
  RadioPara.VolValid = false;
  return(TRUE);
#else
  return false;
#endif
}




BOOL AR620xPutSquelch(PDeviceDescriptor_t d, int Squelch) {
#define	SET_SQUELCH
#ifdef SET_SQUELCH
uint8_t  szTmp[MAX_CMD_LEN];
uint8_t len;
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        len = 0;
        szTmp [len++] = HEADER_ID ;
        szTmp [len++] = PROTOKOL_ID ;
        szTmp [len++] =2;
        szTmp [len++] =4;
        szTmp [len++] = 6 + (Squelch-1)*2;
        CRC.intVal16 =  CRCBitwise(szTmp, len);
        szTmp [len++] = CRC.intVal8[1];
        szTmp [len++] = CRC.intVal8[0];
        SendCommand(d, szTmp,len);
        if(iAR620DebugLevel) StartupStore(_T(". AR620x Squelch  %i%s"), Squelch,NEWLINE);
        RadioPara.Squelch = Squelch;
      }
#endif
  RadioPara.SqValid = false;
  return(TRUE);
}


BOOL AR620xPutFreqActive(PDeviceDescriptor_t d, unsigned khz, const TCHAR* StationName) {
int len;
uint8_t  szTmp[MAX_CMD_LEN];
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        if(iAR620DebugLevel) StartupStore(_T(". AR620x Active Station %7.3fMHz %s%s"), (khz / 1000.), StationName,NEWLINE);
        len = SetAR620xStation(szTmp ,ACTIVE_STATION, khz, StationName);
        SendCommand(d, szTmp,len);
        RadioPara.ActiveKhz = khz;
        if(StationName) {
          _sntprintf(RadioPara.ActiveName, NAME_SIZE, _T("%s"),StationName);
        }
      }
  RadioPara.ActiveValid = false;
  return(TRUE);
}


BOOL AR620xPutFreqStandby(PDeviceDescriptor_t d, unsigned khz,  const TCHAR* StationName) {
int len;
uint8_t  szTmp[MAX_CMD_LEN];
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        len = SetAR620xStation(szTmp ,PASSIVE_STATION, khz, StationName);
        SendCommand(d, szTmp,len);
        RadioPara.PassiveKhz =  khz;
        if(StationName != NULL)
          _sntprintf(RadioPara.PassiveName, NAME_SIZE  ,_T("%s"),StationName) ;
        if(iAR620DebugLevel) StartupStore(_T(". AR620x Standby Station %7.3fMHz %s"), (khz / 1000.), StationName);
      }
  RadioPara.PassiveValid = false;
  return(TRUE);
}


BOOL AR620xStationSwap(PDeviceDescriptor_t d) {
uint8_t len=0;
uint8_t  szTmp[MAX_CMD_LEN];
  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        IntConvertStruct ActiveFreqIdx;  ActiveFreqIdx.intVal16  = Frq2Idx(RadioPara.ActiveKhz );
        IntConvertStruct PassiveFreqIdx; PassiveFreqIdx.intVal16 = Frq2Idx(RadioPara.PassiveKhz);
        szTmp [len++] = HEADER_ID ;
        szTmp [len++] = PROTOKOL_ID ;
        szTmp [len++] = 5;
        szTmp [len++] = 22;
        szTmp [len++] = PassiveFreqIdx.intVal8[1];
        szTmp [len++] = PassiveFreqIdx.intVal8[0];
        szTmp [len++] = ActiveFreqIdx.intVal8[1];
        szTmp [len++] = ActiveFreqIdx.intVal8[0];
        CRC.intVal16 =  CRCBitwise(szTmp, len);
        szTmp [len++] = CRC.intVal8[1];
        szTmp [len++] = CRC.intVal8[0];
        SendCommand(d, szTmp,len);
      }
  RadioPara.ActiveValid = false;
  RadioPara.PassiveValid = false;
  return(TRUE);
}


BOOL AR620xRadioMode(PDeviceDescriptor_t d, int mode) {
uint8_t len;
uint8_t  szTmp[MAX_CMD_LEN];

  if(d != NULL)
    if(!d->Disabled)
      if (d->Com)
      {
        if( mode > 0  )
        {
          sStatus.intVal16 |= DAUL;  // turn Dual Mode On
          if(iAR620DebugLevel) StartupStore(_T(". AR620x  Dual on %s"), NEWLINE);
        }
        else
        {
          sStatus.intVal16 &= ~DAUL;   // turn Dual Mode Off
          if(iAR620DebugLevel)StartupStore(_T(". AR620x  Dual off %s"), NEWLINE);
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
        SendCommand(d, szTmp,len);
      }
  RadioPara.DualValid = false;
  return(TRUE);
}

/*****************************************************************************
 * this function converts a AR620x answer 
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


uint32_t ulState;

int processed=0;

LKASSERT(szCommand !=NULL);
LKASSERT(d !=NULL);


  switch ((unsigned char)(szCommand[3] & 0x7F))
  {
    case 0:

    break;

    case 3:

        RadioPara.Changed = true;
        RadioPara.VolValid = true;
        RadioPara.Volume = (50-(int)szCommand[4])/5;
        if(iAR620DebugLevel )  StartupStore(_T("AR620x Volume %u  %s") , RadioPara.Volume  ,NEWLINE);

    break;

    case 4:

        RadioPara.Changed = true;
        RadioPara.Squelch = (int)(szCommand[4]-6)/2+1;  // 6 + (Squelch-1)*2
        if(iAR620DebugLevel )  StartupStore(_T("AR620x Squelch %u  %s") , RadioPara.Squelch  ,NEWLINE);

    break;

    case 12:

        RadioPara.Changed = true;
        RadioPara.DualValid = true;
        sStatus.intVal8[1] = szCommand[4] ;
        sStatus.intVal8[0] = szCommand[5] ;
        if(sStatus.intVal16 & DAUL)
         RadioPara.Dual = true;
        else
         RadioPara.Dual = false;
        if(iAR620DebugLevel )  StartupStore(_T("AR620x Dual %u  %s") , RadioPara.Dual  ,NEWLINE);
      
    break;
#ifdef RADIO_VOLTAGE
    case 21:

        GPS_INFO.ExtBatt2_Voltage =   8.5 + szCommand[4] *0.1;
        RadioPara.Changed = true;
        if(iAR620DebugLevel  == 2)   StartupStore(_T("AR620x Supply Voltage: %4.1fV %s"),  GPS_INFO.ExtBatt2_Voltage  ,NEWLINE);

    break;
#endif           
    case 22:

        RadioPara.Changed = true;
        RadioPara.ActiveValid = true;
        sFrequency.intVal8[1] = szCommand[4] ;
        sFrequency.intVal8[0] = szCommand[5] ;
        RadioPara.ActiveKhz =  Idx2Freq(sFrequency.intVal16);
        UpdateStationName(RadioPara.ActiveName, RadioPara.ActiveKhz);

        if(iAR620DebugLevel ) StartupStore(_T("AR620x <AF %u  %7.3f"), sFrequency.intVal16, RadioPara.ActiveKhz / 1000.);

        sFrequency.intVal8[1] = szCommand[6];
        sFrequency.intVal8[0] = szCommand[7] ;
        RadioPara.PassiveValid = true;
        RadioPara.PassiveKhz =  Idx2Freq(sFrequency.intVal16);
        UpdateStationName(RadioPara.PassiveName, RadioPara.PassiveKhz);

        if(iAR620DebugLevel ) StartupStore(_T("AR620x <PF: %u %7.3f"), sFrequency.intVal16, RadioPara.PassiveKhz /1000.);
        RadioPara.Changed = true;
      
    break;
    case 64:

          ulState = szCommand[4] << 24 | szCommand[5] << 16 | szCommand[6] << 8 | szCommand[7];
        RadioPara.TX        = ((ulState & (BIT(5)| BIT(6))) > 0) ? true : false;
        RadioPara.RX_active = ((ulState & BIT(7)) > 0)  ? true : false;
        RadioPara.RX_standy = ((ulState & BIT(8)) > 0)  ? true : false;
        RadioPara.RX        = (RadioPara.RX_active ||   RadioPara.RX_standy );
        RadioPara.Changed = true;

        if(iAR620DebugLevel )                         StartupStore(_T("AR620x 0x%X %s")  ,ulState , NEWLINE);
        if(iAR620DebugLevel ) if(RadioPara.RX)        StartupStore(_T("AR620x Rx  %s")        , NEWLINE);
        if(iAR620DebugLevel ) if(RadioPara.RX_active) StartupStore(_T("AR620x Rx Active  %s") , NEWLINE);
        if(iAR620DebugLevel ) if(RadioPara.RX_standy) StartupStore(_T("AR620x Rx Passive %s") , NEWLINE);
        if(iAR620DebugLevel ) if(RadioPara.TX)        StartupStore(_T("AR620x Tx  %s")        , NEWLINE);

    break;
    default:
    break;
  }

  return processed;  /* return the number of converted characters */
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
  if((uint8_t)String[cnt] == HEADER_ID)
    Recbuflen =0;

  if(Recbuflen >= REC_BUFSIZE)
    Recbuflen =0;

  LKASSERT(Recbuflen < REC_BUFSIZE);
  Command[Recbuflen++] =(uint8_t) String[cnt++];

  if(Recbuflen == 2)
    if(Command[Recbuflen-1] != PROTOKOL_ID)
      Recbuflen =0;

  if(Recbuflen >= 3)
  {
     CommandLength = Command[2];
     if(Recbuflen >= (CommandLength+5) ) // all received
     {
       if(iAR620DebugLevel ==2) for(int i=0; i < (CommandLength+4);i++)   StartupStore(_T("AR620x  Cmd: 0x%02X  %s") ,Command[i] ,NEWLINE);
       CRC.intVal8[1] =  Command[CommandLength+3];
       CRC.intVal8[0] =  Command[CommandLength+4];
       if(iAR620DebugLevel ==2) StartupStore(_T("AR620x  CRC 0x%04X %s") ,CRC.intVal16,NEWLINE);
       CalCRC =CRCBitwise(Command, CommandLength+3);
       if(CalCRC  == CRC.intVal16)
       {
           if(!bSending)
           {
             if(iAR620DebugLevel ==2 )  StartupStore(_T("AR620x  Process Command %u  %s") ,Command[3]  ,NEWLINE);
             AR620x_Convert_Answer(d, Command, CommandLength+5,CalCRC);
           }
           else
             if(iAR620DebugLevel)  StartupStore(_T("AR620x  skip Command %u  %s") ,Command[3]  ,NEWLINE);

       }
       else
       {
    	   if(iAR620DebugLevel)StartupStore(_T("AR620x  CRC check fail! Command 0x%04X  0x%04X %s") ,CRC.intVal16,CalCRC ,NEWLINE);
       }
       Recbuflen = 0;

     }
  }


}

return  RadioPara.Changed;
}



} // namespace

void AR620xInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Dittel AR620x"));
  d->IsRadio        = AR620xIsRadio;
  d->PutVolume      = AR620xPutVolume;
  d->PutSquelch     = AR620xPutSquelch;
  d->PutFreqActive  = AR620xPutFreqActive;
  d->PutFreqStandby = AR620xPutFreqStandby;
  d->StationSwap    = AR620xStationSwap;
  d->ParseStream    = AR620xParseString;
  d->PutRadioMode      = AR620xRadioMode;
  RadioPara.Enabled8_33 = TRUE;
  sStatus.intVal16 = SQUELCH; // BIT7 for Squelch enabled
}
