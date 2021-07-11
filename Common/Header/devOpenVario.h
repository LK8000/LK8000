/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: devLX16xx.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */
//__________________________________________________________compilation_control_

#ifndef __DevOpenVario_H_
#define __DevOpenVario_H_

//_____________________________________________________________________includes_

#include "devBase.h"

//___________________________________________________________class_declarations_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
class DevOpenVario : public DevBase {
//----------------------------------------------------------------------------
public:

  /// Registers device into device subsystem.
  static bool Register();

//----------------------------------------------------------------------------
protected:

  /// Protected only constructor - class should not be instantiated.

  DevOpenVario() { }

  /// Installs device specific handlers.
  static BOOL Install(PDeviceDescriptor_t d);


  static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);

  /// Parses POV sentence.
  static BOOL POV(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);


  /// Returns device name (max length is @c DEVNAMESIZE).
  static const TCHAR* GetName();


};

//______________________________________________________________________________

#endif // __DevOpenVario_H_
