/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: device.cpp,v 8.6 2010/12/13 10:21:06 root Exp root $
*/

#include "externs.h"
#include "Dialogs/dlgProgress.h"
#include "utils/stl_utils.h"

// A note about locking.
//  The ComPort RX threads lock using FlightData critical section.
//  ComPort::StopRxThread and ComPort::Close both wait for these threads to
//  exit before returning.  Both these functions are called with the Comm
//  critical section locked.  Therefore, there is a locking dependency upon
//  Comm -> FlightData.
//
//  If someone locks FlightData and then Comm, there is a high possibility
//  of deadlock.  So, FlightData must never be locked after Comm.  Ever.
//  Thankfully WinCE "critical sections" are recursive locks.


//  remember that so still COMM port definitions have to be duplicated also inside dlgConfiguration... 
//  change COMn: to \\.\COMn  without :  so that we can use COM10-COM99 devices

#if (WINDOWSPC>0)
static  const TCHAR *COMMPort[] = {TEXT("COM1"),TEXT("COM2"),TEXT("COM3"),TEXT("COM4"),TEXT("COM5"),TEXT("COM6"),TEXT("COM7"),TEXT("COM8"),TEXT("COM9"),TEXT("COM10"),
TEXT("COM11"),TEXT("COM12"),TEXT("COM13"),TEXT("COM14"),TEXT("COM15"),TEXT("COM16"),TEXT("COM17"),TEXT("COM18"),TEXT("COM19"),
TEXT("COM20"),TEXT("COM21"),TEXT("COM22"),TEXT("COM23"),TEXT("COM24"),TEXT("COM25"),TEXT("COM26"),TEXT("COM27"),TEXT("COM28"),
TEXT("COM29"),TEXT("COM30"),TEXT("COM31"),TEXT("COM32"),
TEXT("COM33"),TEXT("COM34"),TEXT("COM35"),TEXT("COM36"),TEXT("COM37"),TEXT("COM38"),TEXT("COM39"),TEXT("COM40"),TEXT("COM0")};
#else
static  const TCHAR *COMMPort[] = {TEXT("COM1:"),TEXT("COM2:"),TEXT("COM3:"),TEXT("COM4:"),TEXT("COM5:"),TEXT("COM6:"),TEXT("COM7:"),TEXT("COM8:"),TEXT("COM9:"),TEXT("COM10:"),TEXT("COM0:"),TEXT("VSP0:"),TEXT("VSP1:")};
#endif
static  const DWORD   dwSpeed[] = {1200,2400,4800,9600,19200,38400,57600,115200};
static  DWORD PortIndex1 = 0;
static  DWORD SpeedIndex1 = 2;
static  DWORD PortIndex2 = 0;
static  DWORD SpeedIndex2 = 2;
static  DWORD Bit1Index = (BitIndex_t)bit8N1;
static  DWORD Bit2Index = (BitIndex_t)bit8N1;
// static  DWORD Bit3Index = (BitIndex_t)bit8N1;

DeviceRegister_t   DeviceRegister[NUMREGDEV];
DeviceDescriptor_t DeviceList[NUMDEV];

DeviceDescriptor_t *pDevPrimaryBaroSource=NULL;
DeviceDescriptor_t *pDevSecondaryBaroSource=NULL;

int DeviceRegisterCount = 0;

static BOOL FlarmDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]);
static void devFormatNMEAString(TCHAR *dst, size_t sz, const TCHAR *text);

// This function is used to determine whether a generic
// baro source needs to be used if available
BOOL devHasBaroSource(void) {
  if (pDevPrimaryBaroSource || pDevSecondaryBaroSource) {
    return TRUE;
  } else {
    return FALSE;
  }
}

#if 0
BOOL devGetBaroAltitude(double *Value){
  // hack, just return GPS_INFO->BaroAltitude
  if (Value == NULL)
    return(FALSE);
  if (GPS_INFO.BaroAltitudeAvailable)
    *Value = GPS_INFO.BaroAltitude;
  return(TRUE);

}
#endif

BOOL ExpectString(PDeviceDescriptor_t d, const TCHAR *token){

  int i=0, ch;

  if (!d->Com)
    return FALSE;

  while ((ch = d->Com->GetChar()) != EOF){

    if (token[i] == (unsigned)ch) 
      i++;
    else
      i=0;

    if ((unsigned)i == _tcslen(token))
      return(TRUE);

  }

  return(FALSE);

}


BOOL devRegister(const TCHAR *Name, int Flags, 
                 BOOL (*Installer)(PDeviceDescriptor_t d)) {
  if (DeviceRegisterCount >= NUMREGDEV) {
    LKASSERT(FALSE);
    return(FALSE);
  }
  DeviceRegister[DeviceRegisterCount].Name = Name;
  DeviceRegister[DeviceRegisterCount].Flags = Flags;
  DeviceRegister[DeviceRegisterCount].Installer = Installer;
  DeviceRegisterCount++;
  return(TRUE);
}

BOOL devRegisterGetName(int Index, TCHAR *Name){
  Name[0] = '\0';
  if (Index < 0 || Index >= DeviceRegisterCount) 
    return (FALSE);
  _tcscpy(Name, DeviceRegister[Index].Name);
  return(TRUE);
}

// This device is not available if Disabled
// Index 0 or 1 
bool devIsDisabled(int Index) {
  if (Index < 0 || Index >1)
	return (true);
	
  return DeviceList[Index].Disabled;
}

static int devIsFalseReturn(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);

}


BOOL devInit(LPCTSTR CommandLine){
  int i;
  TCHAR DeviceName[DEVNAMESIZE+1];
  PDeviceDescriptor_t pDevNmeaOut = NULL;
  static bool doinit=true;

  for (i=0; i<NUMDEV; i++){
    DeviceList[i].Port = -1;
    DeviceList[i].fhLogFile = NULL;
    DeviceList[i].Name[0] = '\0';
    DeviceList[i].ParseNMEA = NULL;
    DeviceList[i].PutMacCready = NULL;
    DeviceList[i].DirectLink = NULL;
    DeviceList[i].PutBugs = NULL;
    DeviceList[i].PutBallast = NULL;
    DeviceList[i].Open = NULL;
    DeviceList[i].Close = NULL;
    DeviceList[i].Init = NULL;
    DeviceList[i].LinkTimeout = NULL;
    DeviceList[i].Declare = NULL;
    DeviceList[i].IsLogger = devIsFalseReturn;
    DeviceList[i].IsGPSSource = devIsFalseReturn;
    DeviceList[i].IsBaroSource = devIsFalseReturn;
    DeviceList[i].IsRadio = devIsFalseReturn;

    DeviceList[i].PutVoice = (int (*)(struct DeviceDescriptor_t *,TCHAR *))devIsFalseReturn;
    DeviceList[i].PortNumber = i;
    DeviceList[i].PutQNH = NULL;
    DeviceList[i].OnSysTicker = NULL;

    DeviceList[i].pDevPipeTo = NULL;
    DeviceList[i].PutVolume = NULL;
    DeviceList[i].PutFreqActive = NULL;
    DeviceList[i].PutFreqStandby = NULL;
    DeviceList[i].IsCondor = devIsFalseReturn;
    DeviceList[i].Disabled = true;

    ComPortStatus[i]=CPS_UNUSED; // 100210
    ComPortHB[i]=0; // counter
    if (doinit) {
	ComPortRx[i]=0;
	ComPortTx[i]=0;
	ComPortErrTx[i]=0;
	ComPortErrRx[i]=0;
	ComPortErrors[i]=0;

	doinit=false;
    }
  }

  pDevPrimaryBaroSource = NULL;
  pDevSecondaryBaroSource=NULL;

  ReadDeviceSettings(0, DeviceName);
  #ifdef DEBUG_DEVSETTING
  StartupStore(_T(".......... ReadDeviceSetting 0, DeviceName=<%s>\n"),DeviceName);
  #endif
	
  const DWORD  maxPortIndex = std::distance(begin(COMMPort), end(COMMPort)) - 1;
  
  PortIndex1 = 0; SpeedIndex1 = 2; Bit1Index=(BitIndex_t)bit8N1;
  ReadPort1Settings(&PortIndex1,&SpeedIndex1,&Bit1Index);
  PortIndex1 = std::min(maxPortIndex, PortIndex1);

  //if (_tcslen(DeviceName)>0) // removed 110530
  if (wcscmp(DeviceName,_T(DEV_DISABLED_NAME))!=0) {
	DeviceList[0].Disabled=false;
	StartupStore(_T(". Device A is <%s> Port=%s%s"),DeviceName,COMMPort[PortIndex1],NEWLINE);
  } else {
	DeviceList[0].Disabled=true;
	StartupStore(_T(". Device A is DISABLED.%s"),NEWLINE);
  }

  for (i=DeviceRegisterCount-1; i>=0; i--) {
    if (DeviceList[0].Disabled) break;

    if ((_tcscmp(DeviceRegister[i].Name, DeviceName) == 0)) {

      ComPort *Com = new ComPort(0);

      // remember: Port1 is the port used by device A, port1 may be Com3 or Com1 etc
	// this is port 1, so index 0 for us. 
      if (!Com->Initialize(COMMPort[PortIndex1], dwSpeed[SpeedIndex1],Bit1Index,0)) {
	   	delete Com;
		ComPortStatus[0]=CPS_OPENKO;
        break;
      }
      ComPortStatus[0]=CPS_OPENOK;

      DeviceRegister[i].Installer(devA());

      if ((pDevNmeaOut == NULL) && 
	  (DeviceRegister[i].Flags & (1l << dfNmeaOut))){
        pDevNmeaOut = devA();
      }

      devA()->Com = Com;

      devInit(devA());
      devOpen(devA(), 0);

      if (devIsBaroSource(devA())) {
        if (pDevPrimaryBaroSource == NULL){
          pDevPrimaryBaroSource = devA();
        } else 
        if (pDevSecondaryBaroSource == NULL){
          pDevSecondaryBaroSource = devA();
        }
      }
      break;
    }
  }

  ReadDeviceSettings(1, DeviceName);
  #ifdef DEBUG_DEVSETTING
  StartupStore(_T(".......... ReadDeviceSetting 1, DeviceName=<%s>\n"),DeviceName);
  #endif

  PortIndex2 = 0; SpeedIndex2 = 2, Bit2Index=(BitIndex_t)bit8N1;
  ReadPort2Settings(&PortIndex2,&SpeedIndex2, &Bit2Index);
  PortIndex2 = std::min(maxPortIndex, PortIndex2);

  //if (_tcslen(DeviceName)>0) // removed 110530
  if (wcscmp(DeviceName,_T(DEV_DISABLED_NAME))!=0) {
	DeviceList[1].Disabled=false;
	StartupStore(_T(". Device B is <%s> Port=%s%s"),DeviceName,COMMPort[PortIndex2],NEWLINE);
  } else {
	DeviceList[1].Disabled=true;
	StartupStore(_T(". Device B is DISABLED.%s"),NEWLINE);
  }

  for (i=DeviceRegisterCount-1; i>=0; i--) {
    if (PortIndex1 == PortIndex2) break;
    if (DeviceList[1].Disabled) break;

    if ((_tcscmp(DeviceRegister[i].Name, DeviceName) == 0)) {
      ComPort *Com = new ComPort(1);

	// this is port 2, so index 1 for us
      if (!Com->Initialize(COMMPort[PortIndex2], dwSpeed[SpeedIndex2],Bit2Index,1)) { // 100210
	delete Com;
	ComPortStatus[1]=CPS_OPENKO;
        break;
      }
      ComPortStatus[1]=CPS_OPENOK;

      DeviceRegister[i].Installer(devB());

      if ((pDevNmeaOut == NULL) && 
          (DeviceRegister[i].Flags & (1l << dfNmeaOut))){
        pDevNmeaOut = devB();
      }

      devB()->Com = Com;

      devInit(devB());
      devOpen(devB(), 1);

      if (devIsBaroSource(devB())) {
        if (pDevPrimaryBaroSource == NULL){
          pDevPrimaryBaroSource = devB();
        } else 
        if (pDevSecondaryBaroSource == NULL){
          pDevSecondaryBaroSource = devB();
        }
      }

      break;
    }
  }

  if (pDevNmeaOut != NULL){
    if (pDevNmeaOut == devA()){
      devB()->pDevPipeTo = devA();
    }
    if (pDevNmeaOut == devB()){
      devA()->pDevPipeTo = devB();
    }
  }

  return(TRUE);
}


BOOL devCloseAll(void){
  int i;

  for (i=0; i<NUMDEV; i++){
    devClose(&DeviceList[i]);
    devCloseLog(&DeviceList[i]);
    ComPortStatus[i]=CPS_CLOSED; // 100210
  }
  return(TRUE);
}


PDeviceDescriptor_t devGetDeviceOnPort(int Port){

  int i;
  
  for (i=0; i<NUMDEV; i++){
    if (DeviceList[i].Port == Port)
      return(&DeviceList[i]);
  }
  return(NULL);
}


// Called from Port task, after assembly of a string from serial port, ending with a LF
BOOL devParseNMEA(int portNum, TCHAR *String, NMEA_INFO *pGPS){
  PDeviceDescriptor_t d;
  d = devGetDeviceOnPort(portNum);

  // if -log something was commanded by line execution, perform specific logging assembly
  if ((d != NULL) && 
      (d->fhLogFile != NULL) && 
      (String != NULL) && (_tcslen(String) > 0)) {

    char  sTmp[500];  // temp multibyte buffer
    TCHAR *pWC = String;
    char  *pC  = sTmp;
    //    static DWORD lastFlush = 0;    
   
    sprintf(pC, "%9ld <", GetTickCount());
    pC = sTmp + strlen(sTmp);
    
    while (*pWC){
      if (*pWC != '\r'){
        *pC = (char)*pWC;
        pC++; 
      }
      pWC++;
    }
    *pC++ = '>';
    *pC++ = '\r';
    *pC++ = '\n';
    *pC++ = '\0';

    fputs(sTmp, d->fhLogFile);
    
  }

  LogNMEA(String, portNum); // We must manage EnableLogNMEA internally from LogNMEA

  if (portNum>=0 && portNum<=1) {
	ComPortHB[portNum]=LKHearthBeats;
  }

  // intercept device specific parser routines 
  if (d != NULL){

    if (d->pDevPipeTo && d->pDevPipeTo->Com) {
	// stream pipe, pass nmea to other device (NmeaOut)
	// TODO code: check TX buffer usage and skip it if buffer is full (outbaudrate < inbaudrate)
	d->pDevPipeTo->Com->WriteString(String);
    }

    if (d->ParseNMEA != NULL)
	if ((d->ParseNMEA)(d, String, pGPS)) {
		//GPSCONNECT  = TRUE; // NO! 121126
		return(TRUE);
	}
  }

  if(String[0]=='$') {  // Additional "if" to find GPS strings
	if(NMEAParser::ParseNMEAString(portNum, String, pGPS)) {
		//GPSCONNECT  = TRUE; // NO! 121126
		return(TRUE);
	} 
  }
  return(FALSE);

}

BOOL devDirectLink(PDeviceDescriptor_t d,	BOOL bLinkEnable)
{
  BOOL result = TRUE;

  if (SIMMODE)
	return TRUE;

  if (d != NULL && d->DirectLink != NULL)
	result = d->DirectLink(d, bLinkEnable);


  return result;
}

BOOL devPutMacCready(PDeviceDescriptor_t d, double MacCready)
{
  BOOL result = TRUE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d != NULL && d->PutMacCready != NULL)
    result = d->PutMacCready(d, MacCready);
  UnlockComm();

  return result;
}


BOOL devRequestFlarmVersion(PDeviceDescriptor_t d)
{

  if (SIMMODE)
    return TRUE;

  if(GPS_INFO.FLARM_Available)
  {
    if (d != NULL)
    {
      if(!d->Disabled)
      {
  	    devWriteNMEAString(d,_T("PFLAV,R"));
        return TRUE;
      }
    }
  }
  return FALSE;
}


BOOL devPutBugs(PDeviceDescriptor_t d, double Bugs)
{
  BOOL result = TRUE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d != NULL && d->PutBugs != NULL)
    result = d->PutBugs(d, Bugs);
  UnlockComm();

  return result;
}

BOOL devPutBallast(PDeviceDescriptor_t d, double Ballast)
{
  BOOL result = TRUE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d != NULL && d->PutBallast != NULL)
    result = d->PutBallast(d, Ballast);
  UnlockComm();

  return result;
}

// Only called from devInit() above which
// is in turn called with LockComm
BOOL devOpen(PDeviceDescriptor_t d, int Port){
  BOOL res = TRUE;

  if (d != NULL && d->Open != NULL)
    res = d->Open(d, Port);

  if (res == TRUE)
    d->Port = Port;

  return res;
}

// Tear down methods should always succeed.
// Called from devInit() above under LockComm
// Also called when shutting down via devCloseAll()
BOOL devClose(PDeviceDescriptor_t d)
{
  if (d != NULL) {
    if (d->Close != NULL)
      d->Close(d);

    ComPort *Com = d->Com;
    d->Com = NULL;

    if (Com) {
      Com->Close();
      delete Com;
    }
  }

  return TRUE;
}

// Only called from devInit() above which
// is in turn called with LockComm
BOOL devInit(PDeviceDescriptor_t d){
  if (d != NULL && d->Init != NULL)
    return ((d->Init)(d));
  else
    return(TRUE);
}

BOOL devLinkTimeout(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d == NULL){
    for (int i=0; i<NUMDEV; i++){
      d = &DeviceList[i];
      if (d->LinkTimeout != NULL)
        (d->LinkTimeout)(d);
    }
    result = TRUE;
  } else {
    if (d->LinkTimeout != NULL)
      result = d->LinkTimeout(d);
  }
  UnlockComm();

  return result;
}


BOOL devDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  BOOL result = FALSE;

  if (SIMMODE)
    return TRUE;
  
  const unsigned BUFF_LEN = 128;
  TCHAR buffer[BUFF_LEN];

  // We must be sure we are not going to attempt task declaration
  // while a port reset is already in progress. If this happens, a Flarm device will not be Flarm anymore
  // until a Flarm nmea sentence is parsed again once. 
  
  // LKTOKEN  _@M1400_ = "Task declaration"
  // LKTOKEN  _@M571_ = "START"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M571_")));
  CreateProgressDialog(buffer);

  /***********************************************************/
  devDirectLink(d,true);
  /***********************************************************/
  LockComm();

  if ((d != NULL) && (d->Declare != NULL))
	result = d->Declare(d, decl, errBufferLen, errBuffer);
  else {
	if ((d != NULL) && NMEAParser::PortIsFlarm(d->Port)) {
		result |= FlarmDeclare(d, decl, errBufferLen, errBuffer);
	}
  }


  UnlockComm();
  /***********************************************************/
  devDirectLink(d,false);
  /***********************************************************/
  CloseProgressDialog();
  
  return result;
}

BOOL devIsLogger(PDeviceDescriptor_t d)
{
  bool result = false;

  LockComm();
  if ((d != NULL) && (d->IsLogger != NULL)) {
    if (d->IsLogger(d)) {
      result = true;
    }
  }
  if ((d != NULL) && !result) {
    result |= NMEAParser::PortIsFlarm(d->Port);
  }
  UnlockComm();

  return result;
}

BOOL devIsGPSSource(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  LockComm();
  if ((d != NULL) && (d->IsGPSSource != NULL))
    result = d->IsGPSSource(d);
  UnlockComm();

  return result;
}

BOOL devIsBaroSource(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  LockComm();
  if ((d != NULL) && (d->IsBaroSource != NULL))
    result = d->IsBaroSource(d);
  UnlockComm();

  return result;
}

BOOL devIsRadio(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  LockComm();
  if ((d != NULL) && (d->IsRadio != NULL))
    result = d->IsRadio(d);
  UnlockComm();

  return result;
}


BOOL devIsCondor(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  LockComm();
  if ((d != NULL) && (d->IsCondor != NULL))
    result = d->IsCondor(d);
  UnlockComm();

  return result;
}



BOOL devOpenLog(PDeviceDescriptor_t d, TCHAR *FileName){
  if (d != NULL){
    d->fhLogFile = _tfopen(FileName, TEXT("a+b"));
    return(d->fhLogFile != NULL);
  } else
    return(FALSE);
}

BOOL devCloseLog(PDeviceDescriptor_t d){
  if (d != NULL && d->fhLogFile != NULL){
    fclose(d->fhLogFile);
    return(TRUE);
  } else
    return(FALSE);
}

BOOL devPutQNH(DeviceDescriptor_t *d, double NewQNH)
{
  BOOL result = FALSE;

  LockComm();
  if (d == NULL){
    for (int i=0; i<NUMDEV; i++){
      d = &DeviceList[i];
      if (d->PutQNH != NULL)
        d->PutQNH(d, NewQNH);
    }
    result = TRUE;
  } else {
    if (d->PutQNH != NULL)
      result = d->PutQNH(d, NewQNH);
  }
  UnlockComm();

  return result;
}

BOOL devOnSysTicker(DeviceDescriptor_t *d)
{
  BOOL result = FALSE;

  LockComm();
  if (d == NULL){
    for (int i=0; i<NUMDEV; i++){
      d = &DeviceList[i];
      if (d->OnSysTicker != NULL)
        d->OnSysTicker(d);
    }
    result = TRUE;
  } else {
    if (d->OnSysTicker != NULL)
      result = d->OnSysTicker(d);

  }

  UnlockComm();

  return result;
}

static void devFormatNMEAString(TCHAR *dst, size_t sz, const TCHAR *text)
{
  BYTE chk;
  int i, len = _tcslen(text);

  for (chk = i = 0; i < len; i++)
    chk ^= (BYTE)text[i];

  _sntprintf(dst, sz, TEXT("$%s*%02X\r\n"), text, chk);
}

void devWriteNMEAString(PDeviceDescriptor_t d, const TCHAR *text)
{
  if(d != NULL)
  {
    if(!d->Disabled)
    {
	  TCHAR tmp[512];
      devFormatNMEAString(tmp, 512, text);

      devDirectLink(d,true);
      LockComm();
      if (d->Com)
        d->Com->WriteString(tmp);
      UnlockComm();
      devDirectLink(d,false);
    }
  }
}


BOOL devPutVolume(PDeviceDescriptor_t d, int Volume)
{
  BOOL result = TRUE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d != NULL && d->PutVolume != NULL)
    result = d->PutVolume(d, Volume);
  UnlockComm();

  return result;
}

BOOL devPutFreqActive(PDeviceDescriptor_t d, double Freq)
{
  BOOL result = TRUE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d != NULL && d->PutFreqActive != NULL)
    result = d->PutFreqActive(d, Freq);
  UnlockComm();

  return result;
}

BOOL devPutFreqStandby(PDeviceDescriptor_t d, double Freq)
{
  BOOL result = TRUE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d != NULL && d->PutFreqStandby != NULL)
    result = d->PutFreqStandby(d, Freq);
  UnlockComm();

  return result;
}


static BOOL 
FlarmDeclareSetGet(PDeviceDescriptor_t d, TCHAR *Buffer) {
  //devWriteNMEAString(d, Buffer);

  TCHAR tmp[512];

  _sntprintf(tmp, 512, TEXT("$%s\r\n"), Buffer);

  if (d->Com)
    d->Com->WriteString(tmp);

  Buffer[6]= _T('A');
  for(int i=0; i < 20; i++) /* try to get expected answer max 20 times*/
  {
    if (ExpectString(d, Buffer))
	  return true;
    Sleep(20);
  }

  return false;

};


BOOL FlarmDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  BOOL result = TRUE;
#define BUFF_LEN 512
  TCHAR Buffer[BUFF_LEN];
 for(int i=0; i < 3; i++)
 {
  d->Com->StopRxThread();
  d->Com->SetRxTimeout(100);                     // set RX timeout to 50[ms]


  _stprintf(Buffer,TEXT("PFLAC,S,PILOT,%s"),decl->PilotName);
  if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  _stprintf(Buffer,TEXT("PFLAC,S,GLIDERID,%s"),decl->AircraftRego);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  _stprintf(Buffer,TEXT("PFLAC,S,GLIDERTYPE,%s"),decl->AircraftType);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  
  _stprintf(Buffer,TEXT("PFLAC,S,COMPID,%s"),decl->CompetitionID);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  
  _stprintf(Buffer,TEXT("PFLAC,S,COMPCLASS,%s"),decl->CompetitionClass);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  
  _stprintf(Buffer,TEXT("PFLAC,S,NEWTASK,Task"));
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  _stprintf(Buffer,TEXT("PFLAC,S,ADDWP,0000000N,00000000E,TAKEOFF"));
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  if(result == TRUE)
    for (int j = 0; j < decl->num_waypoints; j++) {
      int DegLat, DegLon;
      double MinLat, MinLon;
      char NoS, EoW;

      DegLat = (int)decl->waypoint[j]->Latitude;
      MinLat = decl->waypoint[j]->Latitude - DegLat;
      NoS = 'N';
      if((MinLat<0) || ((MinLat-DegLat==0) && (DegLat<0)))
      {
	    NoS = 'S';
	    DegLat *= -1; MinLat *= -1;
      }
      MinLat *= 60;
      MinLat *= 1000;
    
      DegLon = (int)decl->waypoint[j]->Longitude;
      MinLon = decl->waypoint[j]->Longitude - DegLon;
      EoW = 'E';
      if((MinLon<0) || ((MinLon-DegLon==0) && (DegLon<0)))
      {
	    EoW = 'W';
	    DegLon *= -1; MinLon *= -1;
      }
      MinLon *=60;
      MinLon *= 1000;

      _stprintf(Buffer,
	      TEXT("PFLAC,S,ADDWP,%02d%05.0f%c,%03d%05.0f%c,%s"),
	      DegLat, MinLat, NoS, DegLon, MinLon, EoW, 
	      decl->waypoint[j]->Name);
      if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  }

  _stprintf(Buffer,TEXT("PFLAC,S,ADDWP,0000000N,00000000E,LANDING"));
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  // Reboot flarm to make declaration active, according to specs

  Sleep(100);

  devFormatNMEAString(Buffer, BUFF_LEN, TEXT("PFLAR,0") );
  if(result == TRUE)
    d->Com->WriteString(Buffer);
  Sleep(100);


  d->Com->SetRxTimeout(RXTIMEOUT);                       // clear timeout
  d->Com->StartRxThread();                       // restart RX thread
  if(result == TRUE)
    return result;
  Sleep(100);
}
 return false; // no success
}
