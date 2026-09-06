/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   http_session.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 18, 2026
 */
#ifndef TRACKING_WINHTTP_HTTP_SESSION_H_
#define TRACKING_WINHTTP_HTTP_SESSION_H_

#include "../http_session_base.h"
#include "winhttp_ptr.h"

class http_session : public http_session_base<http_session> {
  friend class http_session_base<http_session>;

  using base = http_session_base<http_session>;
  using optional_string = base::optional_string;

 public:
  http_session() = default;

 private:
  static bool ssl_available_impl();

  std::string request_impl(const std::string& url,
                           const optional_string& post_data,
                           const optional_string& content_type) const;

  winhttp_session_ptr session;
};

#endif  // TRACKING_WINHTTP_HTTP_SESSION_H_
