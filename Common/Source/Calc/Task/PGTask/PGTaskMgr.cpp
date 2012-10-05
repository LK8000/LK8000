/*
 * PGTaskMgr.cpp
 *
 *  Created on: 11 sept. 2012
 *      Author: Bruno
 */

#include "PGTaskMgr.h"
#include "externs.h"
#include "AATDistance.h"
#include "Waypointparser.h"

#include "algorithm"
#include "math.h"

const ProjPt ProjPt::null;

// WGS84 data
const PGTaskMgr::DATUM PGTaskMgr::m_Datum = (PGTaskMgr::DATUM){
    6378137.0, // a
    6356752.3142, // b
    0.00335281066474748, // f = 1/298.257223563
    0.006694380004260807, // esq
    0.0818191909289062, // e
};

PGTaskMgr::PGTaskMgr() {

}

PGTaskMgr::~PGTaskMgr() {

}

void PGTaskMgr::Initialize() {
    LockTaskData();
    // build Mercator Reference Grid
    // find center of Task
    double minlat = 0.0, minlon = 0.0, maxlat = 0.0, maxlon = 0.0;
    for (int curwp = 0; ValidWayPoint(Task[curwp].Index); ++curwp) {
        if (curwp == 0) {
            maxlat = minlat = WayPointList[Task[curwp].Index].Latitude;
            maxlat = minlon = WayPointList[Task[curwp].Index].Longitude;
        } else {
            minlat = std::min(minlat, WayPointList[Task[curwp].Index].Latitude);
            maxlat = std::max(maxlat, WayPointList[Task[curwp].Index].Latitude);

            minlon = std::min(minlon, WayPointList[Task[curwp].Index].Longitude);
            maxlon = std::max(maxlon, WayPointList[Task[curwp].Index].Longitude);
        }
    }

    m_Grid.lat0 = ((minlat + maxlat) / 2) * DEG_TO_RAD;
    m_Grid.lon0 = ((minlon + maxlon) / 2) * DEG_TO_RAD;
    m_Grid.k0 = 1;
    m_Grid.false_e = 0.0; // ????
    m_Grid.false_n = 0.0; // ????

    // build task point list
    for (int curwp = 0; ValidWayPoint(Task[curwp].Index); ++curwp) {
        m_Task.resize(curwp + 1);

        LatLon2Grid(WayPointList[Task[curwp].Index].Latitude*DEG_TO_RAD,
                WayPointList[Task[curwp].Index].Longitude*DEG_TO_RAD,
                m_Task[curwp].m_Center.m_Y,
                m_Task[curwp].m_Center.m_X);

        double lat, lon;
        Grid2LatLon(m_Task[curwp].m_Center.m_Y, m_Task[curwp].m_Center.m_X, lat, lon);
        lat *= RAD_TO_DEG;
        lon *= RAD_TO_DEG;

        if (curwp == 0) {
            m_Task[curwp].m_Radius = StartRadius;
        } else if (ValidWayPoint(Task[curwp + 1].Index)) {
            m_Task[curwp].m_Radius = (Task[curwp].AATCircleRadius);
        } else {
            m_Task[curwp].m_Radius = FinishRadius;
        }

        m_Task[curwp].m_bExit = ((curwp > 0) ? (Task[curwp].OutCircle) : !PGStartOut);
    }
    UnlockTaskData();
}

void PGTaskMgr::Optimize(NMEA_INFO *Basic) {
    if (((size_t) ActiveWayPoint) >= m_Task.size()) {
        return;
    }

    ProjPt PrevPos;
    LatLon2Grid(Basic->Latitude*DEG_TO_RAD, Basic->Longitude*DEG_TO_RAD, PrevPos.m_Y, PrevPos.m_X);

    for (size_t i = ActiveWayPoint; i < m_Task.size(); ++i) {
        if ((i + 1) < m_Task.size()) {
            m_Task[i].Optimize(PrevPos, m_Task[i + 1].getOptimized());
        } else {
            if (FinishLine == 1) {
                // Find prev Tp not same as current.
                int j = i - 1;
                while (j > 0 && m_Task[j].getCenter() == m_Task[i].getCenter()) {
                    --j;
                }
                if (j >= 0) { // if previous doesn't exit don't optimize, use center.
                    m_Task[i].OptimizeFinishLine(PrevPos, m_Task[j].m_Center);
                }
            } else {
                m_Task[i].Optimize(PrevPos, ProjPt::null);
            }
        }
        PrevPos = m_Task[i].getOptimized();
    }
}

void PGTaskMgr::getOptimized(const int i, double& lat, double& lon) const {
    Grid2LatLon(m_Task[i].getOptimized().m_Y, m_Task[i].getOptimized().m_X, lat, lon);
    lat *= RAD_TO_DEG;
    lon *= RAD_TO_DEG;
}

//====================================
// Local Grid to Lat/Lon conversion
//====================================

void PGTaskMgr::Grid2LatLon(double N, double E, double& lat, double& lon) const {
    double a = m_Datum.a; // Semi-major axis of reference ellipsoid
    double f = m_Datum.f; // Ellipsoidal flattening
    double b = a * (1 - f);

    double lat0 = m_Grid.lat0;
    double lon0 = m_Grid.lon0;

    double k0 = m_Grid.k0;

    double N0 = m_Grid.false_n;
    double E0 = m_Grid.false_e;


    double e2 = 2 * f - (f * f);
    double e4 = e2*e2;
    double e6 = e4*e2;

    double A0 = 1 - e2 / 4 - 3 * e4 / 64 - 5 * e6 / 256;
    double A2 = 3 * (e2 + e4 / 4 + 15 * e6 / 256) / 8;
    double A4 = 15 * (e4 + 3 * e6 / 4) / 256;
    double A6 = 35 * e6 / 3072;

    double m0 = a * (A0 * lat0 - A2 * sin(2 * lat0) + A4 * sin(4 * lat0) - A6 * sin(6 * lat0));

    //================
    // GRID -> Lat/Lon
    //================
    // http://www.linz.govt.nz/geodetic/conversion-coordinates/projection-conversions/transverse-mercator-preliminary-computations#lbl1

    double N1 = N - N0;
    double m = m0 + N1 / k0;
    double n = (a - b) / (a + b);

    double G = a * (1 - n)*(1 - n * n)*(1 + 9 * n * n / 4 + 225 * n * n * n * n / 64);
    double s = m / G;
    double ph = s + (3 * n / 2 - 27 * n * n * n / 32) * sin(2 * s)+(21 * n * n / 16 - 55 * n * n * n * n / 32) * sin(4 * s)+(151 * n * n * n / 96) * sin(6 * s)+(1097 * n * n * n * n / 512) * sin(8 * s);

    double r = a * (1 - e2) / pow(1 - e2 * sin(s) * sin(s), 1.5);
    double nu = a / sqrt(1 - e2 * sin(s) * sin(s));
    double ps = nu / r;
    double t = tan(s);
    double E1 = E - E0;
    double x = E1 / (k0 * nu);


    double T1 = t / (k0 * r) * E1 * x / 2;
    double T2 = t / (k0 * r) * E1 * x * x * x / 24 * (-4 * ps * ps + 9 * ps * (1 - t * t) + 12 * t * t);
    double T3 = t / (k0 * r) * E1 * x * x * x / 720 * (8 * ps * ps * ps * ps * (11 - 24 * t * t) - 12 * ps * ps * ps * (21 - 71 * t * t) + 15 * ps * ps * (15 - 98 * t * t + 15 * t * t * t * t) + 180 * ps * (5 * t * t - 3 * t * t * t * t) + 360 * t * t * t * t);
    double T4 = t / (k0 * r) * E1 * x * x * x / 40320 * (1385 - 3633 * t * t + 4095 * t * t * t * t + 1575 * t * t * t * t * t * t);

    lat = ph - T1 + T2 - T3 + T4;


    //	t = tan(lat);

    double secph = 1 / cos(ph);
    double T5 = x*secph;
    double T6 = x * x * x * secph / 6 * (ps + 2 * t * t);
    double T7 = x * x * x * x * x * secph / 120 * (-4 * ps * ps * ps + (1 - 6 * t * t) + ps * ps * (9 - 68 * t * t) + 72 * ps * t * t*+24 * t * t * t * t);
    double T8 = x * x * x * x * x * x * x * secph / 5040 * (61 + 662 * t * t + 1320 * t * t * t * t + 720 * t * t * t * t * t * t);

    lon = lon0 + T5 - T6 + T7 - T8;
}

//====================================
// Lat/Lon to Local Grid conversion
//====================================

void PGTaskMgr::LatLon2Grid(double lat, double lon, double& N, double& E) const {
    // Datum data for Lat/Lon to TM conversion

    double a = m_Datum.a; // Semi-major axis of reference ellipsoid
    double f = m_Datum.f; // Ellipsoidal flattening

    double lat0 = m_Grid.lat0;
    double lon0 = m_Grid.lon0;

    double k0 = m_Grid.k0;

    double N0 = m_Grid.false_n;
    double E0 = m_Grid.false_e;


    //===============
    // Lat/Lon -> TM
    //===============
    double slat1 = sin(lat);
    double clat1 = cos(lat);
    double clat1sq = clat1*clat1;

    double e2 = 2 * f - (f * f);
    double e4 = e2*e2;
    double e6 = e4*e2;

    double A0 = 1 - e2 / 4 - 3 * e4 / 64 - 5 * e6 / 256;
    double A2 = 3 * (e2 + e4 / 4 + 15 * e6 / 256) / 8;
    double A4 = 15 * (e4 + 3 * e6 / 4) / 256;
    double A6 = 35 * e6 / 3072;

    double m = a * (A0 * lat - A2 * sin(2 * lat) + A4 * sin(4 * lat) - A6 * sin(6 * lat));
    double m0 = a * (A0 * lat0 - A2 * sin(2 * lat0) + A4 * sin(4 * lat0) - A6 * sin(6 * lat0));

    double r = a * (1 - e2) / pow((1 - (e2 * (slat1 * slat1))), 1.5);
    double n = a / sqrt(1 - (e2 * (slat1 * slat1)));
    double ps = n / r;
    double t = tan(lat);
    double o = lon - lon0;

    double K1 = k0 * (m - m0);
    double K2 = k0 * o * o * n * slat1 * clat1 / 2;
    double K3 = k0 * o * o * o * o * n * slat1 * clat1 * clat1sq / 24 * (4 * ps * ps + ps - t * t) / 24;
    double K4 = k0 * o * o * o * o * o * o * n * slat1 * clat1sq * clat1sq * clat1 * (8 * ps * ps * ps * ps * (11 - (24 * t * t)) - 28 * ps * ps * ps * (1 - 6 * t * t) + ps * ps * (1 - 32 * t * t) - ps * 2 * t * t + t * t * t * t) / 720;
    double K5 = k0 * o * o * o * o * o * o * o * o * n * slat1 * clat1sq * clat1sq * clat1sq * clat1 * (1385 - 311 * t * t * 543 * t * t * t * t - t * t * t * t * t) / 40320;
    // ING north
    N = N0 + K1 + K2 + K3 + K4 + K5;

    double K6 = o * o * clat1sq * (ps - t * t) / 6;
    double K7 = o * o * o * o * clat1sq * clat1sq * (4 * ps * ps * ps * (1 - 6 * t * t) + ps * ps * (1 + 8 * t * t) - ps * 2 * t * t + t * t * t * t) / 120;
    double K8 = o * o * o * o * o * o * clat1sq * clat1sq * clat1sq * (61 - 479 * t * t + 179 * t * t * t * t - t * t * t * t * t * t);
    // ING east
    E = E0 + k0 * n * o * clat1 * (1 + K6 + K7 + K8);
}
