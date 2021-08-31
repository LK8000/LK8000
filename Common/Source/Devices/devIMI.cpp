/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

/**
 * IMI driver methods are based on the source code provided by Juraj Rojko from IMI-Gliding.
 */

#include "externs.h"
#include "devIMI.h"
#include "utils/stringext.h"

#ifndef PACKED
#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#error 'PACKED' not defined
#endif
#endif


/* *********************** C O N S T A N T S ************************** */

static const unsigned IMIDECL_PLT_LENGTH = 30;
static const unsigned IMIDECL_CM2_LENGTH = 30;
static const unsigned IMIDECL_GTY_LENGTH = 20;
static const unsigned IMIDECL_GID_LENGTH = 12;
static const unsigned IMIDECL_CID_LENGTH = 4;
static const unsigned IMIDECL_CCL_LENGTH = 20;
static const unsigned IMIDECL_CLB_LENGTH = 20;
static const unsigned IMIDECL_SIT_LENGTH = 20;

static const unsigned IMIDECL_TASK_NAME_LENGTH = 30;

static const unsigned IMIDECL_WP_NAME_LENGTH   = 12;
static const unsigned IMIDECL_MAX_WAYPOINTS    = 15;



/* *********************** M E S S A G E S ************************** */

struct CDevIMI::TDeviceInfo {
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


struct CDevIMI::TDeclarationHeader {
  IMIBYTE id, device;
  IMIWORD sn;
  IMIDWORD flightNumber;
  IMIBYTE hwVersion;
  IMIBYTE swVersion;
  IMIBYTE gps;
  IMIBYTE sensor;
  IMIDATETIMESEC date;
  IMICHAR plt[IMIDECL_PLT_LENGTH];
  IMIBYTE db1Day, db1Month;
  IMIWORD db1Year;
  IMICHAR cm2[IMIDECL_CM2_LENGTH];
  IMIBYTE db2Day, db2Month;
  IMIWORD db2Year;
  IMICHAR gty[IMIDECL_GTY_LENGTH];
  IMICHAR gid[IMIDECL_GID_LENGTH];
  IMICHAR cid[IMIDECL_CID_LENGTH];
  IMICHAR ccl[IMIDECL_CCL_LENGTH];
  IMICHAR clb[IMIDECL_CLB_LENGTH];
  IMICHAR sit[IMIDECL_SIT_LENGTH];
  IMICHAR tskName[IMIDECL_TASK_NAME_LENGTH];
  IMIWORD tskNumber;
  IMIBYTE tskDay, tskMonth;
  IMIWORD tskYear;
  IMIDATETIMESEC recStartDateTime;
  IMIWORD flightOfDay;
  IMIWORD reserved1;
  IMIDATETIMESEC flightStartDateTime;
  IMIBYTE reserved2[28];
} PACKED;


struct CDevIMI::TObservationZone {
  IMIDWORD style:3;        // 0 -> default, 1-5 -> direction of course, the same value as in SeeYou
                           //0 - default = ignore observation zone setting and use default OZ stored in Erixx
                           //1 - fixed angle
                           //2 - symmetrical (invalid for start and finish WP)
                           //3 - to next point (invalid for finish WP)
                           //4 - to prev point (invalid of start WP)
                           //5 - to start point (invalid for start WP)

  IMIDWORD A1:11;          // angle * 10, 0-180 degrees, values 0-1800 (= angle modulo 180 * 10)
  IMIDWORD R1:18;          // radius in meters (max. radius 250km)

  IMIDWORD reduce:1;       // = reduce leg distance (for cylinder for example)
  IMIDWORD move:1;         // = currently not used in Erixx
  IMIDWORD line_only:1;    // = Line only (not cylinder nor sector, angle is ignored)

  IMIDWORD A2:11;          // angle * 10, 0-180 degrees, values 0-1800 (= angle modulo 180 * 10)
  IMIDWORD R2:18;          // radius in meters (max. radius 250km)

  IMIDWORD A12:12;         // angle * 10, 0,0-360,0 (modulo 360 * 10), used when style = 1 = fixed value
  IMIDWORD maxAlt: 14;     // maximum altitude of OZ in meters (0-16km). 0 =ignore maximum altitude

  IMIDWORD reserved: 6;
} PACKED;


struct CDevIMI::TWaypoint {
  IMIDWORD lon:25;
  IMIDWORD reserved1:7;

  IMIDWORD lat:25;
  IMIDWORD reserved2:7;

  IMICHAR name[IMIDECL_WP_NAME_LENGTH];

  TObservationZone oz;
} PACKED;


struct CDevIMI::TDeclaration {
  TDeclarationHeader header;
  TWaypoint wp[IMIDECL_MAX_WAYPOINTS];
  IMIBYTE reserved[sizeof(TWaypoint) - sizeof(IMIWORD)];
  IMIWORD crc16;
} PACKED;


struct CDevIMI::TMsg {
  IMIBYTE syncChar1, syncChar2;
  IMIWORD sn;
  IMIBYTE msgID, parameter1;
  IMIWORD parameter2;
  IMIWORD parameter3;
  IMIWORD payloadSize;
  IMIBYTE payload[COMM_MAX_PAYLOAD_SIZE];
  IMIWORD crc16;
} PACKED;

#define IMICOMM_MAX_MSG_SIZE (sizeof(TMsg))
#define IMICOMM_MSG_HEADER_SIZE ((size_t)(&(((TMsg *)0)->payload)))
#define IMICOMM_BIGPARAM1(param) ((IMIBYTE)((param) >> 16))
#define IMICOMM_BIGPARAM2(param) ((IMIWORD)(param))




/* *********************** M S G   P A R S E R ************************** */

/**
 * @brief Message parser class
 */
class CDevIMI::CMsgParser {
  /**
   * @brief Parser state
   */
  enum TState {
    STATE_NOT_SYNC,                               /**< @brief Synchronization bits not found */
    STATE_COMM_MSG                                /**< @brief Parsing message body */
  };

  TState _state;                                  /**< @brief Parser state */
  IMIBYTE _msgBuffer[IMICOMM_MAX_MSG_SIZE];       /**< @brief Parsed message buffer */
  unsigned _msgBufferPos;                         /**< @brief Current position in a message buffer */
  unsigned _msgBytesLeft;                         /**< @brief Remaining number of bytes of the message to parse */

  bool Check(const TMsg *msg, IMIDWORD size) const;

public:
  void Reset();
  const TMsg *Parse(const IMIBYTE buffer[], IMIDWORD size);
};


/**
 * @brief Resets the state of the parser
 */
void CDevIMI::CMsgParser::Reset()
{
  _msgBytesLeft = 0;
  _msgBufferPos = 0;
  _state = STATE_NOT_SYNC;
}


/**
 * @brief Verifies received message
 *
 * @param msg Message to check
 * @param size Size of received message
 *
 * @return Verification status
 */
bool CDevIMI::CMsgParser::Check(const TMsg *msg, IMIDWORD size) const
{
  // minimal size of comm message
  if(size < IMICOMM_MSG_HEADER_SIZE + IMICOMM_CRC_LEN)
    return false;

  // check signature
  if(msg->syncChar1 != IMICOMM_SYNC_CHAR1 || msg->syncChar2 != IMICOMM_SYNC_CHAR2)
    return false;

  // check size
  if(msg->payloadSize != size - IMICOMM_MSG_HEADER_SIZE - IMICOMM_CRC_LEN)
    return false;

  // check CRC
  IMIWORD crc1 = CDevIMI::CRC16Checksum(((const IMIBYTE*)msg) + IMICOMM_SYNC_LEN, IMICOMM_MSG_HEADER_SIZE + msg->payloadSize - IMICOMM_SYNC_LEN);
  IMIWORD crc2 = (IMIWORD)(((const IMIBYTE*)msg)[size - 1]) | ((IMIWORD)(((const IMIBYTE*)msg)[size - 2]) << 8);
  if(crc1 != crc2)
    return false;

  return true;
}


/**
 * @brief Parses received message chunk
 *
 * @param buffer Buffer with received data
 * @param size The size of received data
 *
 * @return Received message or 0 if invalid on incomplete.
 */
const CDevIMI::TMsg *CDevIMI::CMsgParser::Parse(const IMIBYTE buffer[], IMIDWORD size)
{
  const IMIBYTE *ptr = buffer;
  const TMsg *msg = 0;

  for(;size; size--) {
    IMIBYTE Byte = *ptr++;

    if(_state == STATE_NOT_SYNC) {
      // verify synchronization chars
      if(Byte == IMICOMM_SYNC_CHAR1 && _msgBufferPos == 0) {
        _msgBuffer[_msgBufferPos++] = Byte;
      }
      else if(Byte == IMICOMM_SYNC_CHAR2 && _msgBufferPos == 1) {
        _msgBuffer[_msgBufferPos++] = Byte;
        _state = STATE_COMM_MSG;
      }
      else {
        _msgBufferPos = 0;
      }
    }
    else if(_state == STATE_COMM_MSG) {
      if(_msgBufferPos < IMICOMM_MSG_HEADER_SIZE) {
        // copy header
        _msgBuffer[_msgBufferPos++] = Byte;
      }
      else {
        if(_msgBufferPos == IMICOMM_MSG_HEADER_SIZE) {
          // verify payload size
          IMIWORD payloadSize;
          memmove(&payloadSize, _msgBuffer + offsetof(TMsg, payloadSize), sizeof(payloadSize));
          _msgBytesLeft = payloadSize + IMICOMM_CRC_LEN;
          if(_msgBytesLeft > COMM_MAX_PAYLOAD_SIZE + IMICOMM_CRC_LEN) {
            // Invalid length
            Reset();
            continue;
          }
        }

        // copy payload
        _msgBytesLeft--;
        if(_msgBufferPos < sizeof(_msgBuffer)) // Just in case
          _msgBuffer[_msgBufferPos++] = Byte;

        if(_msgBytesLeft == 0) {
          // end of message
          if(Check((TMsg *)_msgBuffer, _msgBufferPos))
            msg = (TMsg *)_msgBuffer;

          // prepare parser for the next message
          Reset();
        }
      }
    }
  }

  return msg;
}



/* *********************** I M I    D E V I C E ************************** */

bool CDevIMI::_connected;
CDevIMI::CMsgParser CDevIMI::_parser;
CDevIMI::TDeviceInfo CDevIMI::_info;
CDevIMI::IMIWORD CDevIMI::_serialNumber;


/**
 * @brief Calculates IMI CRC value
 *
 * @param message Message for which CRC should be provided
 * @param bytes The size of the message
 *
 * @return IMI CRC value
 */
CDevIMI::IMIWORD CDevIMI::CRC16Checksum(const void *message, unsigned bytes)
{
  const IMIBYTE *pData = (const IMIBYTE *)message;

  IMIWORD crc = 0xFFFF;
  for(;bytes; bytes--) {
    crc  = (IMIBYTE)(crc >> 8) | (crc << 8);
    crc ^= *pData++;
    crc ^= (IMIBYTE)(crc & 0xff) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xff) << 4) << 1;
  }

  if (crc == 0xFFFF)
    crc = 0xAAAA;

  return crc;
}


/**
 * @brief Coordinates converter helper
 */
struct CDevIMI::TAngle
{
  union {
    struct {
      IMIDWORD milliminutes:16;
      IMIDWORD degrees:8;
      IMIDWORD sign:1;
    };
    IMIDWORD value;
  };
};


/**
 * @brief Sets data in IMI Waypoint structure
 *
 * @param decl LK task declaration
 * @param imiIdx The index of IMI waypoint to set
 * @param imiWp IMI waypoint structure to set
 */
void CDevIMI::IMIWaypoint(const Declaration_t &decl, unsigned imiIdx, TWaypoint &imiWp)
{
  unsigned idx = imiIdx == 0 ? 0 :
    (imiIdx == (unsigned)decl.num_waypoints + 1 ? imiIdx - 2 : imiIdx - 1);
  const WAYPOINT &wp = *decl.waypoint[idx];

  // set name
  to_usascii(wp.Name, imiWp.name);

  // set latitude
  TAngle a;
  double angle = wp.Latitude;
  if((a.sign = (angle < 0) ? 1 : 0) != 0)
    angle *= -1;
  a.degrees = static_cast<IMIDWORD>(angle);
  a.milliminutes = static_cast<IMIDWORD>((angle - a.degrees) * 60 * 1000);
  imiWp.lat = a.value;

  // set longitude
  angle = wp.Longitude;
  if((a.sign = (angle < 0) ? 1 : 0) != 0)
    angle *= -1;
  a.degrees = static_cast<IMIDWORD>(angle);
  a.milliminutes = static_cast<IMIDWORD>((angle - a.degrees) * 60 * 1000);
  imiWp.lon = a.value;

  // TAKEOFF and LANDING do not have OZs
  if(imiIdx == 0 || imiIdx == (unsigned)decl.num_waypoints + 1)
    return;

  // set observation zones
  if(imiIdx == 1) {
    // START
    imiWp.oz.style = 3;
    switch(StartLine) {
    case 0: // cylinder
      imiWp.oz.A1 = 1800;
      break;
    case 1: // line
      imiWp.oz.line_only = 1;
      break;
    case 2: // fai sector
      imiWp.oz.A1 = 450;
      break;
    }
    imiWp.oz.R1 = std::min(250000., StartRadius);
  }
  else if(imiIdx == (unsigned)decl.num_waypoints) {
    // FINISH
    imiWp.oz.style = 4;
    switch(FinishLine) {
    case 0: // cylinder
      imiWp.oz.A1 = 1800;
      break;
    case 1: // line
      imiWp.oz.line_only = 1;
      break;
    case 2: // fai sector
      imiWp.oz.A1 = 450;
      break;
    }
    imiWp.oz.R1 = std::min(250000., FinishRadius);
  }
  else {
    // TPs
    if(UseAATTarget()) {
      imiWp.oz.style = 1;
      switch(Task[idx].AATType) {
      case CIRCLE:
        imiWp.oz.A1 = 1800;
        imiWp.oz.R1 = std::min(250000., Task[idx].AATCircleRadius);
        break;
      case SECTOR:
        imiWp.oz.A1 = ((int)(360 + Task[idx].AATFinishRadial - Task[idx].AATStartRadial) % 360) * 10 / 2;
        imiWp.oz.A12 = ((int)(Task[idx].AATStartRadial * 10) + 1800 + imiWp.oz.A1) % 3600;
        imiWp.oz.R1 = std::min(250000., Task[idx].AATSectorRadius);
        break;
      }
    }
    else {
      imiWp.oz.style = 2;
      switch(SectorType) {
      case 0: // cylinder
        imiWp.oz.A1 = 1800;
        imiWp.oz.R1 = std::min(250000., SectorRadius);
        break;
      case 1: // sector
        imiWp.oz.A1 = 450;
        imiWp.oz.R1 = std::min(250000., SectorRadius);
        break;
      case 2: // German DAe 0.5/10
        imiWp.oz.A1 = 450;
        imiWp.oz.R1 = 10000;
        imiWp.oz.A2 = 1800;
        imiWp.oz.R2 = 500;
        break;
      }
    }
  }

  // other unused data
  imiWp.oz.maxAlt = 0;
  imiWp.oz.reduce = 0;
  imiWp.oz.move   = 0;
}


/**
 * @brief Sends message buffer to a device
 *
 * @param d Device handle
 * @param errBufSize The size of the buffer for error string
 * @param errBuf The buffer for error string
 * @param msg IMI message to send
 *
 * @return Operation status
 */
bool CDevIMI::Send(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[], const TMsg &msg)
{
  bool status = ComWrite(d, &msg, IMICOMM_MSG_HEADER_SIZE + msg.payloadSize + 2, errBufSize, errBuf);

  if(status)
    ComFlush(d);

  return status;
}


/**
 * @brief Prepares and sends the message to a device
 *
 * @param d Device handle
 * @param errBufSize The size of the buffer for error string
 * @param errBuf The buffer for error string
 * @param msgID ID of the message to send
 * @param payload Payload buffer to use for the message
 * @param payloadSize The size of the payload buffer
 * @param parameter1 1st parameter for to put in the message
 * @param parameter2 2nd parameter for to put in the message
 * @param parameter3 3rd parameter for to put in the message
 *
 * @return Operation status
 */
bool CDevIMI::Send(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[],
                   IMIBYTE msgID, const void *payload /* =0 */, IMIWORD payloadSize /* =0 */,
                   IMIBYTE parameter1 /* =0 */, IMIWORD parameter2 /* =0 */, IMIWORD parameter3 /* =0 */)
{
  if(payloadSize > COMM_MAX_PAYLOAD_SIZE) {
    return false;
  }

  TMsg msg;
  memset(&msg, 0, sizeof(msg));

  msg.syncChar1 = IMICOMM_SYNC_CHAR1;
  msg.syncChar2 = IMICOMM_SYNC_CHAR2;
  msg.sn = _serialNumber;
  msg.msgID = msgID;
  msg.parameter1 = parameter1;
  msg.parameter2 = parameter2;
  msg.parameter3 = parameter3;
  msg.payloadSize = payloadSize;

  if(payloadSize > 0) {
    assert(payload);
    memcpy(msg.payload, payload, payloadSize);
  }

  IMIWORD crc = CRC16Checksum(((IMIBYTE*)&msg) + 2, payloadSize + IMICOMM_MSG_HEADER_SIZE - 2);
  msg.payload[payloadSize] = (IMIBYTE)(crc >> 8);
  msg.payload[payloadSize + 1] = (IMIBYTE)crc;

  return Send(d, errBufSize, errBuf, msg);
}


/**
 * @brief Receives a message from the device
 *
 * @param d Device handle
 * @param errBufSize The size of the buffer for error string
 * @param errBuf The buffer for error string
 * @param extraTimeout Additional timeout to wait for the message
 * @param expectedPayloadSize Expected size of the message
 *
 * @return Pointer to a message structure if expected message was received or 0 otherwise
 */
const CDevIMI::TMsg *CDevIMI::Receive(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[],
                                      unsigned extraTimeout, unsigned expectedPayloadSize)
{
  if(expectedPayloadSize > COMM_MAX_PAYLOAD_SIZE)
    expectedPayloadSize = COMM_MAX_PAYLOAD_SIZE;

  // set timeout
  unsigned baudRate = d->Com->GetBaudrate();
  if(baudRate == 0U) {
     baudRate = 4800U;
  }
  unsigned timeout = extraTimeout + 10000 * (expectedPayloadSize + sizeof(IMICOMM_MSG_HEADER_SIZE) + 10) / baudRate;
  int orgTimeout;
  if(!SetRxTimeout(d, timeout, orgTimeout, errBufSize, errBuf))
    return 0;

  // wait for the message
  const TMsg *msg = 0;
  PeriodClock timeNow;
  timeNow.Update();
  while( !timeNow.Check(timeout) ) {
    // read message
    IMIBYTE buffer[64];
    IMIDWORD bytesRead = d->Com->Read(buffer, sizeof(buffer));
    if(bytesRead == 0)
      continue;

    // parse message
    const TMsg *lastMsg = _parser.Parse(buffer, bytesRead);
    if(lastMsg) {
      // message received
      if(lastMsg->msgID == MSG_ACK_NOTCONFIG) {
        Disconnect(d, errBufSize, errBuf);
      }
      else if(lastMsg->msgID != MSG_CFG_KEEPCONFIG) {
        msg = lastMsg;
      }
      break;
    }
  }

  // restore timeout
  if(!SetRxTimeout(d, orgTimeout, orgTimeout, errBufSize, errBuf))
    return 0;

  return msg;
}


/**
 * @brief Sends a message and waits for a confirmation from the device
 *
 * @param d Device handle
 * @param errBufSize The size of the buffer for error string
 * @param errBuf The buffer for error string
 * @param msgID ID of the message to send
 * @param payload Payload buffer to use for the message
 * @param payloadSize The size of the payload buffer
 * @param reMsgID Expected ID of the message to receive
 * @param retPayloadSize Expected size of the received message
 * @param parameter1 1st parameter for to put in the message
 * @param parameter2 2nd parameter for to put in the message
 * @param parameter3 3rd parameter for to put in the message
 * @param extraTimeout Additional timeout to wait for the message
 * @param retry Number of send retries
 *
 * @return Pointer to a message structure if expected message was received or 0 otherwise
 */
const CDevIMI::TMsg *CDevIMI::SendRet(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[],
                                      IMIBYTE msgID, const void *payload, IMIWORD payloadSize,
                                      IMIBYTE reMsgID, IMIWORD retPayloadSize,
                                      IMIBYTE parameter1 /* =0 */, IMIWORD parameter2 /* =0 */, IMIWORD parameter3 /* =0 */,
                                      unsigned extraTimeout /* =300 */, int retry /* =4 */)
{
  unsigned baudRate = d->Com->GetBaudrate();
  if(baudRate == 0) {
     baudRate = 4800U;
  }
  extraTimeout += 10000 * (payloadSize + sizeof(IMICOMM_MSG_HEADER_SIZE) + 10) / baudRate;
  while(retry--) {
    if(Send(d, errBufSize, errBuf, msgID, payload, payloadSize, parameter1, parameter2, parameter3)) {
      const TMsg *msg = Receive(d, errBufSize, errBuf, extraTimeout, retPayloadSize);
      if(msg && msg->msgID == reMsgID && (retPayloadSize == (IMIWORD)-1 || msg->payloadSize == retPayloadSize))
        return msg;
    }
  }

  return 0;
}



/**
 * @brief Connects to the device
 *
 * @param d Device handle
 * @param errBufSize The size of the buffer for error string
 * @param errBuf The buffer for error string
 *
 * @return Operation status
 */
bool CDevIMI::Connect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  if(_connected)
    if(!Disconnect(d, errBufSize, errBuf))
      return false;

  _connected = false;
  memset(&_info, 0, sizeof(_info));
  _serialNumber = 0;
  _parser.Reset();

  // check connectivity
  const TMsg *msg = 0;
  for(unsigned i=0; i<IMICOMM_CONNECT_RETRIES_COUNT && (!msg || msg->msgID != MSG_CFG_HELLO); i++) {
    if(Send(d, errBufSize, errBuf, MSG_CFG_HELLO))
      msg = Receive(d, errBufSize, errBuf, 200, 0);
  }
  if(msg) {
    if(msg->msgID == MSG_CFG_HELLO) {
      _serialNumber = msg->sn;
    }
    else {
      // LKTOKEN  _@M1414_ = "Device not responsive!"
      _sntprintf(errBuf, errBufSize, _T("%s"), MsgToken(1414));
      return false;
    }
  }
  else if(errBuf[0] == '\0') {
    // LKTOKEN  _@M1414_ = "Device not responsive!"
    _sntprintf(errBuf, errBufSize, _T("%s"), MsgToken(1414));
    return false;
  }

  // configure baudrate
  unsigned long baudRate = d->Com->GetBaudrate();
  if(baudRate == 0U) {
      baudRate = 4800U;
  }
  if(!Send(d, errBufSize, errBuf, MSG_CFG_STARTCONFIG, 0, 0, IMICOMM_BIGPARAM1(baudRate), IMICOMM_BIGPARAM2(baudRate)))
    return false;

  // get device info
  msg = 0;
  for(int i = 0; i < 5 && (!msg || msg->msgID != MSG_CFG_DEVICEINFO); i++) {
    if(Send(d, errBufSize, errBuf, MSG_CFG_DEVICEINFO))
      msg = Receive(d, errBufSize, errBuf, 400, sizeof(TDeviceInfo));
  }
  if(msg) {
    if(msg->msgID == MSG_CFG_DEVICEINFO) {
      if(msg->payloadSize == sizeof(TDeviceInfo)) {
        memcpy(&_info, msg->payload, sizeof(TDeviceInfo));
      }
      else if(msg->payloadSize == 16) {
        // old version of the structure
        memset(&_info, 0, sizeof(TDeviceInfo));
        memcpy(&_info, msg->payload, 16);
      }
      _connected = true;
      return true;
    }
  }
  else if(errBuf[0] == '\0') {
    // LKTOKEN  _@M1414_ = "Device not responsive!"
    _sntprintf(errBuf, errBufSize, _T("%s"), MsgToken(1414));
    return false;
  }

  return false;
}


/**
 * @brief Sends task declaration
 *
 * @param d Device handle
 * @param decl Task declaration data
 * @param errBufSize The size of the buffer for error string
 * @param errBuf The buffer for error string
 *
 * @return Operation status
 */
bool CDevIMI::DeclarationWrite(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufSize, TCHAR errBuf[])
{
  if(!_connected) {
    // LKTOKEN  _@M1411_ = "Device not connected!"
    _sntprintf(errBuf, errBufSize, _T("%s"), MsgToken(1411));
    return false;
  }

  TDeclaration imiDecl;
  memset(&imiDecl, 0, sizeof(imiDecl));

  // idecl.date ignored - will be set by FR
  to_usascii(decl.PilotName,        imiDecl.header.plt);
  // decl.header.db1Year = year; decl.header.db1Month = month; decl.header.db1Day = day;
  to_usascii(decl.AircraftType,     imiDecl.header.gty);
  to_usascii(decl.AircraftRego,     imiDecl.header.gid);
  to_usascii(decl.CompetitionID,    imiDecl.header.cid);
  to_usascii(decl.CompetitionClass, imiDecl.header.ccl);
  // strncpy(decl.header.clb, idecl.clb, sizeof(decl.header.clb));
  // strncpy(decl.header.sit, idecl.sit, sizeof(decl.header.sit));
  // strncpy(decl.header.cm2, idecl.cm2, sizeof(decl.header.cm2));
  // decl.header.db2Year = year; decl.header.db2Month = month; decl.header.db2Day = day;
  TCHAR tskName[IMIDECL_TASK_NAME_LENGTH];
  TaskFileName(IMIDECL_TASK_NAME_LENGTH, tskName);
  to_usascii(tskName, imiDecl.header.tskName);
  // decl.header.tskYear = year; decl.header.tskMonth = month; decl.header.tskDay = day;
  // decl.header.tskNumber = MIN(9999, idecl.tskNumber);

  IMIWaypoint(decl, 0, imiDecl.wp[0]);
  for(int i=0; i<decl.num_waypoints; i++)
    IMIWaypoint(decl, i + 1, imiDecl.wp[i + 1]);
  IMIWaypoint(decl, decl.num_waypoints + 1, imiDecl.wp[decl.num_waypoints + 1]);

  // send declaration for current task
  const TMsg *msg = SendRet(d, errBufSize, errBuf, MSG_DECLARATION, &imiDecl, sizeof(imiDecl), MSG_ACK_SUCCESS, 0, static_cast<IMIBYTE>(-1));
  if(!msg && errBuf[0] == '\0') {
    // LKTOKEN  _@M1415_ = "Declaration not accepted!"
    _sntprintf(errBuf, errBufSize, _T("%s"), MsgToken(1415));
  }

  return msg;
}


/**
 * @brief Disconnects from the device
 *
 * @param d Device handle
 * @param errBufSize The size of the buffer for error string
 * @param errBuf The buffer for error string
 *
 * @return Operation status
 */
bool CDevIMI::Disconnect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  if(_connected) {
    if(Send(d, errBufSize, errBuf, MSG_CFG_BYE)) {
      _connected = false;
      memset(&_info, 0, sizeof(_info));
      _serialNumber = 0;
      return true;
    }
  }
  return false;
}

BOOL CDevIMI::DeclareTask(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[])
{
  // verify WP number
  if(!CheckWPCount(*decl, 2, 13, errBufSize, errBuf))
    return false;

  // stop Rx thread
  if(!StopRxThread(d, errBufSize, errBuf))
    return false;

  // set new Rx timeout
  int orgRxTimeout;
  bool status = SetRxTimeout(d, 2000, orgRxTimeout, errBufSize, errBuf);
  if(status) {
    // connect to the device
    ShowProgress(decl_enable);
    status = Connect(d, errBufSize, errBuf);
    if(status) {
      // task declaration
      ShowProgress(decl_send);
      status = status && DeclarationWrite(d, *decl, errBufSize, errBuf);
    }

    // disconnect
    ShowProgress(decl_disable);
    status = Disconnect(d, status ? errBufSize : 0, errBuf) && status;

    // restore Rx timeout (we must try that always; don't overwrite error descr)
    status = SetRxTimeout(d, orgRxTimeout, orgRxTimeout, status ? errBufSize : 0, errBuf) && status;
  }

  // restart Rx thread
  status = StartRxThread(d, status ? errBufSize : 0, errBuf) && status;

  return status;
}



void CDevIMI::Install(PDeviceDescriptor_t d)
{
  _connected = false;
  memset(&_info, 0, sizeof(_info));
  _serialNumber = 0;

  _tcscpy(d->Name, GetName());
  d->Declare      = DeclareTask;
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
}
