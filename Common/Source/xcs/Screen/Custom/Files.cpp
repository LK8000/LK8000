/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

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

#include "Files.hpp"
#include "OS/FileUtil.hpp"
#include "Compiler.h"

//#define USE_TAHOMA  // only for compatibility checks in LK

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

static const char *const all_font_paths[] = {
#ifdef __APPLE__
#if TARGET_OS_IPHONE
  "/System/Library/Fonts/Cache/Arial.ttf",
  "/System/Library/Fonts/Cache/Georgia.ttf",
  "/System/Library/Fonts/Cache/TimesNewRoman.ttf",
#else
  "/Library/Fonts/Tahoma.ttf",
  "/Library/Fonts/Georgia.ttf",
  "/Library/Fonts/Arial Narrow.ttf",
  "/Library/Fonts/Times New Roman.ttf",
  "/Library/Fonts/Arial.ttf",
  "/Library/Fonts/Microsoft/Arial.ttf",
#endif
#elif defined(WIN32) && !defined(HAVE_POSIX)
  /* just for the experimental WINSDL target */
  "c:\\windows\\fonts\\arial.ttf",
#elif defined(KOBO)
  "/opt/LK8000/share/fonts/DejaVuSansCondensed.ttf",
#else
#ifdef USE_TAHOMA
  "/usr/share/fonts/truetype/ms/TAHOMA.TTF",
  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf",
#endif

  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansCondensed.ttf",
  "/usr/share/fonts/TTF/dejavu/DejaVuSansCondensed.ttf",
  "/usr/share/fonts/truetype/DejaVuSansCondensed.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf",
  "/usr/share/fonts/dejavu/DejaVuSansCondensed.ttf",
  "/usr/share/fonts/truetype/droid/DroidSans.ttf",
  "/usr/share/fonts/truetype/ttf-droid/DroidSans.ttf",
  "/usr/share/fonts/TTF/droid/DroidSans.ttf",
  "/usr/share/fonts/droid/DroidSans.ttf",
  "/usr/share/fonts/truetype/droid/DroidSans.ttf",
  "/usr/share/fonts/truetype/DroidSans.ttf",
  "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf",
  "/usr/share/fonts/corefonts/arial.ttf",
  "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
  "/usr/share/fonts/freefont-ttf/FreeSans.ttf",
  "/usr/share/fonts/truetype/unifont/unifont.ttf",
  "/usr/share/fonts/unifont/unifont.ttf",
  "/usr/share/fonts/corefonts/tahoma.ttf",
#endif
  nullptr
};

static const char *const all_bold_font_paths[] = {
#ifdef __APPLE__
#if TARGET_OS_IPHONE
  "/System/Library/Fonts/Cache/ArialBold.ttf",
  "/System/Library/Fonts/Cache/GeorgiaBold.ttf",
  "/System/Library/Fonts/Cache/TimesNewRomanBold.ttf",
#else
  "/Library/Fonts/Tahoma Bold.ttf",
  "/Library/Fonts/Georgia Bold.ttf",
  "/Library/Fonts/Arial Narrow Bold.ttf",
  "/Library/Fonts/Arial Bold.ttf",
  "/Library/Fonts/Microsoft/Arial Bold.ttf",
#endif
#elif defined(KOBO)
  "/opt/LK8000/share/fonts/DejaVuSansCondensed-Bold.ttf",
#elif defined(HAVE_POSIX)
#ifdef USE_TAHOMA
  "/usr/share/fonts/truetype/ms/TAHOMABD.TTF",
  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans-Bold.ttf",
#endif
  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansCondensed-Bold.ttf",
  "/usr/share/fonts/TTF/dejavu/DejaVuSansCondensed-Bold.ttf",
  "/usr/share/fonts/truetype/DejaVuSansCondensed-Bold.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf",
  "/usr/share/fonts/dejavu/DejaVuSansCondensed-Bold.ttf",
  "/usr/share/fonts/truetype/droid/DroidSans-Bold.ttf",
  "/usr/share/fonts/truetype/ttf-droid/DroidSans-Bold.ttf",
  "/usr/share/fonts/droid/DroidSans-Bold.ttf",
  "/usr/share/fonts/TTF/droid/DroidSans-Bold.ttf",
  "/usr/share/fonts/truetype/droid/DroidSans-Bold.ttf",
  "/usr/share/fonts/truetype/DroidSans-Bold.ttf",
  "/usr/share/fonts/truetype/msttcorefonts/Arial_Bold.ttf",
  "/usr/share/fonts/corefonts/arialbd.ttf",
  "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
  "/usr/share/fonts/freefont-ttf/FreeSansBold.ttf",
#endif
  nullptr
};

static const char *const all_italic_font_paths[] = {
#ifdef __APPLE__
#if TARGET_OS_IPHONE
  "/System/Library/Fonts/Cache/ArialItalic.ttf",
  "/System/Library/Fonts/Cache/GeorgiaItalic.ttf",
  "/System/Library/Fonts/Cache/TimesNewRomanItalic.ttf",
#else
  "/Library/Fonts/Arial Italic.ttf",
  "/Library/Fonts/Microsoft/Arial Italic.ttf",
  "/Library/Fonts/Georgia Italic.ttf",
  "/Library/Fonts/Arial Narrow Italic.ttf",
#endif
#elif defined(KOBO)
  "/opt/LK8000/share/fonts/DejaVuSansCondensed-Oblique.ttf",
#elif defined(HAVE_POSIX)
#ifdef USE_TAHOMA
  "/usr/share/fonts/truetype/ms/TAHOMA.TTF",
  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans-Oblique.ttf",
#endif

  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansCondensed-Oblique.ttf",
  "/usr/share/fonts/TTF/dejavu/DejaVuSansCondensed-Oblique.ttf",
  "/usr/share/fonts/truetype/DejaVuSansCondensed-Oblique.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Oblique.ttf",
  "/usr/share/fonts/dejavu/DejaVuSansCondensed-Oblique.ttf",
  "/usr/share/fonts/truetype/ttf-bitstream-vera/VeraIt.ttf",
  "/usr/share/fonts/truetype/msttcorefonts/Arial_Italic.ttf",
  "/usr/share/fonts/corefonts/ariali.ttf",
  "/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf",
  "/usr/share/fonts/freefont-ttf/FreeSansOblique.ttf",
#endif
  nullptr
};

static const char *const all_bold_italic_font_paths[] = {
#ifdef __APPLE__
#if TARGET_OS_IPHONE
  "/System/Library/Fonts/Cache/ArialBoldItalic.ttf",
  "/System/Library/Fonts/Cache/GeorgiaBoldItalic.ttf",
  "/System/Library/Fonts/Cache/TimesNewRomanBoldItalic.ttf",
#else
  "/Library/Fonts/Arial Bold Italic.ttf",
  "/Library/Fonts/Microsoft/Arial Bold Italic.ttf",
  "/Library/Fonts/Georgia Bold Italic.ttf",
  "/Library/Fonts/Arial Narrow Bold Italic.ttf",
#endif
#elif defined(KOBO)
  "/opt/LK8000/share/fonts/DejaVuSansCondensed-BoldOblique.ttf",
#elif defined(HAVE_POSIX)
  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansCondensed-BoldOblique.ttf",
  "/usr/share/fonts/TTF/dejavu/DejaVuSansCondensed-BoldOblique.ttf",
  "/usr/share/fonts/truetype/DejaVuSansCondensed-BoldOblique.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-BoldOblique.ttf",
  "/usr/share/fonts/dejavu/DejaVuSansCondensed-BoldOblique.ttf",
  "/usr/share/fonts/truetype/ttf-bitstream-vera/VeraBI.ttf",
  "/usr/share/fonts/truetype/msttcorefonts/Arial_Bold_Italic.ttf",
  "/usr/share/fonts/corefonts/arialbi.ttf",
  "/usr/share/fonts/truetype/freefont/FreeSansBoldOblique.ttf",
  "/usr/share/fonts/freefont-ttf/FreeSansBoldOblique.ttf",
#endif
  nullptr
};

static const char *const all_monospace_font_paths[] = {
#ifdef __APPLE__
#if TARGET_OS_IPHONE
  "/System/Library/Fonts/Cache/CourierNew.ttf",
#else
  "/Library/Fonts/Courier New.ttf",
#endif
#elif defined(KOBO)
  "/opt/LK8000/share/fonts/DejaVuSansMono.ttf",
#else
  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf",
  "/usr/share/fonts/truetype/DejaVuSansMono.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
  "/usr/share/fonts/truetype/droid/DroidSansMono.ttf",
  "/usr/share/fonts/truetype/ttf-droid/DroidSansMono.ttf",
  "/usr/share/fonts/TTF/droid/DroidSansMono.ttf",
  "/usr/share/fonts/truetype/DroidSansMono.ttf",
  "/usr/share/fonts/truetype/droid/DroidSansMono.ttf",
  "/usr/share/fonts/truetype/msttcorefonts/couri.ttf",
  "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
  "/usr/share/fonts/TTF/freefont/FreeMono.ttf",
#endif
  nullptr
};

gcc_const
static const char *
FindFile(const char *const*list)
{
  for (const char *const* i = list; *i != nullptr; ++i)
    if (File::Exists(*i))
      return *i;

  return nullptr;
}

const char *
FindDefaultFont()
{
  return FindFile(all_font_paths);
}

const char *
FindDefaultBoldFont()
{
  return FindFile(all_bold_font_paths);
}

const char *
FindDefaultItalicFont()
{
  return FindFile(all_italic_font_paths);
}

const char *
FindDefaultBoldItalicFont()
{
  return FindFile(all_bold_italic_font_paths);
}

const char *
FindDefaultMonospaceFont()
{
  return FindFile(all_monospace_font_paths);
}
