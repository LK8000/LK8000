/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include <time.h>

#include "StdAfx.h"
#include "Dialogs.h"
#include "externs.h"
#include "devLXNano.h"

//______________________________________________________________________defines_

/// polynom for LX data CRC
#define LX_CRC_POLY 0x69

//____________________________________________________________class_definitions_

/// synchronization (switch from NMEA to command mode and vice versa)
static const char PKT_SYN = '\x16';

/// start command
static const char PKT_STX = '\x02';

/// acknowledged  (CRC OK)
static const char PKT_ACK = '\x06';

/// not acknowledged (CRC checksum is wrong)
static const char PKT_NAK = '\x15';

// commands supported by Nano:

/// read logger info
/// (returns "\r\nVersion NANO<version>\r\nSN<sw_serial>,HW<hw_serial>\r\n")
//static const char PKT_LOGERINFO = '\x'; // unknown

/// read flight info and task declaration
static const char PKT_PCREAD    = '\xC9';

/// write flight info and task declaration
static const char PKT_PCWRITE   = '\xCA';

/// write Lx class (char LxClass[9] + CRC)
static const char PKT_CCWRITE   = '\xD0';

/// read Lx class (char LxClass[9] + CRC)
static const char PKT_CCREAD    = '\xCF';

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Registers device into device subsystem.
///
/// @retval true  when device has been registered successfully
/// @retval false device cannot be registered
///
//static
bool DevLXNano::Register()
{
  return(devRegister(GetName(),
    cap_gps | cap_baro_alt | cap_speed | cap_vario | cap_logger, Install));
} // Register()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
BOOL DevLXNano::Install
(
  PDeviceDescriptor_t d ///< device descriptor to be installed
)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs      = NULL;
  d->PutBallast   = NULL;
  d->Open         = NULL;
  d->Close        = NULL;
  d->Init         = NULL;
  d->LinkTimeout  = GetTrue;
  d->Declare      = DeclareTask;
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;

  StartupStore(_T(". LX Nano installed (platform=%s test=%lu)%s"),
    PlatfEndian::IsBE() ? _T("be") : _T("le"),
    PlatfEndian::ToBE(0x01000000), NEWLINE);

  //TODO delete
  StartupStore(_T("sizeof Flight=%u + Task=%u + crc=%u = Decl=%u %s"),
    sizeof(Decl::Flight), sizeof(Decl::Task), sizeof(byte), sizeof(Decl), NEWLINE);

  return(true);
} // Install()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Returns device name (max length is @c DEVNAMESIZE).
///
//static
const TCHAR* DevLXNano::GetName()
{
  return(_T("LX Nano"));
} // GetName()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes declaration into the logger.
///
/// @retval true  declaration has been written successfully
/// @retval false error during declaration (description in @p errBuf)
///
//static
BOOL DevLXNano::DeclareTask
(
  PDeviceDescriptor_t d, ///< device descriptor to be installed
  Declaration_t*   lkDecl, ///< LK task declaration data
  unsigned errBufSize,   ///< error message buffer size
  TCHAR    errBuf[]      ///< [out] error message
)
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
/// @retval true  mode successfully set
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::StartNMEAMode
(
  PDeviceDescriptor_t d, ///< device descriptor
  unsigned errBufSize,   ///< error message buffer size
  TCHAR    errBuf[]      ///< [out] error message
)
{
  ComWrite(d, PKT_SYN, errBufSize, errBuf);

  if (!ComExpect(d, "$$$", 32, NULL, errBufSize, errBuf))
    return(false);

  return(true);
} // StartNMEAMode()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Starts LX command mode.
///
/// @retval true  mode successfully set
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::StartCMDMode
(
  PDeviceDescriptor_t d, ///< device descriptor
  unsigned errBufSize,   ///< error message buffer size
  TCHAR    errBuf[]      ///< [out] error message
)
{
  // we have to wait longer while enabling declaration phase because we have
  // to parse all NMEA sequences that are incomming before declaration mode
  // is enabled.
  ComWrite(d, PKT_SYN, errBufSize, errBuf);
  ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf);

  ComWrite(d, PKT_SYN, errBufSize, errBuf);
  ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf);

  ComWrite(d, PKT_SYN, errBufSize, errBuf);
  if (!ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf))
    return(false);

  return(true);
} // StartCMDMode()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Fills out decl->Flight data.
///
/// @retval true  declaration successfully filled out
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::FillFlight
(
  const Declaration_t& lkDecl, ///< LK task declaration data
  Decl&    decl,               ///< task declaration data for device
  unsigned errBufSize,         ///< error message buffer size
  TCHAR    errBuf[]            ///< [out] error message
)
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
/// @retval true  declaration successfully filled out
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::FillTask
(
  const Declaration_t& lkDecl, ///< LK task declaration data
  Decl&    decl,               ///< task declaration data for device
  unsigned errBufSize,         ///< error message buffer size
  TCHAR    errBuf[]            ///< [out] error message
)
{
  if (!CheckWPCount(lkDecl,
      Decl::min_tp_count, Decl::max_tp_count, errBufSize, errBuf))
    return(false);

  int tpCount = lkDecl.num_waypoints;

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
    struct tm* utc = gmtime(&sysTime);

    task.di = utc->tm_mday;
    task.mi = utc->tm_mon + 1;
    task.yi = utc->tm_year % 100;
  }

  //task.input_time = 0;

  task.fd = task.di;
  task.fm = task.mi;
  task.fy = task.yi;

  task.taskid = 1;

  task.num_of_tp = (char) tpCount;

  for (int i = 0; i < tpCount; i++)
  {
    Decl::TpType type;

    if (i == 0)
      type = Decl::tp_takeoff;
    else if (i == tpCount - 1)
      type = Decl::tp_landing;
    else
      type = Decl::tp_regular;

    decl.SetWaypoint(*lkDecl.waypoint[i], type, i);
  }

  return(true);
} // FillTask()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes declaration into the device.
/// The CRC will be calculated before.
///
/// @retval true  declaration successfully written
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::WriteDecl
(
  PDeviceDescriptor_t d,   ///< device descriptor
  Decl&       decl,        ///< task declaration data for device
  unsigned    errBufSize,  ///< error message buffer size
  TCHAR       errBuf[]     ///< [out] error message
)
{
  #if (LX_SEND_BYTESTREAM)
  
  byte buf[sizeof(Decl)];
  
  decl.CalcCrc();
  
  int size = decl.ToStream(buf);
  
  bool status =      ComWrite(d, PKT_STX, errBufSize, errBuf);
  status = status && ComWrite(d, PKT_PCWRITE, errBufSize, errBuf);
  status = status && ComWrite(d, buf, size, errBufSize, errBuf);
  status = status && ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf);
  
  #else
  
  decl.ConvertToBE();
  decl.CalcCrc();
  
  bool status =      ComWrite(d, PKT_STX, errBufSize, errBuf);
  status = status && ComWrite(d, PKT_PCWRITE, errBufSize, errBuf);
  status = status && ComWrite(d, &decl, sizeof(decl), errBufSize, errBuf);
  status = status && ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf);
  
  #endif
  
  return(status);
} // WriteDecl()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes competition class declaration into the device.
/// The CRC will be calculated before.
///
/// @retval true  declaration successfully written
/// @retval false error (description in @p errBuf)
///
//static
bool DevLXNano::WriteClass
(
  PDeviceDescriptor_t d,    ///< device descriptor
  Class&       lxClass,     ///< competition class for device
  unsigned     errBufSize,  ///< error message buffer size
  TCHAR        errBuf[]     ///< [out] error message
)
{
  lxClass.CalcCrc();
  
  bool status =      ComWrite(d, PKT_STX, errBufSize, errBuf);
  status = status && ComWrite(d, PKT_CCWRITE, errBufSize, errBuf);
  status = status && ComWrite(d, &lxClass, sizeof(lxClass), errBufSize, errBuf);
  status = status && ComExpect(d, PKT_ACK, 512, NULL, errBufSize, errBuf);

  return(status);
} // WriteClass()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Calculate LX CRC value for the given data.
///
//static
byte DevLXNano::CalcCrc
(
  int   length, ///< data length
  void* data    ///< data to be CRC calculated on
)
{
  byte crcVal = 0xFF;

  char* pc = (char*) data;

  // for all data in object, skipping last CRC value
  for (int i = 0; i < length; i++)
  {
    char d = *pc++;
    for (int count = 8; --count >= 0; d <<= 1)
    {
      char tmp = crcVal ^ d;
      crcVal <<= 1;
      if (tmp < 0)
        crcVal ^= LX_CRC_POLY;
    }
  }

  return(crcVal);
} // CalcCrc()



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
void DevLXNano::Decl::SetString
(
  StrId str_id,     ///< string ID (values >=0 denotes Task.name[] index)
  const TCHAR* text ///< string to be set (will be converted into ASCII)
)
{
  char *output;
  int outSize;

  if (str_id >= 0 && str_id < (int) max_tp_count)
  {
    output = task.name[str_id]; outSize = sizeof(task.name[0]);
  }
  else switch(str_id)
  {
    case fl_pilot:
      output = flight.pilot; outSize = sizeof(flight.pilot); break;

    case fl_glider:
      output = flight.glider; outSize = sizeof(flight.glider); break;

    case fl_reg_num:
      output = flight.reg_num; outSize = sizeof(flight.reg_num); break;

    case fl_cmp_num:
      output = flight.cmp_num; outSize = sizeof(flight.cmp_num); break;

    case fl_observer:
      output = flight.observer; outSize = sizeof(flight.observer); break;

    case fl_gps:
      output = flight.gps; outSize = sizeof(flight.gps); break;

    default:
      // invalid id
      return;
  }

  Wide2Ascii(text, outSize, output);
} // SetString()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the waypoint data to the @c task member.
///
void DevLXNano::Decl::SetWaypoint
(
  const WAYPOINT &wp,  ///< waypoint data
  TpType  type,        ///< waypoint type
  int     idx          ///< waypoint index
)
{
    task.tpt[idx] = (byte) type;
    task.lon[idx] = (int32_t) (wp.Longitude * 60000);
    task.lat[idx] = (int32_t) (wp.Latitude * 60000);

    // set TP name
    SetString((StrId) idx, wp.Name);
} // SetWaypoint()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Convert multi-byte values into big-endian format.
///
void DevLXNano::Decl::ConvertToBE()
{
  flight.oo_id = PlatfEndian::ToBE(flight.oo_id);
  task.input_time = PlatfEndian::ToBE(task.input_time);
  task.taskid = PlatfEndian::ToBE(task.taskid);

  for (int i = 0; i < task.num_of_tp; i++)
  {
    task.lon[i] = PlatfEndian::ToBE(task.lon[i]);
    task.lat[i] = PlatfEndian::ToBE(task.lat[i]);
  }
} // ConvertToBE()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Convert data to byte-stream for sending to device.
///
/// \return number of bytes converted
///
int DevLXNano::Decl::ToStream
(
  void* buf ///< [out] buffer (large enough for storing all data)
)
{
  uint32_t tmp;
  byte* dst = (byte*) buf;

  #define LX_ADD_TO_STREAM(var) { memcpy(dst, &var, sizeof(var)); dst += sizeof(var); }
  LX_ADD_TO_STREAM(flight.flag);
  tmp = PlatfEndian::ToBE(flight.oo_id);
  LX_ADD_TO_STREAM(tmp);
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
  tmp = PlatfEndian::ToBE(task.input_time);
  LX_ADD_TO_STREAM(tmp);
  LX_ADD_TO_STREAM(task.di);
  LX_ADD_TO_STREAM(task.mi);
  LX_ADD_TO_STREAM(task.yi);
  LX_ADD_TO_STREAM(task.fd);
  LX_ADD_TO_STREAM(task.fm);
  LX_ADD_TO_STREAM(task.fy);
  tmp = PlatfEndian::ToBE(task.taskid);
  LX_ADD_TO_STREAM(tmp);
  LX_ADD_TO_STREAM(task.num_of_tp);

  for (int i = 0; i < task.num_of_tp; i++)
    LX_ADD_TO_STREAM(task.tpt[i]);

  for (int i = 0; i < task.num_of_tp; i++)
  {
    tmp = PlatfEndian::ToBE(task.lon[i]);
    LX_ADD_TO_STREAM(tmp);
  }

  for (int i = 0; i < task.num_of_tp; i++)
  {
    tmp = PlatfEndian::ToBE(task.lat[i]);
    LX_ADD_TO_STREAM(tmp);
  }

  for (int i = 0; i < task.num_of_tp; i++)
    LX_ADD_TO_STREAM(task.name[i]);

  LX_ADD_TO_STREAM(crc);

  return(dst - (byte*) buf);
} // ToStream()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Initializes @c crc member with computed CRC value.
///
void DevLXNano::Decl::CalcCrc()
{
  crc = DevLXNano::CalcCrc(sizeof(*this) - 1, this);
} // CalcCrc()



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
void DevLXNano::Class::SetName
(
  const TCHAR* text ///< string to be set (will be converted into ASCII)
)
{
  Wide2Ascii(text, sizeof(name), name);
} // SetName()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Initializes @c crc member with computed CRC value.
///
void DevLXNano::Class::CalcCrc()
{
  crc = DevLXNano::CalcCrc(sizeof(*this) - 1, this);
} // CalcCrc()
