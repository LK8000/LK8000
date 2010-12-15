/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKUtils.h,v 1.1 2010/12/15 12:30:04 root Exp root $
*/

// MapWindow::TextColor() supported colours
#define RGB_BLACK	RGB(0,0,0)
#define RGB_WHITE	RGB(255,255,255)
#define RGB_BLUE	RGB(0x00,0x00,0xff)
#define RGB_LIGHTBLUE	RGB(179,179,255)
#define RGB_GREY	RGB(0x99,0x99,0x99)
#define RGB_LIGHTGREY	RGB(0xcc,0xcc,0xcc)
#define RGB_VDARKGREY	RGB(0xf,0xf,0xf)		// was 0x0d
#define RGB_LIGHTORANGE	RGB(255,184,51)
#define RGB_ORANGE	RGB(255,165,0)
#define RGB_GREEN	RGB(0,255,0)
#define RGB_DARKGREEN	RGB(40,80,40)
#define RGB_LIGHTGREEN	RGB(0xc2,0xff,0xc2)
#define RGB_RED		RGB(255,0,0)
#define RGB_LIGHTRED	RGB(0xff,0xc2,0xc2)
#define RGB_YELLOW	RGB(255,255,0)
#define RGB_LIGHTYELLOW	RGB(0xff,0xff,0xc2)
#define RGB_CYAN	RGB(0,255,255)
#define RGB_LIGHTCYAN	RGB(0xc2,0xff,0xff)
#define RGB_MAGENTA	RGB(255,0,255)
// unsupported
#define RGB_AMBER	RGB(0xff,0xbe,0x00)
#define RGB_INDIGO	RGB(0x4b,0x00,0x82)
#define RGB_DARKRED	RGB(200,0,0)
#define RGB_VDARKRED	RGB(160,0,0)
#define RGB_DARKBLUE	RGB(0x00,0x00,160)
#define RGB_DARKYELLOW	RGB(255,100,0)
// dy2 for vario
#define RGB_DARKYELLOW2	RGB(255,215,0)
#define RGB_ICEWHITE	RGB(0xf0,0xf0,0xf0)
#define RGB_DARKWHITE	RGB(0xd9,0xd9,0xd9)
// unsupported, special use
#define RGB_SBLACK	RGB(0,0,1)
#define RGB_SWHITE	RGB(255,255,254)
// background mapspace
#define RGB_MDARK	RGB_BLACK
#define RGB_MLIGHT	RGB_DARKWHITE
// background navboxes
#define RGB_NDARK	RGB_VDARKGREY
#define RGB_NLIGHT	RGB_ICEWHITE
// inverted things on map
#define RGB_INVDRAW	RGB_LIGHTGREEN
// background moving map with no terrrain
#define RGB_LCDGREEN	RGB(197,223,194)
#define RGB_LAKE	RGB(88,158,253)
#define RGB_GGREY	RGB(117,160,150)
#define RGB_LCDDARKGREEN	RGB(59,179,158)
#define RGB_EMERALD	RGB(5,139,111)
#define RGB_DARKSLATE	RGB(89,113,125)
#define RGB_RIFLEGREY	RGB(41,52,46)

#define RGB_MIDDLEGREY	RGB(128,128,128)
// Original colors
// MessageBoxX
#define RGB_BADYELLOW	RGB(0xda, 0xdb, 0xab)
// mapwindows crosshair
#define RGB_DARKGREY	RGB(50,50,50)
// task line color, originally darg green
#define RGB_DGREEN1	RGB(0,120,0)
// buttons
#define RGB_BUTTGREEN	RGB(0xA0,0xE0,0xA0)

// LK8000 colors
#define RGB_LGREEN1	RGB(0xa9,0xda,0xc3)
#define RGB_LBLUE1	RGB(0xa9,0xd9,0xda)
#define RGB_PETROL	RGB(0,66,66)

// -------------  PROGRAM COLORS IN USE ---------------------
// MessageBox 
#define RGB_TASKLINECOL RGB_DGREEN1
#define RGB_BUTTONS	RGB_BUTTGREEN

// The box title background, originally yellow, used only if LKCOLOR
#define RGB_MENUTITLEBG	RGB_BLACK
#define RGB_MENUTITLEFG RGB_WHITE

// LK new colors for windows
#define RGB_WINBACKGROUND	RGB_PETROL
#define RGB_WINFOREGROUND	RGB_WHITE
// WindowControls, used only with LKCOLOR otherwise WHITE
#define RGB_LISTHIGHLIGHTBG	RGB(133,255,133)
#define RGB_LISTHIGHLIGHTCORNER RGB_PETROL
#define RGB_LISTFG		RGB_BLACK
#define RGB_LISTBG		RGB_LCDGREEN
// slider: borders and box
#define RGB_SCROLLBARBORDER	RGB_BLACK
#define RGB_SCROLLBARBOX	RGB_BLACK

// Config buttons,  BG not yet used, cannot change!
#define RGB_BUTTONFG		RGB_BLACK
#define RGB_BUTTONBG		RGB(154,184,195)


// WriteText modalities
#define WTALIGN_LEFT	0
#define WTALIGN_RIGHT	1
#define WTALIGN_CENTER	2
// modes, currently a bool!
#define WTMODE_NORMAL	0
#define WTMODE_OUTLINED	1


// Waypoint TYPE definition
#define WPT_UNKNOWN		0
#define WPT_AIRPORT		1
#define WPT_OUTLANDING		2
#define WPT_TURNPOINT		3

// Reserved Waypoints positions
// ATTENTION!! Adjust also in Defines.h   NUMRESWP !!!
#define RESWP_INVALIDNUMBER	1.23	// an invalid number for latitude, longitude, altitude etc.
#define RESWP_TAKEOFF		0
#define RESWP_TAKEOFF_NAME	"TAKEOFF"
#if NEWVIRTUALS
#define RESWP_LASTTHERMAL	1
#define RESWP_LASTTHERMAL_NAME	"THERMAL"
#define RESWP_TEAMMATE		2
#define RESWP_TEAMMATE_NAME	"TEAMMATE"
#define RESWP_FLARMTARGET	3
#define RESWP_FLARMTARGET_NAME	"TARGET"
#define RESWP_END		3
#else
#define RESWP_END		0
#endif
// WayPointList .Number int identifier 
#define RESWP_ID		9999




