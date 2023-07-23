/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#include "externs.h"
#include "Globals.h"
#include "devKRT2.h"
#include "Comm/device.h"
#include "utils/stringext.h"
#include "Radio.h"

namespace {

constexpr uint8_t STX = 0x02; /* STX Command prefix hex code                  */
constexpr uint8_t ACK = 0x06; /* acknolage hex code                           */
constexpr uint8_t NAK = 0x15; /* not acknolage hex code                       */
//#define RESEND_ON_NAK       /* switch for command retry on transmission fail  */

BOOL KRT2IsRadio(DeviceDescriptor_t* d) {
  return TRUE;
}

bool device_found = false; 

BOOL OpenClose(DeviceDescriptor_t* d) {
  device_found = false;
  return TRUE;
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
int SetKRT2Station(uint8_t *Command, uint8_t slot, unsigned khz, const TCHAR* Station) {
  LKASSERT(Command !=NULL)
  if (!Command) {
    return false;
  }

  auto MHz = static_cast<uint8_t>(khz / 1000U);
  auto Chan = static_cast<uint8_t>((khz % 1000U) / 5U);

  char Airfield[10];

  if(Station) {
    to_usascii(Station, Airfield);
  }

  // airfield : remove ctrl char and pad with space
  for (int i = 0 ; i < 10; i++) {
    if((Airfield[i] < 32) || (Airfield[i] > 126)) {
   	  Airfield[i] = ' ';
    }
  }

  unsigned len = 0;
  Command[len++] = STX;
  Command[len++] = slot;
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
  Command[len++] = MHz ^ Chan ;

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
  if (Command == NULL) {
    return false;
  }
  int len =0;
  Command[len++] = STX;
  Command[len++] = 'A';
  Command[len++] = (unsigned char)Vol;
  Command[len++] = (unsigned char)Squelch;
  Command[len++] = (unsigned char)Vox;
  Command[len++] = (unsigned char)Squelch + (unsigned char)Vox;
  return len;
}


BOOL KRT2PutVolume(DeviceDescriptor_t* d, int Volume) {

  if(d && !d->Disabled && d->Com) {
    uint8_t  szTmp[255];
    int len = SetKRT2Audio(szTmp, Volume,  RadioPara.Squelch, RadioPara.Vox);
    if(len > 0) {
      d->Com->Write(szTmp, len);
      TestLog(_T(". KRT2 Volume  %i"), Volume);
      RadioPara.Volume = Volume;
    }
  }
  return TRUE;
}


BOOL KRT2PutSquelch(DeviceDescriptor_t* d, int Squelch) {

  if(d && !d->Disabled && d->Com) {
    uint8_t szTmp[255];
    int len = SetKRT2Audio(szTmp, RadioPara.Volume, Squelch, RadioPara.Vox);
    if(len > 0) {
      d->Com->Write(szTmp, len);
      TestLog(_T(". KRT2 Squelch  %i"), Squelch);
      RadioPara.Squelch = Squelch;
    }
  }
  return TRUE;
}


BOOL KRT2PutFreq(DeviceDescriptor_t* d, char ur, unsigned khz, const TCHAR* StationName) {
  if(d && !d->Disabled && d->Com) {
    uint8_t szTmp[25];

    int len = SetKRT2Station(szTmp, ur, khz, StationName);
    d->Com->Write(szTmp, len);

    TestLog(_T(". KRT2 Active Station %7.3fMHz %s"), khz / 1000., StationName);
  }
  return TRUE;
}

BOOL KRT2PutFreqActive(DeviceDescriptor_t* d, unsigned khz, const TCHAR* StationName) {
  return KRT2PutFreq(d, 'U', khz, StationName);
}

BOOL KRT2PutFreqStandby(DeviceDescriptor_t* d, unsigned khz,  const TCHAR* StationName) {
  return KRT2PutFreq(d, 'R', khz, StationName);
}

BOOL KRT2StationSwap(DeviceDescriptor_t* d) {
  if (d && !d->Disabled && d->Com) {
    uint8_t szTmp[] = { STX, 'C' }; 
    d->Com->Write(szTmp, std::size(szTmp));
    TestLog(_T(". KRT2  station swap"));
  }
  return TRUE;
}

BOOL KRT2RadioMode(DeviceDescriptor_t* d, int mode) {
  if (d && !d->Disabled && d->Com) {
    if (mode > 0) {
      uint8_t Cmd[] = { STX, 'O' }; // turn Dual Mode On
      d->Com->Write(Cmd, std::size(Cmd));
      TestLog(_T(". KRT2  Dual on"));
    }
    else {
      uint8_t Cmd[] = { STX, 'o' }; // turn Dual Mode Off
      d->Com->Write(Cmd, std::size(Cmd));
      TestLog(_T(". KRT2  Dual off"));
    }
  }
  return TRUE;
}

/*****************************************************************************
 * this function converts a KRT answer sting to a NMEA answer
 *
 * szAnswer       NMEA Answer
 * Answerlen      number of valid characters in the NMEA answerstring
 * szCommand      KRT2 binary code to be converted
 * len            length of the KRT2 binary code to be converted
 ****************************************************************************/
int KRT2_Convert_Answer(DeviceDescriptor_t* d, uint8_t *szCommand, int len) {
  if(d == NULL) return 0;
  if(szCommand == NULL) return 0;
  if(len == 0)          return 0;

  int processed = 0;
  static int counter = 0;

  if (szCommand[0] == 'S') {
    d->Com->Write('x');

    TestLog(_T("KRT2 heartbeat: #%i"), counter++);
    if(!std::exchange(device_found, true)) {
      DoStatusMessage(MsgToken(1358)); // RADIO DETECTED
    }
    processed++;
  }
  else if(szCommand[0] == ACK) {
    processed++;
  }
  else if(szCommand[0] == NAK) {
#ifdef RESEND_ON_NAK
    if (iReSendCount++ < 5) {
      /****************************************************************
       * here we try a second transmission of the last command
       ****************************************************************/

      iReSendCount++;
    }
#else
    TestLog(_T(". KRT2 no acknolage!"));
#endif
    processed++;
  }
  else if(szCommand[0] == STX) {
    processed++;
    if(len > 1) {
      processed++;
      switch (szCommand[1]) {
        case 'U':
          RadioPara.ActiveValid = false;
          if (len >= 13) {
            if (szCommand[12] != (szCommand[2] ^ szCommand[3])) {
              DoStatusMessage(_T("Checksum Fail"));
            }
            else {
              RadioPara.ActiveValid = true;
              RadioPara.ActiveKhz = (szCommand[2] * 1000U) + (szCommand[3] * 5U);
              for (unsigned i=0; i < 8; i++) {
                RadioPara.ActiveName[i] = szCommand[4+i];
              }
              RadioPara.ActiveName[8] = 0;
              TrimRight(RadioPara.ActiveName);
              if (_tcslen(RadioPara.ActiveName) == 0) {
                if (UpdateStationName(RadioPara.ActiveName, RadioPara.ActiveKhz)) {
                  devPutFreqActive(RadioPara.ActiveKhz, RadioPara.ActiveName);
                }
              }
              TestLog(_T("Active: %s %7.3fMHz"),  RadioPara.ActiveName, RadioPara.ActiveKhz / 1000.);
              processed = 13;
            }
          }
          else {
            processed = 0;
          }
          break;

        case 'R':
          RadioPara.PassiveValid = false;
          if (len >= 13) {
            if (szCommand[12] != (szCommand[2] ^ szCommand[3])) {
              DoStatusMessage(_T("Checksum Fail"));
            }
            else {
              RadioPara.PassiveValid = true;
              RadioPara.PassiveKhz=   (szCommand[2] * 1000U) + (szCommand[3] * 5U);
              for (unsigned i = 0; i < 8; i++) {
                RadioPara.PassiveName[i] = szCommand[4+i];
              }
              RadioPara.PassiveName[8] = 0;
              TrimRight(RadioPara.PassiveName);
              if (_tcslen(RadioPara.PassiveName) == 0) {
                if (UpdateStationName(RadioPara.PassiveName, RadioPara.PassiveKhz)) {
                  devPutFreqStandby(RadioPara.ActiveKhz, RadioPara.PassiveName);
                }
              }
              TestLog(_T("Passive: %s %7.3fMHz"),  RadioPara.PassiveName, RadioPara.PassiveKhz / 1000.);
              processed = 13;
            }
          }
          else {
            processed = 0;
          }
          break;

        case 'A':
          RadioPara.VolValid = false;
          RadioPara.SqValid = false;
          if(len >= 6) {
            if( szCommand[5] != (szCommand[3]+ szCommand[4])) {
              DoStatusMessage(_T("Checksum Fail"));
            }
            else {
              RadioPara.VolValid = true;
              RadioPara.SqValid = true;

              if(RadioPara.Volume != (int)szCommand[2]) {
                RadioPara.Volume = (int)szCommand[2];
                TestLog(_T("%s %i "),  MsgToken(2310), RadioPara.Volume );  // _@M2310_ Vol
              }
              if( RadioPara.Squelch != (int)szCommand[3]) {
                RadioPara.Squelch  = (int)szCommand[3];
                TestLog(_T("%s %i "),  MsgToken(2311), RadioPara.Squelch );  //_@M2311_ Sqw
              }
              if(RadioPara.Vox != (int)szCommand[4]) {
                RadioPara.Vox= (int)szCommand[4];
                TestLog(_T("Vox %i "), RadioPara.Vox );
              }
              processed = 6;
            }
          }
          else {
            processed = 0;
          }
          break;

        case 'C':
          RadioPara.ActiveValid = false;
          RadioPara.PassiveValid = false;
          if(len >= 2) {
            std::swap(RadioPara.ActiveKhz, RadioPara.PassiveKhz);
            std::swap(RadioPara.ActiveName, RadioPara.PassiveName);
            TestLog(_T("Swap "));
          }
          break;

        case 'O':
          RadioPara.DualValid = false;
          if(len >= 2) {
            RadioPara.DualValid = true;
            RadioPara.Dual = true;
            TestLog(_T("Dual ON "));
          }
          break;

        case 'o':
          RadioPara.DualValid = false;
          if(len >= 2) {
            RadioPara.DualValid = true;
            RadioPara.Dual = false;
            TestLog(_T("Dual OFF "));
          }
          break;

        case '8':
          TestLog(_T("STA,8_33KHZ"));
          RadioPara.Enabled8_33 = true;
          break;

        case '6':
          TestLog(_T("STA,25KHZ"));
          RadioPara.Enabled8_33 = false;
          break;

        case '1':
          TestLog(_T("SIDETONE"));
          break;

        case '2':
          TestLog(_T("STA,ARBIT"));
          break;

        case '3':
          TestLog(_T("INTERCOM"));
          break;

        case '4':
          TestLog(_T("EXT_AUD"));
          break;

        case 'B':
          TestLog(_T("BAT_LOW"));
          RadioPara.lowBAT = true;
          break;

        case 'D':
          TestLog(_T("BAT_OK"));
          RadioPara.lowBAT = false;
          break;

        case 'J':
          TestLog(_T("RX_ON"));
          break;

        case 'V':
          TestLog(_T("RX_OFF"));
          RadioPara.RX_active = false;
          RadioPara.RX_standy = false;
          break;

        case 'K':
          TestLog(_T("TX_ON"));
          RadioPara.TX= true;
          break;

        case 'L':
          TestLog(_T("TE_ON"));
          break;

        case 'Y':
          TestLog(_T("RX_TX_OFF"));
          RadioPara.TX= false;
          break;

        case 'M':
          TestLog(_T("RX_AF"));
          RadioPara.RX_active = true;
          break;

        case 'm': 
          TestLog(_T("RX_SF"));
          RadioPara.RX_standy = true;
          break;

        case 'E':
          TestLog(_T("STX_E"));
          break;

        case 'H':
          TestLog(_T("STX_H"));
          break;

        case 'e':
          TestLog(_T("ERR,PLL"));
          break;

        case 'F':
          TestLog(_T("ERR,RELEASE"));
          break;

        case 'a':
          TestLog(_T("ERR,ADC"));
          break;

        case 'b':
          TestLog(_T("ERR,ANT"));
          break;

        case 'c':
          TestLog(_T("ERR,FPA"));
          break;

        case 'd':
          TestLog(_T("ERR,FUSE"));
          break;

        case 'f':
          TestLog(_T("ERR,KEYBLOCK"));
          break;

        case 'g':
          TestLog(_T("ERR,I2C"));
          break;

        case 'h':
          TestLog(_T("ERR,ID10"));
          break;

        default:
          TestLog(_T("ERR,UNKNOWN COMMAND 0x%X"), (int)szCommand[1]);
          break;
      } //   switch (szCommand[1])
    } //   if(len > 1)
  } //   if(szCommand[0] == STX)
  return processed;  /* return the number of converted characters */
}


BOOL KRT2ParseString(DeviceDescriptor_t* d, char *String, int len, NMEA_INFO *GPS_INFO) {
  if(d == NULL) return 0;
  if(String == NULL) return 0;
  if(len == 0) return 0;

  #define REC_BUFSIZE 128
  int cnt=0;
  static uint16_t Recbuflen =0;
  static uint16_t CommandLength=REC_BUFSIZE;
  static uint8_t Command[REC_BUFSIZE];

  while (cnt < len) {
    if(CommandLength == REC_BUFSIZE) {
      if(String[cnt] ==STX) {
        Recbuflen =0;
      }
    }
    if(Recbuflen >= REC_BUFSIZE) {
      Recbuflen =0;
    }
    LKASSERT(Recbuflen < REC_BUFSIZE);

    DebugLog(_T(". KRT2   Raw Input: Recbuflen:%u 0x%02X %c"),Recbuflen, (uint8_t)String[cnt], String[cnt]);
    Command[Recbuflen++] =(char) String[cnt++];
    if (Recbuflen == 1) {
      switch (Command[0]) {
        case 'S': CommandLength = 1; break;
        case ACK: CommandLength = 1; break;
        case NAK: CommandLength = 1; break;
        default: break;
      }
    }

    if (Recbuflen == 2) {
      switch(Command[1]) {
        case 'U': CommandLength = 13; break;
        case 'R': CommandLength = 13; break;
        case 'A': CommandLength = 6;  break;
        case 'C': CommandLength = 2;  break;
        case 'O': CommandLength = 2;  break;
        case 'o': CommandLength = 2;  break;
        default : CommandLength = 2;  break;
      }
    }

    if (Recbuflen == CommandLength) {
      // all received
      KRT2_Convert_Answer(d, Command, CommandLength);
      RadioPara.Changed = true;
      Recbuflen = 0;
      CommandLength = REC_BUFSIZE;
    }
  } //  (cnt < len)
  return  RadioPara.Changed;
}

} // namespace

void KRT2Install(DeviceDescriptor_t* d){
  _tcscpy(d->Name, TEXT("Dittel KRT2"));

  d->Open = OpenClose;
  d->Close = OpenClose;

  d->IsRadio        = KRT2IsRadio;
  d->PutVolume      = KRT2PutVolume;
  d->PutSquelch     = KRT2PutSquelch;
  d->PutFreqActive  = KRT2PutFreqActive;
  d->PutFreqStandby = KRT2PutFreqStandby;
  d->StationSwap    = KRT2StationSwap;
  d->ParseStream    = KRT2ParseString;
  d->PutRadioMode   = KRT2RadioMode;
}
