/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererMgr.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 2:33 AM
 */

#ifndef TASKRENDERERMGR_H
#define TASKRENDERERMGR_H

#include "TaskRenderer.h"

struct GeoPoint;

class TaskRendererMgr {
public:
    TaskRendererMgr();
    virtual ~TaskRendererMgr();

    TaskRendererMgr(const TaskRendererMgr&) = delete;

    void CalculateScreenPosition(const rectObj &screenbounds, const ScreenProjection& _Proj);

    void SetCircle(unsigned idx, const GeoPoint& center, double radius);
    void SetSector(unsigned idx, const GeoPoint& center, double radius, double start, double end);
    void SetDae(unsigned idx, const GeoPoint& center, double start, double end);
    void SetLine(unsigned idx, const GeoPoint& center, double radius, double radial);
    void SetStartSector(unsigned idx, const GeoPoint& center, double radius, double start, double end);

    void Clear();

    const TaskRenderer* GetRenderer(unsigned idx) const;

private:
    typedef std::unique_ptr<TaskRenderer> TaskRenderer_ptr;
    typedef std::vector<TaskRenderer_ptr> renderer_list_t;

    renderer_list_t _renderer_list;
};

extern TaskRendererMgr gTaskSectorRenderer;
extern TaskRendererMgr gStartSectorRenderer;

#endif /* TASKRENDERERMGR_H */
