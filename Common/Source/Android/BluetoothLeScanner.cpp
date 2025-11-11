/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BluetoothLeScanner.cpp
 * Author: Bruno de Lacheisserie
 */

#include "BluetoothLeScanner.h"
#include <utility>
#include "Java/Env.hxx"
#include "Java/Ref.hxx"
#include "Java/Class.hxx"
#include "Android/NativeLeScanCallback.hpp"
#include "MessageLog.h"

namespace {

Java::TrivialClass cls;
jmethodID ctor;
jmethodID start_method;
jmethodID stop_method;

}  // namespace

void BluetoothLeScanner::Initialise(JNIEnv* env) {
  cls.Find(env, "org/LK8000/BluetoothLeScanner");

  ctor = env->GetMethodID(cls, "<init>", "()V");
  start_method = env->GetMethodID(cls, "start", "(Lorg/LK8000/NativeLeScanCallback;)V");
  stop_method = env->GetMethodID(cls, "stop", "()V");
}

void BluetoothLeScanner::Deinitialise(JNIEnv* env) {
  cls.ClearOptional(env);
}

BluetoothLeScanner::BluetoothLeScanner(WndForm *pWndForm, callback_t callback) : _pWndForm(pWndForm), _callback(std::move(callback)) {
  if (!cls) {
    throw std::runtime_error("java class not found");
  }
  JNIEnv *env = Java::GetEnv();

  obj = NewObjectRethrow(env, cls, ctor);

  Java::LocalObject obj_callback = NativeLeScanCallback::Create(env, *this);
  env->CallVoidMethod(obj, start_method, obj_callback.Get());
  Java::RethrowException(env);
}

BluetoothLeScanner::~BluetoothLeScanner() {
  if (obj) {
    obj.GetEnv()->CallVoidMethod(obj, stop_method);
  }
}
