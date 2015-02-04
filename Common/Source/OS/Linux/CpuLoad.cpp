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
