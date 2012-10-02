/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"


//
// invertable is used coped with LKTextBlack: if both are active, then text is forced reversed
//
void MapWindow::LKWriteText(HDC hDC, const TCHAR* wText, int x, int y, 
                          int maxsize, const bool mode, const short align, COLORREF rgb_text, bool invertable ) {

	SIZE tsize;
	if (maxsize==0) maxsize=_tcslen(wText);
  
	GetTextExtentPoint(hDC, wText, maxsize, &tsize);

	// by default, LK8000 is white on black, i.e. inverted
	if ((!INVERTCOLORS) || (LKTextBlack&&invertable)) switch(rgb_text) { // 091110
		case RGB_WHITE:
			rgb_text=RGB_BLACK;
			break;
		case RGB_BLACK:
			rgb_text=RGB_WHITE;
			break;
		case RGB_SBLACK:		// FIXED MISSING 100511
			rgb_text=RGB_SWHITE;
			break;
		case RGB_LIGHTGREEN:
			// rgb_text=RGB_DARKBLUE; 100915
			rgb_text=RGB_DARKGREEN;
			break;
		case RGB_LIGHTRED:
			rgb_text=RGB_DARKRED;
			break;
		case RGB_LIGHTYELLOW:
			rgb_text=RGB_DARKYELLOW;
			break;
		case RGB_SWHITE:
			rgb_text=RGB_SBLACK;
			break;
		case RGB_AMBER:
			rgb_text=RGB_ORANGE;
			break;
		case RGB_PETROL:
			rgb_text=RGB_ICEWHITE;
			break;
	}

	switch(align) {
		case WTALIGN_RIGHT:
			x -= tsize.cx;
			break;
		case WTALIGN_CENTER:
			x -= tsize.cx/2;
			y -= tsize.cy/2;
			break;
	}
//rgb_text=RGB_MAGENTA;

	switch(mode) {
		case  WTMODE_OUTLINED:
			switch (rgb_text ) {
				// Here we invert colors, looking at the foreground. The trick is that the foreground
				// colour is slightly different white to white, in order to understand how to invert it
				// correctly!
				case RGB_BLACK:
				//case RGB_SWHITE:
					// text black, light background
					SetTextColor(hDC,RGB_WHITE);
					break;
				case RGB_SWHITE:  
					SetTextColor(hDC,RGB_SBLACK);
					break;
				case RGB_SBLACK:
					SetTextColor(hDC,RGB_SWHITE);
					break;
				case RGB_DARKBLUE:
					SetTextColor(hDC,RGB_WHITE);
					break;
				case RGB_GREEN:
					SetTextColor(hDC,RGB_BLACK);
					break;
				case RGB_PETROL:
				case RGB_DARKGREY:
				case RGB_VDARKGREY:
				case RGB_DARKGREEN:
					SetTextColor(hDC,RGB_WHITE);
					break;
				case RGB_ICEWHITE:  
					SetTextColor(hDC,RGB_DARKBLUE);
					break;
				case RGB_WHITENOREV:
					SetTextColor(hDC,RGB_BLACK);
					break;
				case RGB_AMBERNOREV:
					SetTextColor(hDC,RGB_BLACK);
					break;
				default:
					// this is the default also for white text. Normally we are writing on a 
					// not-too-light background
					SetTextColor(hDC,RGB_BLACK);
					break;
			}
				

#if (WINDOWSPC>0)
			ExtTextOut(hDC, x+1, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x+2, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-1, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-2, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y+1, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y-1, 0, NULL, wText, maxsize, NULL);

			if (ScreenSize == (ScreenSize_t)ss800x480) {
				ExtTextOut(hDC, x, y+2, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x, y-2, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x-3, y, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x+3, y, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x, y+3, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x, y-3, 0, NULL, wText, maxsize, NULL); 
			}

			SetTextColor(hDC,rgb_text); 
			ExtTextOut(hDC, x, y, 0, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK); 
#else

			ExtTextOut(hDC, x+2, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x+1, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-1, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-2, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y+1, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y-1, ETO_OPAQUE, NULL, wText, maxsize, NULL);

			if (ScreenSize == (ScreenSize_t)ss800x480) {
				ExtTextOut(hDC, x+3, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x-3, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y+2, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y-2, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y+3, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y-3, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			}

			SetTextColor(hDC,rgb_text);

			ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK);
#endif
			break;

		case WTMODE_NORMAL:

#if (WINDOWSPC>0)
			SetTextColor(hDC,rgb_text); 
			ExtTextOut(hDC, x, y, 0, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK); 
#else
			SetTextColor(hDC,rgb_text); 
      			ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK); 
#endif
			break;

	}

//	SelectObject(hDC, hbOld);

	return;

}

//
// Box black, text white, or inverted.
// Clip region is normally MapRect for forcing writing on any part of the screen.
// Or DrawRect for the current terrain area.
// Or, of course, anything else.
// A note about DrawRect: in main moving map, DrawRect is the part of screen excluding BottomBar, 
// when the bottom bar is opaque. So choose carefully.
//
void MapWindow::LKWriteBoxedText(HDC hDC, RECT *clipRect, const TCHAR* wText, int x, int y, int maxsize, const short align ) {

	SIZE tsize;
	if (maxsize==0) maxsize=_tcslen(wText);
  
	GetTextExtentPoint(hDC, wText, maxsize, &tsize);
	short vy;
	switch(align) {
		case WTALIGN_LEFT:
			vy=y+tsize.cy+NIBLSCALE(2)+1;
			if (vy>=clipRect->bottom) return;
			Rectangle(hDC, x+tsize.cx+NIBLSCALE(8), vy, x, y);
			x += NIBLSCALE(4);
			break;
		case WTALIGN_RIGHT:
			vy=y+tsize.cy+NIBLSCALE(2)+1;
			if (vy>=clipRect->bottom) return;
			Rectangle(hDC, x-tsize.cx-NIBLSCALE(8), vy, x, y);
			x -= (tsize.cx+NIBLSCALE(4));
			break;
		case WTALIGN_CENTER:
			vy=y+(tsize.cy/2)+NIBLSCALE(1)+1;
			if (vy>=clipRect->bottom) return;
			Rectangle(hDC, 
				x-(tsize.cx/2)-NIBLSCALE(4), 
				y-(tsize.cy/2)-NIBLSCALE(1)-1,
				x+(tsize.cx/2)+NIBLSCALE(4), 
				vy);
			x -= (tsize.cx/2);
			// just a trick to avoid calculating:
			// y -= ((tsize.cy/2)+NIBLSCALE(1));
			y -= (vy-y);
			break;
	}
	y += NIBLSCALE(1);

	if (INVERTCOLORS)
		SetTextColor(hDC,RGB_WHITE); 
	else
		SetTextColor(hDC,RGB_BLACK); 

	ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);

	SetTextColor(hDC,RGB_BLACK); 
	
	return;

}

