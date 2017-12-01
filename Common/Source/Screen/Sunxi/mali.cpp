/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   mali.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 29 décembre 2017
 */
#ifdef HAVE_MALI

#include "mali.h"
#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fstream>
#include <regex>
#include "sunxi_disp_ioctl.h"
#include "OS/Process.hpp"


PixelSize mali::GetScreenSize() {
  PixelSize size(0,0);
  int fd = open("/dev/disp", O_RDWR);
  if(fd) {

    uint32_t args[4] = { 0, 0, 0, 0 };
    size.cx = ioctl(fd, DISP_CMD_SCN_GET_WIDTH, args);
    size.cy = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT, args);
    close(fd);
  }
  return size;
}

DisplayOrientation_t mali::GetScreenOrientation() {
  
  Run("/bin/mount", "/dev/mmcblk0p1", "/boot");

  DisplayOrientation_t orientation = DisplayOrientation_t::DEFAULT;
  std::ifstream ifs("/boot/config.uEnv", std::ifstream::in);
  if(ifs.is_open()) {
    std::regex pair_regex("^(rotation)=([0-3])$");
    std::smatch pair_match;
    std::string line;
    while (std::getline (ifs, line)) {
      if (std::regex_match(line, pair_match, pair_regex)) {
        if (pair_match.size() == 3) {
          orientation = static_cast<DisplayOrientation_t>(strtoul(pair_match[2].str().c_str(), nullptr, 10));
          break;
        }
      }
    }
    ifs.close();
  }
  Run("/bin/umount", "/boot"); 
  
  return orientation;
}

#endif /* HAVE_MALI */
