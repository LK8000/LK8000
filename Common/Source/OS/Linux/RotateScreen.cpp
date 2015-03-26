/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   RotateScreen.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 8 décembre 2014
 */

#include "externs.h"
#include "Hardware/RotateDisplay.hpp"
#include "LKInterface/CScreenOrientation.h"
#include "Hardware/RotateDisplay.hpp"
#include "DisplayOrientation.hpp"
#include "MessageLog.h"

#ifdef KOBO
#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#endif


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
    event_queue->SetMouseRotation(orientation);
#endif
    
    MainWindow.CheckResize();
    
    
    return false;
}
