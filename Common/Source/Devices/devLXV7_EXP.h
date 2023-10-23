/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: devLX16xx.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */
//__________________________________________________________compilation_control_

#ifndef __DevLXV7_EXP_H_
#define __DevLXV7_EXP_H_

//_____________________________________________________________________includes_

#include "devBase.h"
#include "Devices/DeviceRegister.h"

//___________________________________________________________class_declarations_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// General LX device (parsing LXWPn sentences).
///
class DevLXV7_EXP : public DevBase
{


  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static constexpr
    DeviceRegister_t Register() {
      return devRegister(GetName(), Install);
    }

    // Send $PFLX2 to request Info from LX.
    static bool SetupLX_Sentence(DeviceDescriptor_t* d);

  //----------------------------------------------------------------------------
  protected:

    /// Protected only constructor - class should not be instantiated.
    DevLXV7_EXP() {}

    /// Installs device specific handlers.
    static void Install(DeviceDescriptor_t* d);

    /// Parses LXWPn sentences.
    static BOOL ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    static BOOL LXV7_EXP_DirectLink(DeviceDescriptor_t* d, BOOL LinkStatus);
    /// Returns device name (max length is @c DEVNAMESIZE).
    static constexpr
    const TCHAR* GetName() {
        return _T("LXV7_EXP");
    }

    /// Parses PLXVF sentence.
    static bool PLXVF(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses PLXVS sentence.
    static bool PLXVS(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses PLXV0 sentence.
    static bool PLXV0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);
    /// Parses LXWP0 sentence.
    static bool LXWP0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP1 sentence.
    static bool LXWP1(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP2 sentence.
    static bool LXWP2(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP3 sentence.
    static bool LXWP3(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

    /// Parses LXWP4 sentence.
    static bool LXWP4(DeviceDescriptor_t* d, const TCHAR* sentence, NMEA_INFO* info);

    // Send GPRMB sentence (next waypoint information).
    static BOOL PutTarget(DeviceDescriptor_t* d, const WAYPOINT& wpt);

}; // DevLX

//______________________________________________________________________________

#endif // __DevLXV7_EXP_H_
