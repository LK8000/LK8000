/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   http_session_base.h
 * Author: Bruno de Lacheisserie
 *
 * Created on June 05, 2026
 */
#ifndef _TRACKING_HTTP_SESSION_BASE_H_
#define _TRACKING_HTTP_SESSION_BASE_H_

#include <cctype>
#include <initializer_list>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

template <class Derived>
class http_session_base {
public:
  struct url_param {
    url_param() = delete;

    template <typename ValueT>
      requires std::constructible_from<std::string, ValueT&&>
    url_param(std::string name, ValueT&& value)
        : name(std::move(name)), value(std::forward<ValueT>(value)) {}

    template <typename ValueT>
      requires(!std::constructible_from<std::string, ValueT&&> &&
               requires(ValueT v) { std::to_string(v); })
    url_param(std::string name, ValueT value)
        : name(std::move(name)), value(std::to_string(value)) {}

    const std::string name;
    const std::string value;
  };

  static bool ssl_available() {
    return Derived::ssl_available_impl();
  }

  std::string get(const std::string& url) const {
    return request(url);
  }

  std::string get(const std::string& url,
                  std::initializer_list<url_param>&& args) const {
    std::string full_url = url;
    if (args.size()) {
      full_url += '?';
      for (const auto& arg : args) {
        full_url += encode_query_component(arg.name);
        full_url += '=';
        full_url += encode_query_component(arg.value);
        full_url += '&';
      }
      full_url.pop_back();
    }
    return get(full_url);
  }

  std::string post(const std::string& url, const std::string& data,
                   const char* content_type = nullptr) const {
    return derived().request_impl(url, &data, content_type);
  }

  std::string request(const char* server_name, int server_port,
                      const char* query_string) const {
    if (!server_name || !query_string) {
      return {};
    }

    std::string url = (server_port == 443) ? "https://" : "http://";
    url += server_name;
    url += ':';
    url += std::to_string(server_port);
    if (query_string[0] != '/' && query_string[0] != '?') {
      url += '/';
    }
    url += query_string;
    return request(url);
  }

  std::string request(const std::string& url) const {
    return derived().request_impl(url, nullptr, nullptr);
  }

protected:
  static std::string encode_query_component(std::string_view value) {
    constexpr char hex[] = "0123456789ABCDEF";

    std::string out;
    out.reserve(value.size());

    for (unsigned char ch : value) {
      bool is_unreserved = (ch >= 'A' && ch <= 'Z') ||
                           (ch >= 'a' && ch <= 'z') ||
                           (ch >= '0' && ch <= '9') ||
                           ch == '-' || ch == '_' || ch == '.' || ch == '~';
      if (is_unreserved) {
        out.push_back(static_cast<char>(ch));
      }
      else {
        out.push_back('%');
        out.push_back(hex[(ch >> 4) & 0x0F]);
        out.push_back(hex[ch & 0x0F]);
      }
    }

    return out;
  }

private:
  const Derived& derived() const {
    static_assert(std::is_base_of_v<http_session_base<Derived>, Derived>,
                  "Derived must inherit from http_session_base<Derived>");
    return static_cast<const Derived&>(*this);
  }
};

#endif // _TRACKING_HTTP_SESSION_BASE_H_