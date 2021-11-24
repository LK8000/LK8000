/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: DeviceRegister.h
 * Author: Bruno de Lacheisserie
 *
 */
#ifndef _DEVICES_DEVICEREGISTER_h_
#define _DEVICES_DEVICEREGISTER_h_

#include "tchar.h"
#include "cstddef"
#include "types.h"

struct DeviceDescriptor_t;

typedef void (*Installer_t)(DeviceDescriptor_t* d);

struct DeviceRegister_t {
  const TCHAR* Name;
  Installer_t Installer;
};

constexpr inline
DeviceRegister_t devRegister(const TCHAR *Name, Installer_t Installer) {
  return { Name, Installer };
}

struct devRegisterIterator {
  static const DeviceRegister_t* begin();
  static const DeviceRegister_t* end();
};

const DeviceRegister_t* GetRegisteredDevice(const TCHAR* Name);

#endif // _DEVICES_DEVICEREGISTER_h_
