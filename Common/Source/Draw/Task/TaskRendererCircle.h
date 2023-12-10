/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererCircle.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 12:44 AM
 */

#ifndef TASKRENDERERCIRCLE_H
#define TASKRENDERERCIRCLE_H

#include "TaskRenderer.h"

class TaskRendererCircle : public TaskRenderer {
public:
    TaskRendererCircle(const GeoPoint& center, double radius);

    bool IsPolygon() const override { 
        return true; 
    }
};

#endif /* TASKRENDERERCIRCLE_H */
