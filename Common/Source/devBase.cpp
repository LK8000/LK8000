/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include "StdAfx.h"
#include "Dialogs.h"
#include "devBase.h"

//____________________________________________________________class_definitions_


// #############################################################################
// *****************************************************************************
//
//   DevBase
//
// *****************************************************************************
// #############################################################################


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts parameter from NMEA sentence into double.
///
/// @retval true the conversion has been successful
/// @retval false either string is empty or cannot be converted
///
//static
bool DevBase::ParToDouble
(
  const TCHAR* sentence, ///< received NMEA sentence
  unsigned int parIdx,   ///< index of parameter to be extracted (from 0)
  double*      value     ///< returned value
)
{
  TCHAR  temp[80];
  TCHAR* stop;

  NMEAParser::ExtractParameter(sentence, temp, parIdx);

  stop = NULL;
  *value = StrToDouble(temp, &stop);

  if (stop == NULL || *stop != '\0')
    return(false);

  return(true);
} // ParToDouble()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constant handler returning always @c true.
///
//static
BOOL DevBase::GetTrue
(
  PDeviceDescriptor_t d ///< device descriptor (unused)
)
{
  return(true);
} // GetTrue()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constant handler returning always @c false.
///
//static
BOOL DevBase::GetFalse
(
  PDeviceDescriptor_t d ///< device descriptor (unused)
)
{
  return(false);
} // GetFalse()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Show declaration progress dialog.
///
//static
void DevBase::ShowProgress
(
  DeclDlg dlgType  ///< message type to be shown
)
{
  const TCHAR* msgId;

  switch (dlgType)
  {
    case decl_enable:
      // LKTOKEN  _@M1401_ = "Enabling declaration mode"
      msgId = _T("_@M1401_"); break;

    case decl_disable:
      // LKTOKEN  _@M1402_ = "Disabling declaration mode"
      msgId = _T("_@M1402_"); break;

    case decl_send:
      // LKTOKEN  _@M1403_ = "Sending declaration"
      msgId = _T("_@M1403_"); break;

    default:
      return;
  }

  TCHAR buffer[max_dlg_msg_sz];

  _sntprintf(buffer, max_dlg_msg_sz, _T("%s..."), gettext(msgId));

  CreateProgressDialog(buffer);

} // ShowProgress()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Checks minimum and maximum waypoint count limits.
///
/// @retval true  WP count is in limits
/// @retval false WP count is outside limits (description in @p errBuf)
///
//static
bool DevBase::CheckWPCount
(
  const Declaration_t& decl, ///< task declaration data
  int      minCount,         ///< minimum WP count
  int      maxCount,         ///< maximum WP count
  unsigned errBufSize,       ///< error message buffer size
  TCHAR    errBuf[]          ///< [out] error message
)
{
  // Must have at least two, max 12 waypoints
  if (decl.num_waypoints < minCount)
  {
    // LKTOKEN  _@M1412_ = "Not enough waypoints!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1412_")));
    return(false);
  }

  if (decl.num_waypoints > maxCount)
  {
    // LKTOKEN  _@M1413_ = "Too many waypoints!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1413_")));
    return(false);
  }

  return(true);
} // CheckWPCount()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts TCHAR[] string into ASCII string (writing as much as possible
/// characters into @p output).
/// Output string will always be terminated by '\0'.
///
/// @retval true  all characters copied
/// @retval false some characters could not be copied due to buffer size
///
//static
bool DevBase::Wide2Ascii
(
  const TCHAR* input,  ///< wide character string
  int        outSize,  ///< output buffer size
  char*      output    ///< output buffer
)
{
  char tmp[512];
  int len = _tcslen(input);

  output[0] = '\0';

  if (len != 0)
  {
    len = WideCharToMultiByte(CP_ACP, 0, input, len, tmp, sizeof(tmp), NULL, NULL);

    if (len == 0)
      return(false);
  }

  tmp[len] = '\0';
  strncat(output, tmp, outSize - 1);

  return(len <= outSize);
} // Wide2Ascii()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Stops port Rx thread.
///
/// @retval true  Rx thread stopped
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::StopRxThread
(
  PDeviceDescriptor_t d, ///< device descriptor
  unsigned errBufSize,   ///< error message buffer size
  TCHAR    errBuf[]      ///< [out] error message
)
{
  if (!d->Com->StopRxThread())
  {
    // LKTOKEN  _@M951_ = "Cannot stop RX thread!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M951_")));
    return(false);
  }

  return(true);
} // StopRxThread()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Starts port Rx thread.
///
/// @retval true  Rx thread started
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::StartRxThread
(
  PDeviceDescriptor_t d, ///< device descriptor
  unsigned errBufSize,   ///< error message buffer size
  TCHAR    errBuf[]      ///< [out] error message
)
{
  if (!d->Com->StartRxThread())
  {
    // LKTOKEN  _@M761_ = "Unable to Start RX Thread on Port"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M761_")));
    return(false);
  }

  return(true);
} // StartRxThread()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Starts port Rx thread.
///
/// @retval true  Rx thread started
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::SetRxTimeout
(
  PDeviceDescriptor_t d, ///< device descriptor
  int      newTimeout,   ///< new timeout to be set
  int&     orgTimeout,   ///< [out] original timeout previously set
  unsigned errBufSize,   ///< error message buffer size
  TCHAR    errBuf[]      ///< [out] error message
)
{
  orgTimeout = d->Com->SetRxTimeout(newTimeout);

  if (orgTimeout < 0)
  {
    // LKTOKEN  _@M759_ = "Unable to Change Settings on Port"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M759_")));
    return(false);
  }

  return(true);
} // SetRxTimeout()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes given data to COM port and checks the result.
///
/// @retval true  data written
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComWrite
(
  PDeviceDescriptor_t d,    ///< device descriptor
  const void* data,         ///< data to be written
  int         length,       ///< data length [bytes]
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
  if (!d->Com->Write(data, length))
  {
    // LKTOKEN  _@M952_ = "Cannot write data to Port!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M952_")));
    StartupStore(_T("ComWrite:  ER [%02X] len=%d%s"), ((const unsigned char*) data)[0], length, NEWLINE);
    return(false);
  }

  //TODO: delete
  StartupStore(_T("ComWrite:  OK [%02X] len=%d%s"), ((const  unsigned char*) data)[0], length, NEWLINE);

  return(true);
} // ComWrite()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes given character to COM port and checks the result.
///
/// @retval true  data written
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComWrite
(
  PDeviceDescriptor_t d,    ///< device descriptor
  char        character,    ///< data to be written
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
  return(ComWrite(d, &character, 1, errBufSize, errBuf));
} // ComWrite()


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
//static
bool DevBase::ComExpect
(
  PDeviceDescriptor_t d,    ///< device descriptor
  const void* expected,     ///< expected data
  int         length,       ///< data length [bytes]
  int         checkChars,   ///< maximum characters to read and check
  void*       rxBuf,        ///< [out] received data (up to checkChars)
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
//TODO - delete:
return(true);

  int ch;
  char* prx = (char*) rxBuf;
  const char* pe  = (const char*) expected;

  if (length <= 0)
    return(true);

  while ((ch = d->Com->GetChar()) != EOF)
  {
    if (prx != NULL)
      *prx++ = ch;

    if (ch == *pe)
    {
      if ((++pe - (const char*) expected) == length)
      {
        StartupStore(_T("ComExpect: OK [%02X] check=%d%s"), (unsigned) ch, checkChars, NEWLINE);
        return(true);
      }
    }
    else
      pe = (const char*) expected;

    if (--checkChars <= 0)
      break;
  }

  //TODO: delete
  StartupStore(_T("ComExpect: ER [%02X] check=%d%s"), (unsigned) ch, checkChars, NEWLINE);

  // LKTOKEN  _@M1414_ = "Device not responsive!"
  _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1414_")));

  return(false);
} // ComExpect()


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
//static
bool DevBase::ComExpect
(
  PDeviceDescriptor_t d,    ///< device descriptor
  char        expected,     ///< expected character
  int         checkChars,   ///< maximum characters to read
  void*       rxBuf,        ///< [out] received data (up to checkChars)
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
  return(ComExpect(d, &expected, 1, checkChars, rxBuf, errBufSize, errBuf));
} // ComExpect()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads data from COM port and checks if they contain expected string.
/// Up to @p checkChars characters is read, then @c false is returned if
/// expected string has not been found.
///
/// If @p rxBuf <> @c NULL, all read characters are stored in the buffer.
/// It must be large enough to store up to @p checkChars.
///
/// @retval true  Rx thread started
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComExpect
(
  PDeviceDescriptor_t d,    ///< device descriptor
  const char* expected,     ///< expected string
  int         checkChars,   ///< maximum characters to read
  void*       rxBuf,        ///< [out] received data (up to checkChars)
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
  return(ComExpect(
    d, expected, strlen(expected), checkChars, rxBuf, errBufSize, errBuf));
} // ComExpect()



// #############################################################################
// *****************************************************************************
//
//   PlatfEndian
//
// *****************************************************************************
// #############################################################################

/// endianness flag - @c true for little endian
//static
bool PlatfEndian::little = PlatfEndian::IsLittle();


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Initialization only function.
///
//static
bool PlatfEndian::IsLittle()
{
  unsigned int test = 0;
  ((byte*) &test)[0] = 1;
  return(test == 1);
} // IsLittle()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Swap 32bit value into bin-endian format.
///
/// @return @p value in bin-endian format
///
//static
uint32_t PlatfEndian::ToBE
(
  uint32_t value ///< value to be returned in BE
)
{
  if (IsBE())
    return(value); // there's no need of conversion on BE platform
  else
    return((uint32_t) ((byte*)&value)[3] +
           (uint32_t)(((byte*)&value)[2] << 8) +
           (uint32_t)(((byte*)&value)[1] << 16) +
           (uint32_t)(((byte*)&value)[0] << 24));
} // ToBE()
