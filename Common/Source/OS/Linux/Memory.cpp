/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Memory.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 8 décembre 2014
 */

#include <stddef.h>
#include <sys/sysinfo.h>


size_t CheckFreeRam(void) {
    struct sysinfo info = {};
    if(sysinfo(&info) == 0) {
        return info.freeram *(unsigned long long)info.mem_unit;
    }
    return ~((size_t)0);
}

size_t FindFreeSpace(const char *path) {
    return ~((size_t)0);
}

void MyCompactHeaps() {
    
}
