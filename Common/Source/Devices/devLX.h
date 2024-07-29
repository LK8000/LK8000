/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: devLX.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */
//__________________________________________________________compilation_control_

#ifndef __DEVLX_H_
#define __DEVLX_H_

//_____________________________________________________________________includes_

#include "devBase.h"
#include "Devices/DeviceRegister.h"

//___________________________________________________________class_declarations_

#define MAX_NMEA_PAR_LEN    30

typedef enum {
  _MC   =0,
  _BAL    ,
  _BUGS   ,
  _STF    ,
  _WIND   ,
  _BARO   ,
  _SPEED  ,
  _VARIO  ,
  _R_TRGT ,
  _GFORCE ,
  _OAT    ,
  _BAT1   ,
  _BAT2   ,
  _POLAR  ,
  _DIRECT ,
  _T_TRGT ,
  _QNH    ,
  _LAST
} ValueStringIndex;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// General LX device (parsing LXWPn sentences).
///
class DevLX : public DevBase
{
  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static constexpr
    DeviceRegister_t Register() {
      return devRegister(_T("LX"), Install);
    }


  //----------------------------------------------------------------------------
  protected:

    /// Protected only constructor - class should not be instantiated.
    DevLX() {}

    /// Installs device specific handlers.
    static void Install(DeviceDescriptor_t* d);

    /// Parses LXWPn sentences.
    static BOOL ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP0 sentence.
    static bool LXWP0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP1 sentence.
    static bool LXWP1(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP2 sentence.
    static bool LXWP2(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP3 sentence.
    static bool LXWP3(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    static bool GPRMB(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Converts TCHAR[] string into US-ASCII string.
    static void Wide2LxAscii(const TCHAR* input, int outSize, char* output);

    template<size_t size>
    static void Wide2LxAscii(const TCHAR* input, char (&output)[size]) {
      Wide2LxAscii(input, size, output);
    }
}; // DevLX

//______________________________________________________________________________

#endif // __DEVLX_H_
