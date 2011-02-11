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

//____________________________________________________local_storage_definitions_

static const union 
{
  byte      abTest[4];
  uint32_t  u32Test;
  
  bool IsBig() const { return( u32Test == 0x44332211); }
} platfEndian = {{ 0x44, 0x33, 0x22, 0x11 }};


//____________________________________________________________class_definitions_



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
  { //TODO
    // LKTOKEN  _@M1413_ = "Cannot stop RX thread!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1413_")));
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
  { //TODO
    // LKTOKEN  _@M1413_ = "Cannot start RX thread!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1413_")));
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
  { //TODO
    // LKTOKEN  _@M1413_ = "Cannot set COM port pars!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1413_")));
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
    //TODO: better msg: Cannot send data to COM
    // LKTOKEN  _@M1411_ = "Device not connected!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1411_")));
    return(false);
  }

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
/// Up to @p waitChars characters is read, then @c false is returned if
/// expected data stream has not been found.
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
  int         waitChars,    ///< maximum characters to read
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
//TODO - delete:
//return(true);
  
  char ch;
  const char* pe = (const char*) expected;

  if (length <= 0)
    return(true);

  while ((ch = d->Com->GetChar()) != EOF)
  {
    if (ch == *pe)
    {
      if ((++pe - (const char*) expected) == length)
        return(true);
    }
    else
      pe = (const char*) expected;

    if (--waitChars <= 0)
      break;
  }

  //TODO: better msg: Device not responding
  // LKTOKEN  _@M1411_ = "Device not connected!"
  _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M1411_")));

  return(false);
} // ComExpect()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads data from COM port and checks if they contain expected character.
/// Up to @p waitChars characters is read, then @c false is returned if
/// expected character has not been found.
///
/// @retval true  expected data received
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComExpect
(
  PDeviceDescriptor_t d,    ///< device descriptor
  char        expected,     ///< expected character
  int         waitChars,    ///< maximum characters to read
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
  return(ComExpect(d, &expected, 1, waitChars, errBufSize, errBuf));
} // ComExpect()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads data from COM port and checks if they contain expected string.
/// Up to @p waitChars characters is read, then @c false is returned if
/// expected string has not been found.
///
/// @retval true  Rx thread started
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComExpect
(
  PDeviceDescriptor_t d,    ///< device descriptor
  const char* expected,     ///< expected string
  int         waitChars,    ///< maximum characters to read
  unsigned    errBufSize,   ///< error message buffer size
  TCHAR       errBuf[]      ///< [out] error message
)
{
  return(ComExpect(
    d, expected, strlen(expected), waitChars, errBufSize, errBuf));
} // ComExpect()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Swap 32bit value into bin-endian format.
///
/// @return @p value in bin-endian format
///
//static
uint32_t DevBase::Swap32ToBE
(
  uint32_t value ///< value to be returned in BE
)
{
  if (platfEndian.IsBig())
    return(value); // there's no need of conversion on BE platform
  else
    return((uint32_t) ((byte*)&value)[3] + 
           (uint32_t)(((byte*)&value)[2] << 8) + 
           (uint32_t)(((byte*)&value)[1] << 16) +
           (uint32_t)(((byte*)&value)[0] << 24));
} // Swap32ToBE()
