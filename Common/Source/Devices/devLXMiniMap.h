/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */
//__________________________________________________________compilation_control_

#ifndef __DEVLXMINIMAP_H_
#define __DEVLXMINIMAP_H_

//_____________________________________________________________________includes_

#include "devBase.h"
#include "devLXNano.h"

//___________________________________________________________class_declarations_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// General LX device (parsing LXWPn sentences).
///
double CalculateQNH(double alt_qne, double alt_qnh);
double QNHAltitudeToStaticPressureEx(double alt);

class DevLXMiniMap : public DevLXNano
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
    DevLXMiniMap() {}




    /// Installs device specific handlers.
    static void Install(PDeviceDescriptor_t d);

    /// Parses LXWPn sentences.
    static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);

    static BOOL LXMiniMapPutMacCready(PDeviceDescriptor_t d, double MacCready);
    static BOOL LXMiniMapOnSysTicker(DeviceDescriptor_t *d);
    static BOOL LXMiniMapPutQNH(DeviceDescriptor_t *d, double NewQNH);
    static BOOL LXMiniMapPutBugs(PDeviceDescriptor_t	d, double	Bugs);
    static BOOL LXMiniMapPutBallast(PDeviceDescriptor_t	d, double	Ballast);

    static BOOL SendPFLX4(DeviceDescriptor_t *d);

    static double CalculateBalastFactor(double Ballast);
    static double CalculateBalast(double Factor);

    static BOOL Open(PDeviceDescriptor_t d);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static constexpr
    const TCHAR* GetName() {
      return _T("LX MiniMap");
    }

    /// Parses LXWP0 sentence.
    static bool LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP1 sentence.
    static bool LXWP1(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP2 sentence.
    static bool LXWP2(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    /// Parses LXWP3 sentence.
    static bool LXWP3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

    static BOOL DeclareTaskMinimap(PDeviceDescriptor_t d, const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);


}; // DevLX

//______________________________________________________________________________

#endif // __DEVLX_H_
