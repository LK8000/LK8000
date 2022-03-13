/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: DevLX_EOS_ERA.h,v 1.1 2015/12/15 10:35:29 root Exp root $
 */

//__Version_1.0____________________________________________Vladimir Fux 12/2015_
 
//__________________________________________________________compilation_control_

#ifndef __DevLX_EOS_ERA_H_
#define __DevLX_EOS_ERA_H_

//_____________________________________________________________________includes_

#include "devLXNano3.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "Parser.h"
#include "WindowControls.h"


//______________________________________________________________________defines_


//___________________________________________________________class_declarations_

// #############################################################################
// *****************************************************************************
//
//   DevLX_EOS_ERA
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX Nano 3 device (parsing LXWPn sentences and declaring tasks).

#define I_MC     0
#define I_BAL    1
#define I_BUGS   2
#define I_BRIGHT 3
#define I_V_VOL  4
#define I_S_VOL  5
#define I_SIGNAL 6



#define REC_NO_ERROR      0
#define REC_TIMEOUT_ERROR 1
#define REC_CRC_ERROR     2
#define REC_ABORTED       3
#define FILENAME_ERROR    4
#define FILE_OPEN_ERROR   5
#define IGC_RECEIVE_ERROR 6
#define REC_NO_DEVICE     7
#define REC_NOMSG         8
#define REC_INVALID_SIZE  9
#define REC_WRONG_BLOCK  10
#define REC_ZERO_BLOCK   11





void StartEOS_IGCReadThread(void) ;
void StopEOS_IGCReadThread(void) ;
bool SetEOSBinaryModeFlag(bool ) ;

uint8_t EOSRecChar( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout) ;
uint8_t EOSRecChar16(DeviceDescriptor_t *d, uint16_t *inchar, uint16_t Timeout) ;
bool EOSBlockReceived(void);
class DevLX_EOS_ERA : public DevLXNanoIII
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
    DevLX_EOS_ERA() {}

    /// Installs device specific handlers.
    static void Install(PDeviceDescriptor_t d);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static constexpr
    const TCHAR* GetName() {
      return _T("LX EOS/ERA");  
    }
    static bool OnTimer(WndForm* pWnd);
    /// Writes declaration into the logger.
    static BOOL DeclareTask(PDeviceDescriptor_t d, const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);

    /// Send one line of ceclaration to logger
    static bool SendDecl(PDeviceDescriptor_t d, unsigned row, unsigned n_rows, TCHAR content[], unsigned errBufSize, TCHAR errBuf[]);

   static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);
   static BOOL EOSParseStream(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO);
   
   static BOOL Config(PDeviceDescriptor_t d);
   static void OnCloseClicked(WndButton* pWnd);
   static void OnIGCDownloadClicked(WndButton* pWnd);
   static void OnValuesClicked(WndButton* pWnd);

   static BOOL LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
 //  static BOOL LXWP1(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
  static BOOL LXWP2(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
  static BOOL LXWP3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
  // static BOOL LXWP4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);


   static BOOL GetTarget(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

   static BOOL LXDT(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL LXBC(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL SENS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info, int ParNo);
   static BOOL SetupLX_Sentence(PDeviceDescriptor_t d);
   static BOOL PutTarget(PDeviceDescriptor_t d);

   static BOOL ShowData(WndForm* wf ,PDeviceDescriptor_t d);
   static void GetDirections(WndButton* pWnd);
   static BOOL ShowValues(void) {return m_bShowValues;};
   static void ShowValues(BOOL bShow) {m_bShowValues = bShow;};
   static BOOL IGCDownload(void) {return bIGC_Download;};
   static void IGCDownload(BOOL bDL) {bIGC_Download = bDL;};

   static BOOL bIGC_Download ;
   static BOOL m_bShowValues;
   static BOOL m_bRadioEnabled;
   static BOOL m_bTriggered;

   static BOOL EOSRadioEnabled(PDeviceDescriptor_t d) { return m_bRadioEnabled;};
   static BOOL EOSPutMacCready(PDeviceDescriptor_t d, double MacCready);
   static BOOL EOSPutBallast(PDeviceDescriptor_t d, double Ballast);
   static BOOL EOSPutBugs(PDeviceDescriptor_t d, double Bugs);
   static BOOL EOSRequestRadioInfo(PDeviceDescriptor_t d);
   static BOOL EOSPutVolume(PDeviceDescriptor_t d, int Volume) ;
   static BOOL EOSPutSquelch(PDeviceDescriptor_t d, int Squelch) ;
   static BOOL EOSPutFreqActive(PDeviceDescriptor_t d, unsigned khz, const TCHAR* StationName);
   static BOOL EOSPutFreqStandby(PDeviceDescriptor_t d, unsigned khz,  const TCHAR* StationName);
   static BOOL EOSStationSwap(PDeviceDescriptor_t d) ;
   static BOOL EOSRadioMode(PDeviceDescriptor_t d, int mode) ;

   static BOOL EOSSetMC(PDeviceDescriptor_t d,float fTmp, const TCHAR *info );
   static BOOL EOSSetBAL(PDeviceDescriptor_t d,float fTmp, const TCHAR *info);
   static BOOL EOSSetBUGS(PDeviceDescriptor_t d,float fTmp, const TCHAR *info);
   static BOOL EOSSetSTF(PDeviceDescriptor_t d,int iTmp, const TCHAR *info);

}; // DevLX_EOS_ERA

//______________________________________________________________________________

#endif // __DevLX_EOS_ERA_H_
