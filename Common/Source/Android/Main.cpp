/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Main.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on October 18, 2016, 10:15 PM
 */

#include "Main.hpp"
#include "Compiler.h"
#include "Java/Object.hxx"
#include "Java/File.hxx"
#include "Android/NativeView.hpp"
#include "Android/Bitmap.hpp"
#include "Android/SoundUtil.hpp"
#include "Android/TextUtil.hpp"
#include "Android/Context.hpp"
#include "Android/Environment.hpp"
#include "Android/InternalSensors.hpp"
#include "Android/NativePortListener.hpp"
#include "Android/NativeInputListener.hpp"
#include "Android/PortBridge.hpp"
#include "Android/BluetoothHelper.hpp"
#include "Android/NativeLeScanCallback.hpp"
#include <Android/IOIOHelper.hpp>

#include "Screen/OpenGL/Init.hpp"
#include "Screen/Debug.hpp"
#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#include "Util/StringAPI.hxx"
#include "Window/WndMain.h"
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Buffer.hpp"
#include "UsbSerialHelper.h"
#include "AndroidFileUtils.h"

unsigned android_api_level;

Context *context;
NativeView *native_view;

IOIOHelper *ioio_helper = nullptr;

extern WndMain* main_window;

extern "C" {
  /* workaround for
     http://code.google.com/p/android/issues/detail?id=23203 copied
     from https://bugzilla.mozilla.org/show_bug.cgi?id=734832 */
  __attribute__((weak)) void *__dso_handle;
}

extern "C"
gcc_visibility_default
JNIEXPORT jint JNICALL
Java_org_LK8000_NativeView_getEglContextClientVersion(JNIEnv *env, jobject obj)
{
#ifdef HAVE_GLES2
  return 2;
#else
  return 1;
#endif
}

extern bool Startup(const char*);
extern void Shutdown();

extern "C"
gcc_visibility_default
JNIEXPORT jboolean JNICALL
Java_org_LK8000_NativeView_initializeNative(JNIEnv *env, jobject obj,
                                            jobject _context,
                                            jint width, jint height,
                                            jint xdpi, jint ydpi,
                                            jint sdk_version, jstring product, jstring language)
{
  android_api_level = sdk_version;

  Java::Object::Initialise(env);
  Java::File::Initialise(env);

  NativeView::Initialise(env);
  Environment::Initialise(env);
  AndroidBitmap::Initialise(env);
  InternalSensors::Initialise(env);
  NativePortListener::Initialise(env);
  NativeInputListener::Initialise(env);
  PortBridge::Initialise(env);
  BluetoothHelper::Initialise(env);
  NativeLeScanCallback::Initialise(env);
  UsbSerialHelper::Initialise(env);

  AndroidFileUtils::Initialise(env);

  const bool have_ioio = IOIOHelper::Initialise(env);

  context = new Context(env, _context);

  OpenGL::Initialise();
  TextUtil::Initialise(env);

  assert(native_view == nullptr);
  native_view = new NativeView(env, obj, width, height, xdpi, ydpi,
                               product, language);

  event_queue = new EventQueue();


#if defined __arm__ || defined __aarch64__
  // Android eInk devices support
  is_dithered = is_dithered || StringIsEqual(native_view->GetProduct(), "Poke_Pro");
  is_dithered = is_dithered || StringIsEqual(native_view->GetProduct(), "C68");
  is_dithered = is_dithered || StringIsEqual(native_view->GetProduct(), "yotaphone2");
  is_dithered = is_dithered || StringIsEqual(native_view->GetProduct(), "alfapilot");

  is_dithered = is_dithered || StringIsEqual(native_view->GetProduct(), "HLTE203T");
  is_eink_colored = is_eink_colored || StringIsEqual(native_view->GetProduct(), "HLTE203T");
#endif


  if (have_ioio) {
    try {
      ioio_helper = new IOIOHelper(env);
    } catch (Java::Exception e) {
      StartupStore("IOIO unavailable : \"%s\"\n", e.what());
    }
  }

  SoundUtil::Initialise(env);


  ScreenInitialized();

  return Startup(nullptr);
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_runNative(JNIEnv *env, jobject obj) {
  InitThreadDebug();

  OpenGL::Initialise();
  main_window->RunModalLoop();
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_deinitializeNative(JNIEnv *env, jobject obj)
{
  Shutdown();

  InitThreadDebug();

  delete event_queue;
  event_queue = nullptr;
  delete native_view;
  native_view = nullptr;

  delete ioio_helper;
  ioio_helper = nullptr;

  AndroidFileUtils::Deinitialise(env);
  TextUtil::Deinitialise(env);
  OpenGL::Deinitialise();
  ScreenDeinitialized();

  delete context;
  context = nullptr;

  UsbSerialHelper::Deinitialise(env);
  IOIOHelper::Deinitialise(env);
  NativeLeScanCallback::Deinitialise(env);
  BluetoothHelper::Deinitialise(env);
  NativeInputListener::Deinitialise(env);
  NativePortListener::Deinitialise(env);
  SoundUtil::Deinitialise(env);
  InternalSensors::Deinitialise(env);
  AndroidBitmap::Deinitialise(env);
  Environment::Deinitialise(env);
  NativeView::Deinitialise(env);
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_resizedNative(JNIEnv *env, jobject obj,
                                         jint width, jint height)
{
  if (event_queue == nullptr)
    return;

  main_window->AnnounceResize({width, height});

  event_queue->Purge(Event::RESIZE);

  Event event(Event::RESIZE, width, height);
  event_queue->Push(event);
}

extern "C"
gcc_visibility_default
JNIEXPORT jboolean JNICALL
Java_org_LK8000_NativeView_pauseNative(JNIEnv *env, jobject obj)
{
  if (event_queue == nullptr) {
    /* pause before we have initialized the event subsystem does not
       work - let's bail out, nothing is lost anyway */
    return false;
  }

  main_window->Pause();

  assert(num_textures == 0);
  assert(num_buffers == 0);
  return true;
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_resumeNative(JNIEnv *env, jobject obj)
{
  if (event_queue == nullptr) {
    /* there is nothing here yet which can be resumed
     * or shutdown in progress ...
     */
    return;
  }

  main_window->Resume();
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_setBatteryPercent(JNIEnv *env, jobject, jint, jint) {
    //TODO : " Not Implemented"
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_setHapticFeedback(JNIEnv * env, jobject, jboolean) {
    //TODO : " Not Implemented"
}
