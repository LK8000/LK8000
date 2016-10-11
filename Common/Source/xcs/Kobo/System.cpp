/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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

#include "System.hpp"
#include "OS/FileUtil.hpp"
#include "OS/PathName.hpp"
#include "OS/Process.hpp"
#include "OS/Sleep.h"
#include "Util/StaticString.hpp"

#include <unistd.h>
#include <sys/stat.h>

#ifdef KOBO
#include <cstdlib>
#include <sys/mount.h>
#include <errno.h>

template<typename... Args>
static bool
InsMod(const char *path, Args... args)
{
  return Run("/sbin/insmod", path, args...);
}

static bool
RmMod(const char *name)
{
  return Run("/sbin/rmmod", name);
}

/**
 * Determine the location of the current program, and build a path to
 * another program in the same directory.
 */
static bool
SiblingPath(const char *name, char *buffer, size_t size)
{
  if (readlink("/proc/self/exe", buffer, size) <= 0)
    return false;

  ReplaceBaseName(buffer, name);
  return true;
}

#endif

bool
KoboReboot()
{
#ifdef KOBO
  return Run("/sbin/reboot");
#else
  return false;
#endif
}

bool
KoboPowerOff()
{
#ifdef KOBO
  char buffer[256];
  if (SiblingPath("PowerOff", buffer, sizeof(buffer)))
    execl(buffer, buffer, nullptr);

  /* fall back */
  return Run("/sbin/poweroff");
#else
  return false;
#endif
}

bool
KoboUmountData()
{
#ifdef KOBO
  return umount("/mnt/onboard") == 0 || errno == EINVAL;
#else
  return true;
#endif
}

bool
KoboMountData()
{
#ifdef KOBO
  Run("/bin/dosfsck", "-a", "-w", "/dev/mmcblk0p3");
  return mount("/dev/mmcblk0p3", "/mnt/onboard", "vfat",
               MS_NOATIME|MS_NODEV|MS_NOEXEC|MS_NOSUID,
               "iocharset=utf8");
#else
  return true;
#endif
}

bool
KoboExportUSBStorage()
{
#ifdef KOBO
  RmMod("g_ether");
  RmMod("g_file_storage");

  InsMod("/drivers/current/usb/gadget/arcotg_udc.ko");
  return InsMod("/drivers/current/usb/gadget/g_file_storage.ko",
                "file=/dev/mmcblk0p3", "stall=0");
#else
  return true;
#endif
}

void
KoboUnexportUSBStorage()
{
#ifdef KOBO
  RmMod("g_ether");
  RmMod("g_file_storage");
  RmMod("arcotg_udc");
#endif
}

bool
IsKoboWifiOn()
{
#ifdef KOBO
  const char* szInterface = std::getenv("INTERFACE");
  if(!szInterface) {
    szInterface = "eth0";
  }
  
  char szPath[250];
  sprintf(szPath, "/sys/class/net/%s", szInterface);
  
  return Directory::Exists(szPath);
#else
  return false;
#endif
}

bool
KoboWifiOn()
{
#ifdef KOBO
  InsMod("/drivers/current/wifi/sdio_wifi_pwr.ko");

  const char * szModule = std::getenv("WIFI_MODULE_PATH");
  if(!szModule) {
    szModule = "/drivers/current/wifi/dhd.ko";
  }
  InsMod(szModule);

  Sleep(2000);
  
  const char* szInterface = std::getenv("INTERFACE");
  if(!szInterface) {
    szInterface = "eth0";
  }


  Run("/sbin/ifconfig", "eth0", "up");
  Run("/bin/wlarm_le", "-i", "eth0", "up");
  Run("/bin/wpa_supplicant", "-i", "eth0",
      "-c", "/etc/wpa_supplicant/wpa_supplicant.conf",
      "-C", "/var/run/wpa_supplicant", "-B");

  Sleep(2000);

  Start("/sbin/udhcpc", "-S", "-i", szInterface,
        "-s", "/etc/udhcpc.d/default.script",
        "-t15", "-T10", "-A3", "-f", "-q");

  return true;
#else
  return false;
#endif
}

bool
KoboWifiOff()
{
#ifdef KOBO
  const char* szInterface = std::getenv("INTERFACE");
  if(!szInterface) {
    szInterface = "eth0";
  }
    
  Run("/usr/bin/killall", "wpa_supplicant", "udhcpc");
  Run("/bin/wlarm_le", "-i", szInterface, "down");
  Run("/sbin/ifconfig", szInterface, "down");
  
  const char * szModule = std::getenv("WIFI_MODULE");
  if(!szModule) {
      szModule = "dhd";
  } 
  RmMod(szModule);
  RmMod("sdio_wifi_pwr");

  return true;
#else
  return false;
#endif
}

void
KoboExecNickel()
{
#ifdef KOBO
  /* our "rcS" will call the original Kobo "rcS" if start_nickel
     exists */
  mkdir("/mnt/onboard/LK8000", 0777);
  mkdir("/mnt/onboard/LK8000/kobo", 0777);
  File::CreateExclusive("/mnt/onboard/LK8000/kobo/start_nickel");

  /* unfortunately, a bug in the Kobo applications forces us to reboot
     the Kobo at this point */
  KoboReboot();
#endif
}

void
KoboRunXCSoar(const char *mode)
{
#ifdef KOBO
  char buffer[256];
  const char *cmd = buffer;

  if (!SiblingPath("xcsoar", buffer, sizeof(buffer)))
    cmd = "/mnt/onboard/XCSoar/xcsoar";

  Run(cmd, mode);
#endif
}

void
KoboRunTelnetd()
{
#ifdef KOBO
  /* telnetd requires /dev/pts - mount it (if it isn't already) */
  if (mkdir("/dev/pts", 0777) == 0)
    mount("none", "/dev/pts", "devpts", MS_RELATIME, NULL);

  Run("/usr/sbin/telnetd", "-l", "/bin/sh");
#endif
}

void
KoboRunFtpd()
{
#ifdef KOBO
  /* ftpd needs to be fired through tcpsvd (or inetd) */
  Start("/usr/bin/tcpsvd", "-E", "0.0.0.0", "21", "ftpd", "-w", "/mnt/onboard");
#endif
}


bool
KoboExportSerialAvailable()
{
    return File::Exists("/drivers/current/usb/gadget/g_serial.ko");
}

bool
KoboExportSerial()
{
#ifdef KOBO
  RmMod("g_ether");
  RmMod("g_file_storage");

  InsMod("/drivers/current/usb/gadget/arcotg_udc.ko");
  if(InsMod("/drivers/current/usb/gadget/g_serial.ko")) {
    for(unsigned i = 0; i < 5; ++i ) {
      if(File::Exists("/dev/ttyGS0")) {
        return true;
      }
      Sleep(100);
    }
  }
      return false;
#else
  return true;
#endif
}

void
KoboUnexportSerial()
{
#ifdef KOBO
  RmMod("g_serial");
  RmMod("arcotg_udc");
#endif
}