/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: RGB.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
#ifndef _RGB_H_
#define _RGB_H_

#include "Screen/LKColor.h"

// MapWindow::TextColor() supported colours
constexpr	LKColor RGB_BLACK(0,0,0);
constexpr	LKColor RGB_WHITE(255,255,255);
constexpr	LKColor RGB_WHITENOREV(254,254,254);		// this color will not be reversed
constexpr	LKColor RGB_BLUE(0x00,0x00,0xff);
constexpr	LKColor RGB_LIGHTBLUE(0x81,0xd4,0xfa);
constexpr	LKColor RGB_GREY(0x99,0x99,0x99);
constexpr	LKColor RGB_LIGHTGREY(0xcc,0xcc,0xcc);
constexpr	LKColor RGB_VDARKGREY(0xf,0xf,0xf);		// was 0x0d
constexpr	LKColor RGB_LIGHTORANGE(255,184,51);
constexpr	LKColor RGB_ORANGE(255,165,0);
constexpr	LKColor RGB_GREEN(0,255,0);
constexpr	LKColor RGB_DARKGREEN(40,80,40);
constexpr	LKColor RGB_LIGHTGREEN(0xc2,0xff,0xc2);
constexpr	LKColor RGB_RED(255,0,0);
constexpr	LKColor RGB_LIGHTRED(0xff,0xc2,0xc2);
constexpr	LKColor RGB_YELLOW(255,255,0);
constexpr	LKColor RGB_LIGHTYELLOW(0xff,0xff,0xc2);
constexpr	LKColor RGB_CYAN(0,255,255);
constexpr	LKColor RGB_LIGHTCYAN(0xc2,0xff,0xff);
constexpr	LKColor RGB_MAGENTA(255,0,255);
// unsupported
constexpr	LKColor RGB_AMBER(0xff,0xbe,0x00);
constexpr	LKColor RGB_AMBERNOREV(0xff,0xbd,0x01);		// amber not reversed
constexpr	LKColor RGB_INDIGO(0x4b,0x00,0x82);
constexpr	LKColor RGB_DARKRED(200,0,0);
constexpr	LKColor RGB_VDARKRED(160,0,0);
constexpr	LKColor RGB_DARKBLUE(0x00,0x00,160);
constexpr	LKColor RGB_DARKYELLOW(255,100,0);
// dy2 for vario
constexpr	LKColor RGB_DARKYELLOW2(255,215,0);
constexpr	LKColor RGB_ICEWHITE(0xf0,0xf0,0xf0);
constexpr	LKColor RGB_DARKWHITE(0xd9,0xd9,0xd9);
// unsupported, special use
constexpr	LKColor RGB_SBLACK(0,0,1);
constexpr	LKColor RGB_SWHITE(255,255,254);
// background mapspace
constexpr	LKColor RGB_MDARK(RGB_BLACK);
constexpr	LKColor RGB_MLIGHT(RGB_DARKWHITE);
// background navboxes
constexpr	LKColor RGB_NDARK(RGB_VDARKGREY);
constexpr	LKColor RGB_NLIGHT(RGB_ICEWHITE);
// inverted things on map
constexpr	LKColor RGB_INVDRAW(RGB_LIGHTGREEN);
// background moving map with no terrrain
constexpr	LKColor RGB_LCDGREEN(197,223,194);
constexpr	LKColor RGB_LAKE(88,158,253);
constexpr	LKColor RGB_GGREY(117,160,150);
constexpr	LKColor RGB_LCDDARKGREEN(59,179,158);
constexpr	LKColor RGB_EMERALD(5,139,111);
constexpr	LKColor RGB_DARKSLATE(89,113,125);
constexpr	LKColor RGB_RIFLEGREY(41,52,46);

constexpr	LKColor RGB_MIDDLEGREY(128,128,128);
// Original colors
// MessageBoxX
constexpr	LKColor RGB_BADYELLOW(0xda, 0xdb, 0xab);
// mapwindows crosshair
constexpr	LKColor RGB_DARKGREY(50,50,50);
// task line color, originally darg green
constexpr	LKColor RGB_DGREEN1(0,120,0);
// buttons
// #define RGB_BUTTGREEN	LKColor(0xA0,0xE0,0xA0) // 1.22b button green

// 2.0 button green
constexpr	LKColor RGB_BUTTGREEN	(160,255,190);

// LK8000 colors
constexpr	LKColor RGB_LGREEN1(0xa9,0xda,0xc3);
constexpr	LKColor RGB_LBLUE1(0xa9,0xd9,0xda);
constexpr	LKColor RGB_PETROLGREEN(0,66,66);
constexpr	LKColor RGB_PETROLBLUE(28,67,90);
constexpr	LKColor RGB_PETROL(RGB_PETROLBLUE);

// -------------  PROGRAM COLORS IN USE ---------------------
// MessageBox 
constexpr	LKColor RGB_TASKLINECOL(RGB_DGREEN1);


#define RGB_BUTTONS	      (IsDithered() ? RGB_WHITE : RGB_BUTTGREEN) // Color of Menu Buttons
#define RGB_WINBACKGROUND	(IsDithered() ? RGB_WHITE : RGB_PETROL) // default BackGronud color for WndForm
#define RGB_WINFOREGROUND	(IsDithered() ? RGB_BLACK : RGB_WHITE) // default foreground color for WndForm (text color)
#define RGB_HIGHTLIGHT     (IsDithered() ? RGB_GREY :  RGB_YELLOW) // default color for Active Window
#define RGB_LISTBG         (IsDithered() ? RGB_WHITE : RGB_LCDGREEN)


// The box title background, originally yellow
constexpr	LKColor RGB_MENUTITLEBG(RGB_BLACK);
constexpr	LKColor RGB_MENUTITLEFG(RGB_WHITE);

// LK new colors for windows
// WindowControls,
constexpr	LKColor RGB_LISTHIGHLIGHTBG(133,255,133);
constexpr	LKColor RGB_LISTHIGHLIGHTCORNER(RGB_PETROL);
constexpr	LKColor RGB_LISTFG(RGB_BLACK);
// slider: borders and box
constexpr	LKColor RGB_SCROLLBARBORDER(RGB_BLACK);
constexpr	LKColor RGB_SCROLLBARBOX(RGB_BLACK);

// Config buttons,  BG not yet used, cannot change!
constexpr	LKColor RGB_BUTTONFG(RGB_BLACK);
constexpr	LKColor RGB_BUTTONBG(154,184,195);

#endif // _RGB_H_
