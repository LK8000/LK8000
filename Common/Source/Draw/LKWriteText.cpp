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
void MapWindow::LKWriteText(LKSurface& Surface, const TCHAR* wText, int x, int y,
                          int maxsize, const bool lwmode, const short align, const LKColor& rgb_text, bool invertable ) {

	SIZE tsize;
	if (maxsize==0) maxsize=_tcslen(wText);
  
	Surface.GetTextSize(wText, maxsize, &tsize);
    LKColor textColor = rgb_text;
	// by default, LK8000 is white on black, i.e. inverted
	if ((!INVERTCOLORS) || (LKTextBlack&&invertable)) {
        switch(rgb_text) { // 091110
		case RGB_WHITE:
			textColor=RGB_BLACK;
			break;
		case RGB_BLACK:
			textColor=RGB_WHITE;
			break;
		case RGB_SBLACK:		// FIXED MISSING 100511
			textColor=RGB_SWHITE;
			break;
		case RGB_LIGHTGREEN:
			// rgb_text=RGB_DARKBLUE; 100915
			textColor=RGB_DARKGREEN;
			break;
		case RGB_LIGHTRED:
			textColor=RGB_DARKRED;
			break;
		case RGB_LIGHTYELLOW:
			textColor=RGB_DARKYELLOW;
			break;
		case RGB_SWHITE:
			textColor=RGB_SBLACK;
			break;
		case RGB_AMBER:
			textColor=RGB_ORANGE;
			break;
		case RGB_PETROL:
			textColor=RGB_ICEWHITE;
			break;
        }
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
bool moreoutline=false;

	switch(lwmode) {
		case  WTMODE_OUTLINED:
			//
			// First set a background color for outlining
			// black outline requires more width, to gain contrast.
			//
			switch (textColor ) {
				// Here we invert colors, looking at the foreground. The trick is that the foreground
				// colour is slightly different white to white, in order to understand how to invert it
				// correctly!
				case RGB_BLACK:
				//case RGB_SWHITE:
					// text black, light background
					Surface.SetTextColor(RGB_WHITE);
					
					break;
				case RGB_SWHITE:  
					Surface.SetTextColor(RGB_SBLACK);
					moreoutline=true;
					break;
				case RGB_SBLACK:
					Surface.SetTextColor(RGB_SWHITE);
					break;
				case RGB_DARKBLUE:
					Surface.SetTextColor(RGB_WHITE);
					break;
				case RGB_GREEN:
					Surface.SetTextColor(RGB_BLACK);
					moreoutline=true;
					break;
				case RGB_PETROL:
				case RGB_DARKGREY:
				case RGB_VDARKGREY:
				case RGB_DARKGREEN:
					Surface.SetTextColor(RGB_WHITE);
					break;
				case RGB_ICEWHITE:  
					Surface.SetTextColor(RGB_DARKBLUE);
					moreoutline=true;
					break;
				case RGB_WHITENOREV:
					Surface.SetTextColor(RGB_BLACK);
					moreoutline=true;
					break;
				case RGB_AMBERNOREV:
					Surface.SetTextColor(RGB_BLACK);
					moreoutline=true;
					break;
				default:
					// this is the default also for white text. Normally we are writing on a 
					// not-too-light background
					Surface.SetTextColor(RGB_BLACK);
					moreoutline=true;
					break;
			}
				

#if 1
			// 
			// Simplified, shadowing better and faster
			// ETO_OPAQUE not necessary since we pass a NULL rect
			//
			Surface.DrawText(x-1, y-1, wText, maxsize);
			Surface.DrawText(x-1, y+1, wText, maxsize);
			Surface.DrawText(x+1, y-1, wText, maxsize);
			Surface.DrawText(x+1, y+1, wText, maxsize);

			// SetTextColor(hDC,RGB_GREY);  // This would give an Emboss effect
			// Surface.DrawText(x, y+2, 0, wText, maxsize);

			if (moreoutline) {
				Surface.DrawText(x-2, y, wText, maxsize);
				Surface.DrawText(x+2, y, wText, maxsize);
				Surface.DrawText(x, y-2, wText, maxsize);
				Surface.DrawText(x, y+2, wText, maxsize);
			}

			Surface.SetTextColor(textColor);
			Surface.DrawText(x, y, wText, maxsize);
			Surface.SetTextColor(RGB_BLACK);
#endif

			break;

		case WTMODE_NORMAL:

			Surface.SetTextColor(textColor);
            Surface.DrawText(x, y, wText, maxsize);
			Surface.SetTextColor(RGB_BLACK);
			break;

	}

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
void MapWindow::LKWriteBoxedText(LKSurface& Surface, const RECT& clipRect, const TCHAR* wText, int x, int y, int maxsize, const short align ,
	const LKColor& dir_rgb, const LKColor& inv_rgb  ) {

  LKColor oldTextColor = Surface.SetTextColor(INVERTCOLORS?dir_rgb:inv_rgb);

  SIZE tsize;
  if (maxsize==0) maxsize=_tcslen(wText);
  
  Surface.GetTextSize(wText, maxsize, &tsize);
  short vy;
  switch(align) {
	case WTALIGN_LEFT:
		vy=y+tsize.cy+NIBLSCALE(2)+1;
		if (vy>=clipRect.bottom) return;
		Surface.Rectangle(x+tsize.cx+NIBLSCALE(8), vy, x, y);
        x += NIBLSCALE(4);
		break;
	case WTALIGN_RIGHT:
		vy=y+tsize.cy+NIBLSCALE(2)+1;
		if (vy>=clipRect.bottom) return;
        Surface.Rectangle(x-tsize.cx-NIBLSCALE(8), vy, x, y);
        x -= (tsize.cx+NIBLSCALE(4));
		break;
	case WTALIGN_CENTER:
		vy=y+(tsize.cy/2)+NIBLSCALE(1)+1;
		if (vy>=clipRect.bottom) return;
		Surface.Rectangle(x-(tsize.cx/2)-NIBLSCALE(4),
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


  Surface.DrawText(x, y, wText, maxsize);

  //SetTextColor(hDC,RGB_BLACK);   THIS WAS FORCED BLACk SO FAR 121005
  Surface.SetTextColor(oldTextColor);
}
