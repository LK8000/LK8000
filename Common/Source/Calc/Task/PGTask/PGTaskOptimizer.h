/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGTaskOptimizer.h
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
#include "../task_zone.h"

class PGTaskOptimizer final {
  friend struct AddTaskPt;

public:
    PGTaskOptimizer() = default;
    PGTaskOptimizer(const PGTaskOptimizer&) = delete;
    PGTaskOptimizer(PGTaskOptimizer&&) = delete;

    void Initialize();
    void Optimize(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

    size_t Count() const {
      return m_Task.size();
    }

    void UpdateTaskPoint(size_t i, TASK_POINT& TskPt) const;

protected:

    struct AddTaskPt_Helper {
      using result_type = void;

      template <sector_type_t type, task_type_t task_type>
      static void invoke(int tp_index, PGTaskOptimizer& task) {
        task.AddTaskPt(tp_index, task::zone_data<type, task_type>::get(tp_index));
      }
    };

    void AddTaskPt(int tp_index, const task::sector_data& data);
    void AddTaskPt(int tp_index, const task::circle_data& data);
    void AddTaskPt(int tp_index, const task::dae_data& data);
    void AddTaskPt(int tp_index, const task::line_data& data);
    void AddTaskPt(int tp_index, const task::ess_circle& data);

private:
    GeoPoint  getOptimized(size_t i) const;

    /**
     * T must inherit from PGTaskPt
     */
    template<typename T, typename ...Args>
    std::enable_if_t<std::is_base_of_v<PGTaskPt, T>, std::unique_ptr<T>>
    Make(const GeoPoint& center, Args&&... args) {
      assert(m_Projection);
      return std::make_unique<T>(m_Projection->Forward(center), std::forward<Args>(args)...);
    }

    using PGTaskPt_ptr = std::unique_ptr<PGTaskPt>;
    std::vector<PGTaskPt_ptr> m_Task;
    std::unique_ptr<TransverseMercator> m_Projection;
};

#endif /* PGTASKMGR_H_ */
