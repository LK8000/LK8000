/*
 * Copyright (C) 2012-2015 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "StaticSocketAddress.hxx"

#include <algorithm>

#include <assert.h>
#include <string.h>

#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

StaticSocketAddress::StaticSocketAddress() {
#ifdef WIN32
	WSADATA data;
	WSAStartup(MAKEWORD(2,2), &data);
#endif
}

StaticSocketAddress::~StaticSocketAddress() {
#ifdef WIN32
	WSACleanup();
#endif
}

StaticSocketAddress &
StaticSocketAddress::operator=(SocketAddress other)
{
	size = std::min(other.GetSize(), GetCapacity());
	memcpy(&address, other.GetAddress(), size);
	return *this;
}


bool
StaticSocketAddress::Lookup(const char *host, const char *service, int socktype)
{
#if defined(_WIN32_WCE) && _WIN32_WCE < 0x0500

	struct hostent* h = gethostbyname(host);
	if(!h) {
		return false;
	}

	static_assert(sizeof(sockaddr) == sizeof(sockaddr_in), "invalid struct size");
	struct sockaddr sockaddr_to;
	struct sockaddr_in *to = (struct sockaddr_in *)&sockaddr_to;
	to->sin_family = h->h_addrtype;
	to->sin_port = htons(strtol(service, nullptr, 10));
	memcpy(&(to->sin_addr.s_addr), h->h_addr, h->h_length);

	(*this) = SocketAddress(&sockaddr_to, sizeof(sockaddr));

#else

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = socktype;

	struct addrinfo *ai;
	if (getaddrinfo(host, service, &hints, &ai) != 0) {
		return false;
	}

	size = ai->ai_addrlen;
	assert(size_t(size) <= sizeof(address));

	memcpy(reinterpret_cast<void *>(&address),
	       reinterpret_cast<void *>(ai->ai_addr), size);
	freeaddrinfo(ai);
#endif

	return true;
}

