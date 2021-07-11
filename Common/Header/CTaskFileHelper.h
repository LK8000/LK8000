/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "Util/tstring.hpp"

// Forward declarations
namespace rapidxml { 
  template<class Ch> class xml_node;
}

class CTaskFileHelper final {
public:
    using xml_node = class rapidxml::xml_node<char>;

    CTaskFileHelper() = default;

    CTaskFileHelper( const CTaskFileHelper& ) = delete;
    CTaskFileHelper& operator=( const CTaskFileHelper& ) = delete;    

    bool Load(const TCHAR* szFileName);
    bool Save(const TCHAR* szFileName);

protected:
    bool SaveOption(xml_node* node);

    bool SaveOptionAAT(xml_node* node);
    bool SaveOptionRace(xml_node* node);
    bool SaveOptionDefault(xml_node* node);
    bool SaveTimeGate(xml_node* node);
    bool SaveTaskRule(xml_node* node);

    bool SaveTaskPointList(xml_node* node);
    bool SaveStartPointList(xml_node* node);
    bool SaveWayPointList(xml_node* node);

    bool SaveTaskPoint(xml_node* node, const unsigned long idx, const TASK_POINT& TaskPt);
    bool SaveStartPoint(xml_node* node, const START_POINT& StartPt);
    bool SaveWayPoint(xml_node* node, const WAYPOINT& WayPoint);

    void LoadOptions(const xml_node* node);
    void LoadOptionAAT(const xml_node* node);
    void LoadOptionRace(const xml_node* node);
    void LoadOptionDefault(const xml_node* node);

    void LoadTimeGate(const xml_node* node);
    void LoadRules(const xml_node* node);

    bool LoadTaskPointList(const xml_node* node);
    bool LoadStartPointList(const xml_node* node);
    void LoadWayPointList(const xml_node* node, const TCHAR *firstWPname, const TCHAR *lastWPname);

    bool LoadTaskPoint(const xml_node* node);
    bool LoadStartPoint(const xml_node* node);
    void LoadWayPoint(const xml_node* node, const TCHAR *firstWPname, const TCHAR *lastWPname);

private:
    std::set<size_t> mWayPointToSave;
    std::map<tstring, size_t> mWayPointLoaded;
    unsigned mFinishIndex = {};
};

#endif	/* CTASKFILEHELPER_H */

