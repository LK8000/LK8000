/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "utils/lookup_table.h"
#include "externs.h"
#include "RGB.h"

// TextColor -> Inverted TextColor
constexpr auto ColorInvert = lookup_table<LKColor, LKColor>({
  {RGB_WHITE, RGB_BLACK},
  {RGB_BLACK, RGB_WHITE},
  {RGB_SBLACK, RGB_SWHITE},
  {RGB_SWHITE, RGB_SBLACK},
  {RGB_LIGHTGREEN, RGB_DARKGREEN},
  {RGB_LIGHTRED, RGB_DARKRED},
  {RGB_LIGHTYELLOW, RGB_DARKYELLOW},
  {RGB_AMBER, RGB_ORANGE},
  {RGB_PETROL, RGB_ICEWHITE}
});

// TextColor -> (OutlineColor , MoreOutline)
constexpr auto ColorOutLine = lookup_table<LKColor, std::pair<LKColor, bool>>({
  {RGB_BLACK, {RGB_WHITE, false}},
  {RGB_SWHITE, {RGB_SBLACK, true}},
  {RGB_SBLACK, {RGB_SWHITE, false}},
  {RGB_DARKBLUE, {RGB_WHITE, false}},
  {RGB_GREEN, {RGB_BLACK, true}},
  {RGB_PETROL, {RGB_WHITE, false}},
  {RGB_DARKGREY, {RGB_WHITE, false}},
  {RGB_VDARKGREY, {RGB_WHITE, false}},
  {RGB_DARKGREEN, {RGB_WHITE, false}},
  {RGB_ICEWHITE, {RGB_DARKBLUE, true}},
  {RGB_WHITENOREV, {RGB_BLACK, true}},
  {RGB_AMBERNOREV, {RGB_BLACK, true}}
});


LKColor MapWindow::GetOutlineColor(LKColor color) {
    return ColorOutLine.get(color, {RGB_BLACK, true}).first;
}

//
// invertable is used coped with LKTextBlack: if both are active, then text is forced reversed
//

void MapWindow::LKWriteText(LKSurface& Surface, const TCHAR* wText, int x, int y,
        const bool lwmode, const short align, const LKColor& rgb_text, bool invertable, RECT* ClipRect) {

    SIZE tsize;

    Surface.GetTextSize(wText, &tsize);
    LKColor textColor = rgb_text;
    // by default, LK8000 is white on black, i.e. inverted
    if ((!INVERTCOLORS) || (LKTextBlack && invertable)) {
        textColor = ColorInvert.get(textColor);
    }

    switch (align) {
        case WTALIGN_RIGHT:
            x -= tsize.cx;
            break;
        case WTALIGN_CENTER:
            x -= tsize.cx / 2;
            y -= tsize.cy / 2;
            break;
    }

    Surface.SetBackgroundTransparent();

    if(lwmode) { // WTMODE_OUTLINED:
        //
        // First set a background color for outlining
        // black outline requires more width, to gain contrast.
        //

        // {RGB_BLACK, true} is the default also for white text. Normally we are writing on a
        // not-too-light background

        auto color_pair = ColorOutLine.get(textColor, {RGB_BLACK, true});

        // Here we invert colors, looking at the foreground. The trick is that the foreground
        // colour is slightly different white to white, in order to understand how to invert it
        // correctly!
        Surface.SetTextColor(color_pair.first);
        bool moreoutline = color_pair.second;

        //
        // Simplified, shadowing better and faster
        // ETO_OPAQUE not necessary since we pass a NULL rect
        //
#ifdef USE_FREETYPE
#warning "to slow, rewrite using freetype outline"
#endif

#if !defined(PNA) || !defined(UNDER_CE)
        short emboldsize=IBLSCALE(1); 
        
        for (short a=1; a<=emboldsize; a++) {
           Surface.DrawText(x - a, y - a, wText, ClipRect);
           Surface.DrawText(x - a, y + a, wText, ClipRect);
           Surface.DrawText(x + a, y - a, wText, ClipRect);
           Surface.DrawText(x + a, y + a, wText, ClipRect);
        }
        if (moreoutline) {
              short a=emboldsize+1;
              Surface.DrawText(x - a, y, wText, ClipRect);
              Surface.DrawText(x + a, y, wText, ClipRect);
              Surface.DrawText(x, y - a, wText, ClipRect);
              Surface.DrawText(x, y + a, wText, ClipRect);
        }
#else
        Surface.DrawText(x - 1, y - 1, wText, ClipRect);
        Surface.DrawText(x - 1, y + 1, wText, ClipRect);
        Surface.DrawText(x + 1, y - 1, wText, ClipRect);
        Surface.DrawText(x + 1, y + 1, wText, ClipRect);

        // SetTextColor(hDC,RGB_GREY);  // This would give an Emboss effect
        // Surface.DrawText(x, y+2, 0, wText, maxsize);

        if (moreoutline) {
            Surface.DrawText(x - 2, y, wText, ClipRect);
            Surface.DrawText(x + 2, y, wText, ClipRect);
            Surface.DrawText(x, y - 2, wText, ClipRect);
            Surface.DrawText(x, y + 2, wText, ClipRect);
        }
#endif

        Surface.SetTextColor(textColor);
        Surface.DrawText(x, y, wText, ClipRect);
        Surface.SetTextColor(RGB_BLACK);

    } else { // WTMODE_NORMAL:

        Surface.SetTextColor(textColor);
        Surface.DrawText(x, y, wText, ClipRect);
        Surface.SetTextColor(RGB_BLACK);

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
void MapWindow::LKWriteBoxedText(LKSurface& Surface, const RECT& clipRect, const TCHAR* wText, int x, int y, const short align ,
	const LKColor& dir_rgb, const LKColor& inv_rgb  ) {

  LKColor oldTextColor = Surface.SetTextColor(INVERTCOLORS?dir_rgb:inv_rgb);

  SIZE tsize;
  Surface.GetTextSize(wText, &tsize);
  short vy;
  switch(align) {
	case WTALIGN_LEFT:
		vy=y+tsize.cy+NIBLSCALE(2)+1;
		if (vy>=clipRect.bottom) return;
		Surface.Rectangle(x, y, x+tsize.cx+NIBLSCALE(8), vy);
        x += NIBLSCALE(4);
		break;
	case WTALIGN_RIGHT:
		vy=y+tsize.cy+NIBLSCALE(2)+1;
		if (vy>=clipRect.bottom) return;
        Surface.Rectangle(x-tsize.cx-NIBLSCALE(8), y, x, vy);
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
  #ifdef __linux__
  y += NIBLSCALE(1)+1;
  #else
  y += NIBLSCALE(1);
  #endif


  Surface.DrawText(x, y, wText);

  //SetTextColor(hDC,RGB_BLACK);   THIS WAS FORCED BLACk SO FAR 121005
  Surface.SetTextColor(oldTextColor);
}
