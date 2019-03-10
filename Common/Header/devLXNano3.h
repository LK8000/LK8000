/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: devLXNano3.h,v 1.1 2015/12/15 10:35:29 root Exp root $
*/

//__Version_1.0____________________________________________Vladimir Fux 12/2015_

//__________________________________________________________compilation_control_

#ifndef __DEVLXNANO3_H_
#define __DEVLXNANO3_H_

//_____________________________________________________________________includes_

#include "devLX.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"


//______________________________________________________________________defines_


//___________________________________________________________class_declarations_

// #############################################################################
// *****************************************************************************
//
//   DevLXNanoIII
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX Nano 3 device (parsing LXWPn sentences and declaring tasks).
///



class DevLXNanoIII : public DevLX
{
  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static bool Register();

    static DeviceDescriptor_t* GetDevice(void) { return m_pDevice; }

    /// Send string as NMEA sentence with prefix '$', suffix '*', and CRC
    static bool SendNmea(PDeviceDescriptor_t, const TCHAR buf[], unsigned errBufSize, TCHAR errBuf[]);
    static bool OnStartIGC_FileRead(TCHAR Filename[]) ;
    static BOOL AbortLX_IGC_FileRead(void);

  //----------------------------------------------------------------------------
  protected:

    /// task declaration structure for device
    class Decl;

    /// competition class
    class Class;

    //..........................................................................

    /// Protected only constructor - class should not be instantiated.
    DevLXNanoIII() {}

    /// Installs device specific handlers.
    static BOOL Install(PDeviceDescriptor_t d);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static const TCHAR* GetName();

    /// Writes declaration into the logger.
    static BOOL DeclareTask(PDeviceDescriptor_t d, Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);

    /// Converts TCHAR[] string into US-ASCII string.
    static bool Wide2LxAscii(const TCHAR* input, int outSize, char* output);

    /// Send one line of ceclaration to logger
    static bool SendDecl(PDeviceDescriptor_t d, unsigned row, unsigned n_rows, TCHAR content[], unsigned errBufSize, TCHAR errBuf[]);

   static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);

   static BOOL Config(PDeviceDescriptor_t d);
   static void OnCloseClicked(WndButton* pWnd);

   static void OnIGCDownloadClicked(WndButton* pWnd);
   static BOOL Open( PDeviceDescriptor_t d);
   static BOOL Close( PDeviceDescriptor_t d);
   static BOOL PLXVC(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);


   static CallBackTableEntry_t CallBackTable[];
   static PDeviceDescriptor_t m_pDevice;
  // static TCHAR m_Filename[200];

  //  static bool PutMacCready(PDeviceDescriptor_t d, double MacCready);

  //----------------------------------------------------------------------------
  //private:


}; // DevLXNanoIII


// #############################################################################
// *****************************************************************************
//
//   DevLXNanoIII::Decl
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX task declaration data.
/// This data are byte-by-byte sent to device.
///



class DevLXNanoIII::Decl
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

    /// Constructor - sets all data to 0.
    Decl();

    // Format waypoint
    void WpFormat(TCHAR buf[], const WAYPOINT* wp, WpType type);


}; // DevLXNanoIII::Decl



// #############################################################################
// *****************************************************************************
//
//   DevLXNanoIII::Class
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX task declaration data - competition class.
/// This data are byte-by-byte sent to device.
///
class DevLXNanoIII::Class
{
  //----------------------------------------------------------------------------
  public:

    /// competition class name
    char  name[9];

    //..........................................................................

    /// Constructor - sets all data to 0.
    Class();

    /// Sets the value of @c name member.
    void SetName(const TCHAR* text);


}; // DevLXNanoIII::Class

//______________________________________________________________________________

#endif // __DEVLXNano3_H_
