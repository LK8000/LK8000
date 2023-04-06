/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BottomBar.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 07 Avril 2023
 */

#ifndef _Draw_BottomBar_h_
#define _Draw_BottomBar_h_

#include "ScreenGeometry.h"
#include "Screen/Point.hpp"
#include "Screen/LKColor.h"
#include <memory>

#include "MapWindow.h"

class LKSurface;

inline PixelScalar BB_ICONSIZE() {
  return IBLSCALE<PixelScalar>(26);
}

class bottom_bar final {
 public:
  bottom_bar() = default;

  void draw(LKSurface& Surface, const PixelRect& rc);

 private:
  unsigned get_current_mode();

  void refresh_layout(LKSurface& Surface, const PixelRect& rc);
  void fill_backgroud(LKSurface& Surface);
  void draw_data(LKSurface& Surface);

  void draw_fly_mode_data(LKSurface& Surface, MapWindow::Mode::TModeFly mode) const;

  void draw_lkvalues_data(LKSurface& Surface, unsigned box_num, short value_id,
                          const TCHAR* custom_title = nullptr) const;

  void draw_lkvalues_data(LKSurface& Surface, const std::array<short, 10> &array) const;

  void draw_cru_data(LKSurface& Surface) const;
  void draw_hgt_data(LKSurface& Surface) const;
  void draw_aux_data(LKSurface& Surface) const;
  void draw_tsk_data(LKSurface& Surface) const;
  void draw_alt_data(LKSurface& Surface) const;
  void draw_sys_data(LKSurface& Surface) const;
  void draw_cus_data(LKSurface& Surface) const;

  void draw_box(LKSurface& Surface, unsigned box_num, TCHAR* Value, TCHAR* Unit, TCHAR* Title, DrawBmp_t& Bmp) const;

  unsigned get_box_count() const { return _row_count * _col_count; }

 private:
  PixelRect _screen_rect = {};
  PixelRect _rect = {};  // bottom bar

  PixelScalar _title_height = {};
  PixelSize _value_size = {};
  PixelSize _box_size = {};

  unsigned _row_count;  // 1 => landscape; 2 => portrait
  unsigned _col_count;  // <= 10

  LKColor _text_color = {};  // set by `fill_background()`

#ifdef USE_GDI
  // to fill transparent background
  std::unique_ptr<LKSurface> _pTempSurface;
#endif
};

#endif  // _Draw_BottomBar_h_
