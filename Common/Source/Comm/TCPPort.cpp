/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   TCPPort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on February 22, 2016, 10:27 PM
 */
#include "externs.h"
#include "TCPPort.h"
#include "utils/stringext.h"
#include <functional>
using namespace std::placeholders;

#ifdef __linux__
// Optionally disable the tcp/ip optimization, to reduce latencies
// This has effect only on write operations.
// Only for Linux based systems
// #define DISABLE_NAGLE
#ifdef DISABLE_NAGLE
#include <netinet/tcp.h>
#endif
#endif

#ifdef __linux__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#endif

static LPCTSTR GetAddress(unsigned idx) {
 if(idx < NUMDEV) {
   return szIpAddress[idx];
 }
 return _T("");
}

static uint16_t GetPort(unsigned idx) {
 if(idx < NUMDEV) {
   return dwIpPort[idx];
 }
 return 0U;
}




int hostname_to_ip(const char * hostname , char* ip)
{
struct hostent *he;
struct in_addr **addr_list;
int i;

if ( (he = gethostbyname( hostname ) ) == NULL)
{
// get the host info
	StartupStore(_T("... gethostbyname failed") );
//herror("gethostbyname");
return 1;
}

addr_list = (struct in_addr **) he->h_addr_list;

for(i = 0; addr_list[i] != NULL; i++)
{
//Return the first one;
strcpy(ip , inet_ntoa(*addr_list[i]) );
return 0;
}

return 1;
}

bool TCPClientPort::Connect() {
    
    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket == INVALID_SOCKET) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPClientPort Port %u Unable to create socket, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, dwError); // 091117

        return false;
    }

#ifdef __linux__
#ifdef DISABLE_NAGLE
  int nodelay_flag = 1;
  setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (void*) &nodelay_flag, sizeof(int));
#endif
#endif

#ifdef UNICODE
    char szaddr[MAX_URL_LEN];
    TCHAR2ascii(GetAddress(GetPortIndex()), szaddr, MAX_URL_LEN);
#else
    const char* szaddr = GetAddress(GetPortIndex());
#endif
    char IPadr[16];
    SOCKADDR_IN sin = { 0 };
    if(hostname_to_ip(szaddr,IPadr ) == 0)  // convert URL to IP address
      sin.sin_addr.s_addr = inet_addr(IPadr);
    else
      sin.sin_addr.s_addr = inet_addr(szaddr);
    sin.sin_port = htons(GetPort(GetPortIndex()));
    sin.sin_family = AF_INET;

#ifdef __linux__
    int arg = fcntl(mSocket, F_GETFL, NULL);
    arg |= O_NONBLOCK;
    fcntl(mSocket, F_SETFL, arg);
#endif

    if(connect(mSocket, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR) {

#ifdef __linux__
        // Wait for Connection succesfull
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(mSocket, &fdset);
        struct timeval tv = {10, 0}; /* 10 second timeout */
        int res = select(mSocket + 1, NULL, &fdset, NULL, &tv);
        if(res > 0) {
            int so_error;
            socklen_t len = sizeof so_error;
            res = getsockopt(mSocket, SOL_SOCKET, SO_ERROR, &so_error, &len);

            if (so_error != 0) {
                StartupStore(_T("... TCPClientPort Port %u <%s> connect Failed, SO_ERROR=%d") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), so_error);
                return false;
            }

        } else if(res == 0) {
            StartupStore(_T("... TCPClientPort Port %u <%s> connect Failed, TIMEOUT") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName()); // 091117
            return false;
        } else {
            StartupStore(_T("... TCPClientPort Port %u <%s> connect Failed, errno=%d") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), errno);
            return false;
        }

#else
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPClientPort Port %u <%s> connect Failed, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), dwError); // 091117
        return false;
#endif
    }

#ifdef __linux__
    arg = fcntl(mSocket, F_GETFL, NULL);
    arg &= (~O_NONBLOCK);;
    fcntl(mSocket, F_SETFL, arg);
#endif


    StartupStore(_T(". TCPClientPort %u Connect <%s> OK") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName());

    return true;
}

bool TCPServerPort::Close() {
    if(!SocketPort::Close()) {
        return false;
    }
    closesocket(mServerSocket);
    mServerSocket = INVALID_SOCKET;
        
    return true;
}

bool TCPServerPort::Connect() {

    mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mServerSocket == INVALID_SOCKET) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPServerPort Port %u Unable to create socket, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, dwError); // 091117

        return false;
    }

    SOCKADDR_IN sin = { 0 };
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(GetPort(GetPortIndex()));
    sin.sin_family = AF_INET;
    
    int reuse = 1;
    if (setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int)) == SOCKET_ERROR) {
        
    }
    
    if(bind (mServerSocket, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPServerPort %u <%s> Bind Failed, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), dwError); // 091117

        return false;
    }
    
    if(listen(mServerSocket, 1) == SOCKET_ERROR) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPServerPort %u <%s> Listen Failed, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), dwError); // 091117

        return false;
    }

    StartupStore(_T(". TCPServerPort %u Connect <%s> OK") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName());

    return true;
}

unsigned TCPServerPort::RxThread() {
    struct timeval timeout;
    timeout.tv_sec = (mTimeout) / 1000;
    timeout.tv_usec = (mTimeout)  % 1000;
    
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.

    u_long iMode = 1;
    int iResult = ioctlsocket(mServerSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR) {
        StartupStore(_T(".... ioctlsocket failed with error: %d%s"), iResult, NEWLINE);
        // if failed, socket still in blocking mode, it's big problem
    }  
    
    while (mServerSocket != INVALID_SOCKET && !StopEvt.tryWait(5)) {

        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(mServerSocket, &readfs);
        // wait for received data
        int iResult = select(mServerSocket + 1, &readfs, NULL, NULL, &timeout);
        if(iResult > 0 && FD_ISSET(mServerSocket, &readfs)) {
            
            /* new client */
            mSocket = accept(mServerSocket, nullptr, nullptr);
            if(mSocket != INVALID_SOCKET) {
                SocketPort::RxThread();
            }
            if(mSocket != INVALID_SOCKET) {
                closesocket(mSocket);
                mSocket = INVALID_SOCKET;
            }
        }
    }
    return 0U;
}


//UDP  ToninoTarsi 2016

bool UDPServerPort::Connect() {

	mSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (mSocket == INVALID_SOCKET) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... UDPServerPort Port %u Unable to create socket, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, dwError); // 091117

        return false;
    }

    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.

    u_long iMode = 1;
    int iResult = ioctlsocket(mSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR) {
        StartupStore(_T(".... ioctlsocket failed with error: %d%s"), iResult, NEWLINE);
        // if failed, socket still in blocking mode, it's big problem
    }

    SOCKADDR_IN sin = { 0 };
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(GetPort(GetPortIndex()));
    sin.sin_family = AF_INET;

    //set timer for recv_socket
    struct timeval tv;
    tv.tv_usec = mTimeout;
    setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO,(char*)&tv,sizeof(tv));

    if(bind (mSocket, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... UDPServerPort %u <%s> Bind Failed, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), dwError); // 091117

        return false;
    }
    StartupStore(_T(". UDPServerPort %u Connect <%s> OK") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName());
    return true;
}

unsigned UDPServerPort::RxThread() {

	while (mSocket != INVALID_SOCKET && !StopEvt.tryWait(5)) {
		int nRecv;
		socklen_t slen = sizeof(mSAddressClient);
		_Buff_t szString;
		if ((nRecv = recvfrom(mSocket, szString, sizeof(szString), 0, (struct sockaddr *) &mSAddressClient, &slen)) != -1)  {
			ScopeLock Lock(CritSec_Comm);
			UpdateStatus();
			if (nRecv > 0) {
				std::for_each(std::begin(szString), std::next(szString, nRecv), std::bind(&UDPServerPort::ProcessChar, this, _1));
			}
		}
	}
	return 0U;
}

bool UDPServerPort::Write(const void *data, size_t size) {

	if (mSocket == INVALID_SOCKET) {
	        unsigned dwError = WSAGetLastError();
	        StartupStore(_T("... UDPServerPort Port socket invalid. Errno : %d"),dwError);
	        return false;
	}
    // if no client sended data yet we try to send to the default Android AP ( 192.168.43.1 ) on port 8000 for LKNET
    if ( mSAddressClient.sin_port == 0 ) {
        SOCKADDR_IN sDefault = { 0 };
        sDefault.sin_addr.s_addr = inet_addr("192.168.43.1");
        sDefault.sin_port = htons(8000);
        sDefault.sin_family = AF_INET;
        return sendto(mSocket, (const char *)data, size, 0, (sockaddr*)&sDefault, sizeof(sDefault));
    }
    else
    	return sendto(mSocket, (const char *)data, size, 0, (sockaddr*)&mSAddressClient, sizeof(mSAddressClient));
}


