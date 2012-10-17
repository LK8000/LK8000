/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: devCaiGpsNav.cpp,v 8.2 2010/12/13 10:04:35 root Exp root $
*/


// CAUTION!
// caiGpsNavParseNMEA is called from com port read thread
// all other functions are called from windows message loop thread


#include "externs.h"

#include "devCaiGpsNav.h"


const TCHAR *CDevCAIGpsNav::GetName()
{
  return(_T("CAI GPS-NAV"));
}


BOOL CDevCAIGpsNav::Init(DeviceDescriptor_t *d)
{
  if(!SIMMODE) {
    d->Com->WriteString(TEXT("\x03"));
    Sleep(50);
    d->Com->WriteString(TEXT("NMEA\r"));
	  
    // This is for a slightly different mode, that
    // apparently outputs pressure info too...
    //(d->Com.WriteString)(TEXT("PNP\r\n"));
    //(d->Com.WriteString)(TEXT("LOG 0\r\n"));
  }
  
  return(TRUE);
}


BOOL CDevCAIGpsNav::DeclareTask(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[])
{
  return false;
}


BOOL CDevCAIGpsNav::Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = NULL;
  d->PutMacCready = NULL;
  d->PutBugs      = NULL;
  d->PutBallast   = NULL;
  d->Open         = NULL;
  d->Close        = NULL;
  d->Init         = Init;
  d->LinkTimeout  = NULL;
  d->Declare      = DeclareTask;
  // There is currently no support for task declaration
  // from XCSoar
  d->IsLogger     = GetFalse;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  
  return TRUE;
}


bool CDevCAIGpsNav::Register()
{
  return devRegister(GetName(), cap_gps | cap_baro_alt | cap_logger, Install);
}
