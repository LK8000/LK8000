/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   http_session.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2024
 */
#ifndef _TRACKING_DEFAULT_HTTP_SESSION_H_
#define _TRACKING_DEFAULT_HTTP_SESSION_H_

#include <string>
#include "../http_session_base.h"

class http_session : public http_session_base<http_session> {
  friend class http_session_base<http_session>;

public:
  http_session();
  ~http_session();

private:
  static bool ssl_available_impl();

  std::string request_impl(const std::string& url,
                          const std::string* post_data,
                          const char* content_type) const;
};

#endif // _TRACKING_DEFAULT_HTTP_SESSION_H_
