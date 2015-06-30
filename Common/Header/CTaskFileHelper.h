/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   CTaskFileHelper.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 19 janvier 2013, 12:25
 */

#ifndef CTASKFILEHELPER_H
#define	CTASKFILEHELPER_H

#include "externs.h"
#include "boost/noncopyable.hpp"
#include "xmlParser.h"
#include "utils/tstring.h"

class CTaskFileHelper : private boost::noncopyable {
public:
    CTaskFileHelper();
    virtual ~CTaskFileHelper();

    bool Load(const TCHAR* szFileName);
    bool Save(const TCHAR* szFileName);

protected:
    bool SaveOption(XMLNode node);

    bool SaveOptionAAT(XMLNode node);
    bool SaveOptionRace(XMLNode node);
    bool SaveOptionDefault(XMLNode node);
    bool SaveTimeGate(XMLNode node);
    bool SaveTaskRule(XMLNode node);

    bool SaveTaskPointList(XMLNode node);
    bool SaveStartPointList(XMLNode node);
    bool SaveWayPointList(XMLNode node);

    bool SaveTaskPoint(XMLNode node, const unsigned long idx, const TASK_POINT& TaskPt);
    bool SaveStartPoint(XMLNode node, const START_POINT& StartPt);
    bool SaveWayPoint(XMLNode node, const WAYPOINT& WayPoint);

    void LoadOptions(XMLNode node);
    void LoadOptionAAT(XMLNode node);
    void LoadOptionRace(XMLNode node);
    void LoadOptionDefault(XMLNode node);

    void LoadTimeGate(XMLNode node);
    void LoadRules(XMLNode node);

    bool LoadTaskPointList(XMLNode node);
    bool LoadStartPointList(XMLNode node);
    void LoadWayPointList(XMLNode node, TCHAR *firstWPname, TCHAR *lastWPname);

    bool LoadTaskPoint(XMLNode node);
    bool LoadStartPoint(XMLNode node);
    void LoadWayPoint(XMLNode node, TCHAR *firstWPname, TCHAR *lastWPname);

private:
    std::set<size_t> mWayPointToSave;
    std::map<std::tstring, size_t> mWayPointLoaded;
    unsigned long mFinishIndex;
};

#endif	/* CTASKFILEHELPER_H */

