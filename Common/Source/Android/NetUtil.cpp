/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   NetUtil.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 20, 2019, 10:15 PM
 */

#include <jni.h>
#include <cassert>
#include "Java/Class.hxx"
#include "NetUtil.h"

namespace NetUtil {

namespace {
    Java::TrivialClass cls;
    jmethodID getNetState_method;
}

bool Initialise(JNIEnv *env) {
    assert(!cls.IsDefined());
    assert(env != nullptr);

    if (!cls.FindOptional(env, "org/LK8000/NetUtil")) {
        return false;
    }
    getNetState_method = env->GetStaticMethodID(cls, "getNetState", "()I");

    return (getNetState_method != nullptr);
}

void Deinitialise(JNIEnv *env) {
    cls.ClearOptional(env);
}

int getNetState(JNIEnv *env) {
    return env->CallStaticIntMethod(cls, getNetState_method);
}

}
