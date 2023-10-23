/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: devVaulter.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */
//__________________________________________________________compilation_control_

#ifndef __DEVVAULTER_H_
#define __DEVVAULTER_H_

//_____________________________________________________________________includes_

#include "devBase.h"
#include "Devices/DeviceRegister.h"

//___________________________________________________________class_declarations_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// General LX device (parsing LXWPn sentences).
///
class DevVaulter : public DevBase
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
    DevVaulter() {}

    /// Installs device specific handlers.
    static void Install(DeviceDescriptor_t* d);

    /// Parses LXWPn sentences.
    static BOOL ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static constexpr
    const TCHAR* GetName() {
      return _T("Vaulter");
    }

    /// Parses PITV5 sentence.
    static bool PITV3(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP3 sentence.
    static bool PITV4(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP4 sentence.
    static bool PITV5(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

}; // DevLX

//______________________________________________________________________________

#endif // __DEVVAULTER_H_
