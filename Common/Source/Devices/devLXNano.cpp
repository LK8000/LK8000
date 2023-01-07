/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include <time.h>
#include "externs.h"
#include "utils/stringext.h"
#include "devLXNano.h"


//______________________________________________________________________defines_

/// polynom for LX data CRC
#define LX_CRC_POLY 0x69

/// helper for ToStream() functions
#define LX_ADD_TO_STREAM(var) { memcpy(dst, &var, sizeof(var)); dst += sizeof(var); }

//____________________________________________________________class_definitions_

// #############################################################################
// *****************************************************************************
//
//   DevLXNano
//
// *****************************************************************************
// #############################################################################


/// synchronization (switch from NMEA to command mode and vice versa)
static const char PKT_SYN = '\x16';

/// start command
static const char PKT_STX = '\x02';

/// acknowledged  (CRC OK)
static const char PKT_ACK = '\x06';

/// not acknowledged (CRC checksum is wrong)
//static const char PKT_NAK = '\x15';

// commands supported by Colibri/Nano:

/// read logger info
/// (returns "\r\nVersion NANO<version>\r\nSN<sw_serial>,HW<hw_serial>\r\n")
//static const char PKT_LOGERINFO = '\x'; // unknown

/// read flight info and task declaration
//static const char PKT_PCREAD    = '\xC9';

/// write flight info and task declaration
static const char PKT_PCWRITE   = '\xCA';

/// write Lx class (char LxClass[9] + CRC)
static const char PKT_CCWRITE   = '\xD0';

/// read Lx class (char LxClass[9] + CRC)
//static const char PKT_CCREAD    = '\xCF';

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
void DevLXNano::Install(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->Declare      = DeclareTask;

  StartupStore(_T(". %s installed (platform=%s test=%u)%s"),
    GetName(),
    PlatfEndian::IsBE() ? _T("be") : _T("le"),
    PlatfEndian::To32BE(0x01000000), NEWLINE);
} // Install()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes declaration into the logger.
///
/// @param d           device descriptor to be installed
/// @param lkDecl      LK task declaration data
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  declaration has been written successfully
/// @retval false error during declaration (description in @p errBuf)
///
//static
BOOL DevLXNano::DeclareTask(PDeviceDescriptor_t d,
  const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[])
{
  Decl  decl;
  Class lxClass;

  if (!FillFlight(*lkDecl, decl, errBufSize, errBuf))
    return(false);

  if (!FillTask(*lkDecl, decl, errBufSize, errBuf))
    return(false);

  // we will use text-defined class
  decl.flight.cmp_cls = Decl::cls_textdef;
  lxClass.SetName(lkDecl->CompetitionClass);

  // stop RX thread
  if (!StopRxThread(d, errBufSize, errBuf))
    return(false);

  // set new Rx timeout
  int  orgRxTimeout;
  bool status = SetRxTimeout(d, 2000, orgRxTimeout, errBufSize, errBuf);

  if (status)
  {
    ShowProgress(decl_enable);
    status = StartCMDMode(d, errBufSize, errBuf);

    if (status)
    {
      ShowProgress(decl_send);
      status = status && WriteDecl(d, decl, errBufSize, errBuf);
      status = status && WriteClass(d, lxClass, errBufSize, errBuf);
    }

    ShowProgress(decl_disable);
    // always do the following step otherwise NMEA will not be sent
    // (don't overwrite error descr)
    status = StartNMEAMode(d, status ? errBufSize : 0, errBuf) && status;

    // restore Rx timeout (we must try that always; don't overwrite error descr)
    status = SetRxTimeout(d, orgRxTimeout,
      orgRxTimeout, status ? errBufSize : 0, errBuf) && status;
  }

  // restart RX thread (we must try that always; don't overwrite error descr)
  status = StartRxThread(d, status ? errBufSize : 0, errBuf) && status;

  return(status);
} // DeclareTask()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Starts LX NMEA mode.
///
/// @param d           device descriptor
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  mode successfully set
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::StartNMEAMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  if(ComWrite(d, PKT_SYN, errBufSize, errBuf)) {
    ComExpect(d, PKT_ACK, 10, NULL, errBufSize, errBuf);
  }

  if(ComWrite(d, PKT_SYN, errBufSize, errBuf)) {
    ComExpect(d, PKT_ACK, 10, NULL, errBufSize, errBuf);
  }

  if(ComWrite(d, PKT_SYN, errBufSize, errBuf)) {
    ComExpect(d, PKT_ACK, 10, NULL, errBufSize, errBuf);
  }

  // XCSoar:
  //if (!ComExpect(d, "$$$", 32, NULL, errBufSize, errBuf))
  //  return(false);

  return(true);
} // StartNMEAMode()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Starts LX command mode.
///
/// @param d           device descriptor
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  mode successfully set
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::StartCMDMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  // we have to wait longer while enabling declaration phase because we have
  // to parse all NMEA sequences that are incomming before declaration mode
  // is enabled.
  if(ComWrite(d, PKT_SYN, errBufSize, errBuf)) {
    ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf);
  }

  if(ComWrite(d, PKT_SYN, errBufSize, errBuf)) {
    ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf);
  }

  if (ComWrite(d, PKT_SYN, errBufSize, errBuf)) {
    if(ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf)) {
      return true;
    }
  }

  return false;

} // StartCMDMode()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Fills out decl->Flight data.
///
/// @param lkDecl      LK task declaration data
/// @param decl        task declaration data for device
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  declaration successfully filled out
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::FillFlight(const Declaration_t& lkDecl, Decl& decl, unsigned errBufSize, TCHAR errBuf[])
{
  decl.SetString(Decl::fl_pilot, lkDecl.PilotName);
  decl.SetString(Decl::fl_glider, lkDecl.AircraftType);
  decl.SetString(Decl::fl_reg_num, lkDecl.AircraftRego);
  decl.SetString(Decl::fl_cmp_num, lkDecl.CompetitionID);
  return(true);
} // FillFlight()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Fills out decl->Task data.
///
/// @param lkDecl      LK task declaration data
/// @param decl        task declaration data for device
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  declaration successfully filled out
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::FillTask(const Declaration_t& lkDecl, Decl& decl, unsigned errBufSize, TCHAR errBuf[])
{
  // to declared Start, TPs, and Finish we will add Takeoff and Landing,
  // so maximum NB of declared TPs is Decl::max_wp_count - 2
  if (!CheckWPCount(lkDecl,
      Decl::min_wp_count - 2, Decl::max_wp_count - 2, errBufSize, errBuf))
    return(false);

  int wpCount = lkDecl.num_waypoints;

  Decl::Task& task = decl.task;

  if (!GPS_INFO.NAVWarning && GPS_INFO.SatellitesUsed > 0 &&
    GPS_INFO.Day >= 1 && GPS_INFO.Day <= 31 && GPS_INFO.Month >= 1 && GPS_INFO.Month <= 12)
  {
    task.di = GPS_INFO.Day;
    task.mi = GPS_INFO.Month;
    task.yi = GPS_INFO.Year % 100;
  }
  else
  { // use system time
    time_t sysTime = time(NULL);
    tm tm_temp = {};
    struct tm* utc = gmtime_r(&sysTime, &tm_temp);

    task.di = utc->tm_mday;
    task.mi = utc->tm_mon + 1;
    task.yi = utc->tm_year % 100;
  }

  task.input_time = 0;

  task.fd = task.di;
  task.fm = task.mi;
  task.fy = task.yi;

  task.taskid = 1; // unused, so default is 1

  task.num_of_tp = (char) wpCount - 2; // minus Start and Finish

  // add Start, TPs, and Finish
  for (int i = 0; i < wpCount; i++)
    decl.SetWaypoint(lkDecl.waypoint[i], Decl::tp_regular, i + 1);

  // add Home as Takeoff and Landing
  if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint) && DeclTakeoffLanding)
  {
    decl.SetWaypoint(&WayPointList[HomeWaypoint], Decl::tp_takeoff, 0);
    decl.SetWaypoint(&WayPointList[HomeWaypoint], Decl::tp_landing, wpCount + 1);
  }
  else
  {
    decl.SetWaypoint(NULL, Decl::tp_takeoff, 0);
    decl.SetWaypoint(NULL, Decl::tp_landing, wpCount + 1);
  }

  return(true);
} // FillTask()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes declaration into the device.
/// The CRC will be calculated before.
///
/// @param d           device descriptor
/// @param decl        task declaration data for device
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  declaration successfully written
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::WriteDecl(PDeviceDescriptor_t d, Decl& decl, unsigned errBufSize, TCHAR errBuf[])
{
  byte buf[sizeof(Decl)];

  int size = decl.ToStream(buf);

  bool status =      ComWrite(d, PKT_STX, errBufSize, errBuf);
  status = status && ComWrite(d, PKT_PCWRITE, errBufSize, errBuf);
  status = status && ComWrite(d, buf, size, errBufSize, errBuf);
  status = status && ComExpect(d, PKT_ACK, 10, NULL, errBufSize, errBuf);

  return(status);
} // WriteDecl()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes competition class declaration into the device.
/// The CRC will be calculated before.
///
/// @param d           device descriptor
/// @param lxClass     competition class for device
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  declaration successfully written
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::WriteClass(PDeviceDescriptor_t d, Class& lxClass, unsigned errBufSize, TCHAR errBuf[])
{
  byte buf[sizeof(Class)];

  int size = lxClass.ToStream(buf);

  bool status =      ComWrite(d, PKT_STX, errBufSize, errBuf);
  status = status && ComWrite(d, PKT_CCWRITE, errBufSize, errBuf);
  status = status && ComWrite(d, buf, size, errBufSize, errBuf);
  status = status && ComExpect(d, PKT_ACK, 10, NULL, errBufSize, errBuf);

  return(status);
} // WriteClass()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Calculate LX CRC value for the given data.
///
/// @param length  data length
/// @param data    data to be CRC calculated on
///
//static
byte DevLXNano::CalcCrc(int length, const void* data) {
  byte crcVal = 0xFF;
  const byte* pc = static_cast<const byte*>(data);

  // for all data in object, skipping last CRC value
  for (int i = 0; i < length; i++) {
    byte d = pc[i];
    for (int count = 8; --count >= 0; d <<= 1) {
      byte tmp = crcVal ^ d;
      crcVal <<= 1;
      if (tmp & 0x80) {
        crcVal ^= LX_CRC_POLY;
      }
    }
  }
  return(crcVal);
} // CalcCrc()



// #############################################################################
// *****************************************************************************
//
//   DevLXNano::Decl
//
// *****************************************************************************
// #############################################################################


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constructor - sets all data to 0.
///
DevLXNano::Decl::Decl()
{
  memset(this, 0, sizeof(*this));
} // Decl()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the value of the specified ASCII string member.
///
/// @param str_id  string ID (values >=0 denotes Task.name[] index)
/// @param text    string to be set (will be converted into ASCII)
///
void DevLXNano::Decl::SetString(StrId str_id, const TCHAR* text)
{
  if (str_id >= 0 && (int) str_id < (int) max_wp_count)
  {
    Wide2LxAscii(text, task.name[str_id]);
  }
  else switch(str_id)
  {
    case fl_pilot:
      Wide2LxAscii(text, flight.pilot);
      break;

    case fl_glider:
      Wide2LxAscii(text, flight.glider);
      break;

    case fl_reg_num:
      Wide2LxAscii(text, flight.reg_num);
      break;

    case fl_cmp_num:
      Wide2LxAscii(text, flight.cmp_num);
      break;

    case fl_observer:
      Wide2LxAscii(text, flight.observer);
      break;

    case fl_gps:
      Wide2LxAscii(text, flight.gps);
      break;

    default:
      // invalid id
      return;
  }

} // SetString()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the waypoint data to the @c task member.
///
/// @param wp    waypoint data (if @c NULL, LAT/LON will be set to 0)
/// @param type  waypoint type
/// @param idx   waypoint index
///
void DevLXNano::Decl::SetWaypoint(const WAYPOINT* wp, WpType type, int idx)
{
  if (wp != NULL)
  {
    task.tpt[idx] = (byte) type;
    task.lon[idx] = (int32_t) (wp->Longitude * 60000);
    task.lat[idx] = (int32_t) (wp->Latitude * 60000);

    // set TP name
    SetString((StrId) idx, wp->Name);
  }
  else
  {
    task.tpt[idx] = (byte) type;
    task.lon[idx] = 0;
    task.lat[idx] = 0;

    // set TP name
    const TCHAR* name;
    switch (type)
    {
      case tp_takeoff: name = _T("TAKEOFF"); break;
      case tp_landing: name = _T("LANDING"); break;
      case tp_regular: name = _T("WP");      break;
      default:         name = _T("");        break;
    }
    SetString((StrId) idx, name);
  }
} // SetWaypoint()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Convert data to byte-stream for sending to device and calculate CRC.
///
/// @param buf  [out] buffer (large enough for storing all data)
///
/// \return number of bytes converted
///
int DevLXNano::Decl::ToStream(void* buf)
{
  uint32_t tmp;
  uint16_t tmpU16;
  int16_t  tmpI16;
  byte*dst = (byte*) buf;

  LX_ADD_TO_STREAM(flight.flag);
  tmpU16 = PlatfEndian::To16BE(flight.oo_id);
  LX_ADD_TO_STREAM(tmpU16);
  LX_ADD_TO_STREAM(flight.pilot);
  LX_ADD_TO_STREAM(flight.glider);
  LX_ADD_TO_STREAM(flight.reg_num);
  LX_ADD_TO_STREAM(flight.cmp_num);
  LX_ADD_TO_STREAM(flight.cmp_cls);
  LX_ADD_TO_STREAM(flight.observer);
  LX_ADD_TO_STREAM(flight.gpsdatum);
  LX_ADD_TO_STREAM(flight.fix_accuracy);
  LX_ADD_TO_STREAM(flight.gps);

  LX_ADD_TO_STREAM(task.flag);
  tmp = PlatfEndian::To32BE(task.input_time);
  LX_ADD_TO_STREAM(tmp);
  LX_ADD_TO_STREAM(task.di);
  LX_ADD_TO_STREAM(task.mi);
  LX_ADD_TO_STREAM(task.yi);
  LX_ADD_TO_STREAM(task.fd);
  LX_ADD_TO_STREAM(task.fm);
  LX_ADD_TO_STREAM(task.fy);
  tmpI16 = PlatfEndian::To16BE(task.taskid);
  LX_ADD_TO_STREAM(tmpI16);
  LX_ADD_TO_STREAM(task.num_of_tp);

  for (int i = 0; i < max_wp_count; i++)
    LX_ADD_TO_STREAM(task.tpt[i]);

  for (int i = 0; i < max_wp_count; i++)
  {
    tmp = PlatfEndian::To32BE(task.lon[i]);
    LX_ADD_TO_STREAM(tmp);
  }

  for (int i = 0; i < max_wp_count; i++)
  {
    tmp = PlatfEndian::To32BE(task.lat[i]);
    LX_ADD_TO_STREAM(tmp);
  }

  for (int i = 0; i < max_wp_count; i++)
    LX_ADD_TO_STREAM(task.name[i]);

  crc = DevLXNano::CalcCrc(dst - (byte*) buf, buf);
  LX_ADD_TO_STREAM(crc);
  #ifdef TESTBENCH
  StartupStore(_T("... LXNANO DECL CRC=%d SIZE=%d" NEWLINE),crc,(unsigned)(dst-(byte*)buf));
  #endif

  return(dst - (byte*) buf);
} // ToStream()



// #############################################################################
// *****************************************************************************
//
//   DevLXNano::Class
//
// *****************************************************************************
// #############################################################################


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constructor - sets all data to 0.
///
DevLXNano::Class::Class()
{
  memset(this, 0, sizeof(*this));
} // Class()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the value of @c name member.
///
/// @param text  string to be set (will be converted into ASCII)
///
void DevLXNano::Class::SetName(const TCHAR* text)
{
  Wide2LxAscii(text, name);
} // SetName()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Convert data to byte-stream for sending to device and calculate CRC.
///
/// @param buf  [out] buffer (large enough for storing all data)
///
/// \return number of bytes converted
///
int DevLXNano::Class::ToStream(void* buf)
{
  byte* dst = (byte*) buf;

  LX_ADD_TO_STREAM(name);

  crc = DevLXNano::CalcCrc(dst - (byte*) buf, buf);
  LX_ADD_TO_STREAM(crc);
  #ifdef TESTBENCH
  StartupStore(_T("... LXNANO DECL CRC=%d%s"),crc,NEWLINE);
  #endif

  return(dst - (byte*) buf);
} // ToStream()
