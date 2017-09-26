/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
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