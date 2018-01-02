/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sms.h
 * Author: Shachar Liberman
 *
 * Created on 09.09.2017
 */

#ifndef SMS_UTIL_HPP
#define SMS_UTIL_HPP

#include <jni.h>

namespace SmsUtil {
  void Initialise(JNIEnv *env);
  void Deinitialise(JNIEnv *env);

  bool SendSms(JNIEnv *env, const char *phoneNumber, const char *message);
};

#endif


