/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGTaskMgr.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 sept. 2012
 */

#ifndef PGTASKMGR_H_
#define PGTASKMGR_H_

#include <vector> // vector
#include <memory> // unique_ptr
#include "PGTaskPt.h"
#include "Geographic/TransverseMercator.h"

class PGTaskMgr final {
public:
    PGTaskMgr() = default;

    void Initialize();
    void Optimize(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

    inline size_t Count() const {
      return m_Task.size();
    }

    void UpdateTaskPoint(size_t i, TASK_POINT& TskPt) const;

protected:
    GeoPoint  getOptimized(size_t i) const;

    void AddCircle(int TpIndex, double Radius);
    void AddLine(int TpIndex, double Radius);
    void AddSector(int TpIndex);
    void AddCone(int TpIndex);
    void AddEssCircle(int TpIndex, double Radius);

    using PGTaskPt_ptr = std::unique_ptr<PGTaskPt>;
    std::vector<PGTaskPt_ptr> m_Task;
    std::unique_ptr<TransverseMercator> m_Projection;
};

#endif /* PGTASKMGR_H_ */
