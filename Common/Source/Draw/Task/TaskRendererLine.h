/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererLine.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 3:04 AM
 */

#ifndef TASKRENDERERLINE_H
#define TASKRENDERERLINE_H

#include "TaskRenderer.h"

class TaskRendererLine : public TaskRenderer {
public:
    TaskRendererLine(const GeoPoint& center, double radius, double radial);

    bool IsPolygon() const override { 
        return true; 
    }
};

#endif /* TASKRENDERERLINE_H */
