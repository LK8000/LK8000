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
  static void Install(DeviceDescriptor_t* d);

  /// Returns device name (max length is @c DEVNAMESIZE).
  static constexpr
  const TCHAR* GetName() {
    return _T("RC Fenix");  
  }

  /// to send data at connection start
  static BOOL Open(DeviceDescriptor_t* d);

  /// Writes declaration into the logger.
  static BOOL DeclareTask(DeviceDescriptor_t* d,const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);

  static BOOL ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);


  static BOOL Config(DeviceDescriptor_t* d);

  static BOOL PutTarget(DeviceDescriptor_t* d, const WAYPOINT& wpt);

  static BOOL FenixPutMacCready(DeviceDescriptor_t* d, double MacCready);
  static BOOL FenixPutBallast(DeviceDescriptor_t* d, double Ballast);
  static BOOL FenixPutBugs(DeviceDescriptor_t* d, double Bugs);
  static BOOL PutQNH(DeviceDescriptor_t* d, double qnh);

}; // DevRCFenix

//______________________________________________________________________________

#endif // __DevRCFenix_H_
