/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: devLX16xx.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */
//__________________________________________________________compilation_control_

#ifndef __DEVLX16xx_H_
#define __DEVLX16xx_H_

//_____________________________________________________________________includes_

#include "devBase.h"
#include "Devices/DeviceRegister.h"
//___________________________________________________________class_declarations_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// General LX device (parsing LXWPn sentences).
///
class DevLX16xx : public DevBase
{
  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static constexpr
    DeviceRegister_t Register() {
      return devRegister(_T("LX16xx"), Install);
    } // Register()

    // Send $PFLX2 to request Info from LX.
    static bool SetupLX_Sentence(DeviceDescriptor_t* d);

  //----------------------------------------------------------------------------
  protected:

    /// Protected only constructor - class should not be instantiated.
    DevLX16xx() {}

    /// Installs device specific handlers.
    static void Install(DeviceDescriptor_t* d);

    static BOOL LX16xxDirectLink(DeviceDescriptor_t* d, BOOL LinkStatus);

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

    /// Parses LXWP4 sentence.
    static bool LXWP4(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

}; // DevLX

//______________________________________________________________________________

#endif // __DEVLX16xx_H_
