/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   NetUtil.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 20, 2019, 10:15 PM
 */

#ifndef _ANDROID_NETUTIL_H_
#define _ANDROID_NETUTIL_H_

#include "Compiler.h"
#include <jni.h>

namespace NetUtil {
    /**
     * Global initialisation.  Looks up the methods of the
     * NetUtil Java class.
     */
    bool Initialise(JNIEnv *env);
    void Deinitialise(JNIEnv *env);

    gcc_pure
    int getNetState(JNIEnv *env);
}

#endif // _ANDROID_NETUTIL_H_
