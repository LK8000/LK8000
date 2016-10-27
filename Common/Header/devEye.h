/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef	DEVEYE_H
#define	DEVEYE_H

#include "devBase.h"


/**
 * @brief Eye device class
 *
 * Class provides support for Eye device.
 */
class CDevEye : public DevBase
{
  // LK interface
  static const TCHAR *GetName();
  static bool PEYA(PDeviceDescriptor_t d, const TCHAR *sentence, NMEA_INFO *info);
  static bool PEYI(PDeviceDescriptor_t d, const TCHAR *sentence, NMEA_INFO *info);
  static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR *sentence, NMEA_INFO *info);
  static BOOL Install(PDeviceDescriptor_t d);

public:
  static bool Register();
};

#endif /* DEVEYE_H */
