/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

  $Id$
*/
#include "externs.h"
#include "InfoBoxLayout.h"
#include "InputEvents.h"
#include "RGB.h"
#include <array>
#include <algorithm>
#include <functional>
#include "Window/WndMain.h"
#include "Window/WndTextLabel.h"
#include "ScreenGeometry.h"
using std::placeholders::_1;

class MenuButton : public WndTextLabel {
public:
    MenuButton() : _MenuId(~0), _EnableMenu(), _LButtonDown() {

    }

    void SetMenuId(unsigned MenuId) { _MenuId = MenuId; }

    void EnableMenu(bool Enable) { _EnableMenu = Enable; }
    bool IsMenuEnabled() { return _EnableMenu; }

protected:
    unsigned _MenuId;
    bool _EnableMenu;
    bool _LButtonDown;

    virtual bool OnLButtonDown(const POINT& Pos) {
        _LButtonDown = true;
        return true;
    }
    
    virtual bool OnLButtonUp(const POINT& Pos) {
        if(_EnableMenu && _LButtonDown) {
            InputEvents::processButton(_MenuId);
        }
        _LButtonDown = false;
        return true;
    }
};

static std::array<MenuButton, NUMBUTTONLABELS> MenuButtons;

void ButtonLabel::GetButtonPosition(unsigned i, const RECT& rc,
				    int *x, int *y,
				    int *sizex, int *sizey) {

  if (1) {	// always calculate positions in LK 2.4

    int hwidth = (rc.right-rc.left)/4;
    int hheight = (rc.bottom-rc.top)/4;

    switch (ScreenLandscape) {

	case 0: // PORTRAIT MODE ONLY

		if (i==0) {
			*sizex = NIBLSCALE(52);
			*sizey = NIBLSCALE(37);
			*x = rc.left-(*sizex); // JMW make it offscreen for now
			*y = (rc.bottom-(*sizey));
		} else if (i>=10) {
				*sizex = NIBLSCALE(57);
				*sizey = NIBLSCALE(45);
				*x = rc.left+2+hwidth*(i-10);
				*y = rc.top+NIBLSCALE(2);
		} else {
			if (i<5) {
				// BOTTOM MENU NAV INFO CONFIG DISPLAY
				//*sizex = NIBLSCALE(52);
				//*sizey = NIBLSCALE(40);
				*sizex = NIBLSCALE(57);
				*sizey = NIBLSCALE(45);
				*x = rc.left+2+hwidth*(i-1);
				*y = (rc.bottom-(*sizey)-NIBLSCALE(2));
			} else {
				*sizex = NIBLSCALE(80);
				*sizey = NIBLSCALE(40);
				*x = rc.right-(*sizex);
				int k = rc.bottom-rc.top-NIBLSCALE(46); 

				*y = (rc.top+(i-5)*k/6+(*sizey/2+NIBLSCALE(3)));
			}
		}
		break;

	case 1: // LANDSCAPE MODE ONLY

		hwidth = (rc.right-rc.left)/5;
		hheight = (rc.bottom-rc.top)/(5);

		int hoffset, voffset;

		// menu buttons 0-4
		// LEFT menu 0 is unused
		if (i==0) {
			*sizex = NIBLSCALE(52);
			*sizey = NIBLSCALE(20);
			*x = rc.left-(*sizex); // JMW make it offscreen for now
			*y = (rc.top);
		} else {
			if (i<5) {
				// RIGHT menu items ( NAV DISPLAY CONFIG INFO ... )
				// new buttons landscape on the right 
				switch(ScreenSize) {
					case ss800x480:
						*sizex = NIBLSCALE(75);
						*sizey = 78;
						voffset= 40;
						// offset distance from right margin
						hoffset=*sizex+5;
						break;
					case ss640x480:
						*sizex = NIBLSCALE(60);
						*sizey = 72-2;
						voffset= 40;
						hoffset=*sizex+3;
						break;
					case ss480x272:
						*sizex = NIBLSCALE(77);
						*sizey = 48-1;
						voffset= 26; 
						hoffset=*sizex+3;
						break;
					case ss480x234:
						*sizex = NIBLSCALE(82);
						*sizey = 41-1;
						voffset= 20;
						hoffset=*sizex+5;
						break;
					case ss320x240:
						*sizex = NIBLSCALE(58);
						*sizey = 34-1;
						voffset= 16;
						hoffset=*sizex+1;
						break;
					case ss400x240:
						*sizex = NIBLSCALE(75);
						*sizey = 39;
						voffset= 20;
						hoffset=*sizex+2;
						break;
					default:
						//
						// AUTORES ACCOMPLISHED
						//
						LKASSERT(ScreenGeometry>0);
						switch(ScreenGeometry) {
						    case SCREEN_GEOMETRY_43:
						        *sizex = NIBLSCALE(60);
					      	        *sizey = (int)(70.0*Screen0Ratio);
						        voffset= (int)(40.0*Screen0Ratio);
						        hoffset=*sizex+3;
							break;

						    case SCREEN_GEOMETRY_53:
						        *sizex = NIBLSCALE(75);
					      	        *sizey = (int)(78.0*Screen0Ratio);
						        voffset= (int)(40.0*Screen0Ratio);
						        hoffset=*sizex+5;
						        break;

						    case SCREEN_GEOMETRY_169:
						    default:
						        *sizex = NIBLSCALE(77);
					      	        *sizey = (int)(47.0*Screen0Ratio);
						        voffset= (int)(26.0*Screen0Ratio);
						        hoffset=*sizex+3;
						        break;
						}
						break;
				}
				*x = rc.right-3-hoffset;	
				*y = (rc.top+hheight*i-(*sizey)/2)-voffset;
			} else {
				// BOTTOM MENUS

				switch(ScreenSize) {
					case ss800x480:
						*sizex = NIBLSCALE(78);
						*sizey = 80-2;
						hoffset=NIBLSCALE(1);
						// distance from bottom
						voffset=1;
						break;
					case ss640x480:
						*sizex = NIBLSCALE(62);
						*sizey = 72-2;
						hoffset= 2;
						voffset=1;
						break;
					case ss480x272:
						*sizex = NIBLSCALE(82);
						*sizey = 48-1;
						hoffset= 2;
						voffset=1;
						break;
					case ss480x234:
						*sizex = NIBLSCALE(86);
						*sizey = 41-1;
						hoffset= 5;
						voffset=1;
						break;
					case ss320x240:
						*sizex = NIBLSCALE(60);
						*sizey = 34-1;
						hoffset= 2;
						voffset=1;
						break;
					case ss400x240:
						*sizex = NIBLSCALE(78);
						*sizey = 40-1;
						hoffset=NIBLSCALE(1);
						// distance from bottom
						voffset=1;
						break;
					default:
						//
						// AUTORES ACCOMPLISHED
						//
						LKASSERT(ScreenGeometry>0);
						switch(ScreenGeometry) {
						    case SCREEN_GEOMETRY_43:
						        *sizex = NIBLSCALE(62);
					      	        *sizey = (int)(70.0*Screen0Ratio);
						        voffset= 1;
						        hoffset= 2;
							break;

						    case SCREEN_GEOMETRY_53:
						        *sizex = NIBLSCALE(78);
					      	        *sizey = (int)(78.0*Screen0Ratio);
						        voffset= 1;
						        hoffset= NIBLSCALE(1);
						        break;

						    case SCREEN_GEOMETRY_169:
						    default:
						        *sizex = NIBLSCALE(82);
					      	        *sizey = (int)(47.0*Screen0Ratio);
						        voffset= 2;
						        hoffset= 1;
						        break;
						}
						break;
				}
				if (i>=10) {
					*x = rc.left+hoffset+hwidth*(i-10);
					*y = rc.top+NIBLSCALE(1);
				} else {
					*x = rc.left+hwidth*(i-5)+ hoffset;
					*y = (rc.bottom-(*sizey))- voffset; 
				}
			}
		}
		break;

	} 
  }
}

void ButtonLabel::CreateButtonLabels(const RECT& rc) {
    int x,y,cx,cy;

    for (unsigned i = 0; i < MenuButtons.size(); ++i) {
        GetButtonPosition(i, rc, &x, &y, &cx, &cy);

        MenuButtons[i].Create(&MainWindow, (RECT){ x, y, x + cx, y + cy });
        if(MenuButtons[i].IsDefined()) {
            MenuButtons[i].SetTextColor(RGB_BLACK);
            MenuButtons[i].SetBkColor(RGB_BUTTONS);
            MenuButtons[i].SetMenuId(i);
        }
    }
}

void ButtonLabel::SetFont(FontReference Font) {
    std::for_each(MenuButtons.begin(), MenuButtons.end(), std::bind(&MenuButton::SetFont, _1, Font));
}

void ButtonLabel::Destroy() {
    std::for_each(MenuButtons.begin(), MenuButtons.end(), std::bind(&MenuButton::Destroy, _1));
}

void ButtonLabel::SetLabelText(unsigned idx, const TCHAR *text) {

    if (idx >= MenuButtons.size()) {
        return;
    }
/*
    // don't try to draw if window isn't initialised
    if (hWndButtonWindow[index] == NULL) return;
*/

    if ((text == NULL) || (*text == _T('\0')) || (*text == _T(' '))) {
        MenuButtons[idx].SetVisible(false);
        MenuButtons[idx].Enable(false);
    } else {
#ifdef LXMINIMAP
        if (InputEvents::getSelectedButtonIndex() == index) {
            MenuButtons[index].SetBkColor(RGB_DARKYELLOW2);
        }
#endif

        TCHAR s[100];
        bool greyed = ExpandMacros(text, s, array_size(s));
        if (greyed) {
            MenuButtons[idx].SetTextColor(LKColor(0x80, 0x80, 0x80));
            MenuButtons[idx].EnableMenu(false);
        } else {
            MenuButtons[idx].SetTextColor(RGB_BLACK);
            MenuButtons[idx].EnableMenu(true);
        }

        if ((s[0]==_T('\0'))||(s[0]==_T(' '))) {
            MenuButtons[idx].SetVisible(false);
            MenuButtons[idx].Enable(false);
        } else {
            MenuButtons[idx].SetWndText(gettext(s));
            MenuButtons[idx].SetTopWnd();
            MenuButtons[idx].SetVisible(true);
            MenuButtons[idx].Enable(true);
        }
    }
}

bool ButtonLabel::IsVisible(unsigned idx) {
    return (idx < MenuButtons.size() ? MenuButtons[idx].IsVisible() : false);
}

bool ButtonLabel::IsEnabled(unsigned idx) {
    return (idx < MenuButtons.size() ? MenuButtons[idx].IsMenuEnabled() : false);
}
