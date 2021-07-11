/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS"
  on XCSoar github.

  LK8000 Tactical Flight Computer -  WWW.LK8000.ORG
  Copyright (C) 2015 The LK8000 Project
  Released under GNU/GPL License v.2 or later
  See CREDITS.TXT file for authors and copyrights


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/Font.hpp"
#include "Screen/Debug.hpp"
#include "Screen/Custom/Files.hpp"
#include "Init.hpp"
#include "Asset.hpp"

#ifndef ENABLE_OPENGL
#include "Thread/Mutex.hpp"
#endif

#ifndef _UNICODE
#include "Util/UTF8.hpp"
#endif

#if defined(__clang__) && defined(__arm__)
/* work around warning: 'register' storage class specifier is
   deprecated */
#define register
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

extern FT_Library ft_library;
#include <algorithm>
#include <functional>
#include <assert.h>

#ifndef ENABLE_OPENGL
/**
 * libfreetype is not thread-safe; this global Mutex is used to
 * protect libfreetype from multi-threaded access.
 */
static Mutex freetype_mutex;
#endif

static FT_Int32 load_flags = FT_LOAD_DEFAULT;
static FT_Render_Mode render_mode = FT_RENDER_MODE_NORMAL;

static const char *font_path;
static const char *bold_font_path;
static const char *italic_font_path;
static const char *bold_italic_font_path;
static const char *monospace_font_path;

gcc_const
static inline bool
IsMono()
{
#ifdef KOBO
  return FreeType::mono;
#else
  return IsDithered();
#endif
}

gcc_const
static inline FT_Long
FT_FLOOR(FT_Long x) 
{
  return x >> 6;
}


gcc_const
static inline FT_Long
FT_CEIL(FT_Long x) 
{
  return x >> 6;
}


gcc_pure
static std::pair<unsigned, const TCHAR *>
NextChar(const TCHAR *p)
{
#ifdef _UNICODE
  return std::make_pair(unsigned(*p), p + 1);
#else
  return NextUTF8(p);
#endif
}

void
Font::Initialise()
{
  if (IsMono()) {
    /* disable anti-aliasing */
    load_flags |= FT_LOAD_TARGET_MONO;
    render_mode = FT_RENDER_MODE_MONO;
  }

  font_path = FindDefaultFont();
  bold_font_path = FindDefaultBoldFont();
  italic_font_path = FindDefaultItalicFont();
  bold_italic_font_path = FindDefaultBoldItalicFont();
  monospace_font_path = FindDefaultMonospaceFont();
}

gcc_pure
static unsigned
GetCapitalHeight(FT_Face face)
{
#ifndef ENABLE_OPENGL
  const ScopeLock protect(freetype_mutex);
#endif

  FT_UInt i = FT_Get_Char_Index(face, 'M');
  if (i == 0)
    return 0;

  FT_Error error = FT_Load_Glyph(face, i, load_flags);
  if (error)
    return 0;

  return FT_CEIL(face->glyph->metrics.height);
}

bool
Font::LoadFile(const char *file, UPixelScalar ptsize, bool bold, bool italic)
{
  assert(IsScreenInitialized());

  Destroy();

  FT_Face new_face = FreeType::Load(file);
  if (new_face == nullptr)
    return false;

  if (ptsize>1000) {
      ptsize-=1000;
      demibold=true;
  } else {
      demibold=false;
  }

  // Paolo: in order to get back desired ppts we must ask for 62, not 64
  FT_Error error = ::FT_Set_Char_Size(new_face, 0, ptsize<<6, 0, 62);
  if (error) {
    ::FT_Done_Face(new_face);
    return false;
  }

  const FT_Fixed y_scale = new_face->size->metrics.y_scale;

  height = FT_CEIL(FT_MulFix(new_face->height, y_scale));
  ascent_height = FT_CEIL(FT_MulFix(new_face->ascender, y_scale));

  capital_height = ::GetCapitalHeight(new_face);
  if (capital_height == 0)
    capital_height = height;
  
  face = new_face;
  return true;
}

bool
Font::Load(const TCHAR *facename, UPixelScalar height, bool bold, bool italic)
{
  LOGFONT lf;
  lf.lfWeight = bold ? 700 : 500;
  lf.lfHeight = height;
  lf.lfItalic = italic;
  lf.lfPitchAndFamily = FF_DONTCARE | VARIABLE_PITCH;
  return Load(lf);
}

bool
Font::Load(const LOGFONT &log_font)
{
  assert(IsScreenInitialized());

  bool bold = log_font.lfWeight >= 700;
  bool italic = log_font.lfItalic;
  const char *path = nullptr;

  /* check for presence of "real" font and clear the bold or italic
   * flags if found so that freetype does not apply them again to
   * produce a "synthetic" bold or italic version of the font */
  if (italic && bold && bold_italic_font_path != nullptr) {
    path = bold_italic_font_path;
    bold = false;
    italic = false;
  } else if (italic && italic_font_path != nullptr) {
    path = italic_font_path;
    italic = false;
  } else if ((log_font.lfPitchAndFamily & 0x03) == FIXED_PITCH &&
      monospace_font_path != nullptr) {
    path = monospace_font_path;
  } else if (bold && bold_font_path != nullptr) {
    path = bold_font_path;
    bold = false;
  } else {
    path = font_path;
  }

  if (path == nullptr)
    return false;

  return LoadFile(path, log_font.lfHeight > 0 ? log_font.lfHeight : 10,
                  bold, italic);
}

void
Font::Destroy()
{
  if (!IsDefined())
    return;

  assert(IsScreenInitialized());

  ::FT_Done_Face(face);
  face = nullptr;
}

PixelSize
Font::TextSize(const TCHAR *text) const
{
  assert(text != nullptr);
#ifndef _UNICODE
  assert(ValidateUTF8(text));
#endif

  const FT_Face face = this->face;
  const bool use_kerning = FT_HAS_KERNING(face);
  unsigned prev_index = 0;

  int x = 0;

#ifndef ENABLE_OPENGL
  const ScopeLock protect(freetype_mutex);
#endif

  FT_GlyphSlot glyph = nullptr;

  while (true) {
    const auto n = NextChar(text);
    if (n.first == 0)
      break;

    const unsigned ch = n.first;
    text = n.second;

    FT_UInt i = FT_Get_Char_Index(face, ch);
    if (i == 0)
      continue;

    FT_Error error = FT_Load_Glyph(face, i, load_flags);
    if (error)
      continue;

    glyph = face->glyph;

    if (use_kerning && x) {
      if (prev_index != 0) {
        FT_Vector delta;
        FT_Get_Kerning(face, prev_index, i, ft_kerning_default, &delta);
        x += delta.x ;
      }
    }
    prev_index = i;

    x += glyph->advance.x;
  }

  if(glyph) {
      /* fix width for last glyph, horiBearingX+Width can be greater than advance, in most case for Oblique font.
       * this fix need to be done only if advance is lower than horiBearingX+Width, otherwise, we broke right aligned text.
       */
      const FT_Pos offset = glyph->metrics.horiBearingX + glyph->metrics.width - glyph->advance.x;
      if(offset > 0) {
        x += offset;
      }
  }

  if(demibold) {
      // glyph are embolben by 32 (0.5 px), so for avoid rouding artefact, we need to had 1px to width;
      x += (1<<6);
  }
  return PixelSize{unsigned(std::max(0, x >> 6 )), height};
}

static void
RenderGlyph(uint8_t *buffer, unsigned buffer_width, unsigned buffer_height,
            const FT_Bitmap &bitmap, int x, int y)
{
  const uint8_t *src = (const uint8_t *)bitmap.buffer;
  int width = bitmap.width, height = bitmap.rows;
  int pitch = bitmap.pitch;

  if (x < 0) {
    src -= x;
    width += x;
    x = 0;
  }

  if (unsigned(x) >= buffer_width || width <= 0)
    return;

  if (unsigned(x + width) > buffer_width)
    width = buffer_width - x;

  if (y < 0) {
    src -= y * pitch;
    height += y;
    y = 0;
  }

  if (unsigned(y) >= buffer_height || height <= 0)
    return;

  if (unsigned(y + height) > buffer_height)
    height = buffer_height - y;

  buffer += unsigned(y) * buffer_width + unsigned(x);
  for (const uint8_t *end = src + height * pitch;
       src != end; src += pitch, buffer += buffer_width) {
    // with Kerning, Glyph can overlapp previous, so, we need merge bitmap.
      std::transform(src, src + width, buffer, buffer, std::bit_or<uint8_t>());
  }
}

static void
ConvertMono(unsigned char *dest, const unsigned char *src, unsigned n)
{
  for (; n >= 8; n -= 8, ++src) {
    for (unsigned i = 0x80; i != 0; i >>= 1)
      *dest++ = (*src & i) ? 0xff : 0x00;
  }

  for (unsigned i = 0x80; n > 0; i >>= 1, --n)
    *dest++ = (*src & i) ? 0xff : 0x00;
}

static void
ConvertMono(FT_Bitmap &dest, const FT_Bitmap &src)
{
  dest = src;
  dest.pitch = dest.width;
  dest.buffer = new unsigned char[dest.pitch * dest.rows];

  unsigned char *d = dest.buffer, *s = src.buffer;
  for (unsigned y = 0; y < unsigned(dest.rows);
       ++y, d += dest.pitch, s += src.pitch)
    ConvertMono(d, s, dest.width);
}

static void
RenderGlyph(uint8_t *buffer, size_t width, size_t height,
            FT_GlyphSlot glyph, int x, int y)
{

  if (IsMono()) {
    /* with anti-aliasing disabled, FreeType writes each pixel in one
       bit; hack: convert it to 1 byte per pixel and then render it */
    FT_Bitmap bitmap;
    ConvertMono(bitmap, glyph->bitmap);
    RenderGlyph(buffer, width, height, bitmap, x, y);
    delete[] bitmap.buffer;
  } else {
    RenderGlyph(buffer, width, height, glyph->bitmap, x, y);
  }
}

//
// 2015-04-18  note by Paolo
//
// The original code from xcsoar was not spacing characters correctly in LK.
// I have no idea if the problem exists in xcsoar, but we had to fix it here.
// After quite some time, and frustration reading the confusing docs of FreeType,
// I came to this solution which is much more accurate.
// We use subpixels assuming we are always grid-aligned, which is true for our case.
// Kerning does work, but we must always check that we are not going below the 
// available space, to avoid overlapping previous glyph. This is necessary since
// the bitmap operations are copying, not merging, bitmaps. I have no idea how 
// Windows is doing this, apparently microsoft does not merge glyphs too.
// Hinting is required to keep vertical alignement, which may hide a bug.
// I am not sure if all of this (long and complicated) work is correct or it is instead
// a workaround for a more complex problem existing elsewhere.
// However it does work for us, against all odds.
// Update april 21st: merging instead of copying makes the kerning process working fine.
//
void
Font::Render(const TCHAR *text, const PixelSize size, void *_buffer) const
{
  assert(text != nullptr);
#ifndef _UNICODE
  assert(ValidateUTF8(text));
#endif

  uint8_t *buffer = (uint8_t *)_buffer;
  std::fill_n(buffer, BufferSize(size), 0);

  unsigned prev_index = 0;

  int x = 0;

#ifndef ENABLE_OPENGL
  const ScopeLock protect(freetype_mutex);
#endif

  bool use_kerning = FT_HAS_KERNING(face);

  while (true) {
    const auto n = NextChar(text);
    if (n.first == 0)
      break;

    const unsigned ch = n.first;
    text = n.second;

    FT_UInt i = FT_Get_Char_Index(face, ch);
    if (i == 0)
      continue;

    FT_Error error = FT_Load_Glyph(face, i, load_flags);
    if (error)
      continue;

    if (use_kerning && x) {
      if (prev_index != 0) {
        FT_Vector delta;
        FT_Get_Kerning(face, prev_index, i, ft_kerning_default, &delta);
        x += delta.x;
      }
    }
    prev_index = i;

    const FT_GlyphSlot& glyph = face->glyph;
    error = FT_Render_Glyph(glyph, render_mode);
    if (error)
      continue;

    const FT_Glyph_Metrics& metrics = glyph->metrics;

    /* 
     *  32,  0  = Microsoft GDI weight=600 (64=32)
     */
    if (demibold) {
      FT_Bitmap_Embolden(ft_library,&glyph->bitmap, 32,0);
    }

    RenderGlyph((uint8_t *)buffer, size.cx, size.cy,
        glyph, (x >> 6)+glyph->bitmap_left , ascent_height - FT_FLOOR(metrics.horiBearingY));

    x += glyph->advance.x; // equivalent to metrics.horiAdvance
  }
}

