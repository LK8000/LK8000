/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include "StdAfx.h"
#include "Dialogs.h"
#include "options.h"
#include "devBase.h"

#include "utils/heapcheck.h"

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
/// @param sentence  received NMEA sentence
/// @param parIdx    index of parameter to be extracted (from 0)
/// @param value     returned value
///
/// @retval true  the conversion has been successful
/// @retval false either string is empty or cannot be converted
///
//static
bool DevBase::ParToDouble(const TCHAR* sentence, unsigned int parIdx, double* value)
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
/// @param d  device descriptor (unused)
///
//static
BOOL DevBase::GetTrue(PDeviceDescriptor_t)
{
  return(true);
} // GetTrue()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constant handler returning always @c false.
///
/// @param d  device descriptor (unused)
///
//static
BOOL DevBase::GetFalse(PDeviceDescriptor_t)
{
  return(false);
} // GetFalse()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Show declaration progress dialog.
///
/// @param dlgType  message type to be shown
///
//static
void DevBase::ShowProgress(DeclDlg dlgType)
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
/// @param decl         task declaration data
/// @param minCount     minimum WP count
/// @param maxCount     maximum WP count
/// @param errBufSize   error message buffer size
/// @param errBuf[]     [out] error message
///
/// @retval true  WP count is in limits
/// @retval false WP count is outside limits (description in @p errBuf)
///
//static
bool DevBase::CheckWPCount(const Declaration_t& decl,
  int minCount, int maxCount, unsigned errBufSize, TCHAR errBuf[])
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
/// Stops port Rx thread.
///
/// @param d           device descriptor
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  Rx thread stopped
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::StopRxThread(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
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
/// @param d           device descriptor
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  Rx thread started
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::StartRxThread(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
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
/// @param d           device descriptor
/// @param newTimeout  new timeout to be set
/// @param orgTimeout  [out] original timeout previously set
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  Rx thread started
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::SetRxTimeout(PDeviceDescriptor_t d,
  int newTimeout, int& orgTimeout, unsigned errBufSize, TCHAR errBuf[])
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
/// @param d           device descriptor
/// @param data        data to be written
/// @param length      data length [bytes]
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  data written
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComWrite(PDeviceDescriptor_t d, const void* data, int length, unsigned errBufSize, TCHAR errBuf[])
{
  bool res;

  if (!d->Com->Write(data, length))
  {
    // LKTOKEN  _@M952_ = "Cannot write data to Port!"
    _sntprintf(errBuf, errBufSize, _T("%s"), gettext(_T("_@M952_")));
    res = false;
  }
  else
    res = true;

  #ifdef DEBUG_DEV_COM
    StartupStore(_T("ComWrite:  %s [%02X] len=%d%s"),
      res ? _T("OK") : _T("ER"), ((const unsigned char*) data)[0], length, NEWLINE);
  #endif

  return(res);
} // ComWrite()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes given character to COM port and checks the result.
///
/// @param d           device descriptor
/// @param character   data to be written
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  data written
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComWrite(PDeviceDescriptor_t d,
  char character, unsigned errBufSize, TCHAR errBuf[])
{
  return(ComWrite(d, &character, 1, errBufSize, errBuf));
} // ComWrite()


///
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Flushes COM port output buffers.
///
/// @param d           device descriptor
///
//static
void DevBase::ComFlush(PDeviceDescriptor_t d)
{
  d->Com->Flush();
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads data from COM port and checks if they contain expected data.
/// Up to @p checkChars characters is read, then @c false is returned if
/// expected data stream has not been found.
///
/// If @p rxBuf <> @c NULL, all read characters are stored in the buffer.
/// It must be large enough to store up to @p checkChars.
///
/// @param d           device descriptor
/// @param expected    expected data
/// @param length,     data length [bytes]
/// @param checkChars  maximum characters to read and check
/// @param rxBuf       [out] received data (up to checkChars)
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  expected data received
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComExpect(PDeviceDescriptor_t d, const void* expected,
  int length, int checkChars, void* rxBuf, unsigned errBufSize, TCHAR errBuf[])
{
  int ch;
  char* prx = (char*) rxBuf;
  const char* pe  = (const char*) expected;

  if (length <= 0)
    return(true);

  char prevch = 0;

  while ((ch = d->Com->GetChar()) != EOF)
  {
    prevch = ch;

    if (prx != NULL)
      *prx++ = ch;

    if (ch == *pe)
    {
      if ((++pe - (const char*) expected) == length)
      {
        #ifdef DEBUG_DEV_COM
          StartupStore(_T("ComExpect: OK [%02X] check=%d%s"), (unsigned) ch, checkChars, NEWLINE);
        #endif
        return(true);
      }
    }
    else
      pe = (const char*) expected;

    if (--checkChars <= 0)
      break;
  }

  #ifdef DEBUG_DEV_COM
    StartupStore(_T("ComExpect: ER [%02X] check=%d%s"),
      (prevch == 0) ? (unsigned) ch : (unsigned char) prevch, checkChars, NEWLINE);
  #endif

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
/// @param d           device descriptor
/// @param expected    expected character
/// @param checkChars  maximum characters to read
/// @param rxBuf       [out] received data (up to checkChars)
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  expected data received
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComExpect(PDeviceDescriptor_t d, char expected,
  int checkChars, void* rxBuf, unsigned errBufSize, TCHAR errBuf[])
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
/// @param d            device descriptor
/// @param expected     expected string
/// @param checkChars   maximum characters to read
/// @param rxBuf        [out] received data (up to checkChars)
/// @param errBufSize   error message buffer size
/// @param errBuf[]     [out] error message
///
/// @retval true  Rx thread started
/// @retval false error (description in @p errBuf)
///
//static
bool DevBase::ComExpect(PDeviceDescriptor_t d, const char* expected,
  int checkChars, void* rxBuf, unsigned errBufSize, TCHAR errBuf[])
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
/// Swap 16bit value into bin-endian format.
///
/// @param value  value to be returned in BE
///
/// @return @p value in bin-endian format
///
//static
uint16_t PlatfEndian::To16BE(uint16_t value)
{
  if (IsBE())
    return(value); // there's no need of conversion on BE platform
  else
    return((uint16_t) ((byte*)&value)[1] +
           (uint16_t)(((byte*)&value)[0] << 8));
} // To16BE()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Swap 32bit value into bin-endian format.
///
/// @param value value to be returned in BE
///
/// @return @p value in bin-endian format
///
//static
uint32_t PlatfEndian::To32BE(uint32_t value)
{
  if (IsBE())
    return(value); // there's no need of conversion on BE platform
  else
    return((uint32_t) ((byte*)&value)[3] +
           (uint32_t)(((byte*)&value)[2] << 8) +
           (uint32_t)(((byte*)&value)[1] << 16) +
           (uint32_t)(((byte*)&value)[0] << 24));
} // To32BE()
