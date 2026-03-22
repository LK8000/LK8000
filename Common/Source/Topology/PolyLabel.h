/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PolyLabel.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 26, 2025
 */

/*
Adapted from https://github.com/mapbox/polylabel.

Original licence ;

ISC License
Copyright (c) 2016 Mapbox

Permission to use, copy, modify, and/or distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright notice
and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
*/

#ifndef _Topology_Polylabel_h_
#define _Topology_Polylabel_h_

#include "shapelib/mapprimitive.h"
#include <span>
#include <algorithm>
#include <limits>
#include <queue>
#include <cmath>

namespace detail {

// get squared distance from a point to a segment
inline double getSegDistSq(const pointObj& p, const pointObj& a, const pointObj& b) {
  auto x = a.x;
  auto y = a.y;
  auto dx = b.x - x;
  auto dy = b.y - y;

  if (dx != 0 || dy != 0) {
    auto t = ((p.x - x) * dx + (p.y - y) * dy) / (dx * dx + dy * dy);

    if (t > 1) {
      x = b.x;
      y = b.y;
    }
    else if (t > 0) {
      x += dx * t;
      y += dy * t;
    }
  }

  dx = p.x - x;
  dy = p.y - y;

  return dx * dx + dy * dy;
}

// signed distance from point to polygon outline (negative if point is outside)
inline double pointToPolygonDist(const pointObj& point, const shapeObj& polygon) {
  bool inside = false;
  auto minDistSq = std::numeric_limits<double>::infinity();

  for (const auto& ring : std::span(polygon.line, polygon.numlines)) {
    for (std::size_t i = 0, len = ring.numpoints, j = len - 1; i < len;
         j = i++) {
      const auto& a = ring.point[i];
      const auto& b = ring.point[j];

      if ((a.y > point.y) != (b.y > point.y) &&
          (point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x)) {
        inside = !inside;
      }

      minDistSq = std::min(minDistSq, getSegDistSq(point, a, b));
    }
  }

  return (inside ? 1 : -1) * std::sqrt(minDistSq);
}

struct Cell {
  Cell() = delete;

  Cell(Cell&&) = default;
  Cell& operator=(Cell&&) = default;

  Cell(const Cell&) = default;
  Cell& operator=(const Cell&) = default;

  Cell(const pointObj& c_, double h_, const shapeObj& polygon)
      : c(c_),
        h(h_),
        d(pointToPolygonDist(c, polygon)),
        max(d + h * std::sqrt(2)) {}

  pointObj c;  // cell center
  double h;    // half the cell size
  double d;    // distance from cell center to polygon
  double max;  // max distance to polygon within a cell
};

// get polygon centroid
inline Cell getCentroidCell(const shapeObj& polygon) {
  double area = 0;
  pointObj c{0, 0};
  const auto& ring = polygon.line[0];

  for (std::size_t i = 0, len = ring.numpoints, j = len - 1; i < len; j = i++) {
    const pointObj& a = ring.point[i];
    const pointObj& b = ring.point[j];
    auto f = a.x * b.y - b.x * a.y;
    c.x += (a.x + b.x) * f;
    c.y += (a.y + b.y) * f;
    area += f * 3;
  }
  if (area == 0) {
    return { ring.point[0], 0, polygon };
  }
  return {{c.x / area, c.y / area}, 0, polygon };
}

}  // namespace detail

inline pointObj PolyLabel(const shapeObj& polygon) {
  using namespace detail;

  if (polygon.numlines <= 0) {
    return {};
  }
  rectObj envelope = polygon.bounds;
  pointObj size = {
    envelope.maxx - envelope.minx,
    envelope.maxy - envelope.miny
  };
  double cellSize = std::min(size.x, size.y);
  if (cellSize == 0) {
    return {envelope.minx, envelope.miny};
  }
  double h = cellSize / 2;

  // a priority queue of cells in order of their "potential" 
  // (max distance to polygon)
  auto compareMax = [](const Cell& a, const Cell& b) {
    return a.max < b.max;
  };
  using Queue =
      std::priority_queue<Cell, std::vector<Cell>, decltype(compareMax)>;
  Queue cellQueue(std::move(compareMax));

  // cover polygon with initial cells
  for (double x = envelope.minx; x < envelope.maxx; x += cellSize) {
    for (double y = envelope.miny; y < envelope.maxy; y += cellSize) {
      cellQueue.emplace(pointObj{x + h, y + h}, h, polygon);
    }
  }

  // take centroid as the first best guess
  auto bestCell = getCentroidCell(polygon);

  // second guess: bounding box centroid
  Cell bboxCell({envelope.minx + size.x / 2.0, envelope.miny + size.y / 2.0}, 0,
                polygon);
  if (bboxCell.d > bestCell.d) {
    bestCell = bboxCell;
  }

  while (!cellQueue.empty()) {
    // pick the most promising cell from the queue
    auto cell = cellQueue.top();
    cellQueue.pop();

    // update the best cell if we found a better one
    if (cell.d > bestCell.d) {
      bestCell = cell;
    }

    // do not drill down further if there's no chance of a better solution
    if (cell.max - bestCell.d <= 1) {
      continue;
    }

    // split the cell into four cells
    h = cell.h / 2;
    cellQueue.emplace(pointObj{cell.c.x - h, cell.c.y - h}, h, polygon);
    cellQueue.emplace(pointObj{cell.c.x + h, cell.c.y - h}, h, polygon);
    cellQueue.emplace(pointObj{cell.c.x - h, cell.c.y + h}, h, polygon);
    cellQueue.emplace(pointObj{cell.c.x + h, cell.c.y + h}, h, polygon);
  }

  return bestCell.c;
}

#endif  // _Topology_Polylabel_h_
