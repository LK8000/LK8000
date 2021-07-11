/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   AndroidFileUtils.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2021, 22:48 PM
 */

#include "AndroidFileUtils.h"
#include <cassert>
#include "Java/String.hxx"
#include "Java/Class.hxx"
#include "Android/Context.hpp"
#include "Android/Main.hpp"

namespace {
    Java::TrivialClass cls;
    jmethodID updateMediaStore_method;
}

void AndroidFileUtils::Initialise(JNIEnv *env) {
    assert(!cls.IsDefined());
    assert(env != nullptr);

    cls.Find(env, "org/LK8000/FileUtils");
    updateMediaStore_method = env->GetStaticMethodID(cls.Get(),
                                                     "updateMediaStore",
                                                     "(Landroid/content/Context;Ljava/lang/String;)V");
}

void AndroidFileUtils::Deinitialise(JNIEnv *env) {
    cls.Clear(env);
}

void AndroidFileUtils::UpdateMediaStore(const char* filePath) {
    if (updateMediaStore_method) {
        JNIEnv *env = Java::GetEnv();

        Java::String str(env, filePath);
        env->CallStaticVoidMethod(cls, updateMediaStore_method, context->Get(), str.Get());
    }
}
