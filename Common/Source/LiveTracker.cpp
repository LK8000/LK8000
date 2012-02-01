/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights
*/

#include "LiveTracker.h"
#include <winsock.h>

static bool _ws_inited = false;
static bool _inited = false;
static HANDLE _hThread;
static DWORD _dwThreadID;

typedef struct {
  unsigned long int unix_timestamp;
  double latitude;
  double longitude;
  double alt;
  double ground_speed;
  double course_over_ground;
  
} livetracker_point_t;

typedef std::deque<livetracker_point_t> PointQueue;

//Protected thread storage
static CCriticalSection _t_mutex;
static bool _t_run = false;
static bool _t_end = false;
static PointQueue _t_points;

static bool InitWinsock();
static DWORD WINAPI LiveTrackerThread(LPVOID lpvoid);

#define isleap(y) ( !((y) % 400) || (!((y) % 4) && ((y) % 100)) )
static time_t monthtoseconds(int isleap,int month)
{
static const long        _secondstomonth[12] = {
        0,
        24L*60*60*31,
        24L*60*60*(31+28),
        24L*60*60*(31+28+31),
        24L*60*60*(31+28+31+30),
        24L*60*60*(31+28+31+30+31),
        24L*60*60*(31+28+31+30+31+30),
        24L*60*60*(31+28+31+30+31+30+31),
        24L*60*60*(31+28+31+30+31+30+31+31),
        24L*60*60*(31+28+31+30+31+30+31+31+30),
        24L*60*60*(31+28+31+30+31+30+31+31+30+31),
        24L*60*60*(31+28+31+30+31+30+31+31+30+31+30)
};
        long ret;
        if ((month > 11)||(month < 0)) {
                return 0;
        }
        ret = _secondstomonth[month];
        if (isleap && (month > 1)) {
                return ret + 24L * 60 * 60;
        }
        return ret;
}

static time_t yeartoseconds(int y)
{
  time_t ret;
  if (y < 1970) {
    return 0;
  }
  ret = (y - 1970)*365L*24*60*60;
  ret += (((y-1) - 1968) / 4)*24L*60*60;
  return ret;
}

unsigned long int mkgmtime(const struct tm *ptmbuf)
{
    time_t t;
    int year = ptmbuf->tm_year + ptmbuf->tm_mon / 12;
    t = yeartoseconds(year+1900);
    t += monthtoseconds(isleap(year+1900), ptmbuf->tm_mon % 12);
    t += (ptmbuf->tm_mday - 1) * 3600L * 24;
    t += ptmbuf->tm_hour * 3600L + ptmbuf->tm_min * 60L + ptmbuf->tm_sec;
    return t;
}


// Init Live Tracker services, if available
void LiveTrackerInit()
{
  //Init winsock if available
  if (InitWinsock()) {
    _ws_inited = true;
    // Create a thread for sending data to the server
    if ((_hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)&LiveTrackerThread, 0, 0, &_dwThreadID)) != NULL)
    {
      SetThreadPriority(_hThread, THREAD_PRIORITY_NORMAL);
      _inited = true;
      StartupStore(TEXT(". LiveTracker inited.%s"),NEWLINE);
    }
  }
  if (!_inited) StartupStore(TEXT(". LiveTracker init failed.%s"),NEWLINE);
}

// Shutdown Live Tracker
void LiveTrackerShutdown()
{
  if (_hThread != NULL) {
    _t_end = false;
    _t_run = false;
    WaitForSingleObject(_hThread, INFINITE);
    CloseHandle(_hThread);
    StartupStore(TEXT(". LiveTracker closed.%s"),NEWLINE);
  }
  if (_ws_inited) WSACleanup();
}

// Update live tracker data, non blocking
void LiveTrackerUpdate(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  livetracker_point_t newpoint;
  
  CCriticalSection::CGuard guard(_t_mutex);
  
  //Check if sending needed (time interval)
  
  if (_t_points.size()>100) {
    // points in queue are full, drop last point
    _t_points.pop_front();
  }
  
  struct tm t;
  time_t t_of_day;
  t.tm_year = Basic->Year - 1900;
  t.tm_mon = Basic->Month - 1; // Month, 0 - jan
  t.tm_mday = Basic->Day;
  t.tm_hour = Basic->Hour;
  t.tm_min = Basic->Minute;
  t.tm_sec = Basic->Second;
  t.tm_isdst = 0; // Is DST on? 1 = yes, 0 = no, -1 = unknown
  t_of_day = mkgmtime(&t);
  
  newpoint.unix_timestamp = t_of_day;
  newpoint.latitude = Basic->Latitude;
  newpoint.longitude = Basic->Longitude;
  newpoint.alt = Calculated->NavAltitude;
  newpoint.ground_speed = Basic->Speed;
  newpoint.course_over_ground = Calculated->Heading;
  
  _t_points.push_back(newpoint);
}  


bool InitWinsock()
{
  WSADATA wsaData;
  WORD version;
  int error;

  // Check that system has winsock dlls
  #if WINDOWSPC>0
  if (GetProcAddress(GetModuleHandle(TEXT("WSOCK32.DLL")), "WSAStartup") == NULL) return false;
  #else
  if (GetProcAddress(GetModuleHandle(TEXT("WINSOCK")), TEXT("WSAStartup")) == NULL) return false;
  #endif
  
  version = MAKEWORD( 1, 1 );
  error = WSAStartup( version, &wsaData );
  if ( error != 0 ) return false;

  /* check for correct version */
  if ( LOBYTE( wsaData.wVersion ) != 1 ||
      HIBYTE( wsaData.wVersion ) != 1 )
  {
      /* incorrect WinSock version */
      WSACleanup();
      return false;
  }
  return true;
}

static bool InterruptibleSleep(int msecs)
{
  int secs = msecs / 1000;
  do {
    if (1) {
      CCriticalSection::CGuard guard(_t_mutex);
      if (!_t_run) return true;
    }
    Sleep(1000);
  } while (secs--);
  return false;
}

static SOCKET EstablishConnection(char *servername)
{
  SOCKET s;
  struct hostent *server;
  struct sockaddr_in sin;
  
  s = socket( AF_INET, SOCK_STREAM, 0 );
  if ( s == INVALID_SOCKET ) return s;

  server = gethostbyname(servername);
  if (server == NULL) return INVALID_SOCKET;
  
  memset( &sin, 0, sizeof sin );
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
  sin.sin_port = htons(80);
  if ( connect( s, (sockaddr*)&sin, sizeof(sockaddr_in) ) == SOCKET_ERROR ) {
    //could not connect to server
    closesocket(s);
    return INVALID_SOCKET;
  }
  return s;
}

static bool SendLiveTrackerData(SOCKET s, livetracker_point_t *sendpoint)
{
  // /track.php?leolive=1&client=YourProgramName&v=1&lat=22.3&lon=40.2&alt=23&sog=40&cog=160&tm=1241422845&user=yourusername&pass=yourpass
  char txbuf[500];
  
  sprintf(txbuf,"GET /track.php?leolive=1&client=%sv%s%s&v=1&lat=%.5lf&lon=%.5lf&alt=%.0lf&sog=%.0lf&cog=%.0lf&tm=%lu&user=_testuser&pass=_testpass HTTP/1.0\r\nHost: test.livetrack24.com\r\n\r\n",
          LKFORK,LKVERSION,LKRELEASE,
          sendpoint->latitude,
          sendpoint->longitude,
          sendpoint->alt,
          sendpoint->ground_speed * 3.6,
          sendpoint->course_over_ground,
          sendpoint->unix_timestamp);
          
  TCHAR utxbuf[500];
  ascii2unicode(txbuf,utxbuf,400);
  StartupStore(TEXT("Livetracker: %s%s"), utxbuf, NEWLINE);
  return true;
}

static DWORD WINAPI LiveTrackerThread (LPVOID lpvoid)
{
  SOCKET s = INVALID_SOCKET;
  livetracker_point_t sendpoint;
  bool sendpoint_valid = false;
  bool connection_valid = false;

  _t_end = false;
  _t_run = true;

  do {
    //Try to connect to the server
    connection_valid = false;
    s = EstablishConnection("test.livetrack24.com");
    if (s == INVALID_SOCKET) {
      if (InterruptibleSleep(10000)) { _t_end = true; return 0; }
      continue;
    }
    connection_valid = true;
    StartupStore(TEXT("LiveTracker connected.%s"),NEWLINE);
    do {
      do {
        if (1) {
          sendpoint_valid = false;
          CCriticalSection::CGuard guard(_t_mutex);
          if (!_t_points.empty()) {
            sendpoint = _t_points.front();
            _t_points.pop_front();
            sendpoint_valid = true;
          }
        } //mutex
        if (sendpoint_valid) {
          connection_valid = SendLiveTrackerData(s, &sendpoint);
        }
      } while (sendpoint_valid && connection_valid);
      if (InterruptibleSleep(20000)) { _t_end = true; return 0; }
    } while (connection_valid);
  } while (_t_run);
  
  if (s!=INVALID_SOCKET) closesocket(s);
  _t_end = true;
  return 0;
}

