#pragma once
#include <windows.h>
#include <combaseapi.h>
#include <utility>

struct ComInit final {
  ComInit() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
      throw std::runtime_error("CoInitializeEx failed");
    }
  }

  ComInit(ComInit&&) = delete;
  ComInit& operator=(ComInit&&) = delete;
  ComInit(const ComInit&) = delete;
  ComInit& operator=(const ComInit&) = delete;

  ~ComInit() {
    CoUninitialize();
  }
};

template <typename ppv_type>
class ComPtr final {
 public:
  ComPtr() = default;
  explicit ComPtr(ppv_type* ppv) : m_ppv(ppv) {}

  ComPtr(ComPtr&& other) {
    std::swap(m_ppv, other.m_ppv);
  }

  ComPtr& operator=(ComPtr&& other) {
    std::swap(m_ppv, other.m_ppv);
    return *this;
  };

  ComPtr(const ComPtr&) = delete;
  ComPtr& operator=(const ComPtr&) = delete;

  ~ComPtr() {
    if (m_ppv) {
      m_ppv->Release();
    }
  }

  void reset() {
    if (m_ppv) {
      m_ppv->Release();
      m_ppv = nullptr;
    }
  }

  ppv_type** operator&() {
    return &m_ppv;
  }

  operator ppv_type*() const {
    return m_ppv;
  }

  ppv_type* operator->() const {
    return m_ppv;
  }

  ppv_type* get() const {
    return m_ppv;
  }

 private:
  ppv_type* m_ppv = nullptr;
};
