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
#include "Android/TextUtil.hpp"
#include "Android/Context.hpp"
#include "Screen/OpenGL/Init.hpp"
#include "Screen/Debug.hpp"
#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#include "Util/StringAPI.hxx"
#include "Window/WndMain.h"
#include "org_LK8000_NativeView.h"
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Buffer.hpp"

unsigned android_api_level;

Context *context;
NativeView *native_view;

extern WndMain MainWindow;

extern "C" {
  /* workaround for
     http://code.google.com/p/android/issues/detail?id=23203 copied
     from https://bugzilla.mozilla.org/show_bug.cgi?id=734832 */
  __attribute__((weak)) void *__dso_handle;
}

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

gcc_visibility_default
JNIEXPORT jboolean JNICALL
Java_org_LK8000_NativeView_initializeNative(JNIEnv *env, jobject obj,
                                            jobject _context,
                                            jint width, jint height,
                                            jint xdpi, jint ydpi,
                                            jint sdk_version, jstring product)
{
  android_api_level = sdk_version;

  Java::Init(env);
  Java::Object::Initialise(env);
  Java::File::Initialise(env);

  NativeView::Initialise(env);
  Environment::Initialise(env);
  AndroidBitmap::Initialise(env);

  context = new Context(env, _context);

  InitialiseDataPath();

  OpenGL::Initialise();
  TextUtil::Initialise(env);

  assert(native_view == nullptr);
  native_view = new NativeView(env, obj, width, height, xdpi, ydpi,
                               product);

  event_queue = new EventQueue();

  ScreenInitialized();

  return Startup(nullptr);
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_runNative(JNIEnv *env, jobject obj)
{
    InitThreadDebug();

    OpenGL::Initialise();
    MainWindow.RunModalLoop();
}

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

  TextUtil::Deinitialise(env);
  OpenGL::Deinitialise();
  ScreenDeinitialized();
  DeinitialiseDataPath();

  delete context;
  context = nullptr;

  AndroidBitmap::Deinitialise(env);
  Environment::Deinitialise(env);
  NativeView::Deinitialise(env);
  Java::URL::Deinitialise(env);
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_resizedNative(JNIEnv *env, jobject obj,
                                         jint width, jint height)
{
  if (event_queue == nullptr)
    return;

  MainWindow.AnnounceResize(width, height);

  event_queue->Purge(Event::RESIZE);

  Event event(Event::RESIZE, width, height);
  event_queue->Push(event);
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_pauseNative(JNIEnv *env, jobject obj)
{
  if (event_queue == nullptr)
    /* pause before we have initialized the event subsystem does not
       work - let's bail out, nothing is lost anyway */
    exit(0);

  MainWindow.Pause();

  assert(num_textures == 0);
  assert(num_buffers == 0);
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_resumeNative(JNIEnv *env, jobject obj)
{
  if (event_queue == nullptr)
    /* there is nothing here yet which can be resumed */
    exit(0);

  MainWindow.Resume();
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_setBatteryPercent(JNIEnv *env, jobject, jint, jint) {
    #warning " Not Implemented"
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NativeView_setHapticFeedback(JNIEnv * env, jobject, jboolean) {
    #warning " Not Implemented"
}
