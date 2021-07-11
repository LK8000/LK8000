/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   UsbSerialHelper.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on April 04, 2017
 */

#include "UsbSerialHelper.h"
#include "Java/Class.hxx"
#include "Java/String.hxx"

namespace UsbSerialHelper {
  static Java::TrivialClass cls;
  static jmethodID isEnabled_method;
  static jmethodID list_method;
  static jmethodID connect_method;
}


bool
UsbSerialHelper::Initialise(JNIEnv *env) {
  assert(!cls.IsDefined());
  assert(env != nullptr);

  if (!cls.FindOptional(env, "org/LK8000/UsbSerialHelper")) {
    /* Android < 3.1 doesn't have Usb Host support */
    return false;
  }
  isEnabled_method = env->GetStaticMethodID(cls, "isEnabled", "()Z");
  list_method = env->GetStaticMethodID(cls, "list", "()[Ljava/lang/String;");
  connect_method = env->GetStaticMethodID(cls, "connect",
                                          "(Ljava/lang/String;I)Lorg/LK8000/AndroidPort;");

  return true;
}

void
UsbSerialHelper::Deinitialise(JNIEnv *env) {
  cls.ClearOptional(env);
}

bool
UsbSerialHelper::isEnabled(JNIEnv *env) {
  return cls.IsDefined() &&
         env->CallStaticBooleanMethod(cls, isEnabled_method);
}

PortBridge* UsbSerialHelper::connectDevice(JNIEnv *env, const char *name, unsigned baud) {

  if(!cls.IsDefined()) {
    return nullptr;
  }

  Java::String name2(env, name);
  jobject obj = env->CallStaticObjectMethod(cls, connect_method, name2.Get(), (int)baud);
  Java::RethrowException(env);
  if (obj == nullptr)
    return nullptr;

  PortBridge *bridge = new PortBridge(env, obj);
  env->DeleteLocalRef(obj);
  return bridge;
}

jobjectArray
UsbSerialHelper::list(JNIEnv *env) {
  if (!cls.IsDefined()) {
    return nullptr;
  }

  return (jobjectArray)env->CallStaticObjectMethod(cls, list_method);
}
