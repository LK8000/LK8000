/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 */

#ifndef __DEVLXV7EASY_H_
#define __DEVLXV7EASY_H_

#include "Devices/DeviceRegister.h"

void LXV7easyInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t LXV7easyRegister() {
    return devRegister(_T("LXV7 easy"), LXV7easyInstall );
}

#endif // __DEVLXV7EASY_H_
