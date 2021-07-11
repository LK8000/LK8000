/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: devLXNano.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
//__________________________________________________________compilation_control_

#ifndef __DEVLXNANO_H_
#define __DEVLXNANO_H_

//_____________________________________________________________________includes_

#include "devLX.h"


//______________________________________________________________________defines_

//#define UNIT_TESTS

//___________________________________________________________class_declarations_

// #############################################################################
// *****************************************************************************
//
//   DevLXNano
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX Colibri/Nano device (parsing LXWPn sentences and declaring tasks).
///
class DevLXNano : public DevLX
{
  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static bool Register();


  //----------------------------------------------------------------------------
  protected:

    /// task declaration structure for device
    class Decl;

    /// competition class
    class Class;

    //..........................................................................

    /// Protected only constructor - class should not be instantiated.
    DevLXNano() {}

    /// Installs device specific handlers.
    static BOOL Install(PDeviceDescriptor_t d);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static const TCHAR* GetName();

    /// Writes declaration into the logger.
    static BOOL DeclareTask(PDeviceDescriptor_t d, Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);

    /// Starts LX NMEA mode.
    static bool StartNMEAMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);

    /// Starts LX command mode.
    static bool StartCMDMode(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);

    /// Fills out decl->Flight data.
    static bool FillFlight(const Declaration_t& lkDecl, Decl& decl, unsigned errBufSize, TCHAR errBuf[]);

    /// Fills out decl->Task data.
    static bool FillTask(const Declaration_t& lkDecl, Decl& decl, unsigned errBufSize, TCHAR errBuf[]);

    /// Writes task declaration into the device.
    static bool WriteDecl(PDeviceDescriptor_t d, Decl& decl, unsigned errBufSize, TCHAR errBuf[]);

    /// Writes competition class declaration into the device.
    static bool WriteClass(PDeviceDescriptor_t d, Class& lxClass, unsigned errBufSize, TCHAR errBuf[]);

    /// Converts TCHAR[] string into US-ASCII string.
    static bool Wide2LxAscii(const TCHAR* input, int outSize, char* output);

    /// Calculate LX CRC value for the given data.
    static byte CalcCrc(int length, const void* data);

  //----------------------------------------------------------------------------
  private:

    #ifdef UNIT_TESTS

    /// Log test suite result().
    static void LogTestResult(const TCHAR* suite, const TCHAR* test, bool result);

    /// Test suite for Wide2LxAscii().
    static void Wide2LxAsciiTest();

    #endif


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
      min_wp_count =  4,  ///< minimum waypoint count
      max_wp_count = 12,  ///< maximum waypoint count
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
      cls_textdef     = 7, ///< class will be written with PKT_CCWRITE)
    }; // Class

    /// waypoint type
    enum WpType
    {
      tp_undef   = 0, ///< waypoint will be ignored
      tp_regular = 1, ///< Start, TP, Finish
      tp_landing = 2,
      tp_takeoff = 3,
    }; // WpType

    //..........................................................................

    /// LX flight declaration data (should be compatible with Nano, Colibri and Posigraph)
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
    }; // Flight

    /// LX task declaration data
    struct Task // s_task
    {
      // auto defined
      byte flag;           ///< can be empty for Nano
      int32_t input_time;  ///< time of declaration (not important, because timestamp before takeoff is used)
      byte di;             ///< day of declaration
      byte mi;             ///< month of declaration
      byte yi;             ///< year of declaration

      // user defined
      byte fd;             ///< intended day of flight [local date]
      byte fm;             ///< intended month of flight [local date]
      byte fy;             ///< intended  year of flight [local date]

      int16_t taskid;      ///< task number of the day (if unused, default is 1)
      char num_of_tp;      ///< ! nb of TPs between Start and Finish (not nb of WPs initialized)
      byte tpt[max_wp_count];     ///< waypoint type (see @c WpType)
      int32_t lon[max_wp_count];
      int32_t lat[max_wp_count];
      char name[max_wp_count][9];
    };

    //..........................................................................

    Flight   flight;
    Task     task;
    byte     crc;

    //..........................................................................

    /// Constructor - sets all data to 0.
    Decl();

    /// Sets the value of the specified ASCII string member.
    void SetString(StrId str_id, const TCHAR* text);

    /// Sets the waypoint data to the @c task member.
    void SetWaypoint(const WAYPOINT* wp, WpType type, int idx);

    /// Convert data to byte-stream for sending to device.
    int ToStream(void* buf);

}; // DevLXNano::Decl



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

    /// Constructor - sets all data to 0.
    Class();

    /// Sets the value of @c name member.
    void SetName(const TCHAR* text);

    /// Convert data to byte-stream for sending to device.
    int ToStream(void* buf);

}; // DevLXNano::Class

//______________________________________________________________________________

#endif // __DEVLXNANO_H_
