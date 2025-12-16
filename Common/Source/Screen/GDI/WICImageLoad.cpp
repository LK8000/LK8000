#include "WICImageLoad.h"
#include <stdexcept>
#include <wincodec.h>
#include "WICContext.h"

using IWICStreamPtr = ComPtr<IWICStream>;
using IWICBitmapDecoderPtr = ComPtr<IWICBitmapDecoder>;
using IWICBitmapFrameDecodePtr = ComPtr<IWICBitmapFrameDecode>;
using IWICFormatConverterPtr = ComPtr<IWICFormatConverter>;

LKBitmap LoadImageFromMemoryWIC(WICContext& ctx,
                                 const std::vector<uint8_t>& data) {
  if (!ctx.factory) {
    throw std::runtime_error("WIC context is not initialized.");
  }

  IWICStreamPtr stream;
  if (FAILED(ctx.factory->CreateStream(&stream))) {
    throw std::runtime_error("Failed to create WIC stream.");
  }

  if (FAILED(stream->InitializeFromMemory((BYTE*)data.data(),
                                          (DWORD)data.size()))) {
    throw std::runtime_error(
        "Failed to initialize WIC stream with image data.");
  }

  IWICBitmapDecoderPtr decoder;
  if (FAILED(ctx.factory->CreateDecoderFromStream(
          stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder))) {
    throw std::runtime_error("Failed to create WIC decoder.");
  }

  IWICBitmapFrameDecodePtr frame;
  if (FAILED(decoder->GetFrame(0, &frame))) {
    throw std::runtime_error("Failed to get WIC frame.");
  }

  IWICFormatConverterPtr converter;
  if (FAILED(ctx.factory->CreateFormatConverter(&converter))) {
    throw std::runtime_error("Failed to create WIC format converter.");
  }

  if (FAILED(converter->Initialize(frame, GUID_WICPixelFormat32bppBGR,
                                   WICBitmapDitherTypeNone, nullptr, 0.0,
                                   WICBitmapPaletteTypeCustom))) {
    throw std::runtime_error("Failed to convert image format.");
  }

  UINT w = 0, h = 0;
  frame->GetSize(&w, &h);

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = w;
  bmi.bmiHeader.biHeight = -(LONG)h;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* bits = nullptr;
  HDC hdc = GetDC(nullptr);
  HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
  ReleaseDC(nullptr, hdc);

  if (FAILED(converter->CopyPixels(nullptr, w * 3, w * h * 3, (BYTE*)bits))) {
    DeleteObject(hBmp);
    throw std::runtime_error("Failed to copy image pixels.");
  }

  return LKBitmap(hBmp);
}
