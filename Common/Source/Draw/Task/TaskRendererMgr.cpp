/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererMgr.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 2:33 AM
 */

#include "externs.h"
#include "TaskRendererMgr.h"
#include "TaskRendererCircle.h"
#include "TaskRendererDae.h"
#include "TaskRendererLine.h"
#include "TaskRendererSector.h"
#include "TaskRendererStartSector.h"
#include "TaskRendererSGPStart.h"

TaskRendererMgr gTaskSectorRenderer;
TaskRendererMgr gStartSectorRenderer;

TaskRendererMgr::TaskRendererMgr() {
    _renderer_list.reserve(std::size(Task));
}

TaskRendererMgr::~TaskRendererMgr() {

}

void TaskRendererMgr::CalculateScreenPosition(const rectObj& screenbounds, const ScreenProjection& _Proj) {

    for (auto& Item : _renderer_list) {
        if (Item) {
            Item->CalculateScreenPosition(screenbounds, _Proj);
        }
    }
}

void TaskRendererMgr::SetCircle(unsigned idx, const GeoPoint& center, double radius) {

    if (_renderer_list.size() <= idx) {
        _renderer_list.resize(idx + 1);
    }

    _renderer_list[idx] = std::make_unique<TaskRendererCircle>(center, radius);
}

void TaskRendererMgr::SetSector(unsigned idx, const GeoPoint& center, double radius, double start, double end) {

    if (_renderer_list.size() <= idx) {
        _renderer_list.resize(idx + 1);
    }

    _renderer_list[idx] = std::make_unique<TaskRendererSector>(center, radius, start, end);
}

void TaskRendererMgr::SetDae(unsigned idx, const GeoPoint& center, double start, double end) {

    if (_renderer_list.size() <= idx) {
        _renderer_list.resize(idx + 1);
    }

    _renderer_list[idx] = std::make_unique<TaskRendererDae>(center, start, end);
}

void TaskRendererMgr::SetLine(unsigned idx, const GeoPoint& center, double radius, double radial) {

    if (_renderer_list.size() <= idx) {
        _renderer_list.resize(idx + 1);
    }

    _renderer_list[idx] = std::make_unique<TaskRendererLine>(center, radius, radial);
}

void TaskRendererMgr::SetStartSector(unsigned idx, const GeoPoint& center, double radius, double start, double end) {

    if (_renderer_list.size() <= idx) {
        _renderer_list.resize(idx + 1);
    }

    _renderer_list[idx] = std::make_unique<TaskRendererStartSector>(center, radius, start, end);
}

void TaskRendererMgr::SetGPStart(unsigned idx, const GeoPoint& center, double radius, double radial) {

    if (_renderer_list.size() <= idx) {
        _renderer_list.resize(idx + 1);
    }

    _renderer_list[idx] = std::make_unique<TaskRendererSGPStart>(center, radius, radial);
}

void TaskRendererMgr::Clear() {
    _renderer_list.clear();
}

const TaskRenderer* TaskRendererMgr::GetRenderer(unsigned idx) const {
    if (idx < _renderer_list.size()) {
        return _renderer_list[idx].get();
    }
    return nullptr;
}
