/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   TunedParameter
   Paolo

*/
#include "externs.h"
/*
#include "LKInterface.h"
#include "resource.h"
#include "Waypointparser.h"
#include "InfoBoxLayout.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "InputEvents.h"
#include "Message.h"
#include "Bitmaps.h"
#include "LKObjects.h"
#include "DoInits.h"
*/

/*
 * The reference system for LK historically is a PNA running at 400Mhz on ARM cpu, with
 * an optimistic rate of 800 bogomips, which we call the Reference Hardware (RH)
 * We compare it to the number of bogomips multiplied by the number of available CPUS.
 * Not accurate, we should consider RISC vs CISC as well, hyperthreads etc. but bogomips
 * themselves are bogus, so it is a rule of thumb only.
 *
 * Here we retune custom parameters based upon cpu speed.
 */



/*
 * This is used by MapWndProc as a timeout during panning, for fast refresh forced.
 * Time is in milliseconds.
 * Our goal is to get to a load average of 1.00 (full use of CPU while panning);
 * CISC processors will make it 2 to 3 times faster than we expect it, so the load will be
 * around 40% max even at 1/20" refresh.
 */
unsigned int TunedParameter_Fastpanning(void) {
    static bool doinit=true;
    static unsigned int fastpanning=700;

    if (!doinit) return fastpanning;

    if (HaveSystemInfo) {
        // On RH the refresh timeout is every 700ms.
        fastpanning = 700/(( SystemInfo_Bogomips() * SystemInfo_Cpus() )/800);
        if (fastpanning>700) fastpanning=700; // min
        if (fastpanning<50)  fastpanning=50;  // max accepted
    } else {
        #if (WINDOWSPC>0)
        fastpanning=250;
        #else
        fastpanning=700; // normally on CE
        #endif
    }

    TestLog(_T("... TunedParameter_Fastpanning set to: %d"), fastpanning);

    doinit=false;
    return fastpanning;
}
