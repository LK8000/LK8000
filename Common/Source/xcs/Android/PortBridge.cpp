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

#include "PortBridge.hpp"
#include "NativePortListener.hpp"
#include "NativeInputListener.hpp"
#include "Java/Class.hxx"

#include <string.h>

jmethodID PortBridge::close_method;
jmethodID PortBridge::setListener_method;
jmethodID PortBridge::setInputListener_method;
jmethodID PortBridge::getState_method;
jmethodID PortBridge::drain_method;
jmethodID PortBridge::getBaudRate_method;
jmethodID PortBridge::setBaudRate_method;
jmethodID PortBridge::write_method;

void
PortBridge::Initialise(JNIEnv *env)
{
  Java::Class cls(env, "org/LK8000/AndroidPort");

  close_method = env->GetMethodID(cls, "close", "()V");
  setListener_method = env->GetMethodID(cls, "setListener",
                                        "(Lorg/LK8000/PortListener;)V");
  setInputListener_method = env->GetMethodID(cls, "setInputListener",
                                             "(Lorg/LK8000/InputListener;)V");
  getState_method = env->GetMethodID(cls, "getState", "()I");
  drain_method = env->GetMethodID(cls, "drain", "()Z");
  getBaudRate_method = env->GetMethodID(cls, "getBaudRate", "()I");
  setBaudRate_method = env->GetMethodID(cls, "setBaudRate", "(I)Z");
  write_method = env->GetMethodID(cls, "write", "([BI)I");
}

PortBridge::PortBridge(const Java::LocalObject& obj)
  : Java::GlobalObject(obj)
  , write_buffer(obj.GetEnv(), obj.GetEnv()->NewByteArray(write_buffer_size)) { }


PortBridge::~PortBridge() {
  JNIEnv *env = Java::GetEnv();
  close(env);
  setInputListener(env, nullptr);
  setListener(env, nullptr);
}

void
PortBridge::setListener(JNIEnv *env, PortListener *_listener)
{
  if (_listener) {
    Java::LocalObject listener = NativePortListener::Create(env, *_listener);
    env->CallVoidMethod(Get(), setListener_method, listener.Get());
  } else {
    env->CallVoidMethod(Get(), setListener_method, nullptr);
  }
}

void
PortBridge::setInputListener(JNIEnv *env, DataHandler *handler)
{
  if (handler) {
    Java::LocalObject listener = NativeInputListener::Create(env, *handler);
    env->CallVoidMethod(Get(), setInputListener_method, listener.Get());
  } else {
    env->CallVoidMethod(Get(), setInputListener_method, nullptr);
  }
}

int
PortBridge::write(JNIEnv *env, const void *data, size_t length)
{
  if (length > write_buffer_size)
    length = write_buffer_size;

  jbyte *dest = env->GetByteArrayElements(write_buffer, nullptr);
  memcpy(dest, data, length);
  env->ReleaseByteArrayElements(write_buffer, dest, 0);

  return env->CallIntMethod(Get(), write_method, write_buffer.Get(), length);
}
