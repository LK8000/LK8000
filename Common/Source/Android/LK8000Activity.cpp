/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LK8000Activity.cpp
 * Author: Bruno de Lacheisserie
 */

#include "LK8000Activity.h"
#include "Event/Event.h"
#include "externs.h"
#include <oboe/Oboe.h>
#include <Form/Form.hpp>
#include "Java/String.hxx"

Java::TrivialClass LK8000Activity::cls;
jmethodID LK8000Activity::check_permissions_method;
jmethodID LK8000Activity::scan_qrcode_method;
jmethodID LK8000Activity::share_file_method;

LK8000Activity* LK8000Activity::activity_instance = nullptr;

void LK8000Activity::Initialise(JNIEnv *env, jobject obj) {
  cls.Find(env, "org/LK8000/LK8000");
  assert(cls.IsDefined());
  check_permissions_method = env->GetMethodID(cls, "checkPermissions", "()V");
  assert(check_permissions_method);

  scan_qrcode_method = env->GetMethodID(cls, "scanQRCode", "()V");
  assert(scan_qrcode_method);

  share_file_method = env->GetMethodID(cls, "shareFile", "(Ljava/lang/String;)V");
  assert(share_file_method);

  assert(activity_instance == nullptr); // memory leak;
  activity_instance = new LK8000Activity(env, obj);
}

void LK8000Activity::Deinitialise(JNIEnv *env, jobject obj) {
  cls.Clear(env);
  delete activity_instance;
  activity_instance = nullptr;
}

void LK8000Activity::PermissionDenied() {
  const ScopeLock lock(permission_status_mutex);
  permission_status = denied;
  event_queue->Push(Event(Event::NOP));
}

void LK8000Activity::PermissionGranted() {
  const ScopeLock lock(permission_status_mutex);
  permission_status = granted;
  event_queue->Push(Event(Event::NOP));
}

LK8000Activity::permission_t LK8000Activity::WaitPermission() {
  if(event_queue && main_window) {
    EventLoop loop(*event_queue, *main_window);
    Event event;
    while (main_window->IsDefined() && loop.Get(event)) {
      loop.Dispatch(event);
      const ScopeLock lock(permission_status_mutex);
      if(permission_status != unknown) {
          return permission_status;
      }
    }
  }
  return unknown;
}

LK8000Activity::permission_t LK8000Activity::GetPermission() {
  const ScopeLock lock(permission_status_mutex);
  return permission_status;
}

void LK8000Activity::RequestPermission() {
  const ScopeLock lock(permission_status_mutex);
  permission_status = unknown;
  Java::GetEnv()->CallVoidMethod(obj, check_permissions_method);
}

void LK8000Activity::ScanQRCode() {
  Java::GetEnv()->CallVoidMethod(obj, scan_qrcode_method);
}

void LK8000Activity::ShareFile(const char *path) {
  JNIEnv *env = Java::GetEnv();

  Java::String stringPath(env, path);
  env->CallVoidMethod(obj, share_file_method, stringPath.Get());
}


extern "C"
JNIEXPORT void JNICALL
gcc_visibility_default
Java_org_LK8000_LK8000_initialiseNative(JNIEnv *env, jobject obj) {
  LK8000Activity::Initialise(env, obj);
}

extern "C"
JNIEXPORT void JNICALL
gcc_visibility_default
Java_org_LK8000_LK8000_deinitialiseNative(JNIEnv *env, jobject obj) {
  LK8000Activity::Deinitialise(env, obj);
}

extern "C"
JNIEXPORT void JNICALL
gcc_visibility_default
Java_org_LK8000_LK8000_onRuntimePermissionDenied(JNIEnv *env, jobject obj) {
  if(LK8000Activity::Get()) {
    LK8000Activity::Get()->PermissionDenied();
  }
}

extern "C"
JNIEXPORT void JNICALL
gcc_visibility_default
Java_org_LK8000_LK8000_onRuntimePermissionGrantedNative(JNIEnv *env, jobject obj) {
  if(LK8000Activity::Get()) {
    LK8000Activity::Get()->PermissionGranted();
  }
}

extern "C"
JNIEXPORT void JNICALL
gcc_visibility_default
Java_org_LK8000_LK8000_setDefaultStreamValues(JNIEnv *env, jclass type,
                                              jint sampleRate, jint framesPerBurst) {

  oboe::DefaultStreamValues::SampleRate = sampleRate;
  oboe::DefaultStreamValues::FramesPerBurst = framesPerBurst;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_LK8000_LK8000_loadQRCodeData(JNIEnv *env, jclass clazz, jstring data_string) {
  const std::string data = Java::String::ToString(env, data_string);
  if (data.find("XCTSK:") == 0) {
    if (LoadXctrackTaskString(std::next(std::begin(data), 6).base(), std::end(data).base())) {
      if (main_window) {
        WindowControl *pWnd = dynamic_cast<WindowControl*>(main_window->GetFocusedWindow());
        if (pWnd) {
          WndForm* pForm = pWnd->GetParentWndForm();
          if (pForm) {
            pForm->SendUser(UM_UPDATE_TASK_OVERVIEW);
          }
        }
      }
    }
  }
}
