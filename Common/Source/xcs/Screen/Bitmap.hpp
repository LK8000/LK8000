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

#ifndef XCSOAR_SCREEN_BITMAP_HPP
#define XCSOAR_SCREEN_BITMAP_HPP

#include "Screen/Point.hpp"

#ifdef USE_MEMORY_CANVAS
#include "Screen/Memory/Buffer.hpp"
#include "Screen/Memory/PixelTraits.hpp"
#endif

#ifdef ANDROID
#include "Java/Object.hxx"
#endif

#ifdef USE_GDI
#include <windows.h>
#endif

#include <assert.h>
#include <tchar.h>

class ResourceId;
class UncompressedImage;
template<typename T> struct ConstBuffer;

#ifdef ENABLE_OPENGL
class GLTexture;
#elif !defined(USE_GDI)
#ifdef GREYSCALE
using BitmapPixelTraits = GreyscalePixelTraits;
#else
using BitmapPixelTraits = BGRAPixelTraits;
#endif
#endif

/**
 * An image loaded from storage.
 */
class Bitmap {
public:
  enum class Type {
    /**
     * A standard bitmap that will be blitted to the screen.  After
     * loading, it will be converted to the screen's pixel format.
     */
    STANDARD,

    /**
     * A monochrome bitmap (1 bit per pixel).
     */
    MONO,
  };

protected:

#ifdef ENABLE_OPENGL
  GLTexture *texture = nullptr;
  PixelSize size;

  bool interpolation = false;

  /**
   * Flip up/down?  Some image formats (such as BMP and TIFF) store
   * the bottom-most row first.
   */
  bool flipped = false;
#elif defined(USE_MEMORY_CANVAS)
  WritableImageBuffer<BitmapPixelTraits> buffer = WritableImageBuffer<BitmapPixelTraits>::Empty();
#else
  HBITMAP bitmap;
#endif

public:
#ifdef USE_GDI
  Bitmap();
#else
  Bitmap() = default;
#endif

  explicit Bitmap(ResourceId id);

#if !defined(USE_GDI) && !defined(ANDROID)
  Bitmap(ConstBuffer<void> buffer);
#endif

  Bitmap(Bitmap &&src);
  Bitmap& operator=(Bitmap &&src );


  ~Bitmap() {
    Reset();
  }

  Bitmap(const Bitmap &other) = delete;
  Bitmap &operator=(const Bitmap &other) = delete;
public:
  bool IsDefined() const {
#ifdef ENABLE_OPENGL
    return texture != nullptr;
#elif defined(USE_MEMORY_CANVAS)
    return buffer.data != nullptr;
#else
    return bitmap != nullptr;
#endif
  }

#ifdef ENABLE_OPENGL
  unsigned GetWidth() const {
    return size.cx;
  }

  unsigned GetHeight() const {
    return size.cy;
  }
#elif defined(USE_MEMORY_CANVAS)
  unsigned GetWidth() const {
    return buffer.width;
  }

  unsigned GetHeight() const {
    return buffer.height;
  }
#else
  unsigned GetWidth() const {
    return GetSize().cx;
  }

  unsigned GetHeight() const {
    return GetSize().cy;
  }
#endif

#ifdef ENABLE_OPENGL
  void EnableInterpolation();
#else
  void EnableInterpolation() {}
#endif

#if !defined(USE_GDI) && !defined(ANDROID)
  bool Load(const UncompressedImage &uncompressed, Type type=Type::STANDARD);
  bool Load(ConstBuffer<void> buffer, Type type=Type::STANDARD);
#endif

  bool Load(ResourceId id, Type type=Type::STANDARD);

#ifdef ANDROID
  bool LoadAssetsFile(const TCHAR *name);
#endif

  /**
   * Load a bitmap and stretch it by the specified zoom factor.
   */
  bool LoadStretch(ResourceId id, unsigned zoom);

#if defined(USE_LIBJPEG) || defined(USE_GDI) || defined(ANDROID)
  bool LoadFile(const TCHAR *path);
#endif

  bool LoadPNGFile(const TCHAR *path);

  void Reset();

  gcc_pure
  const PixelSize GetSize() const;

#ifdef ENABLE_OPENGL
  GLTexture *GetNative() const {
    return texture;
  }
#elif defined(USE_MEMORY_CANVAS)
  ConstImageBuffer<BitmapPixelTraits> GetNative() const {
    return buffer;
  }
#else
  HBITMAP GetNative() const {
    assert(IsDefined());

    return bitmap;
  }
#endif

#ifdef ANDROID
private:
  bool Set(const Java::LocalObject& bmp, Type type);
  bool MakeTexture(const Java::LocalObject& bmp, Type type);
#endif
};

#endif
