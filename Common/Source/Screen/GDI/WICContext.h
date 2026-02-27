#pragma once

#include <windows.h>
#include <wincodec.h>
#include <stdexcept>
#include "OS/Win/ComHelper.h"


class WICContext {
 public:
  WICContext() {
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWICImagingFactory, (void**)&factory);
    if (FAILED(hr)) {
      throw std::runtime_error("CoCreateInstance for WICImagingFactory failed");
    }
  }

  ~WICContext() noexcept {
    // Explicitly release the factory while COM is still initialized.
    factory.reset();
  }

  // Bool-like check: true when the factory is available.
  explicit operator bool() const noexcept { 
    return factory.get() != nullptr;
  }

  // Convenience helpers that wrap calls to the imaging factory and
  // propagate failures as exceptions. Returning `ComPtr<T>` makes
  // ownership clear and keeps callers concise.
  ComPtr<IWICStream> CreateStream() const {
    ComPtr<IWICStream> stream;
    if (!factory || FAILED(factory->CreateStream(&stream))) {
      throw std::runtime_error("Failed to create WIC stream.");
    }
    return stream;
  }

  ComPtr<IWICBitmapDecoder> CreateDecoderFromStream(const ComPtr<IWICStream>& stream) const {
    ComPtr<IWICBitmapDecoder> decoder;
    if (!factory || FAILED(factory->CreateDecoderFromStream(
                      stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder))) {
      throw std::runtime_error("Failed to create WIC decoder.");
    }
    return decoder;
  }

  ComPtr<IWICFormatConverter> CreateFormatConverter() const {
    ComPtr<IWICFormatConverter> converter;
    if (!factory || FAILED(factory->CreateFormatConverter(&converter))) {
      throw std::runtime_error("Failed to create WIC format converter.");
    }
    return converter;
  }

  // Non-copyable
  WICContext(const WICContext&) = delete;
  WICContext& operator=(const WICContext&) = delete;
  // Non-Movable
  WICContext(WICContext&&) = delete;
  WICContext& operator=(WICContext&&) = delete;


 private:
  // Ensure COM is initialized for the lifetime of this context *before*
  // any COM pointers are destroyed. Declaring `com_init` first causes it
  // to be constructed before `factory` and destroyed after `factory`, so
  // `factory->Release()` runs while COM is still initialized.
  ComInit com_init;
  ComPtr<IWICImagingFactory> factory;
};
