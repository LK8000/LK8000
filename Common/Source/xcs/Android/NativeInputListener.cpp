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

#include "NativeInputListener.hpp"
#include "IO/DataHandler.hpp"
#include "Java/Class.hxx"
#include "Java/Env.hxx"
#include "Java/Array.hxx"
#include "Java/UUID.h"

#include <stddef.h>

namespace {
  Java::TrivialClass cls;
  jmethodID ctor;
  jfieldID ptr_field;
};

extern "C"
JNIEXPORT void JNICALL
Java_org_LK8000_NativeInputListener_dataReceived(JNIEnv *env, jobject obj,
                                                 jbyteArray data, jint length)
{
  jlong ptr = env->GetLongField(obj, ptr_field);
  if (ptr == 0)
    /* not yet set */
    return;

  auto& handler = *reinterpret_cast<DataHandler*>(ptr);

  Java::ByteArrayElements array(env, data);
  handler.DataReceived(array, length);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_LK8000_NativeInputListener_onCharacteristicChanged(JNIEnv* env, jobject obj,
                                                                 jlong serviceMsb, jlong serviceLsb,
                                                                 jlong characteristicMsb, jlong characteristicLsb,
                                                                 jbyteArray data, jint length) {

  if (length > 0) {
    jlong ptr = env->GetLongField(obj, ptr_field);
    if (ptr == 0) {
      /* not yet set */
      return;
    }

    auto& handler = *reinterpret_cast<DataHandler*>(ptr);

    Java::ByteArrayElements array(env, data);
    handler.OnCharacteristicChanged( uuid_t(serviceMsb, serviceLsb), uuid_t(characteristicMsb, characteristicLsb), array, length);
  }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_LK8000_NativeInputListener_doEnableNotification(JNIEnv* env, jobject obj,
                                                              jobject service, jobject characteristic) {

  using namespace Java::UUID;

  jlong ptr = env->GetLongField(obj, ptr_field);
  if (ptr == 0) {
    /* not yet set */
    return false;
  }

  auto& handler = *reinterpret_cast<DataHandler*>(ptr);
  return handler.DoEnableNotification(to_uuid_t(env, service), to_uuid_t(env, characteristic));
}

void
NativeInputListener::Initialise(JNIEnv *env)
{
  cls.Find(env, "org/LK8000/NativeInputListener");

  ctor = env->GetMethodID(cls, "<init>", "(J)V");
  ptr_field = env->GetFieldID(cls, "ptr", "J");
}

void
NativeInputListener::Deinitialise(JNIEnv *env)
{
  cls.Clear(env);
}

Java::LocalObject
NativeInputListener::Create(JNIEnv *env, DataHandler &handler)
{
  assert(cls != nullptr);

  return NewObjectRethrow(env, cls, ctor, (jlong)&handler);
}
