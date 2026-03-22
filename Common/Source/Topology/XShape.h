/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: XShape.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#ifndef _Terrain_XShape_h_
#define _Terrain_XShape_h_

#include "Topology/shapelib/mapserver.h"
#include "types.h"
#include "Util/tstring.hpp"
#include <memory>
#include "Screen/Point.hpp"

class ShapeSpecialRenderer;
class LKSurface;
class LKBrush;
class LKPen;
class LKIcon;
class ScreenProjection;
struct PixelRect;

struct ShapeRenderer {
  struct callback_ref_t {
    using fn_t = bool (*)(const void*, RasterPoint);

    const void* ctx;
    fn_t fn;

    template <typename F>
    callback_ref_t(F&&) = delete;

    template <typename F>
    callback_ref_t(F& callable)
        : ctx(std::addressof(callable)),
          fn([](const void* opaque_ctx, RasterPoint position) {
            return (*static_cast<const F*>(opaque_ctx))(position);
          }) {}

    bool operator()(RasterPoint position) const {
      return fn(ctx, position);
    }
  };

  virtual ~ShapeRenderer() = default;

  virtual void Draw(LKSurface& Surface, const ScreenProjection& _Proj,
                    const PixelRect& ClipRect, callback_ref_t callback) = 0;
};

class XShape final {
 public:
  XShape(shapefileObj* shpfile, int i, int field);
  ~XShape();

  // no default ctor
  XShape() = delete;
  // no copy
  XShape(const XShape&) = delete;
  XShape& operator=(const XShape&) = delete;
  // no move
  XShape(XShape&&) = delete;
  XShape& operator=(XShape&&) = delete;

  /**
   * return true if shape have label
   */
  bool HasLabel() const {
    return !label.empty();
  }

  bool Overlap(const rectObj& bounds) const;

  void clear();

  bool renderSpecial(ShapeSpecialRenderer& renderer, LKSurface& Surface,
                     RasterPoint position, const PixelRect& ClipRect) const;

  bool nearestItem(int category, double lon, double lat) const;

  void Draw(ShapeSpecialRenderer& special_renderer, LKSurface& Surface,
            const PixelRect& ClipRect, const ScreenProjection& _Proj,
            const LKBrush& Brush, const LKPen& Pen, const LKIcon& Bitmap,
            bool DisplayLabel);

  bool hide = false;
  shapeObj shape;

 private:
  tstring label;
  std::unique_ptr<ShapeRenderer> renderer;
};

#endif  // _Terrain_XShape_h_
