/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   http_session.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2024
 */
#include "http_session.h"
#include "../../MessageLog.h"
#include <regex>
#include <utility>

#ifdef WIN32
    #include <winsock2.h>
#endif

#ifdef __linux__
	#include <stdio.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <sys/ioctl.h>

namespace {

    using SOCKET = int;
    constexpr SOCKET SOCKET_ERROR = -1;
    constexpr SOCKET INVALID_SOCKET = -1;

    int closesocket(int fd) {
        return close(fd);
    }

    template<typename... Args>
    int ioctlsocket(int fd, int request, Args... args) {
        return ioctl(fd, request, args...);
    }

} // namespace

#endif


namespace {

class ScopeSocket {
public:
    ScopeSocket() = default;
    ScopeSocket(const ScopeSocket&) = delete;
    ScopeSocket(ScopeSocket&& src) noexcept {
        s = std::exchange(src.s, INVALID_SOCKET);
    }

    ScopeSocket(int af,int type,int protocol) { 
        s = socket( af, type, protocol);
    }

    ~ScopeSocket() {
        if (s != INVALID_SOCKET) {
            closesocket(s);
        }
    }

    bool operator!() const {
        return s == INVALID_SOCKET;
    }

    SOCKET get() const {
        return s;
    }

private:
    SOCKET s = INVALID_SOCKET;
};

// Establish a connection with the data server
// Returns a valid SOCKET if ok, INVALID_SOCKET if failed 
ScopeSocket EstablishConnection(const char *servername, int serverport) {
	struct hostent *server = gethostbyname(servername);
	if (!server)
		return {};

	ScopeSocket s(AF_INET, SOCK_STREAM, 0);
	if (!s) {
		return {};
    }

	struct sockaddr_in sin = {};
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ((struct in_addr *) (server->h_addr))->s_addr;
	sin.sin_port = htons(serverport);
	if (connect(s.get(), (sockaddr*) &sin, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		//could not connect to server
		return {};
	}

	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled;
	// If iMode != 0, non-blocking mode is enabled.

	u_long iMode = 1;
	int iResult = ioctlsocket(s.get(), FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR) {
		StartupStore(_T(".... ioctlsocket failed with error: %d"), iResult);
		// if failed, socket still in blocking mode, it's big problem
	}

	return s;
}

int ReadData(SOCKET s, void *szString, size_t size) {

	struct timeval timeout = { 10, 0 };

	fd_set readfs;
	FD_ZERO(&readfs);
	FD_SET(s, &readfs);

	// wait for received data
	int iResult = select(s + 1, &readfs, nullptr, nullptr, &timeout);
	if (iResult == 0) {
		return SOCKET_ERROR; // timeout
	}

	if ((iResult != SOCKET_ERROR) && FD_ISSET(s, &readfs)) {
		// Data ready to read
		iResult = recv(s, static_cast<char*>(szString), size, 0);
	}

	return iResult;
}

template<size_t size>
int ReadData(SOCKET s, char (&buffer)[size]) {
    return ReadData(s, buffer, size);
}

int _WriteData(SOCKET s, const void *data, size_t length) {

	struct timeval timeout = { 10, 0 };

	fd_set writefs;
	FD_ZERO(&writefs);
	FD_SET(s, &writefs);

	// wait for socket ready to write
	int iResult = select(s + 1, nullptr, &writefs, nullptr, &timeout);
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

		iResult = send(s, static_cast<const char*>(data), length, flags);
	}

	return iResult;
}

int WriteData(SOCKET s, const char *data, size_t length) {
  size_t sent=0;
  while(sent < length) {
    int tmpres = _WriteData(s, data + sent, length - sent);
    if( tmpres == SOCKET_ERROR ) {
      sent = -1;
    }
    sent += tmpres;
  }
  return sent;
}

int WriteData(SOCKET s, const std::string_view& string) {
    return WriteData(s, string.data(), string.size());
}

} // namespace

bool http_session::ssl_available() {
	return false;
}

http_session::http_session() {
#ifdef WIN32
	WSADATA wsaData;

	WORD version = MAKEWORD( 1, 1 );
	int error = WSAStartup( version, &wsaData );
	if ( error != 0 ) {
		throw std::runtime_error("failed to init winsock");
	}

	/* check for correct version */
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
			HIBYTE( wsaData.wVersion ) != 1 )
	{
		/* incorrect WinSock version */
		WSACleanup();
		throw std::runtime_error(" incorrect WinSock version");
	}
#endif
}

http_session::~http_session() {
#ifdef WIN32
    WSACleanup();
#endif
}

std::string http_session::request(const char* server_name, int server_port, const char* query_string) const {

	ScopeSocket s = EstablishConnection(server_name, server_port);
	if (!s) {
		return {};
    }

	//Send the query to the server
	int tmpres = WriteData(s.get(), "GET ");
	if (tmpres < 0) {
		return {};
	}
	tmpres = WriteData(s.get(), query_string);
	if (tmpres < 0) {
		return {};
	}
	tmpres = WriteData(s.get(), " HTTP/1.0\r\nHost: ");
	if (tmpres < 0) {
		return {};
	}
	tmpres = WriteData(s.get(), server_name);
	if (tmpres < 0) {
		return {};
	}
	tmpres = WriteData(s.get(), "\r\n\r\n");
	if (tmpres < 0) {
		return {};
	}

	std::string response;

	int rxfsm = 0;
	char recvbuf[BUFSIZ];

	//Receive the page
	while ((tmpres = ReadData(s.get(), recvbuf)) > 0) {
		for (int i = 0; i < tmpres; i++) {
			char cdata = recvbuf[i];
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
					break;
				}
				rxfsm = 0;
				break;
			case 4:
				//Content chr
                response += cdata;
				break;
			} //sw
		} //for
	} //wh

	return response;
}

std::string http_session::request(const std::string& url) const {
    const static std::regex re(R"(^(.*:)//([A-Za-z0-9\-\.]+)(:[0-9]+)?(.*)$)");
    std::smatch match;
    if (std::regex_match(url, match, re)) {

        const std::string host = match[2];
        const std::string port = match[3];
        const std::string rest = match[4];

        return http_session::request(host.c_str(), port.empty() ? 80 : strtoul(port.c_str(), nullptr, 10), rest.c_str());
    }
    return {};
}
