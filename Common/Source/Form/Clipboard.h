/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Clipboard.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 10, 2024
 */

#ifndef FORM_CLIPBOARD_H
#define	FORM_CLIPBOARD_H

#include "Util/tstring.hpp"

bool ClipboardAvailable();

tstring GetClipboardData();

#endif // FORM_CLIPBOARD_H
