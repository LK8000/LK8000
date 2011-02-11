/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//__________________________________________________________compilation_control_

#ifndef __DEVBASE_H_
#define __DEVBASE_H_

//_____________________________________________________________________includes_

#include "device.h"

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

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Protected only constructor - class should not be instantiatied.
    ///
    DevBase() {}

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Converts parameter from NMEA sentence into double.
    ///
    /// @retval true the conversion has been successful
    /// @retval false either string is empty or cannot be converted
    ///
    static bool ParToDouble
    (
      const TCHAR* sentence, ///< received NMEA sentence
      unsigned int parIdx,   ///< index of parameter to be extracted (from 0)
      double*      value     ///< returned value
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Constant handler returning always @c true.
    ///
    static BOOL GetTrue
    (
      PDeviceDescriptor_t d ///< device descriptor (unused)
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Constant handler returning always @c false.
    ///
    static BOOL GetFalse
    (
      PDeviceDescriptor_t d ///< device descriptor (unused)
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Show declaration progress dialog.
    ///
    static void ShowProgress
    (
      DeclDlg dlgType  ///< message type to be shown
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Checks minimum and maximum waypoint count limits.
    ///
    /// @retval true  WP count is in limits
    /// @retval false WP count is outside limits (description in @p errBuf)
    ///
    static bool CheckWPCount
    (
      const Declaration_t& decl, ///< task declaration data
      int      minCount,         ///< minimum WP count
      int      maxCount,         ///< maximum WP count
      unsigned errBufSize,       ///< error message buffer size
      TCHAR    errBuf[]          ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Converts TCHAR[] string into ASCII string (writing as much as possible
    /// characters into @p output).
    /// Output string will always be terminated by '\0'.
    ///
    /// @retval true  all characters copied
    /// @retval false some characters could not be copied due to buffer size
    ///
    static bool Wide2Ascii
    (
      const TCHAR* input,  ///< wide character string
      int        outSize,  ///< output buffer size
      char*      output    ///< output buffer
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Stops port Rx thread.
    ///
    /// @retval true  Rx thread stopped
    /// @retval false error (description in @p errBuf)
    ///
    static bool StopRxThread
    (
      PDeviceDescriptor_t d, ///< device descriptor
      unsigned errBufSize,   ///< error message buffer size
      TCHAR    errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Starts port Rx thread.
    ///
    /// @retval true  Rx thread started
    /// @retval false error (description in @p errBuf)
    ///
    static bool StartRxThread
    (
      PDeviceDescriptor_t d, ///< device descriptor
      unsigned errBufSize,   ///< error message buffer size
      TCHAR    errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Starts port Rx thread.
    ///
    /// @retval true  Rx thread started
    /// @retval false error (description in @p errBuf)
    ///
    static bool SetRxTimeout
    (
      PDeviceDescriptor_t d, ///< device descriptor
      int      newTimeout,   ///< new timeout to be set
      int&     orgTimeout,   ///< [out] original timeout previously set
      unsigned errBufSize,   ///< error message buffer size
      TCHAR    errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Writes given data to COM port and checks the result.
    ///
    /// @retval true  data written
    /// @retval false error (description in @p errBuf)
    ///
    static bool ComWrite
    (
      PDeviceDescriptor_t d,    ///< device descriptor
      const void* data,         ///< data to be written
      int         length,       ///< data length [bytes]
      unsigned    errBufSize,   ///< error message buffer size
      TCHAR       errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Writes given character to COM port and checks the result.
    ///
    /// @retval true  data written
    /// @retval false error (description in @p errBuf)
    ///
    static bool ComWrite
    (
      PDeviceDescriptor_t d,    ///< device descriptor
      char        character,    ///< data to be written
      unsigned    errBufSize,   ///< error message buffer size
      TCHAR       errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Reads data from COM port and checks if they contain expected data.
    /// Up to @p checkChars characters is read, then @c false is returned if
    /// expected data stream has not been found.
    ///
    /// If @p rxBuf <> @c NULL, all read characters are stored in the buffer.
    /// It must be large enough to store up to @p checkChars.
    ///
    /// @retval true  expected data received
    /// @retval false error (description in @p errBuf)
    ///
    static bool ComExpect
    (
      PDeviceDescriptor_t d,    ///< device descriptor
      const void* expected,     ///< expected data
      int         length,       ///< data length [bytes]
      int         checkChars,   ///< maximum characters to read and check
      void*       rxBuf,        ///< [out] received data (up to checkChars)
      unsigned    errBufSize,   ///< error message buffer size
      TCHAR       errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Reads data from COM port and checks if they contain expected character.
    /// Up to @p checkChars characters is read, then @c false is returned if
    /// expected character has not been found.
    ///
    /// If @p rxBuf <> @c NULL, all read characters are stored in the buffer.
    /// It must be large enough to store up to @p checkChars.
    ///
    /// @retval true  expected data received
    /// @retval false error (description in @p errBuf)
    ///
    static bool ComExpect
    (
      PDeviceDescriptor_t d,    ///< device descriptor
      char        expected,     ///< expected character
      int         checkChars,   ///< maximum characters to read
      void*       rxBuf,        ///< [out] received data (up to checkChars)
      unsigned    errBufSize,   ///< error message buffer size
      TCHAR       errBuf[]      ///< [out] error message
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Reads data from COM port and checks if they contain expected string.
    /// Up to @p checkChars characters is read, then @c false is returned if
    /// expected string has not been found.
    ///
    /// If @p rxBuf <> @c NULL, all read characters are stored in the buffer.
    /// It must be large enough to store up to @p checkChars.
    ///
    /// @retval true  expected data received
    /// @retval false error (description in @p errBuf)
    ///
    static bool ComExpect
    (
      PDeviceDescriptor_t d,    ///< device descriptor
      const char* expected,     ///< expected string
      int         checkChars,   ///< maximum characters to read
      void*       rxBuf,        ///< [out] received data (up to checkChars)
      unsigned    errBufSize,   ///< error message buffer size
      TCHAR       errBuf[]      ///< [out] error message
    );

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

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Returns @c true if the platform is little-endian.
    ///
    static bool IsLE() { return(little);  }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Returns @c true if the platform is big-endian.
    ///
    static bool IsBE() { return(!little); }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Swap 32bit value into bin-endian format.
    ///
    /// @return @p value in bin-endian format
    ///
    static uint32_t ToBE
    (
      uint32_t value ///< value to be returned in BE
    );

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Swap 32bit value into bin-endian format.
    ///
    /// @return @p value in bin-endian format
    ///
    static int32_t ToBE
    (
      int32_t value ///< value to be returned in BE
    )
    {
      return((int32_t) ToBE((uint32_t) value));
    }

  //----------------------------------------------------------------------------
  protected:

    /// endianness flag - @c true for little endian
    static bool little;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Initialization only function.
    ///
    static bool IsLittle();

}; // PlatfEndian


//______________________________________________________________________________

#endif // __DEVBASE_H_
