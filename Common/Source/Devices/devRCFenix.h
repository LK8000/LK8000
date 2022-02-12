/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: DevRCFenix.h,v 1.0 2022/01/10 10:35:29 root Exp root $
 */

//__________________________________________________________compilation_control_

#ifndef __DevRCFenix_H_
#define __DevRCFenix_H_

//_____________________________________________________________________includes_

#include "devLX_EOS_ERA.h"



//______________________________________________________________________defines_


//___________________________________________________________class_declarations_

// #############################################################################
// *****************************************************************************
//
//   DevRCFenix
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class DevRCFenix : public DevLX_EOS_ERA
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
  DevRCFenix() = delete;

  /// Installs device specific handlers.
  static void Install(PDeviceDescriptor_t d);

  /// Returns device name (max length is @c DEVNAMESIZE).
  static constexpr
  const TCHAR* GetName() {
    return _T("RC Fenix");  
  }

  static BOOL FormatTP( TCHAR* DeclStrings, int num, int total,const WAYPOINT *wp);
  /// Writes declaration into the logger.
  static BOOL DeclareTask(PDeviceDescriptor_t d,const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);

  static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);

  static BOOL Config(PDeviceDescriptor_t d);

  static BOOL SetupFenix_Sentence(PDeviceDescriptor_t d);
  static BOOL PutTarget(PDeviceDescriptor_t d);

  static BOOL FenixPutMacCready(PDeviceDescriptor_t d, double MacCready);
  static BOOL FenixPutBallast(PDeviceDescriptor_t d, double Ballast);
  static BOOL FenixPutBugs(PDeviceDescriptor_t d, double Bugs);

}; // DevRCFenix

//______________________________________________________________________________

#endif // __DevRCFenix_H_
