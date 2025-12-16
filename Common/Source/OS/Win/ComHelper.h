#pragma once
#include <windows.h>
#include <combaseapi.h>
#include <utility>

struct ComInit {
  ComInit() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  }
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
