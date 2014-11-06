/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include <functional>

using std::placeholders::_1;

// TextColor -> Inverted TextColor
typedef std::pair<LKColor, LKColor> Color2Color_t;
static const Color2Color_t ColorInvert [] = {
    std::make_pair(RGB_WHITE, RGB_BLACK),
    std::make_pair(RGB_BLACK, RGB_WHITE),
    std::make_pair(RGB_SBLACK, RGB_SWHITE),
    std::make_pair(RGB_SWHITE, RGB_SBLACK),
    std::make_pair(RGB_LIGHTGREEN, RGB_DARKGREEN),
    std::make_pair(RGB_LIGHTRED, RGB_DARKRED),
    std::make_pair(RGB_LIGHTYELLOW, RGB_DARKYELLOW),
    std::make_pair(RGB_AMBER, RGB_ORANGE),
    std::make_pair(RGB_PETROL, RGB_ICEWHITE)
};

// TextColor -> (OutlineColor , MoreOutline)
typedef std::pair<LKColor, std::pair<LKColor, bool> > Color2Outline_t;
static const Color2Outline_t ColorOutLine [] = {
    std::make_pair(RGB_BLACK, std::make_pair(RGB_WHITE, false)),
    std::make_pair(RGB_SWHITE, std::make_pair(RGB_SBLACK, true)),
    std::make_pair(RGB_SBLACK, std::make_pair(RGB_SWHITE, false)),
    std::make_pair(RGB_DARKBLUE, std::make_pair(RGB_WHITE, false)),
    std::make_pair(RGB_GREEN, std::make_pair(RGB_BLACK, true)),
    std::make_pair(RGB_PETROL, std::make_pair(RGB_WHITE, false)),
    std::make_pair(RGB_DARKGREY, std::make_pair(RGB_WHITE, false)),
    std::make_pair(RGB_VDARKGREY, std::make_pair(RGB_WHITE, false)),
    std::make_pair(RGB_DARKGREEN, std::make_pair(RGB_WHITE, false)),
    std::make_pair(RGB_ICEWHITE, std::make_pair(RGB_DARKBLUE, true)),
    std::make_pair(RGB_WHITENOREV, std::make_pair(RGB_BLACK, true)),
    std::make_pair(RGB_AMBERNOREV, std::make_pair(RGB_BLACK, true))
};

//
// invertable is used coped with LKTextBlack: if both are active, then text is forced reversed
//

void MapWindow::LKWriteText(LKSurface& Surface, const TCHAR* wText, int x, int y,
        int maxsize, const bool lwmode, const short align, const LKColor& rgb_text, bool invertable) {

    SIZE tsize;
    if (maxsize == 0) maxsize = _tcslen(wText);

    Surface.GetTextSize(wText, maxsize, &tsize);
    LKColor textColor = rgb_text;
    // by default, LK8000 is white on black, i.e. inverted
    if ((!INVERTCOLORS) || (LKTextBlack && invertable)) {
        const Color2Color_t* It = std::find_if(std::begin(ColorInvert), std::end(ColorInvert),
                                                std::bind(
                                                    std::equal_to< Color2Color_t::first_type >(),
                                                    std::bind(&Color2Color_t::first, _1), textColor));

        if (It != std::end(ColorInvert)) {
            textColor = It->second;
        }
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
    //rgb_text=RGB_MAGENTA;
    bool moreoutline = false;

    switch (lwmode) {
        case WTMODE_OUTLINED:
        {
            //
            // First set a background color for outlining
            // black outline requires more width, to gain contrast.
            //

            const Color2Outline_t* It = std::find_if(std::begin(ColorOutLine), std::end(ColorOutLine),
                                                        std::bind(
                                                            std::equal_to<Color2Outline_t::first_type>(),
                                                            std::bind(&Color2Outline_t::first, _1), textColor));
            if (It != std::end(ColorOutLine)) {
                // Here we invert colors, looking at the foreground. The trick is that the foreground
                // colour is slightly different white to white, in order to understand how to invert it
                // correctly!
                Surface.SetTextColor(It->second.first);
                moreoutline = It->second.second;
            } else {
                // this is the default also for white text. Normally we are writing on a 
                // not-too-light background                
                Surface.SetTextColor(RGB_BLACK);
                moreoutline = true;
            }

            // 
            // Simplified, shadowing better and faster
            // ETO_OPAQUE not necessary since we pass a NULL rect
            //
            Surface.DrawText(x - 1, y - 1, wText, maxsize);
            Surface.DrawText(x - 1, y + 1, wText, maxsize);
            Surface.DrawText(x + 1, y - 1, wText, maxsize);
            Surface.DrawText(x + 1, y + 1, wText, maxsize);

            // SetTextColor(hDC,RGB_GREY);  // This would give an Emboss effect
            // Surface.DrawText(x, y+2, 0, wText, maxsize);

            if (moreoutline) {
                Surface.DrawText(x - 2, y, wText, maxsize);
                Surface.DrawText(x + 2, y, wText, maxsize);
                Surface.DrawText(x, y - 2, wText, maxsize);
                Surface.DrawText(x, y + 2, wText, maxsize);
            }

            Surface.SetTextColor(textColor);
            Surface.DrawText(x, y, wText, maxsize);
            Surface.SetTextColor(RGB_BLACK);
            break;
        }
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
