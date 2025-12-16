#pragma once

#include <windows.h>
#include <wincodec.h>
#include <stdexcept>
#include "OS/Win/ComHelper.h"

using IWICImagingFactoryPtr = ComPtr<IWICImagingFactory>;

struct WICContext {
  IWICImagingFactoryPtr factory;

  WICContext() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
      throw std::runtime_error("CoInitializeEx failed");
    }
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWICImagingFactory, (void**)&factory);
    if (FAILED(hr)) {
      CoUninitialize();
      throw std::runtime_error("CoCreateInstance for WICImagingFactory failed");
    }
  }

  ~WICContext() {
    CoUninitialize();
  }

  // Non-copyable
  WICContext(const WICContext&) = delete;
  WICContext& operator=(const WICContext&) = delete;
  // Non-Movable
  WICContext(WICContext&&) = delete;
  WICContext& operator=(WICContext&&) = delete;
};
