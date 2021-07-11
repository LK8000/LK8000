/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: RGB.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
#include "Screen/LKColor.h"

// MapWindow::TextColor() supported colours
#define RGB_BLACK	LKColor(0,0,0)
#define RGB_WHITE	LKColor(255,255,255)
#define RGB_WHITENOREV	LKColor(254,254,254)		// this color will not be reversed
#define RGB_BLUE	LKColor(0x00,0x00,0xff)
#define RGB_LIGHTBLUE	LKColor(0x81,0xd4,0xfa)
#define RGB_GREY	LKColor(0x99,0x99,0x99)
#define RGB_LIGHTGREY	LKColor(0xcc,0xcc,0xcc)
#define RGB_VDARKGREY	LKColor(0xf,0xf,0xf)		// was 0x0d
#define RGB_LIGHTORANGE	LKColor(255,184,51)
#define RGB_ORANGE	LKColor(255,165,0)
#define RGB_GREEN	LKColor(0,255,0)
#define RGB_DARKGREEN	LKColor(40,80,40)
#define RGB_LIGHTGREEN	LKColor(0xc2,0xff,0xc2)
#define RGB_RED		LKColor(255,0,0)
#define RGB_LIGHTRED	LKColor(0xff,0xc2,0xc2)
#define RGB_YELLOW	LKColor(255,255,0)
#define RGB_LIGHTYELLOW	LKColor(0xff,0xff,0xc2)
#define RGB_CYAN	LKColor(0,255,255)
#define RGB_LIGHTCYAN	LKColor(0xc2,0xff,0xff)
#define RGB_MAGENTA	LKColor(255,0,255)
// unsupported
#define RGB_AMBER	LKColor(0xff,0xbe,0x00)
#define RGB_AMBERNOREV	LKColor(0xff,0xbd,0x01)		// amber not reversed
#define RGB_INDIGO	LKColor(0x4b,0x00,0x82)
#define RGB_DARKRED	LKColor(200,0,0)
#define RGB_VDARKRED	LKColor(160,0,0)
#define RGB_DARKBLUE	LKColor(0x00,0x00,160)
#define RGB_DARKYELLOW	LKColor(255,100,0)
// dy2 for vario
#define RGB_DARKYELLOW2	LKColor(255,215,0)
#define RGB_ICEWHITE	LKColor(0xf0,0xf0,0xf0)
#define RGB_DARKWHITE	LKColor(0xd9,0xd9,0xd9)
// unsupported, special use
#define RGB_SBLACK	LKColor(0,0,1)
#define RGB_SWHITE	LKColor(255,255,254)
// background mapspace
#define RGB_MDARK	RGB_BLACK
#define RGB_MLIGHT	RGB_DARKWHITE
// background navboxes
#define RGB_NDARK	RGB_VDARKGREY
#define RGB_NLIGHT	RGB_ICEWHITE
// inverted things on map
#define RGB_INVDRAW	RGB_LIGHTGREEN
// background moving map with no terrrain
#define RGB_LCDGREEN	LKColor(197,223,194)
#define RGB_LAKE	LKColor(88,158,253)
#define RGB_GGREY	LKColor(117,160,150)
#define RGB_LCDDARKGREEN	LKColor(59,179,158)
#define RGB_EMERALD	LKColor(5,139,111)
#define RGB_DARKSLATE	LKColor(89,113,125)
#define RGB_RIFLEGREY	LKColor(41,52,46)

#define RGB_MIDDLEGREY	LKColor(128,128,128)
// Original colors
// MessageBoxX
#define RGB_BADYELLOW	RGB(0xda, 0xdb, 0xab)
// mapwindows crosshair
#define RGB_DARKGREY	LKColor(50,50,50)
// task line color, originally darg green
#define RGB_DGREEN1	LKColor(0,120,0)
// buttons
// #define RGB_BUTTGREEN	LKColor(0xA0,0xE0,0xA0) // 1.22b button green

// 2.0 button green
#define RGB_BUTTGREEN	LKColor(160,255,190)

// LK8000 colors
#define RGB_LGREEN1	LKColor(0xa9,0xda,0xc3)
#define RGB_LBLUE1	LKColor(0xa9,0xd9,0xda)
#define RGB_PETROLGREEN	LKColor(0,66,66)
#define RGB_PETROLBLUE	LKColor(28,67,90)
#define RGB_PETROL	RGB_PETROLBLUE

// -------------  PROGRAM COLORS IN USE ---------------------
// MessageBox 
#define RGB_TASKLINECOL RGB_DGREEN1


#define RGB_BUTTONS	      (IsDithered() ? RGB_WHITE : RGB_BUTTGREEN) // Color of Menu Buttons
#define RGB_WINBACKGROUND	(IsDithered() ? RGB_WHITE : RGB_PETROL) // default BackGronud color for WndForm
#define RGB_WINFOREGROUND	(IsDithered() ? RGB_BLACK : RGB_WHITE) // default foreground color for WndForm (text color)
#define RGB_HIGHTLIGHT     (IsDithered() ? RGB_GREY :  RGB_YELLOW) // default color for Active Window
#define RGB_LISTBG         (IsDithered() ? RGB_WHITE : RGB_LCDGREEN)


// The box title background, originally yellow
#define RGB_MENUTITLEBG	RGB_BLACK
#define RGB_MENUTITLEFG RGB_WHITE

// LK new colors for windows
// WindowControls,
#define RGB_LISTHIGHLIGHTBG	LKColor(133,255,133)
#define RGB_LISTHIGHLIGHTCORNER RGB_PETROL
#define RGB_LISTFG		RGB_BLACK
// slider: borders and box
#define RGB_SCROLLBARBORDER	RGB_BLACK
#define RGB_SCROLLBARBOX	RGB_BLACK

// Config buttons,  BG not yet used, cannot change!
#define RGB_BUTTONFG		RGB_BLACK
#define RGB_BUTTONBG		LKColor(154,184,195)

