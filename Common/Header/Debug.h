#ifndef LK8000_DEBUG_H
#define LK8000_DEBUG_H
/*
 * Use this place to activate DEBUG options, please explain their usage
 *
 * This file is included automatically by options.h.
 * It works only in TESTBENCH mode
 */

#if TESTBENCH
#define _DEBUG_STOP_RXTHREAD // log close rx thread step for debug hang ....

/*
#define DEBUG_FOCUS 1	  // show SETFOCUS and KILLFOCUS traces in LK
#define DEBUG_DEVSETTING  // startupstore messages to understand sequence of device and com port init
#define DEBUG_DBLCLK      // show double click is being pressed
#define VENTA_DEBUG_EVENT // show key events, actually very few.
#define VENTA_DEBUG_KEY   // activates scan key codes, so you know what an hardware key is mapped to
                          // the HW key scan code is displayed on the screen!
#define DEBUG_ROTARY      // write in DEBUG.TXT located in the same place of .exe , append mode
#define DEBUG_DEV_COM     // log device communication through DevBase class methods
#define DEBUG_GPS         //  Parser.cpp will show more debug messages about wrong fixes
#define DEBUGNPM          // port monitor and hearthbeats in Parser
#define DEBUG_BESTALTERNATE     // full bestalternate messages inside DEBUG.TXT in home directory
#define DEBUG_AIRSPACE  // Airspace code debug messages to runtime.log
*/
#endif

#endif
