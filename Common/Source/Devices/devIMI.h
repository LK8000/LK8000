/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: devIMI.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#ifndef	DEVIMI_H
#define	DEVIMI_H

#include "devBase.h"
#include "Devices/DeviceRegister.h"

/**
 * @brief IMI-Gliding ERIXX device class
 *
 * Class provides support for IMI-Gliding ERIXX IGC certifed logger.
 *
 * @note IMI driver methods are based on the source code provided by Juraj Rojko from IMI-Gliding.
 */
class CDevIMI : public DevBase
{
  typedef char IMICHAR;                         // 8bit text character
  typedef unsigned char IMIBYTE;                // 8bit unsigned
  typedef unsigned short IMIWORD;               // 16bit unsigned
  typedef uint32_t IMIDWORD;                    // 32bit unsigned
  typedef short IMISWORD;                       // 16bit unsigned
  typedef uint32_t IMIDATETIMESEC;              // 32bit unsigned

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
    MSG_FLIGHT_DELETEALL = 0x42
  };

  // messages
  struct TDeviceInfo;
  struct TDeclarationHeader;
  struct TObservationZone;
  struct TWaypoint;
  struct TDeclaration;
  struct TMsg;

  // helpers
  struct TAngle;

  // message parser
  class CMsgParser;

  // constants
  static const unsigned IMICOMM_CONNECT_RETRIES_COUNT = 10;
  static const IMIBYTE IMICOMM_SYNC_CHAR1             = 'E';
  static const IMIBYTE IMICOMM_SYNC_CHAR2             = 'X';
  static const unsigned IMICOMM_SYNC_LEN              = 2;
  static const unsigned IMICOMM_CRC_LEN               = 2;
  static const unsigned COMM_MAX_PAYLOAD_SIZE         = 1024;

  // variables
  static bool _connected;
  static CMsgParser _parser;
  static TDeviceInfo _info;
  static IMIWORD _serialNumber;

  // IMI tools
  static IMIWORD CRC16Checksum(const void *message, unsigned bytes);
  static void IMIWaypoint(const Declaration_t &decl, unsigned imiIdx, TWaypoint &imiWp);
  static bool Send(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[], const TMsg &msg);
  static bool Send(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[],
                   IMIBYTE msgID, const void *payload = 0, IMIWORD payloadSize = 0,
                   IMIBYTE parameter1 = 0, IMIWORD parameter2 = 0, IMIWORD parameter3 = 0);
  static const TMsg *Receive(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[],
                             unsigned extraTimeout, unsigned expectedPayloadSize);
  static const TMsg *SendRet(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[],
                             IMIBYTE msgID, const void *payload, IMIWORD payloadSize,
                             IMIBYTE reMsgID, IMIWORD retPayloadSize,
                             IMIBYTE parameter1 = 0, IMIWORD parameter2 = 0, IMIWORD parameter3 = 0,
                             unsigned extraTimeout = 500, int retry = 4);

  // IMI interface
  static bool Connect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);
  static bool DeclarationWrite(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufSize, TCHAR errBuf[]);
  static bool Disconnect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);

  // LK interface
  static BOOL DeclareTask(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[]);
  static void Install(PDeviceDescriptor_t d);

  static constexpr 
  const TCHAR *GetName() {
    return _T("IMI ERIXX");
  }

public:
  
  static constexpr
  DeviceRegister_t Register() {
    return devRegister(GetName(), Install);
  }
};

#endif /* DEVIMI_H */
