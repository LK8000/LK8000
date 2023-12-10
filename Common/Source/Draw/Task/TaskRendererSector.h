/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererSector.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 3:01 AM
 */

#ifndef TASKRENDERERSECTOR_H
#define TASKRENDERERSECTOR_H

#include "TaskRenderer.h"

class TaskRendererSector : public TaskRenderer {
public:
    TaskRendererSector(const GeoPoint& center, double radius, double start, double end);

    bool IsPolygon() const override {
        return _GeoPoints.size() > 2;
    }
};

#endif /* TASKRENDERERSECTOR_H */
