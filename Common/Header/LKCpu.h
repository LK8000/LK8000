/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#ifndef LKCPU_H
#define LKCPU_H

  extern void lscpu_init(void);

  extern char *SystemInfo_Architecture(void);
  extern char *SystemInfo_Vendor(void);

  extern int SystemInfo_Cpus(void);
  extern unsigned int SystemInfo_Mhz(void);
  extern unsigned int SystemInfo_Bogomips(void);




#endif
