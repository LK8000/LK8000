/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "StdAfx.h"
#include "devIMI.h"
#include "Dialogs.h"
#include "utils/stringext.h"
#include "externs.h"

#include "utils/heapcheck.h"


#define IMIDECL_PLT_LENGTH 30
#define IMIDECL_CM2_LENGTH 30
#define IMIDECL_GTY_LENGTH 20 
#define IMIDECL_GID_LENGTH 12
#define IMIDECL_CID_LENGTH 4
#define IMIDECL_CCL_LENGTH 20
#define IMIDECL_CLB_LENGTH 20
#define IMIDECL_SIT_LENGTH 20

#define IMIDECL_TASK_NAME_LENGTH 30

#define IMIDECL_WP_NAME_LENGTH 12
#define IMIDECL_MAX_WAYPOINTS 15


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
  
  
struct CDevIMI::TAngle
{
  union {
    struct {
      IMIDWORD milliminutes:16;
      IMIDWORD degrees:8;
      IMIDWORD sign:1;
    };
    struct {
      IMIDWORD milliminutes:16;
      IMIDWORD degrees:8;
      IMIDWORD sign:1;
    }x;
    IMIDWORD value;
  };
};


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


  
#define IMICOMM_MSG_HEADER_SIZE ((unsigned)(&(((TMsg *)0)->payload)))
#define IMICOMM_BIGPARAM1(param) ((IMIBYTE)((param) >> 16))
#define IMICOMM_BIGPARAM2(param) ((IMIWORD)(param))
  
bool CDevIMI::_connected;
CDevIMI::CMsgParser CDevIMI::_parser;
CDevIMI::TDeviceInfo CDevIMI::_info;
CDevIMI::IMIWORD CDevIMI::_serialNumber;
  


void CDevIMI::CMsgParser::Reset()
{
  _msgLen = 0;
  _msgBufferPos = 0;
  _state = STATE_NOT_SYNC;
}


const CDevIMI::TMsg *CDevIMI::CMsgParser::Parse(const IMIBYTE buffer[], IMIDWORD size)
{
  const IMIBYTE *ptr = buffer;
  const TMsg *msg = 0;
  
  for(;size; size--) {
    IMIBYTE byte = *ptr++;
    
    if(_state == STATE_NOT_SYNC) {
      if(byte == IMICOMM_SYNC_CHAR1 && _msgBufferPos == 0) {
        _msgBuffer[_msgBufferPos++] = byte;
      }
      else if(byte == IMICOMM_SYNC_CHAR2 && _msgBufferPos == 1) {
        _msgBuffer[_msgBufferPos++] = byte;
        _state = STATE_COMM_MSG;
      }
      else {
        _msgBufferPos = 0;
      }
    }
    else if(_state == STATE_COMM_MSG) {
      if(_msgBufferPos < IMICOMM_MSG_HEADER_SIZE) {
        _msgBuffer[_msgBufferPos++] = byte;
      }
      else {
        if(_msgBufferPos == IMICOMM_MSG_HEADER_SIZE) {
          _msgLen = ((TMsg *)_msgBuffer)->payloadSize + IMICOMM_CRC_LEN;
          
          if(_msgLen > COMM_MAX_PAYLOAD_SIZE + IMICOMM_CRC_LEN) {
            // Invalid length
            Reset();
            continue;
          }
        }
        
        _msgLen--;
        
        if(_msgBufferPos < sizeof(_msgBuffer)) // Just in case
          _msgBuffer[_msgBufferPos++] = byte;
        
        if(_msgLen == 0) {
          if(Check((TMsg *)_msgBuffer, _msgBufferPos)) {
            msg = (TMsg *)_msgBuffer;
          }
          else {
          }
          Reset();
        }
      }
    }
  }

  if(_msgBufferPos == IMICOMM_MSG_HEADER_SIZE)
    msg = (TMsg *)_msgBuffer;
  
  return msg;
}


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
  
  IMIWORD crc1 = CDevIMI::CRC16Checksum(((IMIBYTE*)msg) + 2, IMICOMM_MSG_HEADER_SIZE + msg->payloadSize - 2);
  IMIWORD crc2 = (IMIWORD)(((IMIBYTE*)msg)[size - 1]) | ((IMIWORD)(((IMIBYTE*)msg)[size - 2]) << 8);
  if(crc1 != crc2)
    return false;
  
  return true;
}




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


void CDevIMI::IMIWaypoint(const Declaration_t &decl, unsigned imiIdx, TWaypoint &imiWp)
{
  unsigned idx = imiIdx == 0 ? 0 :
    (imiIdx == (unsigned)decl.num_waypoints + 1 ? imiIdx - 2 : imiIdx - 1);
  const WAYPOINT &wp = *decl.waypoint[idx];
  unicode2usascii(wp.Name, imiWp.name, sizeof(imiWp.name));
  
  TAngle a;
  double angle = wp.Latitude;
  if((a.sign = (angle < 0) ? 1 : 0) != 0)
    angle *= -1;
  a.degrees = angle;
  a.milliminutes = (angle - a.degrees) * 60 * 1000;
  imiWp.lat = a.value;
  
  angle = wp.Longitude;
  if((a.sign = (angle < 0) ? 1 : 0) != 0)
    angle *= -1;
  a.degrees = angle;
  a.milliminutes = (angle - a.degrees) * 60 * 1000;
  imiWp.lon = a.value;
  
  if(imiIdx == 0 || imiIdx == (unsigned)decl.num_waypoints + 1)
    return;
  
  if(imiIdx == 1) {
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
    imiWp.oz.R1 = std::min((DWORD)250000, StartRadius);
  }
  else if(imiIdx == (unsigned)decl.num_waypoints) {
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
    imiWp.oz.R1 = std::min((DWORD)250000, FinishRadius);
  }
  else {
    imiWp.oz.style = 2;
    switch(SectorType) {
    case 0: // cylinder
      imiWp.oz.A1 = 1800;
      imiWp.oz.R1 = std::min((DWORD)250000, SectorRadius);
      break;
    case 1: // sector
      imiWp.oz.A1 = 450;
      imiWp.oz.R1 = std::min((DWORD)250000, SectorRadius);
      break;
    case 2: // German DAe 0.5/10
      imiWp.oz.A1 = 450;
      imiWp.oz.R1 = 10000;
      imiWp.oz.A2 = 1800;
      imiWp.oz.R2 = 500;
      break;
    }
  }
  
  imiWp.oz.maxAlt = 0;
  imiWp.oz.reduce = 0;
  imiWp.oz.move   = 0;
}


bool CDevIMI::Send(PDeviceDescriptor_t d, const TMsg &msg, unsigned errBufSize, TCHAR errBuf[])
{
  return ComWrite(d, &msg, IMICOMM_MSG_HEADER_SIZE + msg.payloadSize + 2, errBufSize, errBuf);
}


bool CDevIMI::Send(PDeviceDescriptor_t d, IMIBYTE msgID, unsigned errBufSize, TCHAR errBuf[], const void *payload /* =0 */, IMIWORD payloadSize /* =0 */, IMIBYTE parameter1 /* =0 */, IMIWORD parameter2 /* =0 */, IMIWORD parameter3 /* =0 */)
{
  if(payloadSize > COMM_MAX_PAYLOAD_SIZE)
    return false;
  
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
  memcpy(msg.payload, payload, payloadSize);
  
  IMIWORD crc = CRC16Checksum(((IMIBYTE*)&msg) + 2, payloadSize + IMICOMM_MSG_HEADER_SIZE - 2); 
  msg.payload[payloadSize] = (IMIBYTE)(crc >> 8);
  msg.payload[payloadSize + 1] = (IMIBYTE)crc;
  
  return Send(d, msg, errBufSize, errBuf);
}


const CDevIMI::TMsg *CDevIMI::Receive(PDeviceDescriptor_t d, unsigned extraTimeout, unsigned expectedPayloadSize, unsigned errBufSize, TCHAR errBuf[])
{
  if(expectedPayloadSize > COMM_MAX_PAYLOAD_SIZE)
    expectedPayloadSize = COMM_MAX_PAYLOAD_SIZE;
  
  // set timeout
  unsigned timeout = extraTimeout + 10000 * (expectedPayloadSize + sizeof(IMICOMM_MSG_HEADER_SIZE) + 10) / d->Com->GetBaudrate();
  //  unsigned timeout = 1000;
  int orgTimeout;
  if(!SetRxTimeout(d, timeout, orgTimeout, errBufSize, errBuf))
    return 0;
  
  const TMsg *msg = 0;
  timeout += GetTickCount();
  while(GetTickCount() < timeout) {
    // read message
    IMIBYTE buffer[64];
    IMIDWORD bytesRead = d->Com->Read(buffer, sizeof(buffer));
    if(bytesRead == 0)
      continue;
    
    const TMsg *lastMsg = _parser.Parse(buffer, bytesRead);
    if(lastMsg) {
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


const CDevIMI::TMsg *CDevIMI::SendRet(PDeviceDescriptor_t d, IMIBYTE msgID, const void *payload, IMIWORD payloadSize, 
                                      IMIBYTE reMsgID, IMIWORD retPayloadSize, unsigned errBufSize, TCHAR errBuf[],
                                      IMIBYTE parameter1 /* =0 */, IMIWORD parameter2 /* =0 */, IMIWORD parameter3 /* =0 */,
                                      unsigned extraTimeout /* =300 */, int retry /* =4 */)
{
  unsigned baudRate = d->Com->GetBaudrate();
  extraTimeout += 10000 * (payloadSize + sizeof(IMICOMM_MSG_HEADER_SIZE) + 10) / baudRate;
  while(retry--) {
    if(Send(d, msgID, errBufSize, errBuf, payload, payloadSize, parameter1, parameter2, parameter3)) {
      const TMsg *msg = Receive(d, extraTimeout, retPayloadSize, errBufSize, errBuf);
      if(msg && msg->msgID == reMsgID && (retPayloadSize == (IMIWORD)-1 || msg->payloadSize == retPayloadSize)) {
        return msg;
      }
    }
  }
  
  return 0;
}



bool CDevIMI::Connect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  if(_connected)
    Disconnect(d, errBufSize, errBuf);
  _connected = false;
  memset(&_info, 0, sizeof(_info));
  _serialNumber = 0;
  _parser.Reset();
  
  // check connectivity
  if(Send(d, MSG_CFG_HELLO, errBufSize, errBuf)) {
    const TMsg *msg = Receive(d, 100, 0, errBufSize, errBuf);
    if(msg) {
      if(msg->msgID == MSG_CFG_HELLO) {
        _serialNumber = msg->sn;
      }
      else {
        // LKTOKEN  _@M1414_ = "Device not responsive!"
        _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1414_")));
        return false;
      }
    }
    else if(errBuf[0] == '\0') {
      // LKTOKEN  _@M1414_ = "Device not responsive!"
      _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1414_")));
      return false;
    }
  }
  else {
    // LKTOKEN  _@M1414_ = "Device not responsive!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1414_")));
    return false;
  }
  
  // configure baudrate
  unsigned long baudRate = d->Com->GetBaudrate();
  if(!Send(d, MSG_CFG_STARTCONFIG, errBufSize, errBuf, 0, 0, IMICOMM_BIGPARAM1(baudRate), IMICOMM_BIGPARAM2(baudRate)))
    return false;
  
  // get device info
  for(int i = 0; i < 4; i++) {
    if(Send(d, MSG_CFG_DEVICEINFO, errBufSize, errBuf)) {
      const TMsg *msg = Receive(d, 300, sizeof(TDeviceInfo), errBufSize, errBuf);
      if(msg) {
        if(msg->msgID == MSG_CFG_DEVICEINFO && msg->payloadSize == sizeof(TDeviceInfo)) {
          memcpy(&_info, msg->payload, sizeof(TDeviceInfo));
          _connected = true;
          return true;
        }
        else if(msg->msgID == MSG_CFG_DEVICEINFO && msg->payloadSize == 16) {
          // old version of the structure
          memset(&_info, 0, sizeof(TDeviceInfo));
          memcpy(&_info, msg->payload, 16);
          _connected = true;
          return true;
        }
      }
      else if(errBuf[0] == '\0') {
        // LKTOKEN  _@M1414_ = "Device not responsive!"
        _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1414_")));
        return false;
      }
    }
  }
  
  return false;
}


bool CDevIMI::DeclarationWrite(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufSize, TCHAR errBuf[])
{
  if(!_connected) {
    // LKTOKEN  _@M1411_ = "Device not connected!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1411_")));
    return false;
  }
  
  TDeclaration imiDecl;
  memset(&imiDecl, 0, sizeof(imiDecl));
  
  // idecl.date ignored - will be set by FR
  unicode2usascii(decl.PilotName,        imiDecl.header.plt, sizeof(imiDecl.header.plt));
  // decl.header.db1Year = year; decl.header.db1Month = month; decl.header.db1Day = day;
  unicode2usascii(decl.AircraftType,     imiDecl.header.gty, sizeof(imiDecl.header.gty));
  unicode2usascii(decl.AircraftRego,     imiDecl.header.gid, sizeof(imiDecl.header.gid));
  unicode2usascii(decl.CompetitionID,    imiDecl.header.cid, sizeof(imiDecl.header.cid));
  unicode2usascii(decl.CompetitionClass, imiDecl.header.ccl, sizeof(imiDecl.header.ccl));
  // strncpy(decl.header.clb, idecl.clb, sizeof(decl.header.clb));
  // strncpy(decl.header.sit, idecl.sit, sizeof(decl.header.sit));
  // strncpy(decl.header.cm2, idecl.cm2, sizeof(decl.header.cm2));
  // decl.header.db2Year = year; decl.header.db2Month = month; decl.header.db2Day = day;
  TCHAR tskName[IMIDECL_TASK_NAME_LENGTH];
  TaskFileName(IMIDECL_TASK_NAME_LENGTH, tskName);
  unicode2usascii(tskName, imiDecl.header.tskName, sizeof(imiDecl.header.tskName));
  // decl.header.tskYear = year; decl.header.tskMonth = month; decl.header.tskDay = day;
  // decl.header.tskNumber = MIN(9999, idecl.tskNumber);
  
  IMIWaypoint(decl, 0, imiDecl.wp[0]);
  for(int i=0; i<decl.num_waypoints; i++)
    IMIWaypoint(decl, i + 1, imiDecl.wp[i + 1]);
  IMIWaypoint(decl, decl.num_waypoints + 1, imiDecl.wp[decl.num_waypoints + 1]);
  
  const TMsg *msg = SendRet(d, MSG_DECLARATION, &imiDecl, sizeof(imiDecl), MSG_ACK_SUCCESS, 0, errBufSize, errBuf);
  return msg != 0;
}


bool CDevIMI::Disconnect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  if(_connected) {
    if(Send(d, MSG_CFG_BYE, errBufSize, errBuf)) {
      _connected = false;
      memset(&_info, 0, sizeof(_info));
      _serialNumber = 0;
      return true;
    }
  }
  return false;
}



const TCHAR *CDevIMI::GetName()
{
  return(_T("IMI ERIXX"));
}


BOOL CDevIMI::DeclareTask(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[])
{
  if(!CheckWPCount(*decl, 2, 13, errBufSize, errBuf))
    return false;
  
  // stop RX thread
  if(!StopRxThread(d, errBufSize, errBuf))
    return false;
  
  // set new Rx timeout
  int orgRxTimeout;
  bool status = SetRxTimeout(d, 2000, orgRxTimeout, errBufSize, errBuf);
  if(status) {
    ShowProgress(decl_enable);
    status = Connect(d, errBufSize, errBuf);
    if(status) {
      ShowProgress(decl_send);
      status = status && DeclarationWrite(d, *decl, errBufSize, errBuf);
    }
    
    ShowProgress(decl_disable);
    status = Disconnect(d, status ? errBufSize : 0, errBuf) && status;
    
    // restore Rx timeout (we must try that always; don't overwrite error descr)
    status = SetRxTimeout(d, orgRxTimeout, orgRxTimeout, status ? errBufSize : 0, errBuf) && status;
  }
  
  status = StartRxThread(d, status ? errBufSize : 0, errBuf) && status;
  
  return status;
}



BOOL CDevIMI::Install(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = NULL;
  d->PutMacCready = NULL;
  d->PutBugs      = NULL;
  d->PutBallast   = NULL;
  d->Open         = NULL;
  d->Close        = NULL;
  d->Init         = NULL;
  d->LinkTimeout  = NULL;
  d->Declare      = DeclareTask;
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  
  return TRUE;
}


bool CDevIMI::Register()
{
  _connected = false;
  memset(&_info, 0, sizeof(_info));
  _serialNumber = 0;
  
  return devRegister(GetName(), cap_gps | cap_baro_alt | cap_logger, Install);
}
