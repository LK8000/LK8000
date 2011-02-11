/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//__________________________________________________________compilation_control_

#ifndef __DEVLXNANO_H_
#define __DEVLXNANO_H_

//_____________________________________________________________________includes_

#include "devLX.h"


#define LX_SEND_BYTESTREAM 1

//_________________________________________________________forward_declarations_
//___________________________________________________________class_declarations_

// #############################################################################
// *****************************************************************************
//
//   DevLXNano
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX Nano device (parsing LXWPn sentences and declaring tasks).
///
class DevLXNano : public DevLX
{
  //----------------------------------------------------------------------------
  public:

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Registers device into device subsystem.
    ///
    /// @retval true  when device has been registered successfully
    /// @retval false device cannot be registered
    ///
    static bool Register();


  //----------------------------------------------------------------------------
  protected:

    /// task declaration structure for device
    class Decl;

    /// competition class
    class Class;

    //..........................................................................

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Protected only constructor - class should not be instantiatied.
    ///
    DevLXNano() {}

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Installs device specific handlers.
    ///
    /// @retval true  when device has been installed successfully
    /// @retval false device cannot be installed
    ///
    static BOOL Install
    (
      PDeviceDescriptor_t d ///< device descriptor to be installed
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Returns device name (max length is @c DEVNAMESIZE).
    ///
    static const TCHAR* GetName();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Writes declaration into the logger.
    ///
    /// @retval true  declaration has been written successfully
    /// @retval false error during declaration (description in @p errBuf)
    ///
    static BOOL DeclareTask
    (
      PDeviceDescriptor_t   d, ///< device descriptor to be installed
      Declaration_t*   lkDecl, ///< task declaration data
      unsigned errBufSize,     ///< error message buffer size
      TCHAR    errBuf[]        ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Starts LX NMEA mode.
    ///
    /// @retval true  mode successfully set
    /// @retval false error (description in @p errBuf)
    ///
    static bool StartNMEAMode
    (
      PDeviceDescriptor_t d, ///< device descriptor
      unsigned errBufSize,   ///< error message buffer size
      TCHAR    errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Starts LX command mode.
    ///
    /// @retval true  mode successfully set
    /// @retval false error (description in @p errBuf)
    ///
    static bool StartCMDMode
    (
      PDeviceDescriptor_t d, ///< device descriptor
      unsigned errBufSize,   ///< error message buffer size
      TCHAR    errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Fills out decl->Flight data.
    ///
    /// @retval true  declaration successfully filled out
    /// @retval false error (description in @p errBuf)
    ///
    static bool FillFlight
    (
      const Declaration_t& lkDecl, ///< LK task declaration data
      Decl&    decl,               ///< task declaration data for device
      unsigned errBufSize,         ///< error message buffer size
      TCHAR    errBuf[]            ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Fills out decl->Task data.
    ///
    /// @retval true  declaration successfully filled out
    /// @retval false error (description in @p errBuf)
    ///
    static bool FillTask
    (
      const Declaration_t& lkDecl, ///< LK task declaration data
      Decl&    decl,               ///< task declaration data for device
      unsigned errBufSize,         ///< error message buffer size
      TCHAR    errBuf[]            ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Writes task declaration into the device.
    /// The CRC will be calculated before.
    ///
    /// @retval true  declaration successfully written
    /// @retval false error (description in @p errBuf)
    ///
    static bool WriteDecl
    (
      PDeviceDescriptor_t d,   ///< device descriptor
      Decl&       decl,        ///< task declaration data for device
      unsigned    errBufSize,  ///< error message buffer size
      TCHAR       errBuf[]     ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Writes competition class declaration into the device.
    /// The CRC will be calculated before.
    ///
    /// @retval true  declaration successfully written
    /// @retval false error (description in @p errBuf)
    ///
    static bool WriteClass
    (
      PDeviceDescriptor_t d,    ///< device descriptor
      Class&       lxClass,     ///< competition class for device
      unsigned     errBufSize,  ///< error message buffer size
      TCHAR        errBuf[]     ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Calculate LX CRC value for the given data.
    ///
    static byte CalcCrc
    (
      int   length, ///< data length
      void* data    ///< data to be CRC calculated on
    );

}; // DevLXNano


// #############################################################################
// *****************************************************************************
//
//   DevLXNano::Decl
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX task declaration data.
/// This data are byte-by-byte sent to device.
///
class DevLXNano::Decl
{
  //----------------------------------------------------------------------------
  public:

    /// class constants
    enum Consts
    {
      min_tp_count =  2,  ///< minimum turn point count
      max_tp_count = 12,  ///< maximum turn point count
    }; // Consts

    /// String member ID (all explicitly defined ids here must have value <0)
    enum StrId
    {
      fl_pilot    = -1,
      fl_glider   = -2,
      fl_reg_num  = -3,
      fl_cmp_num  = -4,
      fl_observer = -5,
      fl_gps      = -6,
    }; // StrId

    /// Competitions Class
    enum Class
    {
      cls_standard    = 0,
      cls_15_meter    = 1,
      cls_open        = 2,
      cls_18_meter    = 3,
      cls_world       = 4,
      cls_double      = 5,
      cls_motor_gl    = 6,
      cls_textdef     = 7, ///< class is be written by PKT_CCWRITE)
    }; // Class

    /// turnpoint type
    enum TpType
    {
      tp_undef   = 0, ///< turn point will be ignored
      tp_regular = 1,
      tp_landing = 2,
      tp_takeoff = 3,
    }; // TpType

    //..........................................................................

    /// LX flight declaration data (should be compatible with Nano, Colibri and Posigraph
    struct Flight // s_flight
    {
      byte flag;          ///< can be empty for Nano
      uint16_t oo_id;     ///< oficial observer id
      char pilot[19];
      char glider[12];
      char reg_num[8];    ///< aircraft registration
      char cmp_num[4];
      byte cmp_cls;       ///< glider class (see @c Class)
      char observer[10];
      byte gpsdatum;
      byte fix_accuracy;
      char gps[60];
    } __attribute__ ((packed)); // Flight

    /// LX task declaration data
    struct Task // s_task
    {
      // auto defined
      byte flag;           ///< can be empty for Nano
      int32_t input_time;  ///< time of declaration (not important, because timestamp before takeoff is used)
      byte di;
      byte mi;
      byte yi;

      // user defined
      byte fd;
      byte fm;
      byte fy;

      int16_t taskid;
      char num_of_tp;
      byte tpt[max_tp_count];     ///< turnpoint type (see @c TpType)
      int32_t lon[max_tp_count];
      int32_t lat[max_tp_count];
      char name[max_tp_count][9];
    } __attribute__ ((packed));

    //..........................................................................

    Flight   flight;
    Task     task;
    //TODO unsigned char reserve;  __attribute__
    byte     crc;

    //..........................................................................

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Constructor - sets all data to 0.
    ///
    Decl();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Sets the value of the specified ASCII string member.
    ///
    void SetString
    (
      StrId str_id,     ///< string ID (values >=0 denotes Task.name[] index)
      const TCHAR* text ///< string to be set (will be converted into ASCII)
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Sets the waypoint data to the @c task member.
    ///
    void SetWaypoint
    (
      const WAYPOINT &wp,  ///< waypoint data
      TpType  type,        ///< waypoint type
      int     idx          ///< waypoint index
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Convert multi-byte values into big-endian format.
    ///
    void ConvertToBE();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Initializes @c crc member with computed CRC value.
    ///
    void CalcCrc();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Convert data to byte-stream for sending to device.
    ///
    /// \return number of bytes converted
    ///
    int ToStream
    (
      void* buf  ///< [out] buffer (large enough for storing all data)
    );

} __attribute__ ((packed)); // DevLXNano::Decl



// #############################################################################
// *****************************************************************************
//
//   DevLXNano::Class
//
// *****************************************************************************
// #############################################################################

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX task declaration data - competition class.
/// This data are byte-by-byte sent to device.
///
class DevLXNano::Class
{
  //----------------------------------------------------------------------------
  public:

    /// competition class name
    char  name[9];
    byte  crc;

    //..........................................................................

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Constructor - sets all data to 0.
    ///
    Class();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Sets the value of @c name member.
    ///
    void SetName
    (
      const TCHAR* text ///< string to be set (will be converted into ASCII)
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Initializes @c crc member with computed CRC value.
    ///
    void CalcCrc();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Convert data to byte-stream for sending to device.
    ///
    /// \return number of bytes converted
    ///
    int ToStream
    (
      void* buf  ///< [out] buffer (large enough for storing all data)
    );

} __attribute__ ((packed)); // DevLXNano::Decl

//______________________________________________________________________________

#endif // __DEVLXNANO_H_
