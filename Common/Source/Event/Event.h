/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Event.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 10 d�cembre 2014, 00:05
 */

#ifndef EVENT_H
#define	EVENT_H

#include "Event/Globals.hpp"
#ifdef ANDROID
#include "Event/Shared/Event.hpp"
#include "Event/Android/Loop.hpp"
#elif defined(ENABLE_SDL)
#include "Event/SDL/Event.hpp"
#include "Event/SDL/Loop.hpp"
#elif defined(USE_POLL_EVENT) || defined(NON_INTERACTIVE)
#include "Event/Shared/Event.hpp"
#include "Event/Poll/Loop.hpp"
#elif defined(USE_GDI)
#include "Event/GDI/Event.hpp"
#include "Event/GDI/Loop.hpp"
#endif
#include "Event/Key.h"
#include "Compiler.h"


#endif	/* EVENT_H */
