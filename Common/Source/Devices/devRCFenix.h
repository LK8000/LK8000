/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: DevRCFenix.h,v 1.0 2022/01/10 10:35:29 root Exp root $
 */

//__Version_1.0____________________________________________Vladimir Fux 12/2015_

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


  //----------------------------------------------------------------------------
  protected:


    //..........................................................................

    /// Protected only constructor - class should not be instantiated.
    DevRCFenix() {}

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


    /// Send one line of ceclaration to logger
   static bool SendDecl(PDeviceDescriptor_t d, unsigned row, unsigned n_rows, TCHAR content[], unsigned errBufSize, TCHAR errBuf[]);

   static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);
//   static BOOL FenixParseStream(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO);
   
   static BOOL Config(PDeviceDescriptor_t d);
   static void OnCloseClicked(WndButton* pWnd);

   static void OnValuesClicked(WndButton* pWnd);

   static BOOL RCDT(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL LXBC(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL SENS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info, int ParNo);
   static BOOL SetupFenix_Sentence(PDeviceDescriptor_t d);
   static BOOL PutTarget(PDeviceDescriptor_t d);


   

   static BOOL FenixRadioEnabled(PDeviceDescriptor_t d) { return false;};
   static BOOL FenixPutMacCready(PDeviceDescriptor_t d, double MacCready);
   static BOOL FenixPutBallast(PDeviceDescriptor_t d, double Ballast);
   static BOOL FenixPutBugs(PDeviceDescriptor_t d, double Bugs);

}; // DevRCFenix



//______________________________________________________________________________

#endif // __DevRCFenix_H_
