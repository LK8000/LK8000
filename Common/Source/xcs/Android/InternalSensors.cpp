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
#include "externs.h"
#include "device.h"
#include "Globals.h"
#include "Geoid.h"
#include "devBase.h"
#include "InternalSensors.hpp"
#include "Context.hpp"
#include "Math/SelfTimingKalmanFilter1d.hpp"
#include "OS/Clock.hpp"
#include "Compiler.h"
#include "Thread/Mutex.hpp"
#include "Baro.h"
#include "Calc/Vario.h"
#include "ComCheck.h"



Java::TrivialClass InternalSensors::gps_cls, InternalSensors::sensors_cls;
jmethodID InternalSensors::gps_ctor_id, InternalSensors::close_method;
jmethodID InternalSensors::sensors_ctor_id;
jmethodID InternalSensors::mid_sensors_getSubscribableSensors;
jmethodID InternalSensors::mid_sensors_subscribeToSensor_;
jmethodID InternalSensors::mid_sensors_cancelSensorSubscription_;
jmethodID InternalSensors::mid_sensors_subscribedToSensor_;
jmethodID InternalSensors::mid_sensors_cancelAllSensorSubscriptions_;

bool
InternalSensors::Initialise(JNIEnv *env)
{
  assert(!gps_cls.IsDefined());
  assert(!sensors_cls.IsDefined());
  assert(env != nullptr);

  gps_cls.Find(env, "org/LK8000/InternalGPS");

  gps_ctor_id = env->GetMethodID(gps_cls, "<init>",
                                 "(Landroid/content/Context;I)V");
  close_method = env->GetMethodID(gps_cls, "close", "()V");

  sensors_cls.Find(env, "org/LK8000/NonGPSSensors");

  sensors_ctor_id = env->GetMethodID(sensors_cls, "<init>",
                                     "(Landroid/content/Context;I)V");

  mid_sensors_getSubscribableSensors =
    env->GetMethodID(sensors_cls, "getSubscribableSensors", "()[I");
  assert(mid_sensors_getSubscribableSensors != nullptr);

  mid_sensors_subscribeToSensor_ =
      env->GetMethodID(sensors_cls, "subscribeToSensor", "(I)Z");
  mid_sensors_cancelSensorSubscription_ =
      env->GetMethodID(sensors_cls, "cancelSensorSubscription", "(I)Z");
  mid_sensors_subscribedToSensor_ =
      env->GetMethodID(sensors_cls, "subscribedToSensor", "(I)Z");
  mid_sensors_cancelAllSensorSubscriptions_ =
      env->GetMethodID(sensors_cls, "cancelAllSensorSubscriptions", "()V");
  assert(mid_sensors_subscribeToSensor_ != nullptr);
  assert(mid_sensors_cancelSensorSubscription_ != nullptr);
  assert(mid_sensors_subscribedToSensor_ != nullptr);
  assert(mid_sensors_cancelAllSensorSubscriptions_ != nullptr);

  return true;
}

void
InternalSensors::Deinitialise(JNIEnv *env)
{
  gps_cls.Clear(env);
  sensors_cls.Clear(env);
}

InternalSensors::InternalSensors(JNIEnv* env, jobject gps_obj, jobject sensors_obj)
    : obj_InternalGPS_(env, gps_obj),
      obj_NonGPSSensors_(env, sensors_obj) {
  // Import the list of subscribable sensors from the NonGPSSensors object.
  getSubscribableSensors(env, sensors_obj);
}

InternalSensors::~InternalSensors() {
  // Unsubscribe from sensors and the GPS.
  cancelAllSensorSubscriptions();
  JNIEnv *env = Java::GetEnv();
  env->CallVoidMethod(obj_InternalGPS_.Get(), close_method);
}

bool InternalSensors::subscribeToSensor(int id) {
  JNIEnv* env = Java::GetEnv();
  return env->CallBooleanMethod(obj_NonGPSSensors_.Get(),
                                mid_sensors_subscribeToSensor_, (jint) id);
}

bool InternalSensors::cancelSensorSubscription(int id) {
  JNIEnv* env = Java::GetEnv();
  return env->CallBooleanMethod(obj_NonGPSSensors_.Get(),
                                mid_sensors_cancelSensorSubscription_,
                                (jint) id);
}

bool InternalSensors::subscribedToSensor(int id) const {
  JNIEnv* env = Java::GetEnv();
  return env->CallBooleanMethod(obj_NonGPSSensors_.Get(),
                                mid_sensors_subscribedToSensor_, (jint) id);
}

void InternalSensors::cancelAllSensorSubscriptions() {
  JNIEnv* env = Java::GetEnv();
  env->CallVoidMethod(obj_NonGPSSensors_.Get(),
                      mid_sensors_cancelAllSensorSubscriptions_);
}

InternalSensors* InternalSensors::create(JNIEnv* env, Context* context,
                                         unsigned int index) {
  assert(sensors_cls != nullptr);
  assert(gps_cls != nullptr);

  // Construct InternalGPS object.
  jobject gps_obj =
    env->NewObject(gps_cls, gps_ctor_id, context->Get(), index);
  assert(gps_obj != nullptr);

  // Construct NonGPSSensors object.
  jobject sensors_obj =
      env->NewObject(sensors_cls, sensors_ctor_id, context->Get(), index);
  assert(sensors_obj != nullptr);

  InternalSensors *internal_sensors =
      new InternalSensors(env, gps_obj, sensors_obj);
  env->DeleteLocalRef(gps_obj);
  env->DeleteLocalRef(sensors_obj);

  return internal_sensors;
}

// Helper for retrieving the set of sensors to which we can subscribe.
void InternalSensors::getSubscribableSensors(JNIEnv* env, jobject sensors_obj) {
  jintArray ss_arr = (jintArray) env->CallObjectMethod(
      obj_NonGPSSensors_.Get(), mid_sensors_getSubscribableSensors);
  jsize ss_arr_size = env->GetArrayLength(ss_arr);
  jint* ss_arr_elems = env->GetIntArrayElements(ss_arr, nullptr);
  subscribable_sensors_.assign(ss_arr_elems, ss_arr_elems + ss_arr_size);
  env->ReleaseIntArrayElements(ss_arr, ss_arr_elems, 0);
}

/*
 * From here to end: C++ functions called by Java to export GPS and sensor
 * information into XCSoar C++ code.
 */

// Helper for the C++ functions called by Java (below).
inline unsigned int getDeviceIndex(JNIEnv *env, jobject obj) {
  jfieldID fid_index = env->GetFieldID(env->GetObjectClass(obj),
                                       "index", "I");
  return env->GetIntField(obj, fid_index);
}

// Implementations of the various C++ functions called by InternalGPS.java.

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_InternalGPS_setConnected(JNIEnv *env, jobject obj, jboolean connected)
{
  unsigned index = getDeviceIndex(env, obj);
  ScopeLock Lock(CritSec_Comm);

  PDeviceDescriptor_t pdev = devX(index);
  if(pdev) {
    pdev->HB = LKHearthBeats;
    pdev->nmeaParser.connected = connected;
    pdev->nmeaParser.gpsValid = false;
  }
}

// Implementations of the various C++ functions called by NonGPSSensors.java.

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NonGPSSensors_setAcceleration(
    JNIEnv* env, jobject obj, jfloat ddx, jfloat ddy, jfloat ddz) {
  // TODO
  /*
  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  */
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NonGPSSensors_setRotation(
    JNIEnv* env, jobject obj,
    jfloat dtheta_x, jfloat dtheta_y, jfloat dtheta_z) {
  // TODO
  /*
  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  */
}

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NonGPSSensors_setMagneticField(
    JNIEnv* env, jobject obj, jfloat h_x, jfloat h_y, jfloat h_z) {
  // TODO
  /*
  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  */
}

/**
 * Helper function for
 * Java_org_LK8000_NonGPSSensors_setBarometricPressure: Given a
 * current measurement of the atmospheric pressure and the rate of
 * change of atmospheric pressure (in millibars and millibars per
 * second), compute the uncompensated vertical speed of the glider in
 * meters per second, assuming standard atmospheric conditions
 * (deviations from these conditions should not matter very
 * much). This calculation can be derived by taking the formula for
 * converting barometric pressure to pressure altitude (see e.g.
 * http://psas.pdx.edu/RocketScience/PressureAltitude_Derived.pdf),
 * expressing it as a function P(t), the atmospheric pressure at time
 * t, then taking the derivative with respect to t. The dP(t)/dt term
 * is the pressure change rate.
 */
gcc_pure
static inline
double ComputeNoncompVario(const double pressure, const double d_pressure) {
  static constexpr double FACTOR(-2260.389548275485);
  static constexpr double EXPONENT(-0.8097374740609689);
  return FACTOR * pow(pressure, EXPONENT) * d_pressure;
}

static BOOL IsBaroSource(PDeviceDescriptor_t d) { return TRUE; }

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_NonGPSSensors_setBarometricPressure(
    JNIEnv* env, jobject obj, jfloat pressure, jfloat sensor_noise_variance) {
  /* We use a Kalman filter to smooth Android device pressure sensor
     noise.  The filter requires two parameters: the first is the
     variance of the distribution of second derivatives of pressure
     values that we expect to see in flight, and the second is the
     maximum time between pressure sensor updates in seconds before
     the filter gives up on smoothing and uses the raw value.
     The pressure acceleration variance used here is actually wider
     than the maximum likelihood variance observed in the data: it
     turns out that the distribution is more heavy-tailed than a
     normal distribution, probably because glider pilots usually
     experience fairly constant pressure change most of the time. */
  static constexpr double KF_VAR_ACCEL(0.0075);
  static constexpr double KF_MAX_DT(60);

  // XXX this shouldn't be a global variable
  static SelfTimingKalmanFilter1d kalman_filter(KF_MAX_DT, KF_VAR_ACCEL);

  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock Lock(CritSec_Comm);

  PDeviceDescriptor_t pdev = devX(index);
  if(pdev) {
    pdev->nmeaParser.connected = true;
    pdev->HB = LKHearthBeats;
    if(!pdev->IsBaroSource) {
      pdev->IsBaroSource = &IsBaroSource;
    }

    /* Kalman filter updates are also protected by the CommPort
       mutex. These should not take long; we won't hog the mutex
       unduly. */
    kalman_filter.Update(pressure, sensor_noise_variance);

    LockFlightData();
    UpdateVarioSource(GPS_INFO, *pdev, ComputeNoncompVario(kalman_filter.GetXAbs(),
                                                          kalman_filter.GetXVel()));
    UpdateBaroSource(&GPS_INFO, 0, pdev, StaticPressureToQNHAltitude(kalman_filter.GetXAbs() * 100));
    UnlockFlightData();
  }
}


extern "C"
JNIEXPORT void JNICALL
Java_org_LK8000_InternalGPS_parseNMEA(JNIEnv *env, jobject instance, jstring jnmea) {
  const char* c_nmea = env->GetStringUTFChars(jnmea, 0);

  int index = getDeviceIndex(env, instance);
  ScopeLock Lock(CritSec_Comm);

  if (ComCheck_ActivePort >= 0 && index == ComCheck_ActivePort) {
    for( auto it = c_nmea; (*it); ++it) {
      ComCheck_AddChar(*it);
    }
  }

  DeviceDescriptor_t* device = devGetDeviceOnPort(index);
  if(device) {
    device->Rx += strlen(c_nmea);
  }

  char* nmea = strdup(c_nmea);

  LockFlightData();
  devParseNMEA(index, nmea, &GPS_INFO);
  UnlockFlightData();

  free(nmea);

  env->ReleaseStringUTFChars(jnmea, c_nmea);
}