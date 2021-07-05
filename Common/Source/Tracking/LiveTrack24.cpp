/*
 LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 Released under GNU/GPL License v.2
 See CREDITS.TXT file for authors and copyrights
 */

//Use to log transactions to the startupstore
//#define LT_DEBUG  1

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
	#include <sys/ioctl.h>

	#define SOCKET int
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	#define closesocket close
	#define ioctlsocket ioctl
#endif
#define DELAY 5000

#include <stdlib.h>
#include <cctype>
#include <sstream>
#include <time.h>
#include <zlib.h>
#include <string>
#include "externs.h"
#include "Tracking.h"
#include "LiveTrack24.h"
#include "LiveTrack24APIKey.h"
#include "utils/stringext.h"
#include "Poco/Event.h"
#include "Poco/ThreadTarget.h"
#include "picojson.h"
#include "utils/hmac_sha2.h"
#include "FlarmCalculations.h"
#include "md5.h"
#include "NavFunctions.h"

#ifdef KOBO
#include "Kobo/System.hpp"
#endif

static bool _ws_inited = false;     //Winsock inited
static bool _inited = false;        //Winsock + thread inited
static Poco::Event NewDataEvent;       //new data event trigger
#define SERVERNAME_MAX  100
static char _server_name[SERVERNAME_MAX];      // server name, or ip
static int _server_port;

// Globals for Info API V2
static const std::string g_deviceID = "LK8000";
static std::string g_ut = "g_ut";
static std::string g_otpQuestion = "98d89cafa872ccdc";
static int g_sync = 0;
static char _username[100];
static char _pilotname[100];
static char _password[100];
static bool flarmwasinit = false;
static bool _t_radar_run = false;             // Thread run
static bool _t_radar_end = false;             // Thread end

// Globals for Tracking API V2
static std::string v2_pwt = ""; // Password token for API V2
static int v2_sid = 0;
static int v2_userid = -1;

static const std::string mapGBase64Index =
		"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()";
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";


// Data point definition to send to the server
typedef struct {
	unsigned long int unix_timestamp;     // Unix timestamp
	int flying;                           // true = flying, triggers a new track
	double latitude;                      // position
	double longitude;                     // position
	double alt;                           // altitude MSL [m]
	double ground_speed;                  // GS [m/s]
	double course_over_ground;            // Heading [deg]
} livetracker_point_t;

// Point FIFO beetween calc thread and data server
typedef std::deque<livetracker_point_t> PointQueue;

//Protected thread storage
static Mutex _t_mutex;                  // Mutex
static bool _t_run = false;             // Thread run
static bool _t_end = false;             // Thread end
static PointQueue _t_points;            // Point FIFO

// Prototypes
static bool InitWinsock();
static void LiveTrackerThread(void);
static void LiveTrackerThread2(void);
static void LiveTrackRadarThread2(void);
static int createSID();
static std::string passwordToken(const std::string& plainTextPassword,
		const std::string& sessionID);

Poco::Thread _ThreadTracker;             //worker thread for Tracker
Poco::Thread _ThreadRadar;        //worker thread for Radar
Poco::ThreadTarget _ThreadTargetTracker(LiveTrackerThread);
Poco::ThreadTarget _ThreadTargetTracker2(LiveTrackerThread2);
Poco::ThreadTarget _ThreadTargetRadar(LiveTrackRadarThread2);

template<typename T>
std::string toString(const T& value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

// Unix timestamp calculation helpers
#define isleap(y) ( !((y) % 400) || (!((y) % 4) && ((y) % 100)) )
static long monthtoseconds(int isleap, int month) {
	static const long _secondstomonth[12] = { 0, 24L * 60 * 60 * 31, 24L * 60
			* 60 * (31 + 28), 24L * 60 * 60 * (31 + 28 + 31), 24L * 60 * 60
			* (31 + 28 + 31 + 30), 24L * 60 * 60 * (31 + 28 + 31 + 30 + 31), 24L
			* 60 * 60 * (31 + 28 + 31 + 30 + 31 + 30), 24L * 60 * 60
			* (31 + 28 + 31 + 30 + 31 + 30 + 31), 24L * 60 * 60
			* (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31), 24L * 60 * 60
			* (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30), 24L * 60 * 60
			* (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31), 24L * 60 * 60
			* (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30) };
	long ret;
	if ((month > 11) || (month < 0)) {
		return 0;
	}
	ret = _secondstomonth[month];
	if (isleap && (month > 1)) {
		return ret + 24L * 60 * 60;
	}
	return ret;
}

static std::string random_string( size_t length )
{
  auto randchar = []() -> char
  {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ std::rand() % max_index ];
  };
  std::string str(length,0);
  std::generate_n( str.begin(), length, randchar );
  return str;
}
static std::string  DeviceID =  random_string(10);


static unsigned long int yeartoseconds(int y) {
	unsigned long int ret, ltemp;
	if (y < 1970) {
		return 0;
	}
	ret = y - 1970;
	ret *= 365L * 24L * 60L * 60L;
	ltemp = (y - 1) - 1968;
	ltemp /= 4;
	ltemp *= 24L * 60L * 60L;
	ret += ltemp;
	return ret;
}

static unsigned long int mkgmtime(const struct tm *ptmbuf) {
	unsigned long int t;
	int year = ptmbuf->tm_year + ptmbuf->tm_mon / 12;
	t = yeartoseconds(year + 1900);
	t += monthtoseconds(isleap(year + 1900), ptmbuf->tm_mon % 12);
	t += (ptmbuf->tm_mday - 1) * 3600L * 24;
	t += ptmbuf->tm_hour * 3600L + ptmbuf->tm_min * 60L + ptmbuf->tm_sec;
	return t;
}

// Encode URLs in a standard form
static char* UrlEncode(const char *szText, char* szDst, int bufsize) {
	static constexpr char hex_chars[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	int iMax = bufsize - 2;
	int j = 0;
	szDst[0] = '\0';
	for (int i = 0; szText[i] && j <= iMax; i++) {
		const char ch = szText[i];
		if (isalnum(ch))
			szDst[j++] = ch;
		else if (ch == ' ')
			szDst[j++] = '+';
		else {
			if (j + 2 > iMax)
				break;
			szDst[j++] = '%';
			szDst[j++] = (hex_chars[((ch) & 0xF0) >> 4]);
			szDst[j++] = (hex_chars[((ch) & 0x0F) >> 0]);
		}
	}
	szDst[j] = '\0';
	return szDst;
}

// Init Live Tracker services, if available
void LiveTrackerInit() {
	if (((tracking::interval == 0)
#ifndef TESTBENCH	     // if not compiled as testbench
	     || (SIMMODE)    // disable tracking for simulation mode
#endif	     
	    )
			&& (!tracking::radar_config || !EnableFLARMMap)) {
		// If livetracker is not enabled at startup, we do nothing,
		// but in this case LK must be restarted, if user enables it!
#if TESTBENCH
		StartupStore(TEXT(". LiveTracker disabled.%s"), NEWLINE);
#endif
		return;
	}

#ifdef KOBO
	if(!IsKoboWifiOn()) {
		KoboWifiOn();
	}
#endif // KOBO

	// If we are using LiveTrack24 RADAR let's increase ghost and zombie times. ( maybe remove ISPARAGLIDER ? )
	if (!_ws_inited && ISPARAGLIDER && tracking::radar_config)
		LKTime_Real = 60, LKTime_Ghost = 120, LKTime_Zombie = 360;

	to_utf8(tracking::usr_config, _username);
	to_utf8(PilotName_Config, _pilotname);
	to_utf8(tracking::pwd_config, _password);
	to_utf8(tracking::server_config, _server_name);

	_server_port = tracking::port_config;

	//Init winsock if available
	if (InitWinsock()) {
		_ws_inited = true;

		// Create a thread for sending data to the server
		if (tracking::interval != 0) {
			std::string snu = std::string(_server_name);
			transform(snu.begin(), snu.end(), snu.begin(), ::toupper);
			if (snu.compare("WWW.LIVETRACK24.COM") == 0) {
				v2_sid = createSID();
				v2_pwt = passwordToken(std::string(_password), toString(v2_sid));
				_ThreadTracker.start(_ThreadTargetTracker2);
				_ThreadTracker.setPriority(Poco::Thread::PRIO_NORMAL);
				StartupStore(
						TEXT(
								". LiveTracker API V2 will use server %s if available.%s"),
						tracking::server_config, NEWLINE);
			} else {
				_ThreadTracker.start(_ThreadTargetTracker);
				_ThreadTracker.setPriority(Poco::Thread::PRIO_NORMAL);
				StartupStore(
						TEXT(
								". LiveTracker API V1 will use server %s if available.%s"),
						tracking::server_config, NEWLINE);
			}

		}

		// Create a thread fo getting radar data from livetrack24.com
		if (tracking::radar_config && EnableFLARMMap) {
			_ThreadRadar.start(_ThreadTargetRadar);
			_ThreadRadar.setPriority(Poco::Thread::PRIO_NORMAL);
			StartupStore(TEXT(". LiveTracker Radar Enabled.%s"), NEWLINE);

		}
		_inited = true;
	}
	if (!_inited)
		StartupStore(TEXT(". LiveTracker init failed.%s"), NEWLINE);
}

// Shutdown Live Tracker
void LiveTrackerShutdown() {
	if (_ThreadTracker.isRunning()) {
		_t_end = false;
		_t_run = false;
		NewDataEvent.set();
		_ThreadTracker.join();
		StartupStore(TEXT(". LiveTracker closed.%s"), NEWLINE);
	}

	if (_ThreadRadar.isRunning()) {
		_t_radar_end = false;
		_t_radar_run = false;
		NewDataEvent.set();
		_ThreadRadar.join();
		StartupStore(TEXT(". LiveRadar closed.%s"), NEWLINE);
	}

#ifdef WIN32
	if (_ws_inited) {
		WSACleanup();
	}
#endif
}

// Update live tracker data, non blocking
void LiveTrackerUpdate(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
	if (!_inited)
		return;               // Do nothing if not inited
	if (tracking::interval == 0)
		return; // Disabled
	if (Basic.NAVWarning)
		return;      // Do not log if no gps fix

//	if (!Calculated->Flying)
//		return;  // Do not feed if not necessary

	static int logtime = 0;

	//Check if sending needed (time interval)
	if (Basic.Time >= logtime) {
		logtime = (int) Basic.Time + tracking::interval;
		if (logtime >= 86400)
			logtime -= 86400;
	} else
		return;

	struct tm t;
	time_t t_of_day;
	t.tm_year = Basic.Year - 1900;
	t.tm_mon = Basic.Month - 1; // Month, 0 - jan
	t.tm_mday = Basic.Day;
	t.tm_hour = Basic.Hour;
	t.tm_min = Basic.Minute;
	t.tm_sec = Basic.Second;
	t.tm_isdst = 0; // Is DST on? 1 = yes, 0 = no, -1 = unknown
	t_of_day = mkgmtime(&t);

	livetracker_point_t newpoint;

	newpoint.unix_timestamp = t_of_day;
	if(tracking::start_config == 0) {
		newpoint.flying = Calculated.Flying;
	} else {
		newpoint.flying = true;
	}
	newpoint.latitude = Basic.Latitude;
	newpoint.longitude = Basic.Longitude;

	/* TODO : GPS Altitude or Baro Altitude if Available ?
	 *  nothing mentioned about that in Livetrack24 API documentation
	 *  in all case we need to use same altitude for push Track data and Pull "Radar' track data.
	 */
	newpoint.alt = Basic.Altitude;

	newpoint.ground_speed = Basic.Speed;
	newpoint.course_over_ground = Calculated.Heading;

	{
		ScopeLock guard(_t_mutex);
		// Half hour FIFO must be enough
		if (_t_points.size() > static_cast<size_t>(1800 / tracking::interval)) {
			// points in queue are full, drop oldest point
			_t_points.pop_front();
		}
		_t_points.emplace_back(newpoint);
	}
	NewDataEvent.set();
}

bool InitWinsock() {
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

static bool InterruptibleSleep(int msecs) {
	int secs = msecs / 1000;
	do {
		if (1) {
			ScopeLock guard(_t_mutex);
			if (!_t_run)
				return true;
		}
		Poco::Thread::sleep(1000);
	} while (secs--);
	return false;
}

// Establish a connection with the data server
// Returns a valid SOCKET if ok, INVALID_SOCKET if failed 
static SOCKET EstablishConnection(const char *servername, int serverport) {
	SOCKET s;
	struct hostent *server;
	struct sockaddr_in sin;

	server = gethostbyname(servername);
	if (server == NULL)
		return INVALID_SOCKET;

	s = socket( AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
		return s;

	memset(&sin, 0, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ((struct in_addr *) (server->h_addr))->s_addr;
	sin.sin_port = htons(serverport);
	if (connect(s, (sockaddr*) &sin, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		//could not connect to server
		closesocket(s);
		return INVALID_SOCKET;
	}

	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled;
	// If iMode != 0, non-blocking mode is enabled.

	u_long iMode = 1;
	int iResult = ioctlsocket(s, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR) {
		StartupStore(_T(".... ioctlsocket failed with error: %d%s"), iResult,
		NEWLINE);
		// if failed, socket still in blocking mode, it's big problem
	}

	return s;
}

static int ReadData(SOCKET s, void *szString, size_t size) {

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	fd_set readfs;
	FD_ZERO(&readfs);
	FD_SET(s, &readfs);

	// wait for received data
	int iResult = select(s + 1, &readfs, NULL, NULL, &timeout);
	if (iResult == 0) {
		return SOCKET_ERROR; // timeout
	}

	if ((iResult != SOCKET_ERROR) && FD_ISSET(s, &readfs)) {
		// Data ready to read
		iResult = recv(s, (char*) szString, size, 0);
	}

	return iResult;
}

static int WriteData(SOCKET s, const void *data, size_t length) {

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	fd_set writefs;
	FD_ZERO(&writefs);
	FD_SET(s, &writefs);

	// wait for socket ready to write
	int iResult = select(s + 1, NULL, &writefs, NULL, &timeout);
	if (iResult == 0) {
		return SOCKET_ERROR; // timeout
	}

	if ((iResult != SOCKET_ERROR) && FD_ISSET(s, &writefs)) {
		// socket ready, Write data.

#ifdef __linux        
		const int flags = MSG_NOSIGNAL; // avoid SIGPIPE if socket is closed by peer.
#else
		const int flags = 0;
#endif

		iResult = send(s, (const char*) data, length, flags);
	}

	return iResult;
}

// Do a transaction with server
// returns received bytes, -1 if transaction failed
static int DoTransactionToServer(const char* server_name, int server_port,
		const char *txbuf, char *rxbuf, unsigned int maxrxbuflen) {

#ifdef LT_DEBUG
	StartupStore(TEXT(".DoTransactionToServer txbuf : %s%s"), txbuf, NEWLINE);
#endif

	int rxfsm = 0;
	unsigned int rxlen = 0;
	char recvbuf[BUFSIZ];
	unsigned char cdata;

	SOCKET s = EstablishConnection(server_name, server_port);
	if (s == INVALID_SOCKET)
		return -1;

	//Send the query to the server
	int tmpres = WriteData(s, "GET ", (unsigned int) strlen("GET "));
	if (tmpres < 0) {
		rxlen = -1;
		goto cleanup;
	}
	tmpres = WriteData(s, txbuf, strlen(txbuf));
	if (tmpres < 0) {
		rxlen = -1;
		goto cleanup;
	}
	tmpres = WriteData(s, " HTTP/1.0\r\nHost: ",
			(unsigned int) strlen(" HTTP/1.0\r\nHost: "));
	if (tmpres < 0) {
		rxlen = -1;
		goto cleanup;
	}
	tmpres = WriteData(s, server_name, (unsigned int) strlen(server_name));
	if (tmpres < 0) {
		rxlen = -1;
		goto cleanup;
	}
	tmpres = WriteData(s, "\r\n\r\n", (unsigned int) strlen("\r\n\r\n"));
	if (tmpres < 0) {
		rxlen = -1;
		goto cleanup;
	}

	//Receive the page
	while ((tmpres = ReadData(s, recvbuf, BUFSIZ)) > 0) {
		for (int i = 0; i < tmpres; i++) {
			cdata = recvbuf[i];
			switch (rxfsm) {
			default:
			case 0:
				if (cdata == '\r')
					rxfsm++;
				break;
			case 1:
				if (cdata == '\n') {
					rxfsm++;
					break;
				}
				rxfsm = 0;
				break;
			case 2:
				if (cdata == '\r') {
					rxfsm++;
					break;
				}
				rxfsm = 0;
				break;
			case 3:
				if (cdata == '\n') {
					rxfsm++;
					rxlen = 0;
					break;
				}
				rxfsm = 0;
				break;
			case 4:
				//Content chr
				if (rxlen < maxrxbuflen) {
					rxbuf[rxlen++] = cdata;
				}
				break;
			} //sw
		} //for
	} //wh

	if (tmpres == SOCKET_ERROR) {
		rxlen = -1;
		goto cleanup;
	}

	rxbuf[std::min(rxlen, maxrxbuflen-1)] = 0;
#ifdef LT_DEBUG
	StartupStore(TEXT(".DoTransactionToServer recv len=%d: %s%s"), rxlen, rxbuf, NEWLINE);
#endif
	cleanup:
	closesocket(s);
	return rxlen;
}

// Get the user id from Leonardo servername
// returns 0=no id, -1=transaction failed
static int GetUserIDFromServer() {
	int retval = -1;
	int rxlen;
	char txbuf[512];
	char username[128];
	char password[128];
	char rxcontent[32];

	to_utf8(tracking::usr_config, txbuf);
	UrlEncode(txbuf, username, std::size(username));
	to_utf8(tracking::pwd_config, txbuf);
	UrlEncode(txbuf, password, std::size(password));
	sprintf(txbuf, "/client.php?op=login&user=%s&pass=%s", username, password);

	rxlen = DoTransactionToServer(_server_name, _server_port, txbuf, rxcontent,
			sizeof(rxcontent));
	if (rxlen > 0) {
		rxcontent[rxlen] = 0;
		retval = -1;
		sscanf(rxcontent, "%d", &retval);
	}
	return retval;
}

static bool SendStartOfTrackPacket(unsigned int *packet_id,
		unsigned int *session_id, int userid) {

	char username[std::size(tracking::usr_config)]; 
	char password[std::size(tracking::pwd_config)];

	char txbuf[500];
	char rxbuf[32];
	int rxlen;
	char phone[64];
	char gps[64];
	unsigned int vehicle_type = 8;
	char vehicle_name[64];
	int rnd;

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
	if (_tcslen(tracking::usr_config) > 0) {
		to_utf8(tracking::usr_config, txbuf);
	} else {
		strncpy(txbuf, "guest", std::size(txbuf));
	}
	UrlEncode(txbuf, username, std::size(username));
	if (_tcslen(tracking::pwd_config) > 0) {
		to_utf8(tracking::pwd_config, txbuf);
	} else {
		strncpy(txbuf, "guest", std::size(txbuf));
	}
	UrlEncode(txbuf, password, std::size(password));
#ifdef PNA
	to_utf8(GlobalModelName, txbuf);
	UrlEncode(txbuf, phone, std::size(phone));
#else
#if (WINDOWSPC>0)
	UrlEncode("PC", phone, std::size(phone));
#else
	UrlEncode("PDA", phone, std::size(phone));
#endif
#endif
	if (SIMMODE)
		UrlEncode("SIMULATED", gps, std::size(gps));
	else
		UrlEncode("GENERIC", gps, std::size(gps));
	/*
	 What is this for?
	 else {
	 GetBaroDeviceName(_t_barodevice, wgps);
	 to_utf8(wgps, txbuf);
	 UrlEncode(txbuf, gps, sizeof(gps));
	 }
	 */

	to_utf8(AircraftType_Config, txbuf);
	UrlEncode(txbuf, vehicle_name, std::size(vehicle_name));
	vehicle_type = 8;
	if (AircraftCategory == umParaglider)
		vehicle_type = 1;
	if (AircraftCategory == umCar)
		vehicle_type = 17100;
	if (AircraftCategory == umGAaircraft)
		vehicle_type = 64;

	*packet_id = 1;
	rnd = rand();
	*session_id = ((rnd << 24) & 0x7F000000) | (userid & 0x00ffffff)
			| 0x80000000;

	sprintf(txbuf,
			"/track.php?leolive=2&sid=%u&pid=1&client=%s&v=%s%s&user=%s&pass=%s&phone=%s&gps=%s&trk1=%u&vtype=%u&vname=%s",
			*session_id,
			LKFORK, LKVERSION, LKRELEASE, username, password, phone, gps,
			tracking::interval, vehicle_type, vehicle_name);

	rxlen = DoTransactionToServer(_server_name, _server_port, txbuf, rxbuf, sizeof(rxbuf));
	if (rxlen == 2 && rxbuf[0] == 'O' && rxbuf[1] == 'K') {
		(*packet_id)++;
		return true;
	}
	return false;
}

static bool SendEndOfTrackPacket(unsigned int *packet_id,
		unsigned int *session_id) {
	char txbuf[500];
	char rxbuf[32];
	int rxlen;

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
	sprintf(txbuf, "/track.php?leolive=3&sid=%u&pid=%u&prid=0", *session_id,
			*packet_id);

	rxlen = DoTransactionToServer(_server_name, _server_port, txbuf, rxbuf, sizeof(rxbuf));
	if (rxlen == 2 && rxbuf[0] == 'O' && rxbuf[1] == 'K') {
		(*packet_id)++;
		return true;
	}
	return false;
}

static bool SendGPSPointPacket(unsigned int *packet_id,
		unsigned int *session_id, livetracker_point_t *sendpoint) {
	char txbuf[500];
	char rxbuf[32];
	int rxlen;

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
	sprintf(txbuf,
			"/track.php?leolive=4&sid=%u&pid=%u&lat=%.5f&lon=%.5f&alt=%.0f&sog=%.0f&cog=%.0f&tm=%lu",
			*session_id, *packet_id, sendpoint->latitude,
			sendpoint->longitude, sendpoint->alt,
			sendpoint->ground_speed * 3.6, sendpoint->course_over_ground,
			sendpoint->unix_timestamp);

	rxlen = DoTransactionToServer(_server_name, _server_port, txbuf, rxbuf,
			sizeof(rxbuf));
	if (rxlen == 2 && rxbuf[0] == 'O' && rxbuf[1] == 'K') {
		(*packet_id)++;
		return true;
	}
	return false;
}

// Leonardo Live Tracker (www.livetrack24.com) data exchange thread
static void LiveTrackerThread() {
	int tracker_fsm = 0;
	livetracker_point_t sendpoint = {};
	bool sendpoint_valid = false;
	bool sendpoint_processed = false;
	bool sendpoint_processed_old = false;
	// Session variables
	unsigned int packet_id = 0;
	unsigned int session_id = 0;
	int userid = -1;

	_t_end = false;
	_t_run = true;

	srand(MonotonicClockMS());

	do {
		if (NewDataEvent.tryWait(5000))
			NewDataEvent.reset();
		if (!_t_run)
			break;
		do {
			if (1) {
				sendpoint_valid = false;
				ScopeLock guard(_t_mutex);
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
						if (userid >= 0)
							tracker_fsm++;
						break;

					case 2:
						//Start of track packet
						sendpoint_processed = SendStartOfTrackPacket(&packet_id,
								&session_id, userid);
						if (sendpoint_processed) {
							StartupStore(
									TEXT(". Livetracker new track started.%s"),
									NEWLINE);
							sendpoint_processed_old = true;
							tracker_fsm++;
						}
						break;

					case 3:
						//Gps point packet
						sendpoint_processed = SendGPSPointPacket(&packet_id,
								&session_id, &sendpoint);

						//Connection lost to server
						if (sendpoint_processed_old && !sendpoint_processed) {
							StartupStore(
									TEXT(
											". Livetracker connection to server lost.%s"),
									NEWLINE);
						}
						//Connection established to server
						if (!sendpoint_processed_old && sendpoint_processed) {
							ScopeLock guard(_t_mutex);
							int queue_size = _t_points.size();
							StartupStore(
									TEXT(
											". Livetracker connection to server established, start sending %d queued packets.%s"),
									queue_size, NEWLINE);
						}
						sendpoint_processed_old = sendpoint_processed;

						if (!sendpoint.flying) {
							tracker_fsm++;
						}
						break;

					case 4:
						//End of track packet
						sendpoint_processed = SendEndOfTrackPacket(&packet_id,
								&session_id);
						if (sendpoint_processed) {
							StartupStore(
									TEXT(
											". Livetracker track finished, sent %d points.%s"),
									packet_id, NEWLINE);
							tracker_fsm = 0;
						}
						break;
					}   // sw

					if (sendpoint_processed) {
						ScopeLock guard(_t_mutex);
						_t_points.pop_front();
					} else
						InterruptibleSleep(2500);
					sendpoint_processed_old = sendpoint_processed;
				} while (!sendpoint_processed && _t_run);
			}
		} while (sendpoint_valid && _t_run);
	} while (_t_run);

	_t_end = true;
}

// Leonardo Live Info (www.livetrack24.com) data exchange thread for API V2

/** Decompress an STL string using zlib and return the original data. */
static bool gzipInflate(const char* compressedBytes, unsigned nBytes,
		std::string& uncompressedBytes) {
	if (nBytes == 0) {
		return false;
	}

	uncompressedBytes.clear();

	unsigned full_length = nBytes;
	unsigned half_length = nBytes / 2;

	unsigned uncompLength = full_length;
	char* uncomp = (char*) calloc(sizeof(char), uncompLength);

	z_stream strm;
	strm.next_in = (Bytef *) compressedBytes;
	strm.avail_in = nBytes;
	strm.total_out = 0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;

	bool done = false;

	if (inflateInit2(&strm, (16+MAX_WBITS)) != Z_OK) {
		free(uncomp);
		return false;
	}

	while (!done) {
		// If our output buffer is too small
		if (strm.total_out >= uncompLength) {
			// Increase size of output buffer
			char* uncomp2 = (char*) calloc(sizeof(char),
					uncompLength + half_length);
			memcpy(uncomp2, uncomp, uncompLength);
			uncompLength += half_length;
			free(uncomp);
			uncomp = uncomp2;
		}

		strm.next_out = (Bytef *) (uncomp + strm.total_out);
		strm.avail_out = uncompLength - strm.total_out;

		// Inflate another chunk.
		int err = inflate(&strm, Z_SYNC_FLUSH);
		if (err == Z_STREAM_END)
			done = true;
		else if (err != Z_OK) {
			break;
		}
	}

	if (inflateEnd(&strm) != Z_OK) {
		free(uncomp);
		return false;
	}

	if(done) {
		std::copy_n(uncomp, strm.total_out, std::back_inserter(uncompressedBytes));
	}

	free(uncomp);
	return done;
}

static std::string otpReply(std::string question) {
	char mac[SHA256_DIGEST_SIZE];

	unsigned int key_size = strlen(appSecret);
	unsigned int message_len = question.length();

	hmac_sha256((unsigned char*) appSecret, key_size,
			(unsigned char*) question.c_str(), message_len,
			(unsigned char*) mac, (unsigned) SHA256_DIGEST_SIZE);

	char mmdString[200];
	for (int i = 0; i < 16; i++) {
		sprintf(&mmdString[i * 2], "%02x", (unsigned char) mac[i]);
	}

	mmdString[16] = 0;

	return std::string(mmdString);
}

static std::string downloadJSON(std::string url) {

	typedef std::array<char, 64*1024> buffer_t;
	std::unique_ptr<buffer_t> rxcontent_ptr(new buffer_t);
	buffer_t& rxcontent(*rxcontent_ptr);

	int rxlen = DoTransactionToServer("api.livetrack24.com", 80, url.c_str(), rxcontent.data(), rxcontent.size());

	if (rxlen == -1)
		return "";



	if (url.find("gzip/1") != std::string::npos) {
		std::string response;
		if (gzipInflate(rxcontent.data(), rxlen, response)) {
			return response;
		}
	}

	return std::string(rxcontent.data(), rxlen);
}

static picojson::value callLiveTrack24(std::string subURL, bool calledSelf =
false) {
	picojson::value res;

	std::string url = "/api/v2/op/" + subURL;
	url += "/ak/" + std::string(appKey) + "/vc/" + otpReply(g_otpQuestion);
	if (calledSelf)
		url += "/di/" + g_deviceID + "/ut/" + g_ut;

	std::string reply = downloadJSON(url);

	if (reply.empty()) {
#ifdef LT_DEBUG
		StartupStore(TEXT(".LiveRadar callLiveTrack24 : Empty response from server%s"),
				NEWLINE);
#endif
		return res;
	}

	std::string err = picojson::parse(res, reply);
	if(!res.is<picojson::object>()) {
		return res;
	}

	if (res.get("qwe").is<std::string>()) {
		g_otpQuestion = res.get("qwe").get<std::string>();
	}
	if (res.get("ut").is<std::string>()) {
		g_ut = res.get("ut").get<std::string>();
	}

	if (res.get("sync").is<double>()) {
		g_sync = ((int) res.get("sync").get<double>());
	}

	if (!calledSelf) {

		if (res.get("newqwe").is<double>()
				&& res.get("newqwe").get<double>() == 1) {
			res = callLiveTrack24(subURL, true);
		}
		if (res.get("reLogin").is<double>()
				&& res.get("reLogin").get<double>() == 1) {
			res = callLiveTrack24(
					"login/username/" + std::string(_username) + "/pass/"
							+ std::string(_password), true);

			if (res.get("ut").is<std::string>()) {
				res = callLiveTrack24(subURL, true);
			}
		}
	}
	return res;
}

static bool LiveTrack24_Radar() {
#ifdef LT_DEBUG
	StartupStore(TEXT(".LiveRadar RADAR %s"), NEWLINE);
#endif

	struct tm t;
	time_t t_of_day;
	t.tm_year = GPS_INFO.Year - 1900;
	t.tm_mon = GPS_INFO.Month - 1; // Month, 0 - jan
	t.tm_mday = GPS_INFO.Day;
	t.tm_hour = GPS_INFO.Hour;
	t.tm_min = GPS_INFO.Minute;
	t.tm_sec = GPS_INFO.Second;
	t.tm_isdst = 0; // Is DST on? 1 = yes, 0 = no, -1 = unknown
	t_of_day = mkgmtime(&t);

	std::ostringstream strsCommand;
	strsCommand << "liveList";
	//strs << "/lat/" << GPS_INFO.Latitude << "/lon/" << GPS_INFO.Longitude << "/radius/30"  ;
	strsCommand << "/friends/1";
	strsCommand << "/sync/" << g_sync;
	strsCommand << "/gzip/1";


	picojson::value json = callLiveTrack24(strsCommand.str());

	if (json.is<picojson::null>()) {
#ifdef LT_DEBUG
		StartupStore(TEXT(".LiveRadar json null%s"), NEWLINE);
#endif
		return false;
	}

	std::string err = picojson::get_last_error();
	if (!err.empty()) {
#ifdef LT_DEBUG
		StartupStore(TEXT(".LiveRadar json error%s"), NEWLINE);
#endif
		return false;
	}
	if (!json.is<picojson::object>()) {
#ifdef LT_DEBUG
		StartupStore(TEXT(".LiveRadar json error not object%s"),
				NEWLINE);
#endif
		return false;
	}

	picojson::array list = json.get("userlist").get<picojson::array>();

#ifdef LT_DEBUG
	StartupStore(TEXT(". LiveRadar list.size =%d %s"), (int)list.size(),
			NEWLINE);
#endif

	for (picojson::array::iterator iter = list.begin(); iter != list.end();
			++iter) {

		double lat = (*iter).get("lat").get<double>();
		double lon = (*iter).get("lon").get<double>();
		double alt = (*iter).get("alt").get<double>();
		double sog = (*iter).get("sog").get<double>() / 3.6;
		int lastTM = (*iter).get("lastTM").get<double>();
		uint32_t userID = (*iter).get("userID").get<double>();
		//		double agl = (*iter).get("sog").get<double>();
		//		int isFlight = (*iter).get("isFlight").get<double>();
		std::string username = (*iter).get("username").get<std::string>();
		int category = (*iter).get("category").get<double>();
		int isLiveDB = (*iter).get("isLiveDB").get<double>();
		transform(username.begin(), username.end(), username.begin(),
				::toupper);

		double Distance, Bearing;
		DistanceBearing(lat, lon, GPS_INFO.Latitude, GPS_INFO.Longitude,
				&Distance, &Bearing);

		// Do not track beyond 30km
		if (Distance > 30000)
			continue;

		double delay = t_of_day - lastTM;

#ifdef LT_DEBUG
		StartupStore(
				TEXT(
						".LiveRadar USER: %d category %d isLiveDB %d  compare %d delay %.0f Distance %.0f %s"),
				userID, category, isLiveDB, username.compare(_username) == 0,
				delay, Distance, NEWLINE);
#endif

		if (delay > 300 || isLiveDB == 0
				|| (category != 1 && category != 2 && category != 4
						&& category != 8) || username.compare(_username) == 0
				|| username.compare(_pilotname) == 0)
			continue;

#ifdef LT_DEBUG
		StartupStore(
				TEXT(
						".LiveRadar USERPASSED: %d category %d isLiveDB %d  compare %d delay %.0f %s"),
				userID, category, isLiveDB, username.compare(_username) == 0,
				delay, NEWLINE);
#endif

		time_t rawtime = lastTM;
		struct tm * ptm;
        struct tm tm_temp = {};
        ptm = gmtime_r(&rawtime, &tm_temp);
		int Time_Fix = (ptm->tm_hour * 3600 + ptm->tm_min * 60 + ptm->tm_sec);
		if (Time_Fix > GPS_INFO.Time)
			Time_Fix = GPS_INFO.Time;

		if (!flarmwasinit) {
			DoStatusMessage(LKGetText(TEXT("_@M279_")), TEXT("LiveTrack24")); // FLARM DETECTED from LiveTrack24
			flarmwasinit = true;
		}

		int flarm_slot = 0;
		bool newtraffic = false;
		GPS_INFO.FLARM_Available = true;
		LastFlarmCommandTime = GPS_INFO.Time; // useless really, we dont call UpdateMonitor from SIM
		flarm_slot = FLARM_FindSlot(&GPS_INFO, userID);

		if (flarm_slot < 0)
			return true;

		if (GPS_INFO.FLARM_Traffic[flarm_slot].Status == LKT_EMPTY) {
			newtraffic = true;
		}
		// before changing timefix, see if it was an old target back locked in!
		CheckBackTarget(GPS_INFO, flarm_slot);

		if (newtraffic) {
			GPS_INFO.FLARM_Traffic[flarm_slot].RadioId = userID;
			GPS_INFO.FLARM_Traffic[flarm_slot].AlarmLevel = 0;
			GPS_INFO.FLARM_Traffic[flarm_slot].TurnRate = 0;
			auto& name = GPS_INFO.FLARM_Traffic[flarm_slot].Name;
			auto& cn = GPS_INFO.FLARM_Traffic[flarm_slot].Cn;

			GPS_INFO.FLARM_Traffic[flarm_slot].UpdateNameFlag=false; // clear flag first
			const TCHAR *fname = LookupFLARMDetails(GPS_INFO.FLARM_Traffic[flarm_slot].RadioId);
			if (fname) {
				LK_tcsncpy(name,fname,MAXFLARMNAME);
				//  Now we have the name, so lookup also for the Cn
				// This will return either real Cn or Name, again
				const TCHAR *cname = LookupFLARMCn(GPS_INFO.FLARM_Traffic[flarm_slot].RadioId);
				if (cname) {
					int cnamelen=_tcslen(cname);
					if (cnamelen<=MAXFLARMCN) {
						_tcscpy(cn, cname);
					} else {
						// else probably it is the Name again, and we create a fake Cn
						from_utf8(username.c_str(), cn);
					}
				} else {
					_tcscpy( GPS_INFO.FLARM_Traffic[flarm_slot].Cn, _T("Err"));
				}

			} else {
				// Else we NEED to set a name, otherwise it will constantly search for it over and over..
				from_utf8(username.c_str(), name);
				from_utf8(username.c_str(), cn);
			}


		}

		double Average30s = 0;
		double TrackBearing = 0;
		double deltaH = 0;
		double deltaT = 0;
		if (GPS_INFO.FLARM_Traffic[flarm_slot].Status != LKT_EMPTY) {
			deltaT = (double) Time_Fix
					- GPS_INFO.FLARM_Traffic[flarm_slot].Time_Fix;
			if (deltaT > 0) {
				DistanceBearing(GPS_INFO.FLARM_Traffic[flarm_slot].Latitude,
						GPS_INFO.FLARM_Traffic[flarm_slot].Longitude, lat, lon,
						&Distance, &Bearing);
				deltaH = alt - GPS_INFO.FLARM_Traffic[flarm_slot].Altitude;
				TrackBearing = Bearing;
				Average30s = deltaH / deltaT;
			}

		}

		GPS_INFO.FLARM_Traffic[flarm_slot].Status = LKT_REAL;
		GPS_INFO.FLARM_Traffic[flarm_slot].Time_Fix = (double) Time_Fix; //GPS_INFO.Time;
		GPS_INFO.FLARM_Traffic[flarm_slot].Latitude = lat;
		GPS_INFO.FLARM_Traffic[flarm_slot].Longitude = lon;
		GPS_INFO.FLARM_Traffic[flarm_slot].Altitude = alt;
		GPS_INFO.FLARM_Traffic[flarm_slot].Speed = sog;
		GPS_INFO.FLARM_Traffic[flarm_slot].TrackBearing = TrackBearing; // to be replaced by Livetrack24 cog
		GPS_INFO.FLARM_Traffic[flarm_slot].Average30s = Average30s;
	}

	return true;
}

static bool InterruptibleSleepRadar(int msecs) {
	int secs = msecs / 1000;
	do {
		if (1) {
			ScopeLock guard(_t_mutex);
			if (!_t_radar_run)
				return true;
		}
		Poco::Thread::sleep(1000);
	} while (secs--);
	return false;
}

static void LiveTrackRadarThread2() {
	_t_radar_end = false;
	_t_radar_run = true;

	InterruptibleSleepRadar(5000);
	do {
		if (!_t_radar_run)
			break;

		if (!LiveTrack24_Radar()) {
			InterruptibleSleepRadar(5000);
		} else {
			InterruptibleSleepRadar(DELAY);

		}
	} while (_t_radar_run);

	_t_radar_end = true;

}

// Leonardo Live Tracker (www.livetrack24.com) data exchange thread for API V2

static std::vector<unsigned char> hex_to_bytes(std::string const& hex) {
	std::vector<unsigned char> bytes;
	bytes.reserve(hex.size() / 2);
	for (std::string::size_type i = 0, i_end = hex.size(); i < i_end; i += 2) {
		unsigned byte;
		std::istringstream hex_byte(hex.substr(i, 2));
		hex_byte >> std::hex >> byte;
		bytes.push_back(static_cast<unsigned char>(byte));
	}
	return bytes;
}

static void replaceAll(std::string& str, const std::string& from,
		const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

static std::string rtrim_commas(std::string& str) {
	size_t first = 0;
	size_t last = str.find_last_not_of(',');
	return str.substr(first, (last - first + 1));
}

static std::string base64_encode(unsigned char const* bytes_to_encode,
		unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
					+ ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
					+ ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
				+ ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
				+ ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

static std::string md5(const std::string& text, bool tolower) {
	std::string lowText = text;
	if (tolower)
		transform(lowText.begin(), lowText.end(), lowText.begin(), ::tolower);
	MD5 md5;
	md5.Update((const unsigned char*) lowText.c_str(),
			(unsigned) strlen(lowText.c_str()));
	md5.Final();
	return std::string(md5.digestChars);
}

static std::string passwordToken(const std::string& plainTextPassword,
		const std::string& sessionID) {

	std::string lowerPassMD5;
	lowerPassMD5 = md5(plainTextPassword, true);
	std::string lowerPassMD5_and_sessionID = lowerPassMD5 + sessionID;
	std::string tokenString = md5(lowerPassMD5_and_sessionID, false);
	tokenString += lowerPassMD5 + std::string(appSecret);
	std::string tokenStringMd5 = md5(tokenString, false);
	std::string base64Token =
			base64_encode(
					reinterpret_cast<const unsigned char*>(&hex_to_bytes(
							tokenStringMd5)[0]), 16);
	replaceAll(base64Token, "+", "-");
	replaceAll(base64Token, "/", "_");
	replaceAll(base64Token, "=", ",");
	std::string passwordToken = rtrim_commas(base64Token);
	return passwordToken;
}

static int GetUserIDFromServer2() {
	int retval = -1;
	char txbuf[512];

	char rxcontent[32];

	std::string pwt0 = passwordToken(std::string(_password), "0");
	sprintf(txbuf, "/api/t/lt/getUserID/%s/%s/%s/0/0/%s/%s", appKey, "1.0",
			"LK8000", pwt0.c_str(), _username);

	int rxlen = DoTransactionToServer("t2.livetrack24.com", 80, txbuf, rxcontent,
			sizeof(rxcontent));
	if (rxlen > 0) {
		rxcontent[std::min<unsigned>(rxlen, std::size(rxcontent)-1)] = 0;

		std::vector<std::string> strings;

		std::istringstream f(rxcontent);
		std::string s;
		while (getline(f, s, '\n')) {
			strings.push_back(s);
		}

		if (strings.size() < 2 || strings[0] != "0;OK")
			return -1;

		retval = -1;
		sscanf(strings[1].c_str(), "%d", &retval);
	}

	return retval;
}

static bool SendEndOfTrackPacket2(unsigned int *packet_id) {
	char rxbuf[32];

	std::ostringstream stringStream;
	stringStream << "/api/t/lt/trackEnd/";
	stringStream << appKey << "/1.0/LK8000" << "/" << v2_sid << "/" << v2_userid
			<< "/" << v2_pwt << "/";
	stringStream << "0" << "/99/" << (*packet_id) + 1 << "/";
	stringStream << "0/0";

	std::string command = stringStream.str();
	int rxlen = DoTransactionToServer("t2.livetrack24.com", 80, command.c_str(), rxbuf, sizeof(rxbuf));
	if (rxlen > 0) {
		rxbuf[rxlen] = 0;

		std::vector<std::string> strings;
		std::istringstream f(rxbuf);
		std::string s;
		while (getline(f, s, '\n')) {
			strings.push_back(s);
		}
		if (strings.size() < 1 || strings[0] != "0;OK") {
			return false;
		}
	}
	return true;
}

static std::string intToGBase64(int num) {
	std::string sign;
	std::string str = "";

	if (num < 0) {
		num = -num;
		sign = "-";
	} else {
		sign = "";
	}

	do {
		str = mapGBase64Index[num & 0x3f] + str;
		num = floor(num / 64);
	} while (num);

	return sign + str;
}

static int createSID() {
	srand(MonotonicClockMS());
	int rnd = rand() % 2147483647;
	return rnd;
}

// Compress array with Delta and RLE
static std::string DeltaRLE(std::vector<int> data) {
	int rle = 0;
	std::string times;
	int dif, lastDif = -99999, last = 0;
	std::string res = "";
	std::string lastRes = "";

	for (unsigned int i = 0; i < data.size(); i++) {
		int datai = (data[i]);

		if (i == 0) {
			res += intToGBase64(datai);
		} else {

			dif = datai - last;
			if ((lastDif == dif) && (rle < 63)) {
				rle++;
				times = intToGBase64(rle);
				res = lastRes
						+ ((dif == 0) ?
								"*" + times :
								(dif > 0 ?
										"$" + times + intToGBase64(dif) :
										"_" + times + intToGBase64(-dif)));
			} else {
				lastDif = dif;
				rle = 1;
				lastRes = res;
				res += (dif == 0) ?
						"." :
						(dif > 0 ?
								":" + intToGBase64(dif) :
								"!" + intToGBase64(-dif));
			}
		}
		last = datai;
	}

	return res;
}




static bool SendGPSPointPacket2(unsigned int *packet_id) {

	char rxbuf[32];

	unsigned long _last_unix_timestamp = 0;
	std::vector<int> TimeList, LatList, LonList, AltList, SOGlist, COGlist;

	{
		ScopeLock guard(_t_mutex);

		if(_t_points.empty()) {
			return false;
		}

		// save last available point time
		//  used to remove successfully sent point a the end.
		//  we can't use point count because queue size is limited
		//  and some points can be removed by insert.
		_last_unix_timestamp = _t_points.back().unix_timestamp;

		for(const auto& point : _t_points) {
			TimeList.emplace_back(point.unix_timestamp);
			LatList.emplace_back(std::floor(point.latitude * 60000.));
			LonList.emplace_back(std::floor(point.longitude * 60000.));
			AltList.emplace_back(point.alt);
			SOGlist.emplace_back(point.ground_speed * 3.6 ) ;
			COGlist.emplace_back(point.course_over_ground);
		}
	}

	std::ostringstream stringStream;
	stringStream << "/api/d/lt/track/";
	stringStream << appKey << "/";
	stringStream << LKVERSION << "." << LKRELEASE  << "/" << DeviceID << "/";
	stringStream << v2_sid << "/";				// SID
	stringStream << v2_userid << "/";				// UserID
	stringStream << v2_pwt << "/";					// Password Token
	stringStream << "9";   					// Privacy
	stringStream << "/0/"; 					//TrackCategory
	stringStream << (*packet_id) + 1 << "/"; 	// PacketID
	stringStream << DeltaRLE(TimeList) << "/";
	stringStream << DeltaRLE(LatList) << "/";
	stringStream << DeltaRLE(LonList) << "/";
	stringStream << DeltaRLE(AltList) << "/";
	stringStream << DeltaRLE(SOGlist) << "/";
	stringStream << DeltaRLE(COGlist) << "/";
	stringStream << "LK8000";  // TrackInfo

	const std::string command = stringStream.str();
	int rxlen = DoTransactionToServer("t2.livetrack24.com", 80, command.c_str(), rxbuf, sizeof(rxbuf));
	if (rxlen > 0) {
		rxbuf[std::min<unsigned>(rxlen, std::size(rxbuf)-1)] = 0;

		std::vector<std::string> strings;

		std::istringstream f(rxbuf);
		std::string s;
		while (getline(f, s, '\n')) {
			strings.push_back(s);
		}

		if (strings.size() < 1 || strings[0] != "0;OK") {
			return false;
		}

		{
			ScopeLock guard(_t_mutex);
			// all points older than "_last_unix_timestamp" was succesfully sent.
			//   -> remove them from queue.
			while(!_t_points.empty() && _t_points.front().unix_timestamp <= _last_unix_timestamp) {
				_t_points.pop_front();
			}
		}
		(*packet_id)++;
#ifdef LT_DEBUG
		StartupStore(TEXT(".Livetrack24 TRACKER sent %d points %s"), nPoints,
				NEWLINE);
#endif

	}
	return true;

}

static void LiveTrackerThread2() {

	int tracker_fsm = 0;
	livetracker_point_t sendpoint = {};
	bool sendpoint_processed = false;
	bool packet_processed = false;

	bool sendpoint_valid = false;
	// Session variables
	unsigned int packet_id = 0;

	_t_end = false;
	_t_run = true;

	do {
		if (NewDataEvent.tryWait(5000))
			NewDataEvent.reset();
		if (!_t_run)
			break;
		sendpoint_processed = false;

		if (1) {
			sendpoint_valid = false;
			packet_processed = false;
			ScopeLock guard(_t_mutex);
			if (!_t_points.empty()) {
				sendpoint = _t_points.front();
				sendpoint_valid = true;
			}
		} //mutex

		if (sendpoint_valid) {
#ifdef LT_DEBUG
			StartupStore(TEXT(". Livetracker TRACKER sendpoint.flying: %d - tracker_fsm: %d %s"), sendpoint.flying,tracker_fsm,NEWLINE);
#endif

			switch (tracker_fsm) {
			default:
			case 0:   // Wait for flying
				if (!sendpoint.flying) {
					ScopeLock guard(_t_mutex);
					_t_points.pop_front();
					sendpoint_processed = true;
					break;
				}
				tracker_fsm++;
				break;
			case 1:
				// Get User ID
				v2_userid = GetUserIDFromServer2();
				sendpoint_processed = false;
				if (v2_userid >= 0)
					tracker_fsm++;
				break;
			case 2:
				//Start of track packet
				sendpoint_processed = false;
				tracker_fsm++;
				break;
			case 3:
				//Gps point packet
				packet_processed = SendGPSPointPacket2(&packet_id);
				if (!sendpoint.flying) {
					tracker_fsm++;
				}
				break;
			case 4:
				//End of track packet
				sendpoint_processed = SendEndOfTrackPacket2(&packet_id);
				StartupStore(TEXT(". Livetracker TRACKER  SendEndOfTrackPacket2 .%d ...%s"), sendpoint_processed, NEWLINE);

				if (sendpoint_processed) {
					tracker_fsm = 0;
				}
				break;
			}   // sw

		}
		if (packet_processed) {
			InterruptibleSleep(DELAY);

		}
	} while (_t_run);

	_t_end = true;
}
