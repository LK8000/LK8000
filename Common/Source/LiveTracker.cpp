/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights
*/
#ifdef WIN32
#ifdef PPC2002
#include <winsock.h>
#else
#include <winsock2.h>
#endif
#endif

#ifdef __linux__
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket close


#endif

#include <stdlib.h>
#include <string.h>
#include <cctype>
#include "externs.h"
#include "LiveTracker.h"
#include "utils/stringext.h"
#include "Poco/Event.h"

//Use to log transactions to the startupstore
//#define LT_DEBUG  1

static bool _ws_inited = false;     //Winsock inited
static bool _inited = false;        //Winsock + thread inited
static Poco::Event NewDataEvent;       //new data event trigger
#define SERVERNAME_MAX  100
static char _server_name[SERVERNAME_MAX];      // server name, or ip
static int _server_port;

// Data point definition to send to the server
typedef struct {
  unsigned long int unix_timestamp;     // Unix timestamp
  int flying;                           // true = flying, triggers a new track
  double latitude;                      // position
  double longitude;                     // position
  double alt;                           // altitude MSL [m]
  double ground_speed;                  // GS [km/h]
  double course_over_ground;            // Heading [deg]
} livetracker_point_t;

// Point FIFO beetween calc thread and data server
typedef std::deque<livetracker_point_t> PointQueue;

//Protected thread storage
static CCriticalSection _t_mutex;       // Mutex
static bool _t_run = false;             // Thread run
static bool _t_end = false;             // Thread end
static PointQueue _t_points;            // Point FIFO

// Prototypes
static bool InitWinsock();
static void LiveTrackerThread(void);

// Unix timestamp calculation helpers
#define isleap(y) ( !((y) % 400) || (!((y) % 4) && ((y) % 100)) )
static long monthtoseconds(int isleap,int month)
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

static unsigned long int yeartoseconds(int y)
{
  unsigned long int ret,ltemp;
  if (y < 1970) {
    return 0;
  }
  ret = y - 1970;
  ret *= 365L*24L*60L*60L;
  ltemp = (y-1) - 1968;
  ltemp /= 4;
  ltemp *= 24L*60L*60L;
  ret += ltemp;
  return ret;
}

static unsigned long int mkgmtime(const struct tm *ptmbuf)
{
    unsigned long int t;
    int year = ptmbuf->tm_year + ptmbuf->tm_mon / 12;
    t = yeartoseconds(year+1900);
    t += monthtoseconds(isleap(year+1900), ptmbuf->tm_mon % 12);
    t += (ptmbuf->tm_mday - 1) * 3600L * 24;
    t += ptmbuf->tm_hour * 3600L + ptmbuf->tm_min * 60L + ptmbuf->tm_sec;
    return t;
}

// Encode URLs in a standard form
static char* UrlEncode(const char *szText, char* szDst, int bufsize) {
  char ch; 
  char szHex[5];
  int iMax,i,j; 
  
  iMax = bufsize-2;
  szDst[0]='\0';
  for (i = 0,j=0; szText[i] && j <= iMax; i++) {
    ch = szText[i];
    if (isalnum(ch))
       szDst[j++]=ch;
    else if (ch == ' ')
      szDst[j++]='+';
    else {
      if (j+2 > iMax) break;
      szDst[j++]='%';
      sprintf(szHex, "%-2.2X", ch);
      strncpy(szDst+j,szHex,2);
      j += 2;
   } 
  }
  szDst[j]='\0';
  return szDst;
}

Poco::ThreadTarget _ThreadTarget(LiveTrackerThread);
Poco::Thread _Thread;             //worker thread handle

// Init Live Tracker services, if available
void LiveTrackerInit()
{
  if (LiveTrackerInterval == 0) {
    // If livetracker is not enabled at startup, we do nothing, 
    // but in this case LK must be restarted, if user enables it!
    #if TESTBENCH
    StartupStore(TEXT(". LiveTracker disabled.%s"), NEWLINE);
    #endif
    return;
  }
  //Init winsock if available
  if (InitWinsock()) {
    _ws_inited = true;

    // Create a thread for sending data to the server
    _Thread.start(_ThreadTarget);
    _Thread.setPriority(Poco::Thread::PRIO_NORMAL);
    TCHAR2ascii(LiveTrackersrv_Config, _server_name, SERVERNAME_MAX);
    _server_name[SERVERNAME_MAX-1]=0;
    _server_port=LiveTrackerport_Config;
    StartupStore(TEXT(". LiveTracker will use server %s if available.%s"), LiveTrackersrv_Config, NEWLINE);
    _inited = true;
  }
  if (!_inited) StartupStore(TEXT(". LiveTracker init failed.%s"),NEWLINE);
}

// Shutdown Live Tracker
void LiveTrackerShutdown()
{
  if (_Thread.isRunning()) {
    _t_end = false;
    _t_run = false;
    NewDataEvent.set();
    _Thread.join();
    StartupStore(TEXT(". LiveTracker closed.%s"),NEWLINE);
  }
#ifdef WIN32
  if (_ws_inited) {
    WSACleanup();
  }
#endif
}

// Update live tracker data, non blocking
void LiveTrackerUpdate(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (!_inited) return;               // Do nothing if not inited
  if (LiveTrackerInterval==0) return; // Disabled
  if (Basic->NAVWarning) return;      // Do not log if no gps fix

  livetracker_point_t newpoint;
  static int logtime = 0;

  
  CCriticalSection::CGuard guard(_t_mutex);

  //Check if sending needed (time interval)
  if (Basic->Time >= logtime) { 
    logtime = (int)Basic->Time + LiveTrackerInterval;
    if (logtime>=86400) logtime-=86400;
  } else return;

  // Half hour FIFO must be enough
  if (_t_points.size() > (unsigned int)(1800 / LiveTrackerInterval)) {
    // points in queue are full, drop oldest point 
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
  newpoint.flying = Calculated->Flying;
  newpoint.latitude = Basic->Latitude;
  newpoint.longitude = Basic->Longitude;
  newpoint.alt = Calculated->NavAltitude;
  newpoint.ground_speed = Basic->Speed;
  newpoint.course_over_ground = Calculated->Heading;
  
  _t_points.push_back(newpoint);
  NewDataEvent.set();
}  


bool InitWinsock()
{
#ifdef WIN32
  WSADATA wsaData;
  
  WORD version = MAKEWORD( 1, 1 );
  int error = WSAStartup( version, &wsaData );
  if ( error != 0 ) return false;

  /* check for correct version */
  if ( LOBYTE( wsaData.wVersion ) != 1 ||
      HIBYTE( wsaData.wVersion ) != 1 )
  {
      /* incorrect WinSock version */
      WSACleanup();
      return false;
  }
#endif
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
    Poco::Thread::sleep(1000);
  } while (secs--);
  return false;
}


// Establish a connection with the data server
// Returns a valid SOCKET if ok, INVALID_SOCKET if failed 
static SOCKET EstablishConnection(const char *servername, int serverport)
{
  SOCKET s;
  struct hostent *server;
  struct sockaddr_in sin;
  
  server = gethostbyname(servername);
  if (server == NULL) return INVALID_SOCKET;

  s = socket( AF_INET, SOCK_STREAM, 0 );
  if ( s == INVALID_SOCKET ) return s;
  
  memset( &sin, 0, sizeof sin );
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
  sin.sin_port = htons(serverport);
  if ( connect( s, (sockaddr*)&sin, sizeof(sockaddr_in) ) == SOCKET_ERROR ) {
    //could not connect to server
    closesocket(s);
    return INVALID_SOCKET;
  }
  return s;
}

// Do a transaction with server
// returns received bytes, -1 if transaction failed
static int DoTransactionToServer(char *txbuf, unsigned int txbuflen, char *rxbuf, unsigned int maxrxbuflen)
{
  SOCKET s = INVALID_SOCKET;
  unsigned int sent = 0;
  int tmpres;
  int rxfsm = 0;
  unsigned int rxlen = 0;
  char recvbuf[BUFSIZ];
  unsigned char cdata;

  #ifdef LT_DEBUG
  TCHAR utxbuf[500];
  ascii2unicode(txbuf,utxbuf,400);
  StartupStore(TEXT("Livetracker send: %s%s"), utxbuf, NEWLINE);
  #endif
  
  s = EstablishConnection(_server_name, _server_port);
  if ( s==INVALID_SOCKET ) return -1;

  //Send the query to the server
  while(sent < txbuflen) {
    tmpres = send(s, txbuf+sent, txbuflen-sent, 0);
    if( tmpres == SOCKET_ERROR ) {
      rxlen = -1;
      goto cleanup;
    }
    sent += tmpres;
  }

  //Receive the page
  while( (tmpres = recv(s, recvbuf, BUFSIZ, 0)) > 0) {
    for (int i=0; i<tmpres; i++) {
      cdata = recvbuf[i];
      switch (rxfsm) {
        default:
        case 0:
          if (cdata=='\r') rxfsm++;
          break;
        case 1:
          if (cdata=='\n') { rxfsm++; break; }
          rxfsm=0;
          break;
        case 2:
          if (cdata=='\r') { rxfsm++; break; }
          rxfsm=0;
          break;
        case 3:
          if (cdata=='\n') { rxfsm++; rxlen=0; break; }
          rxfsm=0;
          break;
        case 4:
          //Content chr
          rxbuf[rxlen] = cdata;
          if (rxlen<maxrxbuflen) rxlen++;
          break;
      }//sw
    } //for
  } //wh

  if(tmpres == SOCKET_ERROR) {
    rxlen = -1;
    goto cleanup;
  }  

  rxbuf[rxlen]=0;
  #ifdef LT_DEBUG
  TCHAR urxbuf[500];
  ascii2unicode(rxbuf,urxbuf,400);
  StartupStore(TEXT("Livetracker recv len=%d: %s%s"), rxlen, urxbuf, NEWLINE);
  #endif
cleanup:
  closesocket(s);
  return rxlen;
}

// Get the user id from Leonardo servername
// returns 0=no id, -1=transaction failed
static int GetUserIDFromServer()
{
  int retval = -1;
  int rxlen;
  char txbuf[512];
  char username[128];
  char password[128];
  char rxcontent[32];
  
  TCHAR2ascii(LiveTrackerusr_Config, txbuf, sizeof(username));
  UrlEncode(txbuf, username, sizeof(username));
  TCHAR2ascii(LiveTrackerpwd_Config, txbuf, sizeof(password));
  UrlEncode(txbuf, password, sizeof(username));
  sprintf(txbuf,"GET /client.php?op=login&user=%s&pass=%s HTTP/1.0\r\nHost: %s\r\n\r\n",
        username, password,
        _server_name);

  rxlen = DoTransactionToServer(txbuf, strlen(txbuf), rxcontent, sizeof(rxcontent));
  if ( rxlen > 0) {
    rxcontent[rxlen]=0;
    retval = -1;
    sscanf(rxcontent,"%d",&retval);
  }
  return retval;
}


// static bool SendConnectionLessPacket(livetracker_point_t *sendpoint)
// {
//   char username[64];
//   char password[64];
//   char txbuf[500];
//   char rxbuf[32];
//   int rxlen;
//   
//   if (1) {
//     CCriticalSection::CGuard guard(_t_mutex);
//     unicode2ascii(LiveTrackerusr_Config, txbuf, sizeof(username));
//     UrlEncode(txbuf, username, sizeof(username));
//     unicode2ascii(LiveTrackerpwd_Config, txbuf, sizeof(password));
//     UrlEncode(txbuf, password, sizeof(username));
//     sprintf(txbuf,"GET /track.php?leolive=1&client=%s&v=%s%s&lat=%.5f&lon=%.5f&alt=%.0f&sog=%.0f&cog=%.0f&tm=%lu&user=%sr&pass=%s HTTP/1.0\r\nHost: %s\r\n\r\n",
//           LKFORK,LKVERSION,LKRELEASE,
//           sendpoint->latitude,
//           sendpoint->longitude,
//           sendpoint->alt,
//           sendpoint->ground_speed * 3.6,
//           sendpoint->course_over_ground,
//           sendpoint->unix_timestamp,
//           username, password,
//           LIVETRACKER_SERVER_NAME);
//   }
//   
//   rxlen = DoTransactionToServer(txbuf, strlen(txbuf), rxbuf, sizeof(rxbuf));
//   if (rxlen==2 && rxbuf[0]=='O' && rxbuf[1]=='K') return true;
//   return false;
// }


static bool SendStartOfTrackPacket(unsigned int *packet_id, unsigned int *session_id, int userid)
{
  char username[128];
  char password[128];
  char txbuf[500];
  char rxbuf[32];
  int rxlen;
  char phone[64];
  char gps[64];
  unsigned int vehicle_type = 8;
  char vehicle_name[64];
  int rnd;
  
  if (1) {
    CCriticalSection::CGuard guard(_t_mutex);
    // START OF TRACK PACKET
    // /track.php?leolive=2&sid=42664778&pid=1&client=YourProgramName&v=1&user=yourusername&pass=yourpass&phone=Nokia 2600c&gps=BT GPS&trk1=4&vtype=16388&vname=vehicle name and model
    // PARAMETERS          
    // leolive=2  // 2 means this is the start of track packet
    // sid=42664778 // the session ID , see below on sessionID section for more information
    // pid=1 // the packet numner of this packet, we start with 1 and increase with each packet send either start/end or with GPS data.
    // client=YourClientName// fixed value use only alphanumerics no spaces, first Letter of words in capitals
    // user=yourusername // there is no need to have a registered user, the user can input his preferred username on the fly, he will be displayed in black instead of blue if not registered.
    // pass=yourpass 
    // v=1 // version of your program you can use free text like 1.4.5
    // trk1=4 // the interval in secs that we will be sending gps points
    // phone=Nokia 2600c // the phone model as it is acquired from a system  call
    // &gps=BT GPS // the GPS name , use the string Internal GPS for phones with integrated GPS
    // vname // the brand + name of the vehicle/glider ie Gradient Golden 2 26
    // 
    // Values for vtype
    // 1=>"Paraglider"
    // 8=>"Glider"
    // 64=>"Powered flight"
    // 17100=>"Car"
    if (_tcslen(LiveTrackerusr_Config)>0 ) {
      TCHAR2ascii(LiveTrackerusr_Config, txbuf, sizeof(txbuf));
    } else {
      strncpy(txbuf, "guest", sizeof(txbuf));
    }
    UrlEncode(txbuf, username, sizeof(username));
    if (_tcslen(LiveTrackerpwd_Config)>0 ) {
      TCHAR2ascii(LiveTrackerpwd_Config, txbuf, sizeof(txbuf));
    } else {
      strncpy(txbuf, "guest", sizeof(txbuf));
    }
    UrlEncode(txbuf, password, sizeof(password));
    #ifdef PNA
      TCHAR2ascii(GlobalModelName, txbuf, sizeof(txbuf));
      UrlEncode(txbuf, phone, sizeof(phone));
    #else
    #if (WINDOWSPC>0)
      UrlEncode("PC", phone, sizeof(phone));
    #else
      UrlEncode("PDA", phone, sizeof(phone));
    #endif
    #endif
    if (SIMMODE)
	UrlEncode("SIMULATED", gps, sizeof(gps));
    else
	UrlEncode("GENERIC", gps, sizeof(gps));
/* 
	What is this for?
      else {
        GetBaroDeviceName(_t_barodevice, wgps); 
        unicode2ascii(wgps, txbuf, sizeof(txbuf));
        UrlEncode(txbuf, gps, sizeof(gps));
      }
*/
    
    TCHAR2ascii(AircraftType_Config, txbuf, sizeof(txbuf));
    UrlEncode(txbuf, vehicle_name, sizeof(vehicle_name));
    vehicle_type = 8;
    if (AircraftCategory == umParaglider) vehicle_type = 1;
    if (AircraftCategory == umCar) vehicle_type = 17100;
    if (AircraftCategory == umGAaircraft) vehicle_type = 64;

    *packet_id = 1;
    rnd = rand();
    *session_id = ( (rnd<<24) &  0x7F000000 ) | ( userid & 0x00ffffff) | 0x80000000;          

    sprintf(txbuf,"GET /track.php?leolive=2&sid=%u&pid=1&client=%s&v=%s%s&user=%s&pass=%s&phone=%s&gps=%s&trk1=%u&vtype=%u&vname=%s HTTP/1.0\r\nHost: %s\r\n\r\n",
          *session_id,  
          LKFORK,LKVERSION,LKRELEASE,
          username, password,
          phone, gps,
          LiveTrackerInterval,
          vehicle_type,
          vehicle_name,
          _server_name);
  }
  
  rxlen = DoTransactionToServer(txbuf, strlen(txbuf), rxbuf, sizeof(rxbuf));
  if (rxlen==2 && rxbuf[0]=='O' && rxbuf[1]=='K') {
    (*packet_id)++;
    return true;
  }
  return false;
}


static bool SendEndOfTrackPacket(unsigned int *packet_id, unsigned int *session_id)
{
  char txbuf[500];
  char rxbuf[32];
  int rxlen;
  
  if (1) {
    CCriticalSection::CGuard guard(_t_mutex);
    // END OF TRACK PACKET
    //  /track.php?leolive=3&sid=42664778&pid=453&prid=0
    // PARAMETERS
    //  leolive=3  // 3 means this is the end of track packet
    //  prid=0 // the  status of the user
    //   0-> "Everything OK"
    //   1-> "Need retrieve"
    //   2-> "Need some help, nothing broken"
    //   3-> "Need help, maybe something broken"
    //   4-> "HELP, SERIOUS INJURY"
    sprintf(txbuf,"GET /track.php?leolive=3&sid=%u&pid=%u&prid=0 HTTP/1.0\r\nHost: %s\r\n\r\n",
          *session_id,
          *packet_id,
          _server_name);
  }
  
  rxlen = DoTransactionToServer(txbuf, strlen(txbuf), rxbuf, sizeof(rxbuf));
  if (rxlen==2 && rxbuf[0]=='O' && rxbuf[1]=='K') {
    (*packet_id)++;
    return true;
  }
  return false;
}


static bool SendGPSPointPacket(unsigned int *packet_id, unsigned int *session_id, livetracker_point_t *sendpoint)
{
  char txbuf[500];
  char rxbuf[32];
  int rxlen;
  
  if (1) {
    CCriticalSection::CGuard guard(_t_mutex);
    // GPS POINT PACKET
    //  /track.php?leolive=4&sid=42664778&pid=321&lat=22.3&lon=40.2&alt=23&sog=40&cog=160&tm=1241422845
    // PARAMETERS
    // leolive=4  // 4 means this is a gps point
    // lat=22.3 // the latitude in decimal notation, use negative numbers for west
    // lon=40.2 // lon in decimal, use negative numbers for south
    // alt=23 // altitude in meters above the MSL (not the geoid if it is possible) , no decimals
    // sog=40 // speed over ground in km/h  no decimals
    // cog=160 // course over ground in degrees 0-360, no decimals
    // tm=1241422845 // the unixt timestamp in GMT of the GPS time, not the phone's time.
    sprintf(txbuf,"GET /track.php?leolive=4&sid=%u&pid=%u&lat=%.5f&lon=%.5f&alt=%.0f&sog=%.0f&cog=%.0f&tm=%lu HTTP/1.0\r\nHost: %s\r\n\r\n",
          *session_id,
          *packet_id,
          sendpoint->latitude,
          sendpoint->longitude,
          sendpoint->alt,
          sendpoint->ground_speed * 3.6,
          sendpoint->course_over_ground,
          sendpoint->unix_timestamp,
          _server_name);
  }
  
  rxlen = DoTransactionToServer(txbuf, strlen(txbuf), rxbuf, sizeof(rxbuf));
  if (rxlen==2 && rxbuf[0]=='O' && rxbuf[1]=='K') {
    (*packet_id)++;
    return true;
  }
  return false;
}



// Leonardo Live Tracker (www.livetrack24.com) data exchange thread
static void LiveTrackerThread()
{
  int tracker_fsm = 0;
  livetracker_point_t sendpoint = {0};
  bool sendpoint_valid = false;
  bool sendpoint_processed = false;
  bool sendpoint_processed_old = false;
  // Session variables
  unsigned int packet_id = 0;
  unsigned int session_id = 0;               
  int userid = -1;
  
  _t_end = false;
  _t_run = true;

  srand(Poco::Timestamp().epochTime());

  do {
    if (NewDataEvent.tryWait(5000)) NewDataEvent.reset();
    if (!_t_run) break;
    do {
      if (1) {
        sendpoint_valid = false;
        CCriticalSection::CGuard guard(_t_mutex);
        if (!_t_points.empty()) {
          sendpoint = _t_points.front();
          sendpoint_valid = true;
        }
      } //mutex
        if (sendpoint_valid) {
          sendpoint_processed = false;
          do {
            switch (tracker_fsm) {
              default:
              case 0:   // Wait for flying
                if (!sendpoint.flying) {
                  sendpoint_processed = true;
                  break;
                }
                tracker_fsm++;
                break;
                  
              case 1:
                // Get User ID
                userid = GetUserIDFromServer();
                sendpoint_processed = false;
                if (userid>=0) tracker_fsm++;
                break;
                
              case 2:
                //Start of track packet
                sendpoint_processed = SendStartOfTrackPacket(&packet_id, &session_id, userid);
                if (sendpoint_processed) {
                  StartupStore(TEXT(". Livetracker new track started.%s"),NEWLINE);
                  sendpoint_processed_old = true;
                  tracker_fsm++;
                }
                break;

              case 3:
                //Gps point packet
                sendpoint_processed = SendGPSPointPacket(&packet_id, &session_id, &sendpoint);
                
                //Connection lost to server
                if (sendpoint_processed_old && !sendpoint_processed) {
                  StartupStore(TEXT(". Livetracker connection to server lost.%s"), NEWLINE);
                }
                //Connection established to server
                if (!sendpoint_processed_old && sendpoint_processed) {
                  CCriticalSection::CGuard guard(_t_mutex);
                  int queue_size = _t_points.size();
                  StartupStore(TEXT(". Livetracker connection to server established, start sending %d queued packets.%s"), queue_size, NEWLINE);
                }
                sendpoint_processed_old = sendpoint_processed;
                
                if (!sendpoint.flying) {
                  tracker_fsm++;
                }
                break;

              case 4:
                //End of track packet
                sendpoint_processed = SendEndOfTrackPacket(&packet_id, &session_id);
                if (sendpoint_processed) {
                  StartupStore(TEXT(". Livetracker track finished, sent %d points.%s"), packet_id, NEWLINE);
                  tracker_fsm=0;
                }
                break;
            }// sw
            
            if (sendpoint_processed) {
              CCriticalSection::CGuard guard(_t_mutex);
              _t_points.pop_front();
            } else InterruptibleSleep(2500);
            sendpoint_processed_old = sendpoint_processed;
          } while (!sendpoint_processed && _t_run);
      }
    } while (sendpoint_valid && _t_run);
  } while (_t_run);
  
  _t_end = true;
}
