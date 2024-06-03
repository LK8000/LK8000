/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererSGPStart.h
 * Author: Bruno de Lacheisserie
 *
 * Created on May 14, 2024, 00:19 AM
 */

#ifndef TASKRENDERERSGPSTART_H
#define TASKRENDERERSGPSTART_H

#include "TaskRenderer.h"

class TaskRendererSGPStart : public TaskRenderer {
public:
    TaskRendererSGPStart(const GeoPoint& center, double radius, double radial);

    bool IsPolygon() const override {
        return false;
    }
};

#endif /* TASKRENDERERSGPSTART_H */
