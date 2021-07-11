/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   AndroidFileUtils.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2021, 22:48 PM
 */

#ifndef ANDROID_ANDROIDFILEUTILS_H
#define ANDROID_ANDROIDFILEUTILS_H

#include "Compiler.h"
#include <jni.h>

namespace AndroidFileUtils {

    void Initialise(JNIEnv *env);
    void Deinitialise(JNIEnv *env);

    void UpdateMediaStore(const char* filePath);
}

#endif //ANDROID_ANDROIDFILEUTILS_H
