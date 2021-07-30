/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   UsbSerialHelper.h
 * Author: Bruno de Lacheisserie
 *
 * Created on April 04, 2017
 */

#ifndef ANDROID_USBSERIALHELPER_H
#define ANDROID_USBSERIALHELPER_H


#include "Compiler.h"
#include <jni.h>
#include "Android/PortBridge.hpp"

namespace UsbSerialHelper {
  /**
   * Global initialisation.  Looks up the methods of the
   * UsbSerialHelper Java class.
   */
  bool Initialise(JNIEnv *env);
  void Deinitialise(JNIEnv *env);

  /**
   * Is the Usb Host available
   */
  gcc_pure
  bool isEnabled(JNIEnv *env);

  /**
   * Returns a list of connected usb devices.
   */
  Java::LocalRef<jobjectArray> list(JNIEnv *env);

  PortBridge* connectDevice(JNIEnv *env, const char *name, unsigned baud);

};

#endif //ANDROID_USBSERIALHELPER_H
