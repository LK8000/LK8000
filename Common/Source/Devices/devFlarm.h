/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#ifndef devFlarm_h__
#define devFlarm_h__
#include "devBase.h"
#include "Util/tstring.hpp"
#include "nmeaistream.h"
#include "dlgTools.h"
#include "Devices/DeviceRegister.h"

class WindowControl;
class WndButton;

#define REC_NO_ERROR      0
#define REC_TIMEOUT_ERROR 1
#define REC_CRC_ERROR     2
#define REC_ABORTED       3
#define FILENAME_ERROR    4
#define FILE_OPEN_ERROR   5
#define IGC_RECEIVE_ERROR 6
#define REC_NO_DEVICE     7
#define REC_NOMSG         8
#define REC_INVALID_SIZE  9

#define NO_FAKE_FLARM
typedef union{
  uint16_t val;
  uint8_t byte[2];
} ConvUnion;

#define STARTFRAME    0x73
#define ESCAPE        0x78
#define ESC_ESC       0x55
#define ESC_START     0x31

#define EOF_          0x1A
#define ACK           0xA0
#define NACK          0xB7
#define PING          0x01
#define SETBAUDRATE   0x02
#define FLASHUPLAOD   0x10
#define FLASHDOWNLOAD 0x11
#define EXIT          0x12
#define SELECTRECORD  0x20
#define GETRECORDINFO 0x21
#define GETIGCDATA    0x22


#define highbyte(a)  (((a)>>8) & 0xFF)
#define lowbyte(a)   ((a) & 0xFF)

uint8_t RecChar( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout);
bool BlockReceived();
bool IsInBinaryMode();
bool SetBinaryModeFlag(bool bBinMode);

class CDevFlarm : public DevBase
{
private:
  // no ctor
  CDevFlarm() {}

  // no copy
  CDevFlarm(const CDevFlarm&) {}
  void operator=(const CDevFlarm&) {}

//Init
public:
  static constexpr
  DeviceRegister_t Register() {
    return devRegister(GetName(), &Install);
  }


  static constexpr
  const TCHAR* GetName() { 
    return TEXT("Flarm"); 
  }

  static BOOL Open(PDeviceDescriptor_t d);
  static BOOL Close (PDeviceDescriptor_t d);
  static DeviceDescriptor_t* GetDevice(void) {
    return m_pDevice;
  }

private:
  static void Install(PDeviceDescriptor_t d);

// Receive data
private:

  static BOOL FlarmParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO);
  static BOOL FlarmParse(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);
  // Send Command
  static BOOL FlarmReboot(PDeviceDescriptor_t d);




  // Config
  static BOOL Config(PDeviceDescriptor_t d);
  static void OnCloseClicked(WndButton* pWnd);


  static void OnIGCDownloadClicked(WndButton* pWnd);
  static void OnRebootClicked(WndButton* pWnd);

  static CallBackTableEntry_t CallBackTable[];
  static PDeviceDescriptor_t m_pDevice;
};

typedef struct
{
  ConvUnion blocksize;
  uint8_t   Version;
  ConvUnion Sequence;
  uint8_t   Command;
  ConvUnion CRC_in;
  /**************************/
  uint8_t   Error;
  volatile BOOL      RecReady;
  uint16_t  PL_Idx;
  uint16_t  BlkIndex;
  /**************************/
  uint8_t   Payload[2000];

} BinBlock;



#endif // devFlarm_h__
