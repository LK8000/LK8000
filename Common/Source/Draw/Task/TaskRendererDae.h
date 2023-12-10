/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererDae.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 3:02 AM
 */

#ifndef TASKRENDERERDAE_H
#define TASKRENDERERDAE_H

#include "TaskRenderer.h"

class TaskRendererDae : public TaskRenderer {
public:
    TaskRendererDae(const GeoPoint& center, double start, double end);

    bool IsPolygon() const override { 
        return true; 
    }
};

#endif /* TASKRENDERERDAE_H */
