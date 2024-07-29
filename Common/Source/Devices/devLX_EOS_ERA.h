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

uint8_t EOSRecChar(DeviceDescriptor_t* d, uint8_t *inchar, uint16_t Timeout) ;
uint8_t EOSRecChar16(DeviceDescriptor_t* d, uint16_t *inchar, uint16_t Timeout) ;
bool EOSBlockReceived(void);
class DevLX_EOS_ERA : public DevLXNanoIII
{
  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static constexpr 
    DeviceRegister_t Register() {
      return devRegister(_T("LX EOS/ERA"), Install);
    }

  //----------------------------------------------------------------------------
  protected:

    /// Protected only constructor - class should not be instantiated.
    DevLX_EOS_ERA() {}

    /// Installs device specific handlers.
    static void Install(DeviceDescriptor_t* d);

    static BOOL Open(DeviceDescriptor_t* d);

    static bool OnTimer(WndForm* pWnd);
    /// Writes declaration into the logger.
    static BOOL DeclareTask(DeviceDescriptor_t* d, const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);

   static BOOL ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);
   static BOOL EOSParseStream(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO);
   
   static BOOL Config(DeviceDescriptor_t* d);
   static void OnCloseClicked(WndButton* pWnd);
   static void OnIGCDownloadClicked(WndButton* pWnd);
   static void OnValuesClicked(WndButton* pWnd);

   static BOOL LXWP0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);
   static BOOL LXWP2(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);
   static BOOL LXWP3(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

   static BOOL GetTarget(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

   static BOOL LXDT(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);
   static BOOL LXBC(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);
   static BOOL SENS(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info, int ParNo);

   static BOOL SetupLX_Sentence(DeviceDescriptor_t* d);
   static BOOL PutTarget(DeviceDescriptor_t* d, const WAYPOINT& wpt);

   static BOOL ShowData(WndForm* wf , DeviceDescriptor_t* d);
   static void GetDirections(WndButton* pWnd);
   static BOOL ShowValues(void) {return m_bShowValues;};
   static void ShowValues(BOOL bShow) {m_bShowValues = bShow;};
   static BOOL IGCDownload(void) {return bIGC_Download;};
   static void IGCDownload(BOOL bDL) {bIGC_Download = bDL;};

   static BOOL bIGC_Download ;
   static BOOL m_bShowValues;
   static BOOL m_bTriggered;

   static BOOL EOSPutMacCready(DeviceDescriptor_t* d, double MacCready);
   static BOOL EOSPutBallast(DeviceDescriptor_t* d, double Ballast);
   static BOOL EOSPutBugs(DeviceDescriptor_t* d, double Bugs);
   static BOOL PutQNH(DeviceDescriptor_t* d, double qnh_mb);
   static BOOL EOSRequestRadioInfo(DeviceDescriptor_t* d);
   static BOOL EOSPutVolume(DeviceDescriptor_t* d, int Volume) ;
   static BOOL EOSPutSquelch(DeviceDescriptor_t* d, int Squelch) ;
   static BOOL EOSPutFreqActive(DeviceDescriptor_t* d, unsigned khz, const TCHAR* StationName);
   static BOOL EOSPutFreqStandby(DeviceDescriptor_t* d, unsigned khz,  const TCHAR* StationName);
   static BOOL EOSStationSwap(DeviceDescriptor_t* d) ;
   static BOOL EOSRadioMode(DeviceDescriptor_t* d, int mode) ;

   static BOOL EOSSetMC(DeviceDescriptor_t* d,float fTmp, const TCHAR *info );
   static BOOL EOSSetBAL(DeviceDescriptor_t* d,float fTmp, const TCHAR *info);
   static BOOL EOSSetBUGS(DeviceDescriptor_t* d,float fTmp, const TCHAR *info);
   static BOOL EOSSetSTF(DeviceDescriptor_t* d,int iTmp, const TCHAR *info);

}; // DevLX_EOS_ERA

//______________________________________________________________________________

#endif // __DevLX_EOS_ERA_H_
