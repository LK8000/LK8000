/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   http_session.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 18, 2026
 */
#include <vector>
#include <sstream>
#include <format>
#include "http_session.h"
#include "Defines.h"
#include "utils/charset_helper.h"
#include "MessageLog.h"

namespace {

// Parse URL into components
struct url_parts {
  std::wstring scheme;
  std::wstring host;
  int port = 0;
  std::wstring path;
  bool is_https = false;
};

bool parse_url(const std::string& url, url_parts& parts) {
  std::wstring wurl = utf8_to_string<wchar_t>(url.c_str());

  URL_COMPONENTS urlComp = {};
  urlComp.dwStructSize = sizeof(urlComp);

  // Set required component lengths
  urlComp.dwSchemeLength = ~0U;
  urlComp.dwHostNameLength = ~0U;
  urlComp.dwUrlPathLength = ~0U;
  urlComp.dwExtraInfoLength = ~0U;

  if (!WinHttpCrackUrl(wurl.c_str(), static_cast<DWORD>(wurl.length()), 0,
                       &urlComp)) {
    return false;
  }

  parts.scheme = std::wstring(urlComp.lpszScheme, urlComp.dwSchemeLength);
  parts.host = std::wstring(urlComp.lpszHostName, urlComp.dwHostNameLength);
  parts.port = urlComp.nPort;
  parts.is_https = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

  // Construct full path with query string
  if (urlComp.lpszUrlPath) {
    parts.path = std::wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
  }
  if (urlComp.lpszExtraInfo) {
    parts.path +=
        std::wstring(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
  }

  if (parts.path.empty()) {
    parts.path = L"/";
  }

  return true;
}

}  // namespace

bool http_session::ssl_available() {
  // WinHTTP always supports SSL/TLS on Windows
  return true;
}

std::string http_session::request_impl(const std::string& url,
                                       const std::string* post_data,
                                       const char* content_type) const {
  try {
    if (!session) {
      return {};
    }

    // Parse URL
    url_parts parts;
    if (!parse_url(url, parts)) {
      return {};
    }

    // Connect to server
    winhttp_handle_ptr hConnect(
        WinHttpConnect(session.get(), parts.host.c_str(), parts.port, 0));

    if (!hConnect) {
      return {};
    }

    // Open request
    const wchar_t* method = post_data ? L"POST" : L"GET";
    DWORD flags = parts.is_https ? WINHTTP_FLAG_SECURE : 0;

    winhttp_handle_ptr hRequest(WinHttpOpenRequest(
        hConnect.get(), method, parts.path.c_str(), NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags));

    if (!hRequest) {
      return {};
    }

    // Set timeouts (in milliseconds):
    DWORD resolveTimeout = 30000;
    DWORD connectTimeout = 30000;
    DWORD sendTimeout = 30000;
    DWORD receiveTimeout = 60000;
    WinHttpSetTimeouts(hRequest.get(), resolveTimeout, connectTimeout,
                       sendTimeout, receiveTimeout);

    // Enable automatic redirect following (max 5 redirects like curl)
    DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_REDIRECT_POLICY,
                     &redirectPolicy, sizeof(redirectPolicy));

    DWORD maxRedirects = 5;
    WinHttpSetOption(hRequest.get(),
                     WINHTTP_OPTION_MAX_HTTP_AUTOMATIC_REDIRECTS, &maxRedirects,
                     sizeof(maxRedirects));

    // Prepare headers
    std::wstring headers;
    if (post_data && content_type) {
      headers = std::format(L"Content-Type: {}\r\n",
                            utf8_to_string<wchar_t>(content_type));
    }

    // Send request
    BOOL result = FALSE;
    if (post_data) {
      result = WinHttpSendRequest(
          hRequest.get(),
          headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
          static_cast<DWORD>(headers.length()),
          const_cast<char*>(post_data->c_str()),
          static_cast<DWORD>(post_data->size()),
          static_cast<DWORD>(post_data->size()), 0);
    }
    else {
      result = WinHttpSendRequest(hRequest.get(), WINHTTP_NO_ADDITIONAL_HEADERS,
                                  0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }

    if (!result) {
      return {};
    }

    // Receive response
    if (!WinHttpReceiveResponse(hRequest.get(), NULL)) {
      return {};
    }

    // Check status code
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest.get(),
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode,
                        &statusCodeSize, WINHTTP_NO_HEADER_INDEX);

    // Read response data
    std::string response;
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;
    std::vector<char> buffer(4096);
    constexpr size_t maxResponseSize = 10 * 1024 * 1024;  // 10 MB limit

    do {
      bytesAvailable = 0;
      if (!WinHttpQueryDataAvailable(hRequest.get(), &bytesAvailable)) {
        break;
      }

      if (bytesAvailable == 0) {
        break;
      }

      // Check response size limit
      if (response.size() + bytesAvailable > maxResponseSize) {
        DebugLog(_T("HTTP response too large, aborting\n"));
        return {};
      }

      // Resize buffer if needed
      if (bytesAvailable > buffer.size()) {
        buffer.resize(bytesAvailable);
      }

      bytesRead = 0;
      if (!WinHttpReadData(hRequest.get(), buffer.data(), bytesAvailable,
                           &bytesRead)) {
        break;
      }

      if (bytesRead > 0) {
        response.append(buffer.data(), bytesRead);
      }

    } while (bytesAvailable > 0);

    return response;
  }
  catch (std::exception& e) {
    DebugLog(_T("http_session : %s\n"), to_tstring(e.what()).c_str());
  }
  return {};
}

std::string http_session::get(const std::string& url) const {
  return request_impl(url, nullptr, nullptr);
}

std::string http_session::post(const std::string& url, const std::string& data,
                               const char* content_type) const {
  return request_impl(url, &data, content_type);
}

std::string http_session::request(const char* server_name, int server_port,
                                  const char* query_string) const {
  if (!server_name || !query_string) {
    return {};
  }

  std::string url = (server_port == 443) ? "https://" : "http://";
  url += server_name;
  url += ":";
  url += std::to_string(server_port);
  if (query_string[0] != '/') {
    url += '/';
  }  
  url += query_string;
  return get(url);
}
