/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   CpuLoad.cpp
 * Author: Paolo
 *
 */
#include "externs.h"

#ifdef USE_LOADAVG_CPU

int CpuSummary() {

    double monoload[3];
    if ( getloadavg(monoload,3)<1) return INVALID_VALUE;

    double numcpus=1;
    if (HaveSystemInfo) numcpus=(double)SystemInfo_Cpus();
    LKASSERT(numcpus>0);

    int summary=(int)(monoload[0]*100/numcpus);

    if (summary>999 || summary<0) summary=INVALID_VALUE;
    return summary;
}

#else

#include <stdio.h>
#include <unistd.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/* Show overall CPU utilization of the system 
 * http://phoxis.org/2013/09/05/finding-overall-and-per-core-cpu-utilization
 */

#define BUF_MAX 1024

class GetCpuLoad_Singleton {
public:

    GetCpuLoad_Singleton() {
        _fp = fopen("/proc/stat", "r");

        if (_fp && read_fields()) {
            total_tick = std::accumulate(std::begin(fields), std::end(fields), (uint64_t)0);
            idle = fields[3]; /* idle ticks index */
        }
        
        lastValue.Update();
    }

    ~GetCpuLoad_Singleton() {
        if (_fp) {
            fclose(_fp);
        }
    }

    int operator()() {

        if (_fp == NULL) {
            return INVALID_VALUE;
        }
        
        // only calculate each 1s  
        if(lastValue.CheckUpdate(1*1000)) {
            if (!read_fields()) {
                return INVALID_VALUE;
            }

            total_tick_old = total_tick;
            idle_old = idle;

            if (!read_fields()) {
                return INVALID_VALUE;
            }

            total_tick = std::accumulate(std::begin(fields), std::end(fields), (uint64_t)0);
            idle = fields[3];

            del_total_tick = total_tick - total_tick_old;
            del_idle = idle - idle_old;
        }
        return std::lrint(((del_total_tick - del_idle) / (double) del_total_tick) * 100);
    }

private:

    bool read_fields() {
        fseek(_fp, 0, SEEK_SET);
        fflush(_fp);
        if (fgets(buffer, BUF_MAX, _fp)) {

        }
        return false;
    }

    FILE* _fp;
    char buffer[BUF_MAX];

    uint64_t fields[10], total_tick, total_tick_old, idle, idle_old, del_total_tick, del_idle;
    
    PeriodClock lastValue;
};

GetCpuLoad_Singleton GetGpuLoad;

int CpuSummary() {
    return GetGpuLoad();
}
#endif
