/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGTaskMgr.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 sept. 2012
 */

#ifndef PGTASKMGR_H_
#define PGTASKMGR_H_

#include "PGTaskPt.h"
#include "vector"

class PGTaskMgr {
public:
    PGTaskMgr();
    virtual ~PGTaskMgr();

    void Initialize();
    void Optimize(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

    void getOptimized(const int i, double& lat, double& lon) const;
    void UpdateTaskPoint(const int i, TASK_POINT& TskPt ) const;

    inline size_t Count() const {
        return m_Task.size();
    }

protected:
    void AddCircle(int TpIndex);
    void AddLine(int TpIndex);
    void AddSector(int TpIndex);
    void AddCone(int TpIndex);
    void AddEssCircle(int TpIndex);

    void Grid2LatLon(double N, double E, double& lat, double& lon) const;
    void LatLon2Grid(double lat, double lon, double& N, double& E) const;

    typedef struct _DTM {
        double a; // a  Equatorial earth radius
        double b; // b  Polar earth radius
        double f; // f= (a-b)/a  Flatenning
        double esq; // esq = 1-(b*b)/(a*a)  Eccentricity Squared
        double e; // sqrt(esq)  Eccentricity
    } DATUM, *PDATUM;

    typedef struct _GRD {
        double lon0;
        double lat0;
        double k0;
        double false_e;
        double false_n;
    } GRID, *PGRID;

    typedef std::vector<PGTaskPt*> Task_t;

    // WGS84 data
    static const DATUM m_Datum;
    GRID m_Grid;

    Task_t m_Task;
};

#endif /* PGTASKMGR_H_ */
