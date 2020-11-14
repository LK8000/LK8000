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
#include "utils/stringext.h"


namespace {

#define ACTIVE_STATION  1
#define PASSIVE_STATION 0

constexpr uint8_t STX = 0x02; /* STX Command prefix hex code                  */
constexpr uint8_t ACK = 0x06; /* acknolage hex code                           */
constexpr uint8_t NAK = 0x15; /* not acknolage hex code                       */
//#define RESEND_ON_NAK       /* switch for command retry on transmission fail  */


#ifdef TESTBENCH
int uiKRT2DebugLevel = 1;
#else
int uiKRT2DebugLevel = 0;
#endif


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
int SetKRT2Station(uint8_t *Command ,int Active_Passive, double fFrequency, const TCHAR* Station)
{
  auto MHz = static_cast<uint8_t>(fFrequency);
  auto kHz = static_cast<uint32_t>(fFrequency *1000.0 - MHz *1000  + 0.5);
  auto Chan = static_cast<uint8_t>(kHz/5);

  char Airfield[10]={"         "};

  LKASSERT(Command !=NULL)
  if(Command == NULL )
    return false;

  unsigned len =0;
  Command[len++] = STX;
  switch (Active_Passive)
  {
    case ACTIVE_STATION:
      Command[len++] = 'U';
    break;
    default:
    case PASSIVE_STATION:
      Command[len++] = 'R';
    break;
  }
  if(Station != NULL) {
    TCHAR2usascii(Station, Airfield, 9);
  }

  for (int i = 0 ; i < 10; i++)
  {
    if((Airfield[i] < 32) || (Airfield[i] > 126))
   	  Airfield[i] = ' ';
  }
  Command[len++] = MHz;
  Command[len++] = Chan;
  Command[len++] = Airfield[0];
  Command[len++] = Airfield[1];
  Command[len++] = Airfield[2];
  Command[len++] = Airfield[3];
  Command[len++] = Airfield[4];
  Command[len++] = Airfield[5];
  Command[len++] = Airfield[6];
  Command[len++] = Airfield[7];
  Command[len++] =MHz ^ Chan ;

  return len;
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
int  SetKRT2Audio(uint8_t *Command ,int Vol, int Squelch, int Vox)
{
  LKASSERT(Command !=NULL)
  if(Command == NULL )
      return false;
  int len =0;
  Command[len++] = STX;
  Command[len++] = 'A';
  Command[len++] = (unsigned char)Vol;
  Command[len++] = (unsigned char)Squelch;
  Command[len++] = (unsigned char)Vox;
  Command[len++] = (unsigned char)Squelch + (unsigned char)Vox;

  return len;

}


BOOL KRT2PutVolume(PDeviceDescriptor_t d, int Volume) {

  if(d && !d->Disabled && d->Com)
  {
    uint8_t  szTmp[255];
    int len = SetKRT2Audio(szTmp, Volume,  RadioPara.Squelch, RadioPara.Vox);
    if(len > 0)
    {
      d->Com->Write(szTmp, len);
      if(uiKRT2DebugLevel) StartupStore(_T(". KRT2 Volume  %i%s"), Volume,NEWLINE);
      RadioPara.Volume = Volume;
    }
  }
  return(TRUE);
}




BOOL KRT2PutSquelch(PDeviceDescriptor_t d, int Squelch) {

  if(d && !d->Disabled && d->Com)
  {
    uint8_t  szTmp[255];
    int  len  = SetKRT2Audio(szTmp, RadioPara.Volume,  Squelch, RadioPara.Vox);
    if(len > 0)
    {
      d->Com->Write(szTmp,len);
      if(uiKRT2DebugLevel) StartupStore(_T(". KRT2 Squelch  %i%s"), Squelch,NEWLINE);
      RadioPara.Squelch = Squelch;
    }
  }
  return(TRUE);
}



BOOL KRT2PutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) {

  if(d && !d->Disabled && d->Com)
  {
    uint8_t szTmp[255];
    int len =SetKRT2Station(szTmp ,ACTIVE_STATION, Freq, StationName);
    d->Com->Write(szTmp, len);
    RadioPara.ActiveFrequency=  Freq;
    if(StationName != NULL)
      _sntprintf(RadioPara.ActiveName, NAME_SIZE,_T("%s"),StationName) ;
    if(uiKRT2DebugLevel) StartupStore(_T(". KRT2 Active Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);
  }
  return(TRUE);
}


BOOL KRT2PutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) {

  if(d && !d->Disabled && d->Com)
  {
    uint8_t szTmp[255];
    int len = SetKRT2Station(szTmp ,PASSIVE_STATION, Freq, StationName);
    d->Com->Write(szTmp, len);

    RadioPara.PassiveFrequency =  Freq;
    if(StationName != NULL)
      _sntprintf(RadioPara.PassiveName , NAME_SIZE ,_T("%s"),StationName) ;
    if(uiKRT2DebugLevel) StartupStore(_T(". KRT2 Standby Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);
  }
  return(TRUE);
}


BOOL KRT2StationSwap(PDeviceDescriptor_t d) {
  if(d && !d->Disabled && d->Com)
  {
    uint8_t szTmp[] = { STX, 'C' }; 
    d->Com->Write(szTmp, std::size(szTmp));
    if(uiKRT2DebugLevel) StartupStore(_T(". KRT2  station swap %s"), NEWLINE);
  }
  return(TRUE);
}


BOOL KRT2RadioMode(PDeviceDescriptor_t d, int mode) {
  if(d && !d->Disabled && d->Com)
  {
    if( mode > 0  )
    {
      uint8_t Cmd[] = { STX, 'O' };   // turn Dual Mode On
      d->Com->Write(Cmd, std::size(Cmd));

      if(uiKRT2DebugLevel) StartupStore(_T(". KRT2  Dual on %s"), NEWLINE);
    }
    else
    {
      uint8_t Cmd[] = { STX, 'o' };   // turn Dual Mode Off
      d->Com->Write(Cmd, std::size(Cmd));

      if(uiKRT2DebugLevel) StartupStore(_T(". KRT2  Dual off %s"), NEWLINE);
    }
  }
  return(TRUE);
}

/*****************************************************************************
 * this function converts a KRT answer sting to a NMEA answer
 *
 * szAnswer       NMEA Answer
 * Answerlen      number of valid characters in the NMEA answerstring
 * szCommand      KRT2 binary code to be converted
 * len            length of the KRT2 binary code to be converted
 ****************************************************************************/
int KRT2_Convert_Answer(DeviceDescriptor_t *d, uint8_t *szCommand, int len)
{
if(d == NULL) return 0;
if(szCommand == NULL) return 0;
if(len == 0)          return 0;
//char szAnswer[180];
TCHAR szTempStr[180] = _T("");

double  fTmp  =0.0;
int processed=0;
static int iDetected = 0;
static bool bFound = false;

static int counter =0;

int i;

    if(szCommand[0] == 'S')
    {
      uint8_t Cmd[] = { 'x' };
      d->Com->Write(Cmd, std::size(Cmd));

      if(uiKRT2DebugLevel) StartupStore(_T("KRT2 heartbeat: #%i %s"),counter++ ,NEWLINE);
      if(bFound == false)
      {
        bFound = true;
        iDetected++;
        if(iDetected < 10)
          DoStatusMessage(LKGetText(TEXT("RADIO DETECTED"))); // RADIO DETECTED
        else
            if(iDetected == 10)
               DoStatusMessage(LKGetText(TEXT("Radio Message disabled")));

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

        iReSendCount++;
      }
#else
      _stprintf(szTempStr,_T(". KRT2 no acknolage!"));
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
        switch (szCommand[1])
        {
          case 'U':
            if(len >= 13)
            {
              if(szCommand[12] != (szCommand[2] ^ szCommand[3]))
                DoStatusMessage(_T("Checksum Fail"));
              else
              {
                RadioPara.ActiveFrequency=  ((double)(unsigned char)szCommand[2]) + ((double)(unsigned char)szCommand[3])/ 200.0;
                for(unsigned i=0; i < 8; i++)
                  RadioPara.ActiveName[i] =   szCommand[4+i];
                RadioPara.ActiveName[8] =0;
                TrimRight(RadioPara.ActiveName);
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
              else
              {
                RadioPara.PassiveFrequency =  ((double)(unsigned char)szCommand[2]) + ((double)(unsigned char)szCommand[3])/ 200.0;
                for(i=0; i < 8; i++)
                  RadioPara.PassiveName[i] =   szCommand[4+i];
                RadioPara.PassiveName[8] =0;
                TrimRight(RadioPara.PassiveName);
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
              else
              {
                if(RadioPara.Volume != (int)szCommand[2])
                {
                  RadioPara.Volume = (int)szCommand[2];
                  _stprintf(szTempStr,_T("%s %i "),  MsgToken(2310), RadioPara.Volume );  // _@M2310_ Vol
                }
                if( RadioPara.Squelch != (int)szCommand[3])
                {
                  RadioPara.Squelch  = (int)szCommand[3];
                  _stprintf(szTempStr,_T("%s %i "),  MsgToken(2311), RadioPara.Squelch );  //_@M2311_ Sqw
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
            RadioPara.Enabled8_33 = true;
          break;
          case '6':  _stprintf(szTempStr,_T("STA,25KHZ"));
            RadioPara.Enabled8_33 = false;
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
            RadioPara.lowBAT = true;
          break;
          case 'D':  _stprintf(szTempStr,_T("BAT_OK"));
            RadioPara.lowBAT = false;
          break;
          case 'J':  _stprintf(szTempStr,_T("RX_ON"));
          
          break;
          case 'V':  _stprintf(szTempStr,_T("RX_OFF"));
            RadioPara.RX_active = false;
            RadioPara.RX_standy = false;
          break;
          case 'K':  _stprintf(szTempStr,_T("TX_ON"));
            RadioPara.TX= true;
          break;
          case 'L': _stprintf(szTempStr,_T("TE_ON"));
          break;

          case 'Y': _stprintf(szTempStr,_T("RX_TX_OFF"));
            RadioPara.TX= false;
          break;
          case 'M':  _stprintf(szTempStr,_T("RX_AF"));
            RadioPara.RX_active = true;
          break;
          case 'm': _stprintf(szTempStr,_T("RX_SF"));
            RadioPara.RX_standy = true;
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
        } //   switch (szCommand[1])
      } //   if(len > 1)
    } //   if(szCommand[0] == STX)

  if(uiKRT2DebugLevel)
  {
    if(processed> 0)
    {
      TCHAR szMessage[250] = _T("");
      _stprintf(szMessage,_T("%s:%s "), MsgToken(2309),szTempStr);
      StartupStore(_T(" %s %s%s"), szMessage,WhatTimeIsIt(),NEWLINE);
      //DoStatusMessage(szMessage);
    }
  }


  return processed;  /* return the number of converted characters */
}


BOOL KRT2ParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
{
  if(d == NULL) return 0;
  if(String == NULL) return 0;
  if(len == 0) return 0;

  #define REC_BUFSIZE 128
  int cnt=0;
  static uint16_t Recbuflen =0;
  static uint16_t CommandLength=REC_BUFSIZE;
  static uint8_t Command[REC_BUFSIZE];

  while (cnt < len)
  {
    if(CommandLength == REC_BUFSIZE)
    {
      if(String[cnt] ==STX)
        Recbuflen =0;
    }
    if(Recbuflen >= REC_BUFSIZE)
      Recbuflen =0;
    LKASSERT(Recbuflen < REC_BUFSIZE);

  if(uiKRT2DebugLevel ==2) StartupStore(_T(". KRT2   Raw Input: Recbuflen:%u  0x%02X %c %s"),  Recbuflen,(uint8_t)String[cnt] ,String[cnt] ,NEWLINE);
    Command[Recbuflen++] =(char) String[cnt++];
    if(Recbuflen == 1)
    {
      switch (Command[0])
      {
        case 'S': CommandLength = 1; break;
        case ACK: CommandLength = 1; break;
        case NAK: CommandLength = 1; break;
        default: break;
      }
    }

    if(Recbuflen == 2)
    {
      switch(Command[1])
      {
        case 'U': CommandLength = 13; break;
        case 'R': CommandLength = 13; break;
        case 'A': CommandLength = 6;  break;
        case 'C': CommandLength = 2;  break;
        case 'O': CommandLength = 2;  break;
        case 'o': CommandLength = 2;  break;
        default : CommandLength = 2;  break;
      }
    }

    if(Recbuflen == (CommandLength) ) // all received
    {
      if(uiKRT2DebugLevel ==2)	 StartupStore(_T("================ %s") ,NEWLINE);
      if(uiKRT2DebugLevel ==2) for(int i=0; i < (CommandLength);i++)   StartupStore(_T(". KRT2   Cmd: 0x%02X  %s") ,Command[i] ,NEWLINE);
      if(uiKRT2DebugLevel ==2)  StartupStore(_T(". KRT2  Process Command %u  %s") ,CommandLength ,NEWLINE);
      KRT2_Convert_Answer(d, Command, CommandLength);
      RadioPara.Changed = true;
      Recbuflen = 0;
      CommandLength = REC_BUFSIZE;
    }
  } //  (cnt < len)
  return  RadioPara.Changed;
}



BOOL KRT2Install(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Dittel KRT2"));
  d->IsRadio        = KRT2IsRadio;
  d->PutVolume      = KRT2PutVolume;
  d->PutSquelch     = KRT2PutSquelch;
  d->PutFreqActive  = KRT2PutFreqActive;
  d->PutFreqStandby = KRT2PutFreqStandby;
  d->StationSwap    = KRT2StationSwap;
  d->ParseStream    = KRT2ParseString;
  d->PutRadioMode   = KRT2RadioMode;
  return TRUE;
}

} // namespace


BOOL KRT2Register(void){
  return(devRegister(
    TEXT("Dittel KRT2"),
    (1l << dfRadio),
    KRT2Install
  ));
}
