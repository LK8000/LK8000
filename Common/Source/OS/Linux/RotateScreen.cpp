/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   RotateScreen.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 8 décembre 2014
 */

#include "externs.h"
#include "../RotateScreen.h"
#include "Hardware/RotateDisplay.hpp"
#include "LKInterface/CScreenOrientation.h"
#include "Hardware/RotateDisplay.hpp"
#include "DisplayOrientation.hpp"
#include "MessageLog.h"
#ifdef ANDROID
#include "Android/NativeView.hpp"
#include <Android/Main.hpp>
#endif

#ifdef KOBO
#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#endif

bool CanRotateScreen() {
#if (defined(ENABLE_SDL) || defined(USE_EGL)) && !defined(USE_FULLSCREEN)
    return false;
#elif defined(ANDROID)
    /**
     * TODO : implement CScreenOrientation::GetScreenSetting() && CScreenOrientation::SetScreenSetting
     * in all case this only used for show screen mode menu button and android use sensor for screen rotate,
     *  so screen rotation is controled by device, no need to have this menu.
     */
    return false;
#else
    return Display::RotateSupported();
#endif
}

bool RotateScreen(short angle) {
    
    DisplayOrientation_t orientation = static_cast<DisplayOrientation_t>(CScreenOrientation::GetScreenSetting());
    if(angle == 90) {
        switch(orientation) {
            case DisplayOrientation_t::DEFAULT:
            case DisplayOrientation_t::PORTRAIT:
              orientation = DisplayOrientation_t::LANDSCAPE;
              break;
            case DisplayOrientation_t::REVERSE_PORTRAIT:
              orientation = DisplayOrientation_t::REVERSE_LANDSCAPE;
              break;
            case DisplayOrientation_t::LANDSCAPE:
              orientation = DisplayOrientation_t::REVERSE_PORTRAIT;
              break;
            case DisplayOrientation_t::REVERSE_LANDSCAPE:
              orientation = DisplayOrientation_t::PORTRAIT;
        }
    } 

    if(angle == 180) {
        switch(orientation) {
            case DisplayOrientation_t::DEFAULT:
            case DisplayOrientation_t::PORTRAIT:
              orientation = DisplayOrientation_t::REVERSE_PORTRAIT;
              break;
            case DisplayOrientation_t::REVERSE_PORTRAIT:
              orientation = DisplayOrientation_t::PORTRAIT;
              break;
            case DisplayOrientation_t::LANDSCAPE:
              orientation = DisplayOrientation_t::REVERSE_LANDSCAPE;
              break;
            case DisplayOrientation_t::REVERSE_LANDSCAPE:
              orientation = DisplayOrientation_t::LANDSCAPE;
              break;
        }
    }

    if (!Display::Rotate(orientation)) {
        StartupStore(_T("Display rotation failed") NEWLINE);
        return false;
    }

#ifdef KOBO
    event_queue->SetDisplayOrientation(orientation);
#endif
    
    main_window->CheckResize();
    
    return true;
}

#ifdef ANDROID
ScopeLockScreen::ScopeLockScreen() :
    previous_state(native_view->getRequestedOrientation())
{
    const PixelSize size = native_view->GetSize();
    const int rotation = native_view->getDisplayOrientation();
    switch(rotation) {
        default:
        case 0: // ROTATION_0
            native_view->setRequestedOrientation( (size.cx > size.cy)
                                                  ? NativeView::ScreenOrientation::LANDSCAPE
                                                  : NativeView::ScreenOrientation::PORTRAIT);
            break;
        case 1: // ROTATION_90
            native_view->setRequestedOrientation( (size.cx > size.cy)
                                                  ? NativeView::ScreenOrientation::LANDSCAPE
                                                  : NativeView::ScreenOrientation::REVERSE_PORTRAIT);
            break;
        case 2: // ROTATION_180
            native_view->setRequestedOrientation( (size.cx > size.cy)
                                                  ? NativeView::ScreenOrientation::REVERSE_LANDSCAPE
                                                  : NativeView::ScreenOrientation::REVERSE_PORTRAIT);
            break;
        case 3: // ROTATION_270
            native_view->setRequestedOrientation( (size.cx > size.cy)
                                                  ? NativeView::ScreenOrientation::REVERSE_LANDSCAPE
                                                  : NativeView::ScreenOrientation::PORTRAIT);
            break;
    }
}

ScopeLockScreen::~ScopeLockScreen() {
    native_view->setRequestedOrientation(static_cast<NativeView::ScreenOrientation>(previous_state));
}
#else
ScopeLockScreen::ScopeLockScreen() :
            previous_state()
{
    // TODO : implement if needed.
}

ScopeLockScreen::~ScopeLockScreen() {
}
#endif