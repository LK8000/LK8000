/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   android_drawable.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 31/03/2017.
 */

#ifndef ANDROID_DRAWABLE_H
#define ANDROID_DRAWABLE_H

#include "resource.h"

static const struct {
  unsigned id;
  const char *name;
} DrawableNames[] = {
    { IDB_EMPTY,           "empty"},
    { IDB_TOWN,            "town"},
    { IDB_LKSMALLTOWN,     "smalltown"},
    { IDB_LKVERYSMALLTOWN, "verysmalltown"}
};

#endif //ANDROID_DRAWABLE_H
