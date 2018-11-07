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
#include "dlgTools.h"
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

    bool OnLButtonDblClick(const POINT& Pos) override {
        _LButtonDown = true;
        return true;
    }

    bool OnLButtonDown(const POINT& Pos) override {
        _LButtonDown = true;
        return true;
    }
    
    bool OnLButtonUp(const POINT& Pos) override {
        if(_EnableMenu && _LButtonDown) {
            InputEvents::processButton(_MenuId);
        }
        _LButtonDown = false;
        return true;
    }
};


/*
 * 
 * 
 * Landscape :
 * 
 *                     0      1      2      3      4
 *                 +------+------+------+------+------+
 *               0 |  10  |  11  |  12  |  13  |   1  |
 *                 +------+------+------+------+------+
 *               1 |  14  |                    |   2  |
 *                 +------+                    +------+
 *               2 |  15  |                    |   3  |
 *                 +------+                    +------+
 *               3 |  16  |                    |   4  |
 *                 +------+------+------+------+------+
 *               4 |   5  |   6  |   7  |   8  |   9  |
 *                 +------+------+------+------+------+
 */
static std::array<MenuButton, NUMBUTTONLABELS> MenuButtons;
const struct {
    int row;
    int col;
} LandscapeLayout[] {
    {0,4},{1,4},{2,4},{3,4},
    {4,0},{4,1},{4,2},{4,3},
    {4,4},{0,0},{0,1},{0,2},
    {0,3},{1,0},{2,0},{3,0}
};
static_assert(NUMBUTTONLABELS == array_size(LandscapeLayout), "Check array size");

/* Portrait :
 * 
 *                         0      1      2      3
 *                     +------+------+------+------+
 *                   0 |  10  |  11  |  12  |  13  |
 *                     +------+------+------+------+
 *                   1 |  14  |             |   5  |
 *                     +------+             +------+
 *                   2 |  15  |             |   6  |
 *                     +------+             +------+
 *                   3 |  16  |             |   7  |
 *                     +------+             +------+
 *                   4 |                    |   8  |
 *                     +                    +------+
 *                   5 |                    |   9  |
 *                     +------+------+------+------+
 *                   6 |   1  |   2  |   3  |   4  |
 *                     +------+------+------+------+
 * 
 */
const struct {
    int row;
    int col;
} PortraitLayout[] {
    {6,0},{6,1},{6,2},{6,3},
    {1,3},{2,3},{3,3},{4,3},
    {5,3},{0,0},{0,1},{0,2},
    {0,3},{1,0},{2,0},{3,0}
};
static_assert(NUMBUTTONLABELS == array_size(PortraitLayout), "Check array size");

static
PixelRect GetButtonPosition(unsigned MenuID, const PixelRect& rcScreen) {
    PixelRect rc = rcScreen;
    rc.Grow(-1); // remove Margin

    unsigned i = MenuID - 1;
    
    LKASSERT(i < MenuButtons.size());
    
    const PixelScalar row   = ScreenLandscape ? LandscapeLayout[i].row : PortraitLayout[i].row;
    const PixelScalar col   = ScreenLandscape ? LandscapeLayout[i].col :PortraitLayout[i].col;
    const PixelScalar nbRow = ScreenLandscape ? 5 : 7;
    const PixelScalar nbCol = ScreenLandscape ? 5 : 4;

    /* we use DLGSCALE here because reference layout are 320x240 screen*/
    const PixelSize size = {
        std::min<PixelScalar>((rc.GetSize().cx-(nbCol-1))/nbCol, DLGSCALE(80)), 
        std::min<PixelScalar>((rc.GetSize().cy-(nbRow-1))/nbRow, DLGSCALE(40))
    };
    
    const double x_interval = (rc.GetSize().cx - size.cx * nbCol) / (nbCol-1);
    const double y_interval = (rc.GetSize().cy - size.cy * nbRow) / (nbRow-1);

    const RasterPoint origin = {
        rc.left + static_cast<PixelScalar>(col * (size.cx + x_interval)),
        rc.top + static_cast<PixelScalar>(row * (size.cy + y_interval))
    };

    return PixelRect(origin, size);
}

void ButtonLabel::CreateButtonLabels(const PixelRect& rcScreen) {
    for (unsigned i = 0; i < MenuButtons.size(); ++i) {
        
        MenuButton& currentButton = MenuButtons[i];

        currentButton.Create(&MainWindow, GetButtonPosition(i+1, rcScreen));
        if(currentButton.IsDefined()) {
            currentButton.SetTextColor(RGB_BLACK);
            currentButton.SetBkColor(RGB_BUTTONS);
            currentButton.SetMenuId(i+1);
        }
    }
}

void ButtonLabel::SetFont(FontReference Font) {
    std::for_each(MenuButtons.begin(), MenuButtons.end(), std::bind(&MenuButton::SetFont, _1, Font));
}

void ButtonLabel::Destroy() {
    std::for_each(MenuButtons.begin(), MenuButtons.end(), std::bind(&MenuButton::Destroy, _1));
}

void ButtonLabel::SetLabelText(unsigned MenuID, const TCHAR *text) {

    unsigned idx = MenuID - 1;
    if(idx >= MenuButtons.size()) {
        BUGSTOP_LKASSERT(false);
        return;
    }


    MenuButton& currentButton = MenuButtons[idx];

    if ((text == NULL) || (*text == _T('\0')) || (*text == _T(' '))) {
        currentButton.SetVisible(false);
        currentButton.Enable(false);
    } else {
#ifdef LXMINIMAP
        if (InputEvents::getSelectedButtonId() == MenuID) {
            currentButton.SetBkColor(RGB_DARKYELLOW2);
        }
#endif

        TCHAR s[100];
        bool greyed = ExpandMacros(text, s, array_size(s));
        if (greyed) {
            currentButton.SetTextColor(LKColor(0x80, 0x80, 0x80));
            currentButton.EnableMenu(false);
        } else {
            currentButton.SetTextColor(RGB_BLACK);
            currentButton.EnableMenu(true);
        }

        if ((s[0]==_T('\0'))||(s[0]==_T(' '))) {
            currentButton.SetVisible(false);
            currentButton.Enable(false);
        } else {
            currentButton.SetWndText(LKGetText(s));
            currentButton.SetTopWnd();
            currentButton.SetVisible(true);
            currentButton.Enable(true);
        }
    }
}

bool ButtonLabel::IsVisible(unsigned MenuID) {
    unsigned i = MenuID - 1;
    if(i < MenuButtons.size()) {
        return MenuButtons[i].IsVisible();
    }
    BUGSTOP_LKASSERT(false);
    return false;
}

bool ButtonLabel::IsEnabled(unsigned MenuID) {
    unsigned i = MenuID - 1;
    if(i < MenuButtons.size()) {
        return MenuButtons[i].IsMenuEnabled();
    }
    BUGSTOP_LKASSERT(false);
    return false;
}
