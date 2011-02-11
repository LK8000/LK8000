/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//__________________________________________________________compilation_control_

#ifndef __DEVLX_H_
#define __DEVLX_H_

//_____________________________________________________________________includes_

//#include <windows.h>
//#include "Sizes.h"
//#include "MapWindow.h"
//#include "device.h"

#include "devBase.h"

//___________________________________________________________class_declarations_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// General LX device (parsing LXWPn sentences).
///
class DevLX : public DevBase
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

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Protected only constructor - class should not be instantiatied.
    ///
    DevLX() {}

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
    /// Parses LXWPn sentences.
    ///
    /// @retval true if the sentence has been parsed
    ///
    static BOOL ParseNMEA
    (
      PDeviceDescriptor_t d, ///< device descriptor
      TCHAR*       sentence, ///< received NMEA sentence
      NMEA_INFO*   info      ///< GPS info to be updated
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Returns device name (max length is @c DEVNAMESIZE).
    ///
    static const TCHAR* GetName();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Parses LXWP0 sentence.
    ///
    /// @retval true if the sentence has been parsed
    ///
    static bool LXWP0
    (
      PDeviceDescriptor_t d, ///< device descriptor
      const TCHAR* sentence, ///< received NMEA sentence
      NMEA_INFO*   info      ///< GPS info to be updated
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Parses LXWP1 sentence.
    ///
    /// @retval true if the sentence has been parsed
    ///
    static bool LXWP1
    (
      PDeviceDescriptor_t d, ///< device descriptor
      const TCHAR* sentence, ///< received NMEA sentence
      NMEA_INFO*   info      ///< GPS info to be updated
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Parses LXWP2 sentence.
    ///
    /// @retval true if the sentence has been parsed
    ///
    static bool LXWP2
    (
      PDeviceDescriptor_t d, ///< device descriptor
      const TCHAR* sentence, ///< received NMEA sentence
      NMEA_INFO*   info      ///< GPS info to be updated
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Parses LXWP3 sentence.
    ///
    /// @retval true if the sentence has been parsed
    ///
    static bool LXWP3
    (
      PDeviceDescriptor_t d, ///< device descriptor
      const TCHAR* sentence, ///< received NMEA sentence
      NMEA_INFO*   info      ///< GPS info to be updated
    );


}; // DevLX

//______________________________________________________________________________

#endif // __DEVLX_H_
