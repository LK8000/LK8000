/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "devDSX.h"
#include "Dialogs/dlgProgress.h"

extern NMEA_INFO GPS_INFO;

static bool DSXPromptWait(PDeviceDescriptor_t d, unsigned waitChars);
static bool DSXSwitchDeclareMode(PDeviceDescriptor_t d, bool enable, unsigned errBufferLen, TCHAR errBuffer[]);
static bool DSXHSend(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufferLen, TCHAR errBuffer[]);
static bool DSXT1Send(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufferLen, TCHAR errBuffer[]);
static inline unsigned long DSXCoord(float gpsCoord);
static bool DSXWaypoint(const WAYPOINT &wp, const TCHAR aatBuffer[], const TCHAR limiter, TCHAR buffer[]);
static bool DSXT2Send(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufferLen, TCHAR errBuffer[]);
static bool DSXT3Send(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned tpIdx, unsigned errBufferLen, TCHAR errBuffer[]);
static BOOL DSXDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]);
static BOOL DSXIsTrue(PDeviceDescriptor_t d);
static BOOL DSXInstall(PDeviceDescriptor_t d);

static const unsigned PROMPT_WAIT_CHARS = 32;
static const unsigned PROMPT_WAIT_CHARS_LONG = 512;


/**
 * @brief Waits for a prompt from the logger
 *
 * Method waits for a prompt sign to appear on the port
 * in the next @p waitChars characters.
 *
 * @param d Device handle
 * @param waitChars The number of characters to check
 *
 * @return @c true when prompt was detected
 */
bool DSXPromptWait(PDeviceDescriptor_t d, unsigned waitChars)
{
  if (!d->Com)
    return FALSE;

  unsigned i=0;
  int ch;
  while((ch = d->Com->GetChar()) != EOF) {
    if(i == waitChars)
      return false;

    if(ch == _T('#'))
      return true;

    i++;
  }

  return false;
}


/**
 * @brief Switches declaration mode
 *
 * @param d Device handle
 * @param enable Specifies if declare mode should be enabled or disabled
 * @param errBufferLen The length of the buffer for error string
 * @param errBuffer The  buffer for error string
 *
 * @return Operation status
 */
bool DSXSwitchDeclareMode(PDeviceDescriptor_t d, bool enable, unsigned errBufferLen, TCHAR errBuffer[])
{
  d->Com->WriteString(enable ? _T("h1\r") : _T("h0\r"));
  // We have to wait longer while enabling declaration phase because we have to parse
  // all NMEA sequences that are incomming before declaration mode is enabled.
  if(!DSXPromptWait(d, enable ? PROMPT_WAIT_CHARS_LONG : PROMPT_WAIT_CHARS)) {
    // LKTOKEN  _@M1411_ = "Device not connected!"
    _tcsncpy(errBuffer, MsgToken(1411), errBufferLen);
    return false;
  }
  return true;
}


// LKTOKEN  _@M1420_ = "Error while declaring!"
#define DSX_H_SEND( HSTR, STR, LEN, TOKEN)                       \
  {                                                              \
    TCHAR tmpBuffer[(LEN) + 1];                                  \
    _tcsncpy(tmpBuffer, (STR), (LEN));                         \
    tmpBuffer[(LEN)] = _T('\0');                                 \
    if(tmpBuffer[0] == _T('\0'))                                 \
      _stprintf(tmpBuffer, _T("-"));                             \
    _stprintf(buffer, _T("%s%s\r"), _T(HSTR), tmpBuffer);        \
    d->Com->WriteString(buffer);                                 \
    if(!DSXPromptWait(d, PROMPT_WAIT_CHARS)) {                   \
      _sntprintf(errBuffer, errBufferLen, _T("%s '%s'!"),        \
                 MsgToken(1420), LKGetText(TEXT(TOKEN))); \
      return false;                                              \
    }                                                            \
  }


/**
 * @brief Sends DSX H sentences
 *
 * DSX H sentences are responsible for setting pilot, glider and competition data:
 *  - @b H1 - Pilot name (max 62 chars)
 *  - @b H2 - Competition ID (max 8 chars)
 *  - @b H3 - Competition Class (max 16 chars)
 *  - @b H4 - Glider ID (max 16 chars)
 *  - @b H5 - Glider Type (max 16 chars)
 *
 * @param d Device handle
 * @param decl Task declaration data
 * @param errBufferLen The length of the buffer for error string
 * @param errBuffer The  buffer for error string
 *
 * @return Declaration status
 */
bool DSXHSend(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  const unsigned HEADER_LEN      = 2;
  const unsigned PILOT_LEN       = 62;
  const unsigned COMP_ID_LEN     = 8;
  const unsigned COMP_CLASS_LEN  = 16;
  const unsigned GLIDER_ID_LEN   = 16;
  const unsigned GLIDER_TYPE_LEN = 16;
  const unsigned BUFFER_MAX_LEN  = PILOT_LEN + HEADER_LEN + 1;
  TCHAR buffer[BUFFER_MAX_LEN];

  // LKTOKEN  _@M524_ = "Pilot name"
  DSX_H_SEND("H1", decl.PilotName,        PILOT_LEN,       "_@M524_");
  // LKTOKEN  _@M938_ = "Competition ID"
  DSX_H_SEND("H2", decl.CompetitionID,    COMP_ID_LEN,     "_@M938_");
  // LKTOKEN  _@M936_ = "Competition Class"
  DSX_H_SEND("H3", decl.CompetitionClass, COMP_CLASS_LEN,  "_@M936_");
  // LKTOKEN  _@M57_ = "Aircraft Reg"
  DSX_H_SEND("H4", decl.AircraftRego,     GLIDER_ID_LEN,   "_@M57_");
  // LKTOKEN  _@M59_ = "Aircraft type"
  DSX_H_SEND("H5", decl.AircraftType,     GLIDER_TYPE_LEN, "_@M59_");

  return true;
}


/**
 * @brief Sends DSX T1 sentence
 *
 * DSX T1 is the first of the declaration sequence: must be the first, and must
 * be sent only one time. It let to send the base data of the task.
 *
 * Syntax:
 * @verbatim
Sentence ID:             T1 (in ASCII)
Task ID:                 4 char ASCII alphanumeric
Intended date of flight: 6 char YYMMDD
Turnpoints number:       1 char ASCII – Turnpoint total number
                         range: hex 0x30..0x3A (max 10 )
Task description:        30 char max ASCII, if shorter it will be ended by the CR(0x0d)
End of the sentence:     ASCII CR (0x0d)
@endverbatim
 *
 * @param d Device handle
 * @param decl Task declaration data
 * @param errBufferLen The length of the buffer for error string
 * @param errBuffer The  buffer for error string
 *
 * @return Declaration status
 */
bool DSXT1Send(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  // Task ID can be hardcoded
  const TCHAR *TASK_ID_STR = _T("LK8K");

  // prepare date string
  TCHAR dateStr[7];
  _stprintf(dateStr, _T("%02u%02u%02u"), GPS_INFO.Year % 100, GPS_INFO.Month, GPS_INFO.Day);

  // prepare description
  const unsigned DESC_LEN = 30;
  TCHAR descBuffer[DESC_LEN + 1];
  TaskFileName(DESC_LEN + 1, descBuffer);

  // send final sequence
  TCHAR buffer[64];
  _stprintf(buffer, _T("T1%s%s%X%s\r"), TASK_ID_STR, dateStr, decl.num_waypoints - 2, descBuffer);
  d->Com->WriteString(buffer);
  if(!DSXPromptWait(d, PROMPT_WAIT_CHARS)) {
    // LKTOKEN  _@M1420_ = "Error while declaring!"
    // LKTOKEN  _@M1421_ = "Task description"
    _sntprintf(errBuffer, errBufferLen, _T("%s '%s'!"), MsgToken(1420), MsgToken(1421));
    return false;
  }

  return true;
}


/**
 * @brief Converts waypoint GPS coordinates to DSX format
 *
 * @param gpsCoord Waypoint coordinate to translate
 *
 * @return Waypoint coordinate in DSX format
 */
unsigned long DSXCoord(float gpsCoord)
{
  int deg = static_cast<int>(gpsCoord);
  float min = (gpsCoord - deg) * 60;
  float dsxCoord = fabs(deg * 100 + min);
  return static_cast<unsigned long>(dsxCoord * 1000.0) + 123456;
}


/**
 * @brief Translates waypoint data to DSX format
 *
 * @param wp Waypoint to translate
 * @param aatBuffer Buffer with optional AAT data
 * @param limiter Character to use for optional description limiting
 * @param buffer Buffer to store the result
 *
 * @return @c true if limiter was set
 */
bool DSXWaypoint(const WAYPOINT &wp, const TCHAR aatBuffer[], const TCHAR limiter, TCHAR buffer[])
{
  // prepare description
  const unsigned DESC_LEN = 30;
  TCHAR descBuffer[DESC_LEN + 1];
  descBuffer[DESC_LEN] = '\0';
  int result = _sntprintf(descBuffer, DESC_LEN, _T("%s"), wp.Name);

  // check if description limiter should be used
  TCHAR limiterChar = '\0';
  if(result < static_cast<int>(DESC_LEN) && result >= 0)
    limiterChar = limiter;

  // fill output buffer with translated data
  TCHAR sign = 0x30 | (wp.Latitude < 0 ? 0x01 : 0) | (wp.Longitude < 0 ? 0x02 : 0);
  _stprintf(buffer, _T("%08lX%08lX%c%s%s%c"), DSXCoord(wp.Latitude), DSXCoord(wp.Longitude), sign, aatBuffer, descBuffer, limiterChar);

  return limiterChar == limiter;
}


/**
 * @brief Sends DSX T2 sentence
 *
 * DSX T2 is the second sentence to be sent. It must be sent only one time.
 * It adds takeoff, start, finish and landing wapoints with their descriptions.
 *
 * Syntax:
 * @verbatim
Sentence ID:              T2 (in ASCII)
Takeoff Latitude:         8 char ASCII, calculated as the later indicated algorithm
Takeoff Longitude:        8 char ASCII, calculated as the later indicated algorithm
Takeoff N/S And E/W:      1 char ASCII, a sum of constant offset 0x30 and LSB:
                            * 0x01 - 0=latitude NORTH, 1=latitude SOUTH
                            * 0x02 - 0=longitude EAST, 1=longitude WEST
Takeoff Description:      30 char max ASCII, if shorter it will be ended by a 0x03
Start Latitude:           8 char ASCII, calculated as the later indicated algorithm
Start Longitude:          8 char ASCII, calculated as the later indicated algorithm
Start N/S And E/W:        1 char ASCII, a sum of constant offset 0x30 and LSB:
                            * 0x01 - 0=latitude NORTH, 1=latitude SOUTH
                            * 0x02 - 0=longitude EAST, 1=longitude WEST
Start Description:        30 char max ASCII, if shorter it will be ended by a 0x03
Finish Latitude:          8 char ASCII, calculated as the later indicated algorithm
Finish Longitude:         8 char ASCII, calculated as the later indicated algorithm
Finish N/S And E/W:       1 char ASCII, a sum of constant offset 0x30 and LSB:
                            * 0x01 - 0=latitude NORTH, 1=latitude SOUTH
                            * 0x02 - 0=longitude EAST, 1=longitude WEST
Finish Description:       30 char max ASCII, if shorter it will be ended by a 0x03
Landing Latitude:         8 char ASCII, calculated as the later indicated algorithm
Landing Longitude:        8 char ASCII, calculated as the later indicated algorithm
Landing N/S And E/W:      1 char ASCII, a sum of constant offset 0x30 and LSB:
                            * 0x01 - 0=latitude NORTH, 1=latitude SOUTH
                            * 0x02 - 0=longitude EAST, 1=longitude WEST
Landing Description:      30 char max ASCII, if shorter it will be ended by a 0x0d
End of the sentence:      ASCII CR (0x0d)
@endverbatim
 *
 * @param d Device handle
 * @param decl Task declaration data
 * @param errBufferLen The length of the buffer for error string
 * @param errBuffer The  buffer for error string
 *
 * @return Declaration status
 */
bool DSXT2Send(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  // translate takeoff
  TCHAR wp1[64];
  DSXWaypoint(*decl.waypoint[0], _T(""), '\03', wp1);

  // translate start
  TCHAR wp2[64];
  DSXWaypoint(*decl.waypoint[0], _T(""), '\03', wp2);

  // translate finish
  TCHAR wp3[64];
  DSXWaypoint(*decl.waypoint[decl.num_waypoints - 1], _T(""), '\03', wp3);

  // translate landing
  TCHAR wp4[64];
  TCHAR endChar = DSXWaypoint(*decl.waypoint[decl.num_waypoints - 1], _T(""), '\r', wp4) ? '\0' : '\r'; // do not duplicate the last limiter

  // send final sequence
  TCHAR buffer[256];
  _stprintf(buffer, _T("T2%s%s%s%s%c"), wp1, wp2, wp3, wp4, endChar);
  d->Com->WriteString(buffer);
  if(!DSXPromptWait(d, PROMPT_WAIT_CHARS)) {
    // LKTOKEN  _@M1420_ = "Error while declaring!"
    // LKTOKEN  _@M1422_ = "Start and Finish"
    _sntprintf(errBuffer, errBufferLen, _T("%s '%s'!"), MsgToken(1420), MsgToken(1422));
    return false;
  }

  return true;
}


/**
 * @brief Sends DSX T3 sentence
 *
 * DSX T3 is the sentence that should be sent for each turnpoint in the task.
 * It provides data about position and description of each turnpoint.
 *
 * Syntax:
 * @verbatim
Sentence ID:              T3 (in ASCII)
Turnpoint index:          1 char ASCII, the first is 0, the last is 9
Turnpoint Latitude:       8 char ASCII, calculated as the later indicated algorithm
Turnpoint Longitude:      8 char ASCII, calculated as the later indicated algorithm
Turnpoint N/S And E/W:    1 char ASCII, a sum of constant offset 0x30 and LSB:
                            * 0x01 - 0=latitude NORTH, 1=latitude SOUTH
                            * 0x02 - 0=longitude EAST, 1=longitude WEST

vvvvvvvvvvvv (AREA TASK RELATED) vvvvvvvvvvvv
Distance 1:               6 char ASCII, distance 1 * 1000 (sent in hex format)
                          IF NORMAL TASK, send 6 times 0 ASCII (000000)
Distance 2:               6 char ASCII, distance 2 * 1000 (sent in hex format)
                          IF NORMAL TASK, send 6 times 0 ASCII (000000)
Bearing 1:                3 char ASCII, bearing 1 dec degrees
                          IF NORMAL TASK, send 3 times 0 ASCII (000)
Bearing 2:                3 char ASCII, bearing 2 dec degrees
                          IF NORMAL TASK, send 3 times 0 ASCII (000)
^^^^^^^^^^^^ (AREA TASK RELATED) ^^^^^^^^^^^^

Turnpoint Description:    30 char max ASCII, if shorter it will be ended by a 0x0d
End of the sentence:      ASCII CR (0x0d)
@endverbatim
 *
 * @param d Device handle
 * @param decl Task declaration data
 * @param tpIdx The index of turnpoint to declare
 * @param errBufferLen The length of the buffer for error string
 * @param errBuffer The  buffer for error string
 *
 * @return Declaration status
 */
bool DSXT3Send(PDeviceDescriptor_t d, const Declaration_t &decl, unsigned tpIdx, unsigned errBufferLen, TCHAR errBuffer[])
{
  // prepare dummy AAT data - DSX does not support AAT for now
  const TCHAR *aatData = _T("000000000000000000");

  // translate turnpoint
  TCHAR tp[128];
  TCHAR endChar = DSXWaypoint(*decl.waypoint[tpIdx + 1], aatData, '\r', tp) ? '\0' : '\r'; // do not duplicate description limiter

  // send final sequence
  TCHAR buffer[128];
  _stprintf(buffer, _T("T3%u%s%c"), tpIdx, tp, endChar);
  d->Com->WriteString(buffer);
  if(!DSXPromptWait(d, PROMPT_WAIT_CHARS)) {
    // LKTOKEN  _@M1420_ = "Error while declaring!"
    // LKTOKEN  _@M749_ = "Turnpoint"
    _sntprintf(errBuffer, errBufferLen, _T("%s '%s %u'!"), MsgToken(1420), MsgToken(749), tpIdx + 1);
    return false;
  }

  return true;
}


BOOL DSXDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  // Must have at least two, max 12 waypoints
  if(decl->num_waypoints < 2) {
    // LKTOKEN  _@M1412_ = "Not enough waypoints!"
    _tcsncpy(errBuffer, MsgToken(1412), errBufferLen);
    return FALSE;
  }
  if(decl->num_waypoints > 12) {
    // LKTOKEN  _@M1413_ = "Too many waypoints!"
    _tcsncpy(errBuffer, MsgToken(1413), errBufferLen);
    return FALSE;
  }

  bool status = true;

  // Stop RX thread
  d->Com->StopRxThread();
  d->Com->SetRxTimeout(3000);                     // set RX timeout to 3000[ms]

  const unsigned BUFF_LEN = 128;
  TCHAR buffer[BUFF_LEN];

  // Enable DSX declaration mode
  // LKTOKEN  _@M1400_ = "Task declaration"
  // LKTOKEN  _@M1401_ = "Enabling declaration mode"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(1401));
  CreateProgressDialog(buffer);
  status = DSXSwitchDeclareMode(d, true, errBufferLen, errBuffer);

  if(status) {
    // Send user, glider and competition data
    // LKTOKEN  _@M1400_ = "Task declaration"
    // LKTOKEN  _@M1403_ = "Sending  declaration"
    _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(1403));
    CreateProgressDialog(buffer);
    status = status && DSXHSend(d, *decl, errBufferLen, errBuffer);

    // Send T1 sentence with task general data
    status = status && DSXT1Send(d, *decl, errBufferLen, errBuffer);

    // Send T2 sentence with takeoff, start, finish, landing data
    status = status && DSXT2Send(d, *decl, errBufferLen, errBuffer);

    // Send T3 sentence for each turnpoint
    unsigned tpCount = decl->num_waypoints - 2;   // skip Start and Finish waypoint
    for(unsigned i=0; i<tpCount; i++)
      status = status && DSXT3Send(d, *decl, i, errBufferLen, errBuffer);
  }

  // Disable DSX declaration mode
  // LKTOKEN  _@M1400_ = "Task declaration"
  // LKTOKEN  _@M1402_ = "Disabling declaration mode"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(1402));
  CreateProgressDialog(buffer);
  status = DSXSwitchDeclareMode(d, false, errBufferLen, errBuffer) && status; // always do that step otherwise NMEA will not be send

  // Restart RX thread
  d->Com->SetRxTimeout(RXTIMEOUT);    // clear timeout
  d->Com->StartRxThread();

  return status;
}


BOOL DSXIsTrue(PDeviceDescriptor_t d)
{
  return TRUE;
}


BOOL DSXInstall(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, TEXT("DSX"));
  d->ParseNMEA = NULL;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = DSXDeclare;
  d->IsLogger = DSXIsTrue;
  d->IsGPSSource = DSXIsTrue;
  d->IsBaroSource = DSXIsTrue;

  return TRUE;
}


BOOL DSXRegister(void)
{
  return devRegister(TEXT("DSX"), 1l << dfGPS | 1l << dfLogger | 1l << dfBaroAlt, DSXInstall);
}
