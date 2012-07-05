/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: devBase.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
//__________________________________________________________compilation_control_

#ifndef __DEVBASE_H_
#define __DEVBASE_H_

//_____________________________________________________________________includes_


//___________________________________________________________class_declarations_


// #############################################################################
// *****************************************************************************
//
//   DevBase
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Base class for all devices.
/// Implements common functionality.
/// All devices must have static members only. No object of this class and
/// subclasses will be instantiated.
///
class DevBase
{
  //----------------------------------------------------------------------------
  protected:

    /// device capabilities
    enum Capability
    {
      cap_gps      = (1l << dfGPS),
      cap_baro_alt = (1l << dfBaroAlt),
      cap_speed    = (1l << dfSpeed),
      cap_vario    = (1l << dfVario),
      cap_logger   = (1l << dfLogger),
	  cap_wind     = (1l << dfWind),
	  cap_nmeaout  = (1l << dfNmeaOut),
	  cap_voice    = (1l << dfVoice),
	  cap_radio    = (1l << dfRadio)
    }; // Capability

    /// message type in declaration progress dialog
    enum DeclDlg
    {
      decl_enable,
      decl_disable,
      decl_send,
    }; // DeclDlg

    /// maximum size of progress dialog message
    static const unsigned max_dlg_msg_sz = 200;

    //..........................................................................

    /// Protected only constructor - class should not be instantiated.
    DevBase() {}

    /// Converts parameter from NMEA sentence into double.
    static bool ParToDouble(
      const TCHAR* sentence, unsigned int parIdx, double* value);

    /// Constant handler returning always @c true.
    static BOOL GetTrue(PDeviceDescriptor_t d);

    /// Constant handler returning always @c false.
    static BOOL GetFalse(PDeviceDescriptor_t d);

    /// Show declaration progress dialog.
    static void ShowProgress(DeclDlg dlgType);

    /// Checks minimum and maximum waypoint count limits.
    static bool CheckWPCount(const Declaration_t& decl, int minCount, int maxCount, unsigned errBufSize, TCHAR errBuf[]);

    /// Stops port Rx thread.
    static bool StopRxThread(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);

    /// Starts port Rx thread.
    static bool StartRxThread(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);

    /// Sets Rx read timeout.
    static bool SetRxTimeout(PDeviceDescriptor_t d, int newTimeout, int& orgTimeout, unsigned errBufSize, TCHAR errBuf[]);

    /// Writes given data to COM port and checks the result.
    static bool ComWrite(PDeviceDescriptor_t d, const void* data, int length, unsigned errBufSize, TCHAR errBuf[]);

    /// Writes given character to COM port and checks the result.
    static bool ComWrite(PDeviceDescriptor_t d, char character, unsigned errBufSize, TCHAR errBuf[]);

    /// Flushes COM port output buffers.
    static void ComFlush(PDeviceDescriptor_t d);

    /// Reads data from COM port and checks if they contain expected data.
    static bool ComExpect(PDeviceDescriptor_t d,
      const void* expected, int length, int checkChars, void* rxBuf, unsigned errBufSize, TCHAR errBuf[]);

    /// Reads data from COM port and checks if they contain expected character.
    static bool ComExpect(PDeviceDescriptor_t d,
      char expected, int checkChars, void* rxBuf, unsigned errBufSize, TCHAR errBuf[]);

    /// Reads data from COM port and checks if they contain expected string.
    static bool ComExpect(PDeviceDescriptor_t d,
      const char* expected, int checkChars, void* rxBuf, unsigned errBufSize, TCHAR errBuf[]);

}; // DevBase



// #############################################################################
// *****************************************************************************
//
//   PlatfEndian
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Class for checking platform endianness and converting numbers to specific
/// format.
///
class PlatfEndian
{
  //----------------------------------------------------------------------------
  public:

    /// Returns @c true if the platform is little-endian.
    static bool IsLE() { return(little);  }

    /// Returns @c true if the platform is big-endian.
    static bool IsBE() { return(!little); }

    /// Swap 16bit value into bin-endian format.
    static uint16_t To16BE(uint16_t value);

    /// Swap 16bit value into bin-endian format.
    static int16_t To16BE(int16_t value)
    {
      return((int16_t) To16BE((uint16_t) value));
    }

    /// Swap 32bit value into bin-endian format.
    static uint32_t To32BE(uint32_t value);

    /// Swap 32bit value into bin-endian format.
    static int32_t To32BE(int32_t value)
    {
      return((int32_t) To32BE((uint32_t) value));
    }

  //----------------------------------------------------------------------------
  protected:

    /// endianness flag - @c true for little endian
    static bool little;

    /// Initialization only function.
    static bool IsLittle();

}; // PlatfEndian


//______________________________________________________________________________

#endif // __DEVBASE_H_
