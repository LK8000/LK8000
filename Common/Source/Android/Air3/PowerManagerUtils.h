/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PowerManagerUtils.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 14/10/2022.
 */


#ifndef ANDROID_AIR3_POWERMANAGERUTILS_H
#define ANDROID_AIR3_POWERMANAGERUTILS_H

namespace PowerManagerUtils {
    bool openModuleFanet() noexcept;
    void closeModuleFanet() noexcept;
}

#endif //ANDROID_AIR3_POWERMANAGERUTILS_H
