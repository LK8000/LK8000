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

#include "LibPNG.hpp"
#include "UncompressedImage.hpp"

#include <png.h>
#include <memory>
#include <assert.h>
#include <string.h>

struct PNGCallbackContext {
  const uint8_t *data;
};

static void
PNGReadCallback(png_structp _ctx, png_bytep area, png_size_t size)
{
  PNGCallbackContext &ctx = *(PNGCallbackContext *)png_get_io_ptr(_ctx);

  memcpy(area, ctx.data, size);
  ctx.data += size;
}

static UncompressedImage::Format
ConvertColorType(int color_type)
{
  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY:
    return UncompressedImage::Format::GRAY;

  case PNG_COLOR_TYPE_RGB:
    return UncompressedImage::Format::RGB;

  case PNG_COLOR_TYPE_RGB_ALPHA:
    return UncompressedImage::Format::RGBA;

  default:
    /* not supported */
    return UncompressedImage::Format::INVALID;
  }
}

static UncompressedImage
LoadPNG(png_structp png_ptr, png_infop info_ptr,
        const void *data, size_t size)
{
  assert(data != nullptr);

  PNGCallbackContext ctx{(const uint8_t *)data};

  png_set_read_fn(png_ptr, &ctx, PNGReadCallback);

  png_read_info(png_ptr, info_ptr);

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
               &color_type, &interlace_type, nullptr, nullptr);

  /* shrink 16 bit to 8 bit */
  png_set_strip_16(png_ptr);

  /* grow 1,2,4 bit to 8 bit */
  png_set_expand(png_ptr);

  if (color_type == PNG_COLOR_TYPE_PALETTE)
    /* no thanks, we don't want a palette, give us RGB or gray
       instead */
    png_set_palette_to_rgb(png_ptr);

  png_read_update_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
               &color_type, &interlace_type, nullptr, nullptr);

  /* check the color type */

  const UncompressedImage::Format format = ConvertColorType(color_type);
  if (format == UncompressedImage::Format::INVALID)
    return UncompressedImage::Invalid();

  /* allocate memory for the uncompressed pixels */

  const unsigned num_channels = png_get_channels(png_ptr, info_ptr);
  const unsigned pitch = (num_channels * bit_depth) / 8 * width;

  try {
    auto uncompressed = std::make_unique<uint8_t[]>(pitch * height);
    auto rows = std::make_unique<png_bytep[]>(height);

    for (unsigned i = 0; i < height; ++i) {
      rows[i] = &uncompressed[i * pitch];
    }

    /* uncompress and import into an OpenGL texture */
    png_read_image(png_ptr, rows.get());

    return UncompressedImage(format, pitch, width, height, uncompressed.release());
  }
  catch(std::bad_alloc& e) {
    fprintf(stderr, "LoadPNG : %s", e.what());
  }
  return UncompressedImage::Invalid();
}

UncompressedImage
LoadPNG(const void *data, size_t size)
{
  png_structp png_ptr =
    png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (png_ptr == nullptr)
    return UncompressedImage::Invalid();

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    return UncompressedImage::Invalid();
  }

  UncompressedImage result = LoadPNG(png_ptr, info_ptr, data, size);
  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  return result;
}

UncompressedImage
LoadPNGFile(const TCHAR *path)
{

    FILE *file = _tfopen(path, _T("rb"));
    if (file == nullptr)
        return UncompressedImage::Invalid();

    /* Create and initialize the png_struct
     * with the desired error handler
     * functions.  If you want to use the
     * default stderr and longjump method,
     * you can supply NULL for the last
     * three parameters.  We also supply the
     * the compiler header file version, so
     * that we know if the application
     * was compiled with a compatible version
     * of the library.  REQUIRED
     */
    png_structp png_ptr =
            png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr) {
        fclose(file);
        return UncompressedImage::Invalid();
    }

    /* Allocate/initialize the memory
     * for image information.  REQUIRED. */
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        fclose(file);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return UncompressedImage::Invalid();
    }
    /* Set error handling if you are
     * using the setjmp/longjmp method
     * (this is the normal method of
     * doing things with libpng).
     * REQUIRED unless you  set up
     * your own error handlers in
     * the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated
         * with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(file);
        /* If we get here, we had a
         * problem reading the file */
        return UncompressedImage::Invalid();
    }

    /* Set up the output control if
     * you are using standard C streams */
    png_init_io(png_ptr, file);

    /* If we have already
     * read some of the signature */
    unsigned int sig_read = 0;
    png_set_sig_bytes(png_ptr, sig_read);

    /*
     * If you have enough memory to read
     * in the entire image at once, and
     * you need to specify only
     * transforms that can be controlled
     * with one of the PNG_TRANSFORM_*
     * bits (this presently excludes
     * dithering, filling, setting
     * background, and doing gamma
     * adjustment), then you can read the
     * entire image (including pixels)
     * into the info structure with this
     * call
     *
     * PNG_TRANSFORM_STRIP_16 |
     * PNG_TRANSFORM_PACKING  forces 8 bit
     * PNG_TRANSFORM_EXPAND forces to
     *  expand a palette into RGB
     */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB, NULL);

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
            &interlace_type, NULL, NULL);

    /* check the color type */
    const UncompressedImage::Format format = ConvertColorType(color_type);
    if (format == UncompressedImage::Format::INVALID) {
        fclose(file);
        return UncompressedImage::Invalid();
    }

    /* allocate memory for the uncompressed pixels */
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    uint8_t *uncompressed = new uint8_t[row_bytes * height];

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (png_uint_32 i = 0; i < height; ++i) {
        memcpy(uncompressed + (row_bytes * i), row_pointers[i], row_bytes);
    }

    /* Clean up after the read,
     * and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    /* Close the file */
    fclose(file);

    /* That's it */
    return UncompressedImage(format, row_bytes, width, height, uncompressed);
}
