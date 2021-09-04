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

#include "devLX.h"
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
class DevLX_EOS_ERA : public DevLX
{
  //----------------------------------------------------------------------------
  public:

    /// Registers device into device subsystem.
    static constexpr 
    DeviceRegister_t Register() {
      return devRegister(GetName(), Install);
    }

    static DeviceDescriptor_t* GetDevice(void) { return m_pDevice; }

    /// Send string as NMEA sentence with prefix '$', suffix '*', and CRC
    static bool SendNmea(PDeviceDescriptor_t, const TCHAR buf[], unsigned errBufSize, TCHAR errBuf[]);
    static bool SendNmea(PDeviceDescriptor_t, const TCHAR buf[]);
    static bool OnStartIGC_FileRead(TCHAR Filename[], uint16_t) ;
    static BOOL AbortLX_IGC_FileRead(void);

  //----------------------------------------------------------------------------
  protected:

    /// task declaration structure for device
    class Decl;

    /// competition class
    class Class;

    //..........................................................................

    /// Protected only constructor - class should not be instantiated.
    DevLX_EOS_ERA() {}

    /// Installs device specific handlers.
    static void Install(PDeviceDescriptor_t d);

    /// Returns device name (max length is @c DEVNAMESIZE).
    static constexpr
    const TCHAR* GetName() {
      return _T("LX EOS/ERA");  
    }

    /// Writes declaration into the logger.
    static BOOL DeclareTask(PDeviceDescriptor_t d, Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]);

    /// Converts TCHAR[] string into US-ASCII string.
    static bool Wide2LxAscii(const TCHAR* input, int outSize, char* output);

    /// Send one line of ceclaration to logger
    static bool SendDecl(PDeviceDescriptor_t d, unsigned row, unsigned n_rows, TCHAR content[], unsigned errBufSize, TCHAR errBuf[]);

   static BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);
   static BOOL EOSParseStream(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO);
   
   static BOOL Config(PDeviceDescriptor_t d);
   static void OnCloseClicked(WndButton* pWnd);
   static void OnCancelClicked(WndButton* pWnd);
   static void OnIGCDownloadClicked(WndButton* pWnd);
   static void OnValuesClicked(WndButton* pWnd);

   static BOOL Open( PDeviceDescriptor_t d);
   static BOOL Close( PDeviceDescriptor_t d);


   static BOOL LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL LXWP1(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL LXWP2(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL LXWP3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL LXWP4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

   static BOOL GPRMB(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL GetTarget(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

   static BOOL LXDT(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL LXBC(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
   static BOOL SENS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info, int ParNo);
   static BOOL SetupLX_Sentence(PDeviceDescriptor_t d);
   static BOOL PutTarget(PDeviceDescriptor_t d);
   static BOOL Values(PDeviceDescriptor_t d);
   static BOOL SetDataText( ValueStringIndex Idx,  const TCHAR ValueText[]);
   static BOOL ShowData(WndForm* wf ,PDeviceDescriptor_t d);
   static void GetDirections(WndButton* pWnd);
   static BOOL ShowValues(void) {return m_bShowValues;};
   static void ShowValues(BOOL bShow) {m_bShowValues = bShow;};
   static BOOL IGCDownload(void) {return bIGC_Download;};
   static void IGCDownload(BOOL bDL) {bIGC_Download = bDL;};
   static BOOL Declare(void) {return m_bDeclare;};
   static void Declare(BOOL bDL) {m_bDeclare = bDL;};
   static int Port(void) { if(m_pDevice) return m_pDevice->PortNumber; else return -1;};

   static PDeviceDescriptor_t Device(void) {return m_pDevice;};
   static void Device(PDeviceDescriptor_t d) {m_pDevice = d;};

   static bool  OnIGCTimeout(WndForm* pWnd);
   static CallBackTableEntry_t CallBackTable[];
   static PDeviceDescriptor_t m_pDevice;
   static BOOL bIGC_Download ;
   static BOOL m_bShowValues;
   static BOOL m_bDeclare;
   static BOOL m_bRadioEnabled;
   static BOOL m_bTriggered;

   static BOOL EOSRadioEnabled(PDeviceDescriptor_t d) { return m_bRadioEnabled;};
   static BOOL EOSPutMacCready(PDeviceDescriptor_t d, double MacCready);
   static BOOL EOSPutBallast(PDeviceDescriptor_t d, double Ballast);
   static BOOL EOSPutBugs(PDeviceDescriptor_t d, double Bugs);
   static BOOL EOSRequestRadioInfo(PDeviceDescriptor_t d);
   static BOOL EOSPutVolume(PDeviceDescriptor_t d, int Volume) ;
   static BOOL EOSPutSquelch(PDeviceDescriptor_t d, int Squelch) ;
   static BOOL EOSPutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) ;
   static BOOL EOSPutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) ;
   static BOOL EOSStationSwap(PDeviceDescriptor_t d) ;
   static BOOL EOSRadioMode(PDeviceDescriptor_t d, int mode) ;

   static BOOL EOSSetMC(PDeviceDescriptor_t d,float fTmp, const TCHAR *info );
   static BOOL EOSSetBAL(PDeviceDescriptor_t d,float fTmp, const TCHAR *info);
   static BOOL EOSSetBUGS(PDeviceDescriptor_t d,float fTmp, const TCHAR *info);
   static BOOL EOSSetSTF(PDeviceDescriptor_t d,int iTmp, const TCHAR *info);
   

   static BOOL  CeckAck(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);

  //----------------------------------------------------------------------------
  //private:


}; // DevLX_EOS_ERA


// #############################################################################
// *****************************************************************************
//
//   DevLX_EOS_ERA::Decl
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX task declaration data.
/// This data are byte-by-byte sent to device.
///



class DevLX_EOS_ERA::Decl
{
  //----------------------------------------------------------------------------
  public:

    /// class constants
    enum Consts
    {
      min_wp_count =  4,  ///< minimum waypoint count
      max_wp_count = 12,  ///< maximum waypoint count
    }; // Consts

    /// String member ID (all explicitly defined ids here must have value <0)
    enum StrId
    {
      fl_pilot    = -1,
      fl_glider   = -2,
      fl_reg_num  = -3,
      fl_cmp_num  = -4,
      fl_observer = -5,
      fl_gps      = -6,
    }; // StrId

    /// Competitions Class
    enum Class
    {
      cls_standard    = 0,
      cls_15_meter    = 1,
      cls_open        = 2,
      cls_18_meter    = 3,
      cls_world       = 4,
      cls_double      = 5,
      cls_motor_gl    = 6,
      cls_textdef     = 7, ///< class will be written with PKT_CCWRITE)
    }; // Class

    /// waypoint type
    enum WpType
    {
      tp_undef   = 0, ///< waypoint will be ignored
      tp_regular = 1, ///< Start, TP, Finish
      tp_landing = 2,
      tp_takeoff = 3,
    }; // WpType

    //..........................................................................

    /// Constructor - sets all data to 0.
    Decl();

    // Format waypoint
    void WpFormat(TCHAR buf[], const WAYPOINT* wp, WpType type, int totalNum);


}; // DevLX_EOS_ERA::Decl



// #############################################################################
// *****************************************************************************
//
//   DevLX_EOS_ERA::Class
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// LX task declaration data - competition class.
/// This data are byte-by-byte sent to device.
///
class DevLX_EOS_ERA::Class
{
  //----------------------------------------------------------------------------
  public:

    /// competition class name
    char  name[9];

    //..........................................................................

    /// Constructor - sets all data to 0.
    Class();

    /// Sets the value of @c name member.
    void SetName(const TCHAR* text);


}; // DevLX_EOS_ERA::Class

//______________________________________________________________________________

#endif // __DevLX_EOS_ERA_H_
