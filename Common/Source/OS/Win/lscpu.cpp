/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

void lscpu_init(void) {
    HaveSystemInfo=0; // just to be sure
};

char *SystemInfo_Architecture(void) {
    return NULL;
}

char *SystemInfo_Vendor(void) {
    return NULL;
}

int SystemInfo_Cpus(void) {
    return 0;
}

unsigned int SystemInfo_Mhz(void) {
    return 0;
}

unsigned int SystemInfo_Bogomips(void) {
    return 0;
}



