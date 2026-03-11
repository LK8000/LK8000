/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: XShapeLabel.cpp,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#include "externs.h"
#include "XShapeLabel.h"
#include "utils/charset_helper.h"
#include "Screen/Point.hpp"
#include "Multimap.h"
#include "ShapeSpecialRenderer.h"

namespace {

bool ValidLabel(const char* src) {
  if (!src) {
    return false;
  }
  if (strcmp(src, "NULL") == 0) {
    return false;
  }
  if (strcmp(src, "UNK") == 0) {
    return false;
  }
  return true;
}

static bool HiddenLabel(const char* src) {
  if (src) {
    if (strcmp(src, "RAILWAY STATION") == 0) {
      return true;
    }
    if (strcmp(src, "RAILROAD STATION") == 0) {
      return true;
    }
  }
  return false;
}

}  // namespace

XShapeLabel::~XShapeLabel() {
  clearLabel();
}

void XShapeLabel::clearLabel() {
  if (label) {
    free(label);
    label = nullptr;
  }
}

void XShapeLabel::setLabel(const char* src) {
  clearLabel();

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
  size_t size = from_unknown_charset(src, label, 0) + 1;
  label = (TCHAR*)malloc(size * sizeof(TCHAR));
  if (label) {
    from_unknown_charset(src, label, size);
  }

  hide = false;
}

void XShapeLabel::clear() {
  XShape::clear();
  clearLabel();
}

bool XShapeLabel::nearestItem(int category, double lon, double lat) const {
  NearestTopoItem* item;
  if (!label || _tcslen(label) == 0) {
    return false;
  }

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
      TestLog(_T("...... Cannot use nearestItem cat=%d <%s>"), category, label);
      return false;
      break;
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
    lk::strcpy(item->Name, label);
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
bool XShapeLabel::renderSpecial(ShapeSpecialRenderer& renderer,
                                LKSurface& Surface, int x, int y,
                                const RECT& ClipRect) const {
  if (!label || !label[0]) {
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

  PixelSize tsize;
  Surface.GetTextSize(label, &tsize);

  const PixelRect brect = {{x - NIBLSCALE(3), y - NIBLSCALE(3)},
                           {tsize.cx + NIBLSCALE(3), tsize.cy + NIBLSCALE(3)}};

  if (MapWindow::checkLabelBlock(brect, ClipRect)) {
    renderer.Add({x, y}, label);
    return true;  // 101016
  }
  return false;  // 101016
}
