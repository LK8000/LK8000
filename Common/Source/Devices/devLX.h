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
      return devRegister(GetName(), Install);
    }


  //----------------------------------------------------------------------------
  protected:

    /// Protected only constructor - class should not be instantiated.
    DevLX() {}

    /// Installs device specific handlers.
    static void Install(PDeviceDescriptor_t d);

    /// Parses LXWPn sentences.
    static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static constexpr
    const TCHAR* GetName() {
      return _T("LX");
    }

    /// Parses LXWP0 sentence.
    static bool LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP1 sentence.
    static bool LXWP1(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP2 sentence.
    static bool LXWP2(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP3 sentence.
    static bool LXWP3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    static bool GPRMB(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Converts TCHAR[] string into US-ASCII string.
    static void Wide2LxAscii(const TCHAR* input, int outSize, char* output);

    template<size_t size>
    static void Wide2LxAscii(const TCHAR* input, char (&output)[size]) {
      Wide2LxAscii(input, size, output);
    }

}; // DevLX

//______________________________________________________________________________

#endif // __DEVLX_H_
