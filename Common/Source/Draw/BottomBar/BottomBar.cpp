/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BottomBar.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 07 Avril 2023
 */

#include "BottomBar.h"
#include "externs.h"
#include "Screen/LKSurface.h"
#include "LKObjects.h"
#include "Asset.hpp"
#include "LKInterface.h"
#ifndef UNICODE
#include "Util/UTF8.hpp"
#endif

void bottom_bar::draw(LKSurface& Surface, const PixelRect& rect) {
  if (rect != _screen_rect) {
    // screen has changed => refresh layout
    refresh_layout(Surface, rect);
  }
  fill_backgroud(Surface);
  draw_data(Surface);
}

unsigned bottom_bar::get_current_mode() {
  using Mode = MapWindow::Mode;
  Mode& mode = MapWindow::mode;

  static Mode::TModeFly lastMode = Mode::MODE_FLY_NONE;
  static short OldBottomMode = -1;

  if (ConfBB0Auto == BBSM_AUTO_THERMALLING) {
    if (mode.Is(Mode::MODE_CIRCLING) && lastMode != Mode::MODE_FLY_CIRCLING) {
      OldBottomMode = BottomMode;
      BottomMode = BM_TRM;
    } else if (!mode.Is(Mode::MODE_CIRCLING) && lastMode == Mode::MODE_FLY_CIRCLING) {
      BottomMode = OldBottomMode;
    }
    lastMode = mode.Fly();
  } else if (ConfBB0Auto == BBSM_FULL_AUTO) {
    if (mode.Is(Mode::MODE_CIRCLING) && lastMode != Mode::MODE_FLY_CIRCLING) {
      BottomMode = BM_TRM;
    } else if (mode.Is(Mode::MODE_CRUISE) && lastMode != Mode::MODE_FLY_CRUISE) {
      BottomMode = BM_CUS2;
    } else if (mode.Is(Mode::MODE_FINAL_GLIDE) && lastMode != Mode::MODE_FLY_FINAL_GLIDE) {
      BottomMode = BM_CUS3;
    }
    lastMode = mode.Fly();
  }
  return BottomMode;
}

void bottom_bar::refresh_layout(LKSurface& Surface, const PixelRect& rect) {
#ifdef USE_GDI
  _pTempSurface = nullptr;
#endif

  const PixelSize screen_size = rect.GetSize();
  if (screen_size.cx > screen_size.cy) {
    // Landscape
    _row_count = 1;
  } else {
    // Portrait
    _row_count = 2;
  }

  const auto oldFont = Surface.SelectObject(LK8BottomBarTitleFont);
  _title_height = Surface.GetTextHeight(_T("M"));

  Surface.SelectObject(LK8BottomBarValueFont);
  // we need to be able to print 12345f with no problems
  _value_size = Surface.GetTextSize(_T("12345"));
  _value_size.cx += NIBLSCALE(1);

  if (!HideUnits) {
    Surface.SelectObject(LK8BottomBarUnitFont);
    // m for meters unit, f is shorter anyway
    _value_size.cx += Surface.GetTextWidth(_T("m"));
  }

  _rect = rect;
  _rect.top = _rect.bottom - _row_count * (_title_height + _value_size.cy) - (_row_count + 1) * IBLSCALE(1);

  int bar_width = _rect.GetSize().cx - BB_ICONSIZE() - IBLSCALE(1);
  _col_count = std::min(iround(static_cast<double>(bar_width) / _value_size.cx), 10);
  _box_size = {static_cast<PixelScalar>(bar_width / _col_count),
               static_cast<PixelScalar>(_title_height + _value_size.cy)};

  Surface.SelectObject(oldFont);

  _screen_rect = rect;
}

void bottom_bar::fill_backgroud(LKSurface& Surface) {
  const auto& brush_bar = INVERTCOLORS ? LKBrush_Black : LKBrush_Nlight;

  if (LKSurface::AlphaBlendSupported() && MapSpaceMode == MSM_MAP && BarOpacity < 100) {
    if (BarOpacity == 0) {
      _text_color = RGB_BLACK;
    } else {
#ifdef USE_GDI
      PixelRect temp_rect({0, 0}, _rect.GetSize());
      if (!_pTempSurface) {
        _pTempSurface = std::make_unique<LKBitmapSurface>(Surface, temp_rect.GetSize());
      }
      _pTempSurface->FillRect(&temp_rect, brush_bar);
      Surface.AlphaBlend(_rect, *_pTempSurface, temp_rect, BarOpacity * 255 / 100);
#else
      const LKBrush AlphaBrush(brush_bar.GetColor().WithAlpha(BarOpacity * 255 / 100));
      Surface.FillRect(&_rect, AlphaBrush);
#endif
      if (BarOpacity > 25) {
        _text_color = RGB_WHITE;
      } else {
        _text_color = RGB_BLACK;
      }
    }
  } else {
    _text_color = RGB_WHITE;
    Surface.FillRect(&_rect, brush_bar);
    if (IsDithered()) {
      Surface.DrawLine(PEN_SOLID, ScreenThinSize, _rect.GetTopLeft(), _rect.GetTopRight(),
                       INVERTCOLORS ? RGB_WHITE : RGB_BLACK, _screen_rect);
    }
  }
}

void bottom_bar::draw_data(LKSurface& Surface) {
  switch (get_current_mode()) {
    case BM_TRM:
      return draw_fly_mode_data(Surface, MapWindow::Mode::MODE_FLY_CIRCLING);
    case BM_CRU:
      return draw_cru_data(Surface);
    case BM_HGH:
      return draw_hgt_data(Surface);
    case BM_AUX:
      return draw_aux_data(Surface);
    case BM_TSK:
      return draw_tsk_data(Surface);
    case BM_ALT:
      return draw_alt_data(Surface);
    case BM_SYS:
      return draw_sys_data(Surface);
    case BM_CUS2:
      return draw_fly_mode_data(Surface, MapWindow::Mode::MODE_FLY_CRUISE);
    case BM_CUS3:
      return draw_fly_mode_data(Surface, MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
    case BM_CUS:
      return draw_cus_data(Surface);
  }
}

void bottom_bar::draw_fly_mode_data(LKSurface& Surface, MapWindow::Mode::TModeFly mode) const {
  for (unsigned i = 1; i <= get_box_count(); ++i) {
    draw_lkvalues_data(Surface, i - 1, GetInfoboxIndex(i, mode));
  }
}

void bottom_bar::draw_lkvalues_data(LKSurface& Surface, unsigned box_num, short value_id,
                                    const TCHAR* custom_title) const {
  if (box_num < get_box_count()) {
    TCHAR Value[LKSIZEBUFFERVALUE];
    TCHAR Unit[LKSIZEBUFFERUNIT];
    TCHAR Title[LKSIZEBUFFERTITLE];
    DrawBmp_t Bmp = BmpNone;

    bool valid = MapWindow::LKFormatValue(value_id, true, Value, Unit, Title, &Bmp);
    if (custom_title) {
      _tcscpy(Title, custom_title);
    }
    draw_box(Surface, box_num, Value, valid ? Unit : nullptr, Title, Bmp);
  }
}

void bottom_bar::draw_lkvalues_data(LKSurface& Surface, const std::array<short, 10>& array) const {
  for (unsigned i = 0; i < std::size(array); ++i) {
    draw_lkvalues_data(Surface, i, array[i]);
  }
}

void bottom_bar::draw_cru_data(LKSurface& Surface) const {
  auto array = []() -> std::array<short, 10> {
    if (ISCAR) {
      return {{LK_ODOMETER,   LK_GNDSPEED,   LK_HNAV,       LK_TRACK, LK_TRIP_SPEED_AVG,
              LK_TIME_LOCAL, LK_TIME_LOCAL, LK_TIME_LOCAL, LK_IAS,   LK_SPEED_MC}};
    } else {
      return {{LK_TL_AVG,  LK_GNDSPEED, LK_HNAV,      LK_TRACK, LK_HEADWINDSPEED,
              LK_LD_INST, LK_LD_AVR,   LK_LD_CRUISE, LK_IAS,   LK_SPEED_MC}};
    }
  };
  draw_lkvalues_data(Surface, array());
}

void bottom_bar::draw_hgt_data(LKSurface& Surface) const {
  std::array<short, 10> array = {{LK_HGPS, LK_HBARO, LK_QFE,     LK_HAGL, LK_FL,
                                 LK_AQNH, LK_HGND,  LK_AALTAGL, LK_IAS,  LK_SPEED_MC}};

  if (Units::GetAltitudeUnit() == unMeter) {
    std::swap(array[4], array[5]);  // swap LK_FL and LK_AQNH
  }
  draw_lkvalues_data(Surface, array);
}

void bottom_bar::draw_aux_data(LKSurface& Surface) const {
  auto array = []() -> std::array<short, 10> {
    if (ISCAR) {
      return {{LK_TC_ALL, LK_TRIP_STEADY, LK_TRIP_TOTAL, LK_SPEED_AVG, LK_DIST_AVG,
              LK_MAXALT, LK_MAXALT,      LK_MAXHGAINED, LK_IAS,       LK_SPEED_MC}};
    } else {
      short second_value = UseContestEngine() ? LK_OLC_CLASSIC_DIST : LK_ODOMETER;

      return {{LK_TC_ALL,   second_value, LK_TIMEFLIGHT, LK_HOME_DIST, LK_MAXALT,
              LK_ODOMETER, LK_PRCCLIMB,  LK_MAXHGAINED, LK_IAS,       LK_SPEED_MC}};
    }
  };

  draw_lkvalues_data(Surface, array());
}

void bottom_bar::draw_tsk_data(LKSurface& Surface) const {
  draw_lkvalues_data(Surface, {{LK_FIN_DIST, LK_FIN_ALTDIFF, LK_FIN_ETE, LK_TASK_DISTCOV, LK_START_ALT, LK_SPEEDTASK_ACH,
                               LK_FIN_GR, LK_SPEEDTASK_AVG, LK_IAS, LK_SPEED_MC}});
}

void bottom_bar::draw_alt_data(LKSurface& Surface) const {
  if (ScreenLandscape) {
    draw_lkvalues_data(Surface, 0, LK_BESTALTERN_GR);
    draw_lkvalues_data(Surface, 1, LK_BESTALTERN_ARRIV, _T("<<<"));
    draw_lkvalues_data(Surface, 2, LK_ALTERN1_GR);
    draw_lkvalues_data(Surface, 3, LK_ALTERN1_ARRIV, _T("<<<"));
    draw_lkvalues_data(Surface, 4, LK_ALTERN2_GR);
    draw_lkvalues_data(Surface, 5, LK_ALTERN2_ARRIV, _T("<<<"));
    draw_lkvalues_data(Surface, 6, LK_HOME_ARRIVAL);
    draw_lkvalues_data(Surface, 7, LK_HOMERADIAL);
    draw_lkvalues_data(Surface, 8, LK_IAS);
    draw_lkvalues_data(Surface, 9, LK_SPEED_MC);
  } else {
    draw_lkvalues_data(Surface, 0, LK_BESTALTERN_GR);
    draw_lkvalues_data(Surface, _col_count, LK_BESTALTERN_ARRIV, _T(""));
    if (_col_count > 1) {
      draw_lkvalues_data(Surface, 1, LK_ALTERN1_GR);
      draw_lkvalues_data(Surface, _col_count + 1, LK_ALTERN1_ARRIV, _T(""));
    }
    if (_col_count > 2) {
      draw_lkvalues_data(Surface, 2, LK_ALTERN2_GR);
      draw_lkvalues_data(Surface, _col_count + 2, LK_ALTERN2_ARRIV, _T(""));
    }
    if (_col_count > 3) {
      draw_lkvalues_data(Surface, 3, LK_HOME_ARRIVAL);
      draw_lkvalues_data(Surface, _col_count + 3, LK_HOMERADIAL);
    }
    if (_col_count > 4) {
      draw_lkvalues_data(Surface, 4, LK_IAS);
      draw_lkvalues_data(Surface, _col_count + 4, LK_SPEED_MC);
    }
  }
}

void bottom_bar::draw_sys_data(LKSurface& Surface) const {
  draw_lkvalues_data(Surface, {{LK_BATTERY, LK_EXTBATT1VOLT, LK_SAT, LK_HBAR_AVAILABLE, LK_CPU, LK_LOGGER,
                               LK_EXTBATT2VOLT, LK_EXTBATTBANK, LK_IAS, LK_SPEED_MC}});
}

void bottom_bar::draw_cus_data(LKSurface& Surface) const {
  for (unsigned i = 1; i <= get_box_count(); ++i) {
    draw_lkvalues_data(Surface, i - 1, GetInfoboxType(i));
  }
}

static void CropString(TCHAR* String, unsigned max_char) {
#ifdef UNICODE
  String[max_char] = _T('\0');
#else
  // utf8 : number of Char are not equal to Number of Byte, we need to iterate each code point
  auto next = NextUTF8(String);
  while (next.second && --max_char) {
    next = NextUTF8(next.second);
  }
  // we can use const cast here "next.second" are inside String content.
  char* pend = const_cast<char*>(next.second);
  if (pend) {
    // check if pend are inside String.
    LKASSERT(static_cast<size_t>(pend - String) <= strlen(String));
    (*pend) = '\0';
  }
#endif
}

/**
 * @Unit : if nullptr, amber color is used to draw the value.
 */
void bottom_bar::draw_box(LKSurface& Surface, unsigned box_num, TCHAR* Value, TCHAR* Unit, TCHAR* Title,
                          DrawBmp_t& Bmp) const {
  PixelRect box_rect = {_rect.GetTopLeft(), _box_size};
  box_rect.Offset((box_num % _col_count) * _box_size.cx, (box_num / _col_count) * _box_size.cy +
          IBLSCALE(2));
  RasterPoint box_center = box_rect.GetCenter();

  CropString(Title, 7);

  Surface.SelectObject(LK8BottomBarTitleFont);
  MapWindow::LKWriteText(Surface, Title, box_center.x, box_rect.top + (_title_height / 2), WTMODE_NORMAL,
                         WTALIGN_CENTER, _text_color, false);

  const LKIcon* pBmpTemp = MapWindow::GetDrawBmpIcon(Bmp);
  if (pBmpTemp) {
    // draw image horizonal centered
    PixelScalar Icon_size = _value_size.cy - NIBLSCALE(2);
    pBmpTemp->Draw(Surface, box_center.x - (Icon_size / 2), box_rect.top + _title_height, Icon_size, Icon_size);
  } else {
    Surface.SelectObject(LK8BottomBarValueFont);

    PixelScalar value_y = box_rect.top + _title_height + (_value_size.cy / 2);
    LKColor value_color = Unit ? _text_color : (IsDithered() ? RGB_WHITE : RGB_AMBER);

    MapWindow::LKWriteText(Surface, Value, box_center.x, value_y, WTMODE_NORMAL, WTALIGN_CENTER, value_color, false);

    if (Unit && !HideUnits) {
      PixelSize value_size = Surface.GetTextSize(Value);
      Surface.SelectObject(LK8BottomBarUnitFont);
      MapWindow::LKWriteText(Surface, Unit, box_center.x + (value_size.cx / 2) + IBLSCALE(1), value_y, WTMODE_NORMAL,
                             WTALIGN_LEFT, _text_color, false);
    }
  }
}
