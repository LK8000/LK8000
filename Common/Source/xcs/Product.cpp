/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
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
#include "options.h"
#include "Product.hpp"
#include "LocalPath.h"
#include "utils/base64.h"
#include "Defines.h"
#include "tchar.h"
#include "utils/unique_file_ptr.h"
#include <random>
#include <cctype>
#include "MessageLog.h"

namespace {

static constexpr TCHAR kDeviceIdFileName[] = _T("device_id");

template<size_t size>
void GetFilePath(TCHAR (&path)[size]) {
  LocalPath(path, _T(LKD_SYSTEM), kDeviceIdFileName);
}

std::string LoadDeviceId() {
  TCHAR path[4096] = {};
  GetFilePath(path);

  auto fp = make_unique_file_ptr(path, _T("rt"));
  if (!fp) {
    return {};
  }

  char buffer[128] = {};
  const char* line = fp.fgets(buffer, sizeof(buffer));

  if (!line) {
    return {};
  }

  std::string id(line);
  while (!id.empty() && (id.back() == '\r' || id.back() == '\n' || id.back() == ' ' || id.back() == '\t')) {
    id.pop_back();
  }

  return IsValidDeviceId(id) ? id : std::string();
}

void SaveDeviceId(const std::string& id) {
  TCHAR path[4096] = {};
  GetFilePath(path);

  auto fp = make_unique_file_ptr(path, _T("wt"));
  if (!fp) {
    StartupStore(_T("Failed to save device ID to %s"), path);
    return;
  }

  if (fp.fputs(id.c_str()) < 0) {
    StartupStore(_T("Error writing device ID to %s"), path);
  }
}

std::string GenerateDeviceId() {
  std::random_device rd;
  uint64_t value =
      ((uint64_t)rd() << 32) | rd();

  return EncodeDeviceId(value);
}

} // namespace

const std::string& GetUniqueDeviceId() {
  static const std::string unique_device_id = []() {
    auto id = LoadDeviceId();
    if (!id.empty()) {
      return id;
    }

    id = GenerateDeviceId();
    SaveDeviceId(id);
    return id;
  }();

  return unique_device_id;
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include <thread>

TEST_CASE("UniqueDeviceId") {

  SUBCASE("GetUniqueDeviceId() returns a valid device ID") {
    const std::string id = GetUniqueDeviceId();
    CHECK(!id.empty());
    CHECK(IsValidDeviceId(id));
  }

  SUBCASE("GetUniqueDeviceId() must return same value on subsequent calls") {
    const std::string id1 = GetUniqueDeviceId();
    const std::string id2 = GetUniqueDeviceId();
    CHECK_EQ(id1, id2);
  }

  SUBCASE(
      "GetUniqueDeviceId() must return same value on subsequent calls in "
      "different threads") {
    const std::string id1 = GetUniqueDeviceId();
    std::string id2;
    std::thread t([&id2]() {
      id2 = GetUniqueDeviceId();
    });
    t.join();
    CHECK_EQ(id1, id2);
  }
}

#endif
