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

#include "SmsUtil.hpp"
#include "Java/Class.hxx"
#include "Java/String.hxx"

namespace SmsUtil {
  static Java::TrivialClass cls;
  static jmethodID sms_method;
}

void
SmsUtil::Initialise(JNIEnv *env)
{
  assert(!cls.IsDefined());
  assert(env != nullptr);

  cls.Find(env, "org/LK8000/SmsUtil");
  sms_method = env->GetStaticMethodID(cls, "sendSMS",
                                       "(Ljava/lang/String;"
                                       "Ljava/lang/String;)Z");
}

void
SmsUtil::Deinitialise(JNIEnv *env)
{
  cls.Clear(env);
}

bool
SmsUtil::SendSms(JNIEnv *env, const char *phoneNumber, const char *message)
{
  Java::String paramPhoneNumber(env, phoneNumber);
  Java::String paramMessage(env, message);
  bool ret = env->CallStaticBooleanMethod(cls, sms_method,paramPhoneNumber.Get(),paramMessage.Get());
  return ret;
}