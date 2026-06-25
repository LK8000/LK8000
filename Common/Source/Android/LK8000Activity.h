/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LK8000Activity.h
 * Author: Bruno de Lacheisserie
 */

#ifndef ANDROID_STUDIO_LK8000ACTIVITY_H
#define ANDROID_STUDIO_LK8000ACTIVITY_H

#include "Thread/Mutex.hpp"
#include "Thread/Cond.hpp"
#include "Java/Object.hxx"
#include "Java/Class.hxx"

class LK8000Activity {
  static Java::TrivialClass cls;
  static jmethodID check_permissions_method;
  static jmethodID scan_qrcode_method;
  static jmethodID share_file_method;
  static jmethodID detect_keyboard_method;
  static jmethodID get_clipboard_text_method;
  static jmethodID set_clipboard_text_method;
  static jmethodID acquire_multicast_lock_method;
  static jmethodID release_multicast_lock_method;
  static LK8000Activity *activity_instance;


public:
  typedef enum {
    unknown,
    denied,
    granted
  } permission_t;

protected:
  Java::GlobalObject obj;

  Mutex permission_status_mutex;
  permission_t permission_status;

  LK8000Activity() = delete;

  LK8000Activity(JNIEnv *_env, jobject _obj) :
          obj(_env, _obj), permission_status(unknown) { }


public:
  static void Initialise(JNIEnv *env, jobject obj);
  static void Deinitialise(JNIEnv *env, jobject obj);

  static LK8000Activity* Get() { return activity_instance; }


  void RequestPermission();

  permission_t GetPermission();
  permission_t WaitPermission();

  void PermissionDenied();
  void PermissionGranted();

  void ScanQRCode();

  void ShareFile(const char *path);

  void DetectKeyboardModel();

  std::string GetClipboardText();
  void SetClipboardText(const std::string &text);

  void AcquireMulticastLock();
  void ReleaseMulticastLock();
};


#endif //ANDROID_STUDIO_LK8000ACTIVITY_H
