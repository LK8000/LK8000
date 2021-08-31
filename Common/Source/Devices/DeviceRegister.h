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
  unsigned Flags;
  Installer_t Installer;
};

constexpr inline
DeviceRegister_t devRegister(const TCHAR *Name, unsigned Flags, Installer_t Installer) {
  return { Name, Flags, Installer };
}

size_t devRegisterCount();

const TCHAR* devRegisterGetName(size_t idx);

const DeviceRegister_t* GetRegisteredDevice(const TCHAR* Name);

#endif // _DEVICES_DEVICEREGISTER_h_
