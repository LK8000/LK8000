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
#include "ShapePolygonRenderer.h"
#include "ShapeLineRenderer.h"
#include "ShapePointRenderer.h"
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

XShape::XShape(shapefileObj* shpfile, int i, int field) {
  msSHPReadShape(shpfile->hSHP, i, &shape);

  const char* src = nullptr;
  if (field >= 0) {
    src = msDBFReadStringAttribute(shpfile->hDBF, i, field);
  }

  // Case1 : NULL or not informative label, we show the shape without label
  if (!ValidLabel(src)) {
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
}

XShape::~XShape() {
  clear();
}

bool XShape::Overlap(const rectObj& bounds) const {
  return (msRectOverlap(&(shape.bounds), &bounds) == MS_TRUE);
}

void XShape::clear() {
  msFreeShape(&shape);
  label.clear();
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
      category, label.c_str(), lat, lon, GPS_INFO.Latitude, GPS_INFO.Longitude,
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
                                LKSurface& Surface, RasterPoint position,
                                const PixelRect& ClipRect) const {
  if (label.empty()) {
    return false;
  }

  if ((GetMultimap_Labels() != MAPLABELS_ALLON) &&
      (GetMultimap_Labels() != MAPLABELS_ONLYTOPO)) {
    return false;
  }

  PixelSize tsize = Surface.GetTextSize(label.c_str());

  if (shape.type == MS_SHAPE_POLYGON) {
    // polygon label is draw centered
    position = position - (tsize / 2);
  }
  else {
    // shift label from center point of shape
    auto offset = NIBLSCALE<PixelScalar>(2);
    position += {offset, offset};
  }

  auto margin = NIBLSCALE<PixelScalar>(3);
  PixelRect brect = {position, tsize};
  brect.Grow(margin);

  if (!ClipRect.IsInside(brect.GetTopLeft()) ||
      !ClipRect.IsInside(brect.GetBottomRight())) {
    return false;
  }

  if (MapWindow::checkLabelBlock(brect, ClipRect)) {
    renderer.Add(std::move(position), label.c_str());
    return true;  // 101016
  }
  return false;  // 101016
}

void XShape::Draw(ShapeSpecialRenderer& special_renderer, LKSurface& Surface,
                  const PixelRect& ClipRect, const ScreenProjection& _Proj,
                  const LKBrush& Brush, const LKPen& Pen, const LKIcon& Bitmap,
                  bool DisplayLabel) {
  if (!renderer) {
    switch (shape.type) {
      case MS_SHAPE_POINT:
        renderer = std::make_unique<ShapePointRenderer>(shape, Bitmap);
        break;
      case MS_SHAPE_LINE:
        renderer = std::make_unique<ShapeLineRenderer>(shape, Pen);
        break;
      case MS_SHAPE_POLYGON:
        renderer = std::make_unique<ShapePolygonRenderer>(shape, Brush);
        break;
    }

  }

  if (renderer) {
    auto callback = [&](RasterPoint position) {
      if (!DisplayLabel || !HasLabel()) {
        return true;
      }
      return renderSpecial(special_renderer, Surface, position, ClipRect);
    };

    renderer->Draw(Surface, _Proj, ClipRect, callback);
  }
}
