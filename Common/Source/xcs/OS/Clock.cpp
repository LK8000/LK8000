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

#include "OS/Clock.hpp"
#include <chrono>
#ifdef ANDROID
#include "Java/Global.hxx"
#include "Java/Class.hxx"
#endif
#ifdef WIN32
#include <windows.h>
#endif

namespace {
  template<typename period, typename rep = typename period::rep>
  rep MonotonicClock() {
  using clock = std::chrono::steady_clock;

  static_assert(std::is_same<clock::period, std::nano>::value,
                "clock::duration must be nanoseconds");

  auto now_steady = clock::now().time_since_epoch();
  auto now = std::chrono::duration_cast<period>(now_steady);

  return now.count();
}

}  // namespace

unsigned MonotonicClockMS() {
  return MonotonicClock<std::chrono::milliseconds>();
}

uint64_t MonotonicClockUS() {
  return MonotonicClock<std::chrono::microseconds>();
}

uint64_t MonotonicClockNS() {
  return MonotonicClock<std::chrono::nanoseconds>();
}

double MonotonicClockFloat() {
  return MonotonicClock<std::chrono::duration<double>>();
}

int
GetSystemUTCOffset()
{
#ifdef ANDROID
  JNIEnv* env = Java::GetEnv();
  Java::Class cls(env, "org/LK8000/LK8000");
  jmethodID method_id = env->GetStaticMethodID(cls, "getSystemUTCOffset", "()I");
  return env->CallStaticIntMethod(cls.Get(),method_id);

#elif defined(HAVE_POSIX)
  // XXX implement
  return 0;
#else
  TIME_ZONE_INFORMATION TimeZoneInformation;
  DWORD tzi = GetTimeZoneInformation(&TimeZoneInformation);

  int offset = -TimeZoneInformation.Bias * 60;
  if (tzi == TIME_ZONE_ID_STANDARD)
    offset -= TimeZoneInformation.StandardBias * 60;

  if (tzi == TIME_ZONE_ID_DAYLIGHT)
    offset -= TimeZoneInformation.DaylightBias * 60;

  return offset;
#endif
}
