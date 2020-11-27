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
#include "../CpuLoad.h"

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
#include <numeric>
#include <inttypes.h>

/* Show overall CPU utilization of the system 
 * http://phoxis.org/2013/09/05/finding-overall-and-per-core-cpu-utilization
 */

#define BUF_MAX 1024

class GetCpuLoad_Singleton {
public:

    GetCpuLoad_Singleton() {
        if(read_fields()) {
            total_tick = std::accumulate(std::begin(fields), std::end(fields), (uint64_t) 0);
            idle = fields[3]; /* idle ticks index */

            lastValue.Update();
        }
    }

    ~GetCpuLoad_Singleton() {
    }

    int operator()() {

        if(!lastValue.IsDefined()) {
          return INVALID_VALUE;
        }

        // only calculate each 1s  
        if(lastValue.CheckUpdate(1*1000)) {
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
        return lrint(((del_total_tick - del_idle) / (double) del_total_tick) * 100);
    }

private:

    bool read_fields() {
        bool bRet = false;
        FILE* _fp = fopen("/proc/stat", "r");
        if(_fp) {
            if (fgets(buffer, BUF_MAX, _fp)) {
                int retval = sscanf(buffer,
                                    "cpu %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64,
                                    &fields[0], &fields[1], &fields[2], &fields[3], &fields[4],
                                    &fields[5], &fields[6], &fields[7], &fields[8], &fields[9]);

                bRet = (retval >= 4); /* Atleast 4 fields is to be read */
            }
            fclose(_fp);
        }
        return bRet;
    }

    char buffer[BUF_MAX];

    uint64_t fields[10], total_tick, total_tick_old, idle, idle_old, del_total_tick, del_idle;
    
    PeriodClock lastValue;
};

GetCpuLoad_Singleton GetGpuLoad;

int CpuSummary() {
    return GetGpuLoad();
}
#endif
