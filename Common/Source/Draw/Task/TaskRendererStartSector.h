/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererStartSector.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 3:04 AM
 */

#ifndef TASKRENDERERSTARTSECTOR_H
#define TASKRENDERERSTARTSECTOR_H

#include "TaskRenderer.h"

class TaskRendererStartSector : public TaskRenderer {
public:
    TaskRendererStartSector(const GeoPoint& center, double radius, double start, double end);

    bool IsPolygon() const override { 
        return true; 
    }
};

#endif /* TASKRENDERERSTARTSECTOR_H */
