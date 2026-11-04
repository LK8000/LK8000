/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ZoomTopology.h
 */
#pragma once

#include "Topology/Topology.h"

struct TopologyZoomLevel {
  topology_category category_;
  double& ref_zoom_;  // ref to zoom level for this category (Global variable)
  const double default_zoom_;  // default zoom level for this category
  const TCHAR* name_;          // WndProperty name in dlgTopology

  double Zoom() const {
    return ref_zoom_ <= 100 ? ref_zoom_ : default_zoom_;
  }

  struct list {
    static const TopologyZoomLevel* begin();
    static const TopologyZoomLevel* end();

    static void reset();
  };

  static const TopologyZoomLevel* get(topology_category category);
  static void set(topology_category category, double level);

  static double DefaultZoom(topology_category category);
  static double Zoom(topology_category category);
};
