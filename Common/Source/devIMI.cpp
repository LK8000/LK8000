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


const TCHAR *CDevIMI::GetName()
{
  return(_T("IMI ERIXX"));
}


bool CDevIMI::Connect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  return true;
}


bool CDevIMI::DeclarationWrite(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[])
{
  return true;
}


bool CDevIMI::Disconnect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  return true;
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
