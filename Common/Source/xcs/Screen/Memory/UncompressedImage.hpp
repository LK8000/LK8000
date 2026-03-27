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

#ifndef XCSOAR_SCREEN_MEMORY_UNCOMPRESSED_IMAGE_HPP
#define XCSOAR_SCREEN_MEMORY_UNCOMPRESSED_IMAGE_HPP

#include "Buffer.hpp"
#include "Screen/Custom/UncompressedImage.hpp"

struct RGBPixelReader {
  const uint8_t *p;

  template<typename PixelTraits>
  typename PixelTraits::color_type Read(PixelTraits) noexcept {
    const uint8_t r = *p++, g = *p++, b = *p++;
    return typename PixelTraits::color_type(r, g, b);
  }
};

/**
 * Read RGBA pixels (4 bytes each), discarding the alpha channel.
 * The memory canvas has no alpha blending, so alpha is dropped
 * and transparent areas composite against an implicit white
 * background.
 */
struct RGBAPixelReader {
  const uint8_t *p;

  template<typename PixelTraits>
  typename PixelTraits::color_type Read(PixelTraits) noexcept {
    const uint8_t r = *p++, g = *p++, b = *p++;
    const uint8_t a = *p++;

    /* Pre-multiply against white so that transparent areas
       render as white instead of black. */
    const uint8_t rb = r + (255 - a) * (255 - r) / 255;
    const uint8_t gb = g + (255 - a) * (255 - g) / 255;
    const uint8_t bb = b + (255 - a) * (255 - b) / 255;
    return typename PixelTraits::color_type(rb, gb, bb);
  }
};

struct GrayPixelReader {
  const uint8_t *p;

  template<typename PixelTraits>
  typename PixelTraits::color_type Read(PixelTraits) noexcept {
    const uint8_t l = *p++;
    return typename PixelTraits::color_type(l, l, l);
  }
};

template<typename PixelTraits, typename Reader>
static inline void
ConvertLine(typename PixelTraits::rpointer_type dest, Reader src,
            unsigned n) noexcept
{
  for (unsigned i = 0; i < n; ++i, dest = PixelTraits::Next(dest, 1)) {
    PixelTraits::WritePixel(dest, src.Read(PixelTraits()));
  }
}

template<typename PixelTraits, typename Format>
static inline void
ConvertImage(WritableImageBuffer<PixelTraits> buffer,
             const uint8_t *src, int src_pitch) noexcept
{
  typename PixelTraits::rpointer_type dest = buffer.data;

  for (unsigned i = 0; i < buffer.height; ++i,
                dest = PixelTraits::NextRow(dest, buffer.pitch, 1),
                src += src_pitch) {
    ConvertLine<PixelTraits>(dest, Format{src}, buffer.width);
  }
}

/**
 * Convert an #UncompressedImage to a WritableImageBuffer.
 *
 * @buffer : destination buffer
 * @uncompressed : source image
 */
template<typename PixelTraits>
static inline void
ImportSurface(WritableImageBuffer<PixelTraits> &buffer,
              const UncompressedImage &uncompressed)
{
  buffer.Allocate(uncompressed.GetWidth(), uncompressed.GetHeight());

  switch (uncompressed.GetFormat()) {
  case UncompressedImage::Format::INVALID:
    assert(false);
    gcc_unreachable();

  case UncompressedImage::Format::RGB:
    ConvertImage<PixelTraits, RGBPixelReader>(buffer,
                                (const uint8_t *)uncompressed.GetData(),
                                uncompressed.GetPitch());
    break;

  case UncompressedImage::Format::RGBA:
    ConvertImage<PixelTraits, RGBAPixelReader>(buffer,
                                (const uint8_t *)uncompressed.GetData(),
                                uncompressed.GetPitch());
    break;

  case UncompressedImage::Format::GRAY:
    ConvertImage<PixelTraits, GrayPixelReader>(buffer,
                                 (const uint8_t *)uncompressed.GetData(),
                                 uncompressed.GetPitch());
    break;
  }
}

#endif
