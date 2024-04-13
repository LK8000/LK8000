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
#ifndef _TRACKING_HTTP_SESSION_H_
#define _TRACKING_HTTP_SESSION_H_

#ifdef USE_CURL
  #include "Curl/http_session.h"
#else 
  #include "Default/http_session.h"
#endif

#endif // _TRACKING_HTTP_SESSION_H_
