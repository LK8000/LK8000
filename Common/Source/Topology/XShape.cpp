/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: XShape.cpp,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#include "externs.h"
#include "XShape.h"
#include "utils/charset_helper.h"
#include "Screen/Point.hpp"
#include "Multimap.h"
#include "ShapeSpecialRenderer.h"
#include <string_view>

using std::string_view_literals::operator""sv;

namespace {

bool ValidLabel(const char* src) {
  if (!src) {
    return false;
  }
  if (src == "NULL"sv) {
    return false;
  }
  if (src == "UNK"sv) {
    return false;
  }
  return true;
}

static bool HiddenLabel(const char* src) {
  if (src) {
    if (src == "RAILWAY STATION"sv) {
      return true;
    }
    if (src == "RAILROAD STATION"sv) {
      return true;
    }
  }
  return false;
}

}  // namespace

XShape::XShape() : hide(false) {
  msInitShape(&shape);
}

XShape::~XShape() {
  clear();
}

void XShape::clear() {
  msFreeShape(&shape);
  label.clear();
}

void XShape::load(shapefileObj* shpfile, int i) {
  msSHPReadShape(shpfile->hSHP, i, &shape);
}

void XShape::setLabel(const char* src) {
  label.clear();

  // Case1 : NULL or not informative label, we show the shape without label
  if (!ValidLabel(src)) {
    hide = false;
    return;
  }

  // Case2 : shapes that do not contain any useful information, we HIDE the
  // shape and the label
  if (HiddenLabel(src)) {
    hide = true;
    return;
  }

  // Any other case : we display shape and its label as well
  label = from_unknown_charset(src);
  hide = false;
}

bool XShape::nearestItem(int category, double lon, double lat) const {
  if (label.empty()) {
    return false;
  }

  NearestTopoItem* item;
  switch (category) {
    case 10:
      item = &NearestWaterArea;
      break;
    case 70:
    case 80:
    case 110:
      item = &NearestBigCity;
      break;
    case 90:
      item = &NearestCity;
      break;
    case 100:
      item = &NearestSmallCity;
      break;
    default:
      TestLog(_T("...... Cannot use nearestItem cat=%d <%s>"), category, label.c_str());
      return false;
  }

  double distance, bearing;
  DistanceBearing(lat, lon, GPS_INFO.Latitude, GPS_INFO.Longitude, &distance,
                  &bearing);

#if DEBUG_NEARESTTOPO
  StartupStore(
      _T("... cat=%d, <%s> lat=%f lon=%f mylat=%f mylon=%f distance=%.2f\n"),
      category, Temp, lat, lon, GPS_INFO.Latitude, GPS_INFO.Longitude,
      distance / 1000);
#endif

  // If first time, use it
  if (!item->Valid || (item->Distance > distance)) {
    item->Latitude = lat;
    item->Longitude = lon;
    lk::strcpy(item->Name, label.c_str());
    item->Distance = distance;
    item->Bearing = bearing;
    item->Valid = true;

#if DEBUG_NEARESTTOPO
    StartupStore(_T(".... cat=%d, <%s> dist=%f nearer\n"), category, Temp,
                 distance);
#endif
    return true;
  }

  return false;
}

// Print topology labels
bool XShape::renderSpecial(ShapeSpecialRenderer& renderer,
                                LKSurface& Surface, int x, int y,
                                const RECT& ClipRect) const {
  if (label.empty()) {
    return false;
  }

  if ((GetMultimap_Labels() != MAPLABELS_ALLON) &&
      (GetMultimap_Labels() != MAPLABELS_ONLYTOPO)) {
    return false;
  }

  // shift label from center point of shape
  x += NIBLSCALE(2);
  y += NIBLSCALE(2);

  if (x > ClipRect.right || y > ClipRect.bottom) {
    return false;
  }

  PixelSize tsize = Surface.GetTextSize(label.c_str());

  const PixelRect brect = {
    {x - NIBLSCALE(3), y - NIBLSCALE(3)},
    {tsize.cx + NIBLSCALE(3), tsize.cy + NIBLSCALE(3)}
  };

  if (MapWindow::checkLabelBlock(brect, ClipRect)) {
    renderer.Add({x, y}, label.c_str());
    return true;  // 101016
  }
  return false;  // 101016
}
