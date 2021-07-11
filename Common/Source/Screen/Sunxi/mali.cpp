/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "sunxi_disp_ioctl.h"


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
#endif /* HAVE_MALI */
