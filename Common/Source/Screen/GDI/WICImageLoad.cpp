#include "WICImageLoad.h"
#include <stdexcept>
#include <string>
#include "WICContext.h"

using IWICBitmapFrameDecodePtr = ComPtr<IWICBitmapFrameDecode>;

LKBitmap LoadImageFromMemoryWIC(WICContext& ctx, const void* data,
                                size_t size) {
  auto stream = ctx.CreateStream();

  if (FAILED(stream->InitializeFromMemory(
          static_cast<BYTE*>(const_cast<void*>(data)),
          static_cast<DWORD>(size)))) {
    throw std::runtime_error(
        "Failed to initialize WIC stream with image data.");
  }

  auto decoder = ctx.CreateDecoderFromStream(stream);

  IWICBitmapFrameDecodePtr frame;
  if (FAILED(decoder->GetFrame(0, &frame))) {
    throw std::runtime_error("Failed to get WIC frame.");
  }

  auto converter = ctx.CreateFormatConverter();

  // Convert to BGRA instead of RGBA to avoid channel swapping
  if (FAILED(converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,
                                   WICBitmapDitherTypeNone, nullptr, 0.0,
                                   WICBitmapPaletteTypeCustom))) {
    throw std::runtime_error("Failed to convert image format.");
  }

  UINT w = 0, h = 0;
  if (FAILED(frame->GetSize(&w, &h))) {
    throw std::runtime_error("Failed to get image dimensions.");
  }

  if (w == 0 || h == 0) {
    throw std::runtime_error("Invalid image dimensions.");
  }

  // Stride for 32bpp (simpler calculation)
  UINT stride = w * 4;
  UINT imageSize = stride * h;

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = w;
  bmi.bmiHeader.biHeight = -static_cast<LONG>(h);  // Negative for top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* bits = nullptr;
  HDC hdc = GetDC(nullptr);
  if (!hdc) {
    throw std::runtime_error("Failed to get DC.");
  }

  HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
  ReleaseDC(nullptr, hdc);

  if (!hBmp || !bits) {
    throw std::runtime_error("Failed to create DIB section.");
  }

  // Copy pixels using the stride
  if (FAILED(converter->CopyPixels(nullptr, stride, imageSize,
                                   static_cast<BYTE*>(bits)))) {
    DeleteObject(hBmp);
    throw std::runtime_error("Failed to copy image pixels.");
  }

  return LKBitmap(hBmp);
}