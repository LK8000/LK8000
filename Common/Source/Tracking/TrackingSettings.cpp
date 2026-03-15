/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "TrackingSettings.h"

#include <charconv>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

#include "Tracking.h"
#include "Settings/write.h"

namespace tracking {
namespace settings_io {
namespace {

using namespace std::string_view_literals;

template <typename>
inline constexpr bool always_false_v = false;

template <typename T>
bool is_valid_enum(std::underlying_type_t<T> value) noexcept {
  if constexpr (std::is_same_v<T, platform>) {
    switch (static_cast<T>(value)) {
      case platform::none:
      case platform::livetrack24:
      case platform::skylines_aero:
      case platform::ffvl:
        return true;
    }
    return false;
  }
  else {
    static_assert(always_false_v<T>, "Unsupported enum type");
  }
}

std::string escape_string(const std::string& field) {
  std::string out;
  for (char c : field) {
    if (c == '\\' || c == ';') {
      out.push_back('\\');
    }
    out.push_back(c);
  }
  return out;
}

template <typename T>
std::string to_string(const T& value) {
  if constexpr (std::is_same_v<T, bool>) {
    return value ? "true" : "false";
  }
  else if constexpr (std::is_integral_v<T>) {
    return std::to_string(value);
  }
  else if constexpr (std::is_enum_v<T>) {
    using underlying = std::underlying_type_t<T>;
    return to_string(static_cast<underlying>(value));
  }
  else if constexpr (std::is_same_v<T, std::string>) {
    return escape_string(value);
  }
  else {
    static_assert(always_false_v<T>, "Unsupported type");
  }
}

// Parse one escaped field.
// Returns true if a ';' separator was consumed.
// Returns false if end-of-string was reached.
// Sets valid=false on malformed input (dangling trailing escape).
bool ParseEscapedField(const char*& p, std::string& out, bool& valid) {
  out.clear();
  valid = true;
  bool escaped = false;

  while (*p != '\0') {
    char c = *p++;
    if (escaped) {
      out.push_back(c);
      escaped = false;
      continue;
    }

    if (c == '\\') {
      escaped = true;
      continue;
    }

    if (c == ';') {
      return true;
    }

    out.push_back(c);
  }

  if (escaped) {
    valid = false;
    return false;
  }

  return false;
}

template <typename T, typename = void>
struct from_string_parser;

template <>
struct from_string_parser<bool> {
  static bool parse(const std::string& s, bool& out) noexcept {
    if (s == "true") {
      out = true;
      return true;
    }
    if (s == "false") {
      out = false;
      return true;
    }
    return false;
  }
};

template <typename T>
struct from_string_parser<
    T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
  static bool parse(const std::string& s, T& out) noexcept {
    const char* begin = s.data();
    const char* end = std::next(begin, s.size());
    auto result = std::from_chars(begin, end, out);
    return result.ec == std::errc() && result.ptr == end;
  }
};

template <typename T>
struct from_string_parser<T, std::enable_if_t<std::is_enum_v<T>>> {
  static bool parse(const std::string& s, T& out) noexcept {
    using underlying = std::underlying_type_t<T>;
    underlying value;
    if (!from_string_parser<underlying>::parse(s, value)) {
      return false;
    }
    if (!is_valid_enum<T>(value)) {
      return false;
    }
    out = static_cast<T>(value);
    return true;
  }
};

template <>
struct from_string_parser<std::string> {
  static bool parse(const std::string& s, std::string& out) {
    out = s;
    return true;
  }
};

template <typename T>
bool from_string(const std::string& s, T& out) noexcept(
    noexcept(from_string_parser<T>::parse(s, out))) {
  return from_string_parser<T>::parse(s, out);
}

enum class FieldParseResult {
  no_match,
  ok,
  error,
};

FieldParseResult ParseProfileField(const std::string& key,
                                   const std::string& val,
                                   Profile& parsed) {
  if (key == "protocol"sv) {
    if (!from_string(val, parsed.protocol)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }
  if (key == "interval"sv) {
    if (!from_string(val, parsed.interval)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }
  if (key == "radar"sv) {
    if (!from_string(val, parsed.radar)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }
  if (key == "always_on"sv) {
    if (!from_string(val, parsed.always_on)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }
  if (key == "port"sv) {
    if (!from_string(val, parsed.port)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }
  if (key == "server"sv) {
    if (!from_string(val, parsed.server)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }
  if (key == "user"sv) {
    if (!from_string(val, parsed.user)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }
  if (key == "password"sv) {
    if (!from_string(val, parsed.password)) {
      return FieldParseResult::error;
    }
    return FieldParseResult::ok;
  }

  // Forward-compatible: ignore unknown keys.
  return FieldParseResult::no_match;
}

bool ParseTrackingKeyValuePayload(const char* payload, Profile& out) {
  Profile parsed{};
  bool valid = true;
  bool seen_any = false;
  const char* cursor = payload;

  while (true) {
    std::string token;
    bool has_next = ParseEscapedField(cursor, token, valid);
    if (!valid) {
      return false;
    }

    if (!token.empty()) {
      auto eq = token.find('=');
      if (eq == std::string::npos) {
        return false;
      }

      std::string key = token.substr(0, eq);
      std::string val = token.substr(eq + 1);

      switch (ParseProfileField(key, val, parsed)) {
        case FieldParseResult::ok:
          seen_any = true;
          break;
        case FieldParseResult::error:
          return false;
        case FieldParseResult::no_match:
          break;
      }
    }

    if (!has_next) {
      break;
    }
  }

  if (!seen_any) {
    return false;
  }

  out = std::move(parsed);
  return true;
}

void SaveProfileSetting(settings::writer& writer_settings,
                        const Profile& profile) {
  std::string v = ";protocol=" + to_string(profile.protocol) +
                  ";interval=" + to_string(profile.interval) +
                  ";radar=" + to_string(profile.radar) +
                  ";always_on=" + to_string(profile.always_on);

  if (!profile.server.empty()) {
    v += ";server=" + to_string(profile.server);
  }
  if (profile.port != 0) {
    v += ";port=" + to_string(profile.port);
  }
  if (!profile.user.empty()) {
    v += ";user=" + to_string(profile.user);
  }
  if (!profile.password.empty()) {
    v += ";password=" + to_string(profile.password);
  }

  writer_settings("Tracking", v);
}

}  // namespace

bool LoadProfileSettings(const char* key, const char* value,
                         std::vector<Profile>& profiles) {
  if (key != "Tracking"sv) {
    return false;
  }

  Profile parsed;
  if (!ParseTrackingKeyValuePayload(value, parsed)) {
    return false;
  }

  profiles.push_back(std::move(parsed));
  return true;
}

void SaveProfileSettings(settings::writer& writer_settings,
                         const std::vector<Profile>& profiles) {
  // Format: "enabled=...;protocol=...;interval=...;..."
  // Text values are escaped: ';' -> '\\;' and '\\' -> '\\\\'.
  for (const auto& profile : profiles) {
    SaveProfileSetting(writer_settings, profile);
  }
}

}  // namespace settings_io
}  // namespace tracking
