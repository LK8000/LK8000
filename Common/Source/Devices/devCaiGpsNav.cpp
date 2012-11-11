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
#include "Dialogs.h"
#include "utils/stringext.h"
#include <vector>
#include <set>

namespace {

  typedef std::set<const WAYPOINT*> CTaskWPSet;
  typedef std::vector<int> CTaskWPIdxArray;

  enum TWPFlags {
    WP_FLAGS_TURNPOINT     = 1 << 0,
    WP_FLAGS_AIRFIELD      = 1 << 1,
    WP_FLAGS_MARKPOINT     = 1 << 2,
    WP_FLAGS_LANDING_POINT = 1 << 3,
    WP_FLAGS_START_POINT   = 1 << 4,
    WP_FLAGS_FINISH_POINT  = 1 << 5,
    WP_FLAGS_HOME_POINT    = 1 << 6,
    WP_FLAGS_THERMAL_POINT = 1 << 7,
    WP_FLAGS_WAYPOINT      = 1 << 8,
    WP_FLAGS_AIRSPACE      = 1 << 9,
  };

  struct TCAIRecordX {
    unsigned char result[3];
    unsigned char units;
    unsigned char unknown1[3];
    char pilot[24];
    char glider[9];
    char compId[3];
    unsigned char unknown2;
    unsigned char x1;
    unsigned char radii_approach;
    unsigned char radii_arrival;
    unsigned char fix_interval_arrival;
    unsigned char fix_interval_enroute;
    unsigned char unknown3[57];
  };


  void EmptyRXBuffer(PDeviceDescriptor_t d)
  {
    d->Com->WriteString(TEXT("\x03"));
    ExpectString(d, TEXT("$$$"));  // empty rx buffer (searching for
                                   // pattern that never occure)
  }

  bool CAICommandModeExpect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    if(!ExpectString(d, TEXT("cmd>"))) {
      // LKTOKEN  _@M1414_ = "Device not responsive!" 
      _sntprintf(errBuf, errBufSize, gettext(_T("_@M1414_")));
      return false;
    }
    return true;
  }
  
  bool CAICommandMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    // enter command mode
    d->Com->WriteString(TEXT("\x03"));
    return CAICommandModeExpect(d, errBufSize, errBuf);
  }
  
  bool CAINMEAMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    if(!CAICommandMode(d, errBufSize, errBuf))
      return false;
    
    d->Com->WriteString(TEXT("NMEA\r"));
    
    // This is for a slightly different mode, that
    // apparently outputs pressure info too...
    //(d->Com.WriteString)(TEXT("PNP\r\n"));
    //(d->Com.WriteString)(TEXT("LOG 0\r\n"));
    
    return true;
  }
  
  bool CAIUploadModeExpect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    if(!ExpectString(d, TEXT("up>"))) {
      // LKTOKEN  _@M1414_ = "Device not responsive!" 
      _sntprintf(errBuf, errBufSize, gettext(_T("_@M1414_")));
      return false;
    }
    return true;
  }
  
  bool CAIUploadMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    if(!CAICommandMode(d, errBufSize, errBuf))
      return false;
    
    // enter upload mode
    d->Com->WriteString(TEXT("upl 1\r"));
    return CAIUploadModeExpect(d, errBufSize, errBuf);
  }
  
  bool CAIDownloadModeExpect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    if(!ExpectString(d, TEXT("dn>"))) {
      // LKTOKEN  _@M1414_ = "Device not responsive!" 
      _sntprintf(errBuf, errBufSize, gettext(_T("_@M1414_")));
      return false;
    }
    return true;
  }
  
  bool CAIDownloadMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    if(!CAICommandMode(d, errBufSize, errBuf))
      return false;
    
    // enter upload mode
    d->Com->WriteString(TEXT("dow 1\r"));
    return CAIDownloadModeExpect(d, errBufSize, errBuf);
  }

  bool WaypointsClear(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
  {
    // clear old points
    d->Com->WriteString(TEXT("cle poi\r"));
    
    // wait for command prompt
    if(!CAICommandModeExpect(d, errBufSize, errBuf))
      return false;
    
    return true;
  }

  bool WaypointUpload(PDeviceDescriptor_t d, const WAYPOINT &wp, int idx, unsigned errBufSize, TCHAR errBuf[])
  {
    int DegLat = (int)wp.Latitude;
    double MinLat = wp.Latitude - DegLat;
    char NoS = 'N'; 
    if(MinLat < 0) {
      NoS = 'S';
      DegLat *= -1; 
      MinLat *= -1;
    }
    MinLat *= 60;
    
    int DegLon = (int)wp.Longitude ;
    double MinLon = wp.Longitude - DegLon;
    char EoW = 'E';
    if(MinLon < 0) {
      EoW = 'W';
      DegLon *= -1; 
      MinLon *= -1;
    }
    MinLon *= 60;
    
    TCHAR name[16];
    LK_tcsncpy(name, wp.Name, 12);
    
    int flags = 0;
    if(wp.Flags & AIRPORT)
      flags |= WP_FLAGS_AIRFIELD;
    if(wp.Flags & TURNPOINT)
      flags |= WP_FLAGS_TURNPOINT;
    if(wp.Flags & LANDPOINT)
      flags |= WP_FLAGS_LANDING_POINT;
    if(wp.Flags & HOME)
      flags |= WP_FLAGS_HOME_POINT;
    if(wp.Flags & START)
      flags |= WP_FLAGS_START_POINT;
    if(wp.Flags & FINISH)
      flags |= WP_FLAGS_FINISH_POINT;
    if(flags == 0)
      flags = WP_FLAGS_WAYPOINT;
    
    TCHAR remark[16];
    if(wp.Comment) {
      LK_tcsncpy(remark, wp.Comment, 12);
    }
    else
      remark[0] = '\0';
    
    // prepare and send command
    TCHAR buffer[128];
    _stprintf(buffer, TEXT("C,%d,%02d%07.4f%c,%03d%07.4f%c,%d,%d,%d,%-12s%-12s\r"),
              idx + 1,
              DegLat, MinLat, NoS,
              DegLon, MinLon, EoW,
              (int)wp.Altitude,
              wp.Number,
              flags,
              name,
              remark);
    d->Com->WriteString(buffer);
    
    // wait for command prompt
    if(!CAIDownloadModeExpect(d, errBufSize, errBuf))
      return false;
    
    return true;
  }
  
  bool WaypointsUpload(PDeviceDescriptor_t d, const CTaskWPSet &wps, unsigned errBufSize, TCHAR errBuf[])
  {
    // set task name
    const unsigned TASK_NAME_LENGTH = 64;
    TCHAR tskName[TASK_NAME_LENGTH + 1];
    TaskFileName(TASK_NAME_LENGTH + 1, tskName);
    _tcsncat(tskName, TEXT("_LK8000"), TASK_NAME_LENGTH + 1);
    
    // prepare and send command
    TCHAR buffer[128];
    _stprintf(buffer, TEXT("Y%-*s\r"), TASK_NAME_LENGTH, tskName);
    d->Com->WriteString(buffer);
    
    // wait for command prompt
    if(!CAIDownloadModeExpect(d, errBufSize, errBuf))
      return false;
    
    // set all waypoints
    int i=0;
    for(CTaskWPSet::const_iterator it=wps.begin(), end=wps.end(); it!=end; ++it, i++)
      if(!WaypointUpload(d, **it, i, errBufSize, errBuf))
        return false;
    
    return true;
  }
  
  bool TaskUpload(PDeviceDescriptor_t d, const CTaskWPIdxArray &task, unsigned errBufSize, TCHAR errBuf[])
  {
    // prepare and send command
    TCHAR buffer[128];
    _stprintf(buffer, TEXT("T,0"));
    for(size_t i=0; i<9; i++) {
      if(i < task.size())
        _stprintf(buffer, TEXT("%s,%d"), buffer, task[i] + 1);
      else
        _stprintf(buffer, TEXT("%s,0"));
    }
    _stprintf(buffer, TEXT("%s\r"), buffer);
    d->Com->WriteString(buffer);
    
    // wait for command prompt
    if(!CAIDownloadModeExpect(d, errBufSize, errBuf))
      return false;
    
    return true;
  }


  bool PilotAndGliderUpload(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufSize, TCHAR errBuf[])
  {
    // enter CAI upload mode
    if(!CAIUploadMode(d, errBufSize, errBuf))
      return false;
    
    // request active configuration
    d->Com->WriteString(TEXT("x\r"));
    
    // get active configuration
    TCAIRecordX recordX;
    Sleep(500); // some params come up 0 if we don't wait!
    d->Com->Read(&recordX, sizeof(recordX));
    if(!CAIUploadModeExpect(d, errBufSize, errBuf))
      return false;
    
    // enter CAI download mode
    if(!CAIDownloadMode(d, errBufSize, errBuf))
      return false;
    
    // prepare and send command
    TCHAR pilot[25];
    LK_tcsncpy(pilot, decl.PilotName, 24);
    TCHAR glider[10];
    LK_tcsncpy(glider, decl.AircraftType, 9);
    TCHAR compId[4];
    LK_tcsncpy(compId, decl.CompetitionID, 3);
    TCHAR buffer[128];
    _stprintf(buffer, TEXT("X,%d,%-24s,%-9s%-3s,%d,%d,%d,%d\r"),
              recordX.units,
              pilot,
              glider,
              compId,
              recordX.x1,
              recordX.radii_approach,
              recordX.radii_arrival,
              recordX.fix_interval_arrival << 8 | recordX.fix_interval_enroute);
    d->Com->WriteString(buffer);
    
    // wait for command prompt
    if(!CAIDownloadModeExpect(d, errBufSize, errBuf))
      return false;
    
    return true;
  }
  
}




const TCHAR *CDevCAIGpsNav::GetName()
{
  return(_T("CAI GPS-NAV"));
}


BOOL CDevCAIGpsNav::Init(DeviceDescriptor_t *d)
{
  if(!SIMMODE) {
    d->Com->WriteString(TEXT("\x03"));
    Sleep(500);
    d->Com->WriteString(TEXT("NMEA\r"));
    
    // This is for a slightly different mode, that
    // apparently outputs pressure info too...
    //(d->Com.WriteString)(TEXT("PNP\r\n"));
    //(d->Com.WriteString)(TEXT("LOG 0\r\n"));
  }
  
  return true;
}


BOOL CDevCAIGpsNav::DeclareTask(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[])
{
  // check requirements
  if(!CheckWPCount(*decl, 1, 9, errBufSize, errBuf))  /// @todo: check min number
    return false;

  // create a unique set of task waypoints
  CTaskWPSet wps;
  for(int i=0; i<decl->num_waypoints; i++)
    wps.insert(decl->waypoint[i]);
  
  // create a list of waypoint indexes in a task
  CTaskWPIdxArray task;
  for(int i=0; i<decl->num_waypoints; i++) {
    int j=0;
    for(CTaskWPSet::const_iterator it=wps.begin(), end=wps.end(); it!=end; ++it, j++)
      if(decl->waypoint[i] == *it)
        task.push_back(j);
  }
  
  const unsigned BUFF_LEN = 128;
  TCHAR buffer[BUFF_LEN];
  // LKTOKEN  _@M1400_ = "Task declaration" 
  // LKTOKEN  _@M1404_ = "Opening connection" 
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M1404_")));
  CreateProgressDialog(buffer);

  // prepare communication
  if(!StopRxThread(d, errBufSize, errBuf))
    return false;
  int timeout;
  bool status = SetRxTimeout(d, 500, timeout, errBufSize, errBuf);
  
  if(status) {
    EmptyRXBuffer(d);
    status = CAICommandMode(d, errBufSize, errBuf);
  }
  
  // LKTOKEN  _@M1400_ = "Task declaration" 
  // LKTOKEN  _@M1403_ = "Sending declaration" 
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M1403_")));
  CreateProgressDialog(buffer);
  
  if(status) {
    int temptimeout;
    status = SetRxTimeout(d, 5000, temptimeout, errBufSize, errBuf);
  }
  
  if(status)
    status = WaypointsClear(d, errBufSize, errBuf);

  if(status)
    // enter CAI download mode
    status = CAIDownloadMode(d, errBufSize, errBuf);
  
  if(status)
    // upload waypoints
    status = WaypointsUpload(d, wps, errBufSize, errBuf);
  
  if(status)
    // upload task
    status = TaskUpload(d, task, errBufSize, errBuf);

  if(status)
    // upload pilot and glider data
    status = PilotAndGliderUpload(d, *decl, errBufSize, errBuf);
  
  // LKTOKEN  _@M1400_ = "Task declaration"
  // LKTOKEN  _@M1406_ = "Closing connection"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M1406_")));
  CreateProgressDialog(buffer);
  
  // restore NMEA mode
  status &= CAINMEAMode(d, errBufSize, errBuf);
  
  // restore regular communication
  status &= SetRxTimeout(d, timeout, timeout, errBufSize, errBuf);
  status &= StartRxThread(d, errBufSize, errBuf);
  
  return status;
}


BOOL CDevCAIGpsNav::Install(PDeviceDescriptor_t d)
{
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
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  return true;
}


bool CDevCAIGpsNav::Register()
{
  return devRegister(GetName(), cap_gps | cap_baro_alt | cap_logger, Install);
}
