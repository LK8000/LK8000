/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#ifndef LSCPU_H
#define LSCPU_H

#include "cpuset.h"

/*
 * global description
 * lscpu - CPU architecture information helper
 *
 * Copyright (C) 2008 Cai Qian <qcai@redhat.com>
 * Copyright (C) 2008 Karel Zak <kzak@redhat.com>
*/

struct lscpu_desc {
        char    *arch;
        char    *vendor;
        char    *family;
        char    *model;
        char    *virtflag;      /* virtualization flag (vmx, svm) */
        int     hyper;          /* hypervisor vendor ID */
        int     virtype;        /* VIRT_PARA|FULL|NONE ? */
        char    *mhz;
        char    *stepping;
        char    *bogomips;
        char    *flags;
        int     mode;           /* rm, lm or/and tm */

        int             ncpus;          /* number of CPUs */
        cpu_set_t       *online;        /* mask with online CPUs */

        int             nnodes;         /* number of NUMA modes */
        cpu_set_t       **nodemaps;     /* array with NUMA nodes */

        /* sockets -- based on core_siblings (internal kernel map of cpuX's
         * hardware threads within the same physical_package_id (socket)) */
        int             nsockets;       /* number of all sockets */
        cpu_set_t       **socketmaps;   /* unique core_siblings */

        /* cores -- based on thread_siblings (internel kernel map of cpuX's
         * hardware threads within the same core as cpuX) */
        int             ncores;         /* number of all cores */
        cpu_set_t       **coremaps;     /* unique thread_siblings */

        int             nthreads;       /* number of threads */

        int             ncaches;
        struct cpu_cache *caches;
};




#endif
