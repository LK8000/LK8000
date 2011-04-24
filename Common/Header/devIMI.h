/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef	DEVIMI_H
#define	DEVIMI_H
 
#include "devBase.h"

class CDevIMI : public DevBase
{
  typedef unsigned char IMIBYTE;                // 8bit unsigned
  typedef unsigned short IMIWORD;               // 16bit unsigned
  typedef unsigned long IMIDWORD;               // 32bit unsigned
  // typedef unsigned __int64 IMIDDWORD;	//64bit unsigned
  
  // typedef signed char IMISBYTE;              //8bit signed
  typedef short IMISWORD;                       // 16bit unsigned
  // typedef long IMISDWORD;                    //32bit unsigned
  // typedef __int64 IMISDDWORD;                //64bit unsigned
  
  // typedef unsigned long IMIDATETIMESEC;      //32bit unsigned
  
  enum TMsgType {
    MSG_ACK_SUCCESS      = 0x00,
    MSG_ACK_FAILURE      = 0x01,
    MSG_ACK_LOGGING      = 0x02,
    MSG_ACK_NOTCONFIG    = 0x03,
    MSG_ACK_INVSTATE     = 0x04,
    
    MSG_CFG_HELLO        = 0x10,
    MSG_CFG_BYE          = 0x11,
    MSG_CFG_FORCESTOP    = 0x12,
    MSG_CFG_STARTCONFIG  = 0x13,
    MSG_CFG_DEVICEINFO   = 0x14,
    MSG_CFG_KEEPCONFIG   = 0x15,
    MSG_CFG_CONFIG_ID    = 0x16,
    MSG_CFG_DEFAULTOZ    = 0x17,
    
    MSG_DECLARATION      = 0x20,
    
    MSG_FLIGHT_INFO      = 0x40,
    MSG_FLIGHT_DELETE    = 0x41,
    MSG_FLIGHT_DELETEALL = 0x42
  };
  
  static const IMIBYTE IMICOMM_SYNC_CHAR1 = 'E';
  static const IMIBYTE IMICOMM_SYNC_CHAR2 = 'X';
  static const unsigned IMICOMM_CRC_LEN = 2;
  static const unsigned COMM_MAX_PAYLOAD_SIZE = 1024;
  
  struct TMsg {
    IMIBYTE syncChar1, syncChar2;
    IMIWORD sn;
    IMIBYTE msgID, parameter1;
    IMIWORD parameter2;
    IMIWORD parameter3;
    IMIWORD payloadSize;
    IMIBYTE payload[COMM_MAX_PAYLOAD_SIZE];
    IMIWORD crc16;
  };
  
  struct TDeviceInfo {
    IMIBYTE device;
    IMIBYTE tampered;
    IMIBYTE hwVersion;
    IMIBYTE swVersion;
    IMIBYTE gps;
    IMIBYTE sensor;
    IMIBYTE flash;
    IMIBYTE eeprom;
    IMIDWORD flashSize;
    IMIDWORD eepromSize;
    IMISWORD sensor0Offset;
    IMISWORD sensor8kOffset;
    IMIWORD buildNumber;
    IMIBYTE reserved[64 - 22];
  } PACKED;
  
#define IMICOMM_MAX_MSG_SIZE (sizeof(TMsg))
#define IMICOMM_MSG_HEADER_SIZE ((unsigned)(&(((TMsg *)0)->payload)))
#define IMICOMM_BIGPARAM1(param) ((IMIBYTE)((param) >> 16))
#define IMICOMM_BIGPARAM2(param) ((IMIWORD)(param))
  
  class CMsgParser {
    enum TState {
      STATE_NOT_SYNC,
      STATE_COMM_MSG
    };
    
    TState _state;
    IMIBYTE _msgBuffer[IMICOMM_MAX_MSG_SIZE];
    unsigned _msgBufferPos;
    unsigned _msgLen;
    
    bool Check(const TMsg *msg, IMIDWORD size) const;
    
  public:
    void Reset();
    const TMsg *Parse(const IMIBYTE buffer[], IMIDWORD size);
  };
  
  static bool _connected;
  static CMsgParser _parser;
  static TDeviceInfo _info;
  static IMIWORD _serialNumber;
  
  // IMI tools
  static IMIWORD CRC16Checksum(const void *message, unsigned bytes);
  static bool Send(PDeviceDescriptor_t d, const TMsg &msg, unsigned errBufSize, TCHAR errBuf[]);
  static bool Send(PDeviceDescriptor_t d, IMIBYTE msgID, unsigned errBufSize, TCHAR errBuf[], const void *payload = 0, IMIWORD payloadSize = 0, IMIBYTE parameter1 = 0, IMIWORD parameter2 = 0);
  static const TMsg *Receive(PDeviceDescriptor_t d, unsigned extraTimeout, unsigned expectedPayloadSize, unsigned errBufSize, TCHAR errBuf[]);
  
  // IMI interface
  static bool Connect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);
  static bool DeclarationWrite(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[]);
  static bool Disconnect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);
  
  // LK interface
  static const TCHAR *GetName();
  static BOOL DeclareTask(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[]);
  static BOOL Install(PDeviceDescriptor_t d);
  
public:
  static bool Register();
};

#endif /* DEVIMI_H */
