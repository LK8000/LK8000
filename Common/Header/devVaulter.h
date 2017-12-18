/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: devVaulter.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
//__________________________________________________________compilation_control_

#ifndef __DEVVAULTER_H_
#define __DEVVAULTER_H_

//_____________________________________________________________________includes_

#include "devBase.h"

//___________________________________________________________class_declarations_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// General LX device (parsing LXWPn sentences).
///
class DevVaulter : public DevBase
{
  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static bool Register();

    // Send GPRMB sentence (next waypoint information).
    static bool PutGPRMB(PDeviceDescriptor_t);

    // Send $PFLX2 to request Info from LX.
    static bool SetupLX_Sentence(PDeviceDescriptor_t d);

    static bool SendInfos(PDeviceDescriptor_t d);
  //----------------------------------------------------------------------------
  protected:

    /// Protected only constructor - class should not be instantiated.
    DevVaulter() {}

    /// Installs device specific handlers.
    static BOOL Install(PDeviceDescriptor_t d);

    static BOOL VaulterDirectLink(PDeviceDescriptor_t d, BOOL LinkStatus);

    /// Parses LXWPn sentences.
    static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static const TCHAR* GetName();

    /// Parses PITV5 sentence.
    static bool PITV5(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP1 sentence.
    static bool LXWP1(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP2 sentence.
    static bool LXWP2(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP3 sentence.
    static bool LXWP3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP4 sentence.
    static bool LXWP4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

}; // DevLX

//______________________________________________________________________________

#endif // __DEVVAULTER_H_
