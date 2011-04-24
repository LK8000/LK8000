/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "StdAfx.h"
#include "devIMI.h"
#include "Dialogs.h"

#include "utils/heapcheck.h"


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
      else
        _msgBufferPos = 0;
    }
    else if(_state == STATE_COMM_MSG) {
      if(_msgBufferPos < IMICOMM_MSG_HEADER_SIZE)
        _msgBuffer[_msgBufferPos++] = byte;
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
          if(Check((TMsg *)_msgBuffer, _msgBufferPos))
            msg = (TMsg *)_msgBuffer;
          
          Reset();
        }
      }
    }
  }
  
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


bool CDevIMI::Send(PDeviceDescriptor_t d, const TMsg &msg, unsigned errBufSize, TCHAR errBuf[])
{
  return ComWrite(d, &msg, IMICOMM_MSG_HEADER_SIZE + msg.payloadSize + 2, errBufSize, errBuf);
}


bool CDevIMI::Send(PDeviceDescriptor_t d, IMIBYTE msgID, unsigned errBufSize, TCHAR errBuf[], const void *payload /* =0 */, IMIWORD payloadSize /* =0 */, IMIBYTE parameter1 /* =0 */, IMIWORD parameter2 /* =0 */)
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
  msg.parameter3 = 0;
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
  int orgTimeout;
  if(!SetRxTimeout(d, timeout, orgTimeout, errBufSize, errBuf))
    return 0;
  
  // read message
  IMIBYTE buffer[64];
  IMIDWORD bytesRead = d->Com->Read(buffer, sizeof(buffer));
  if(bytesRead == 0) {
    // LKTOKEN  _@M1414_ = "Device not responsive!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1414_")));
    return 0;
  }
  const TMsg *msg = 0;
  const TMsg *lastMsg = _parser.Parse(buffer, bytesRead);
  if(lastMsg) {
    if(lastMsg->msgID == MSG_ACK_NOTCONFIG)
      Disconnect(d, errBufSize, errBuf);
    else if(lastMsg->msgID != MSG_CFG_KEEPCONFIG)
      msg = lastMsg;
  }
  
  // restore timeout
  if(!SetRxTimeout(d, orgTimeout, orgTimeout, errBufSize, errBuf))
    return 0;
  
  return msg;
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
      if(msg->msgID == MSG_CFG_HELLO)
        _serialNumber = msg->sn;
      else
        return false;
    }
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
    }
  }
  
  return false;
}


bool CDevIMI::DeclarationWrite(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[])
{
  return true;
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
      status = status && DeclarationWrite(d, decl, errBufSize, errBuf);
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
  return devRegister(GetName(), cap_gps | cap_baro_alt | cap_logger, Install);
}
