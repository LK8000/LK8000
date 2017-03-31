//
// Created by bruno on 3/31/17.
//

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
