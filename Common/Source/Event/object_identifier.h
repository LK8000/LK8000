/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   object_identifier.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 22 april 2025
 */

#ifndef EVENT_OBJECT_IDENTIFIER_H
#define EVENT_OBJECT_IDENTIFIER_H
#include <variant>
#include <cstdint>
#include <cstddef>
#include <utility>
#include "../Airspace/LKAirspace.h"

struct im_airspace {
  im_airspace() = delete;

  // copy ctor must be explicitly defined to avoid "maybe uninitialized" warning when defaulted
  im_airspace(const im_airspace& other) noexcept {  
    pAirspace = other.pAirspace;
  }

  im_airspace(im_airspace&& other) noexcept {
    pAirspace = std::exchange(other.pAirspace, nullptr);
  }

  explicit im_airspace(const CAirspacePtr& pAsp) : pAirspace(pAsp) {}

  im_airspace& operator=(const im_airspace& other) noexcept {
    pAirspace = other.pAirspace;
    return *this;
  }

  im_airspace& operator=(im_airspace&& other) noexcept {
    pAirspace = std::exchange(other.pAirspace, nullptr);
    return *this;
  }

  CAirspacePtr pAirspace = {};

  bool operator==(const im_airspace& rhs) const {
    return pAirspace == rhs.pAirspace;
  }
};

struct im_waypoint {
  im_waypoint() = delete;

  size_t idx;

  bool operator==(const im_waypoint& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_flarm {
  im_flarm() = delete;

  int idx;

  bool operator==(const im_flarm& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_task_pt {
  im_task_pt() = delete;

  int idx;

  bool operator==(const im_task_pt& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_oracle {
  im_oracle() = delete;

  int idx;

  bool operator==(const im_oracle& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_weatherst {
  im_weatherst() = delete;

  size_t idx;

  bool operator==(const im_weatherst& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_own_pos {
  bool operator==(const im_own_pos& rhs) const {
    return true;
  }
};

struct im_team {
  bool operator==(const im_team& rhs) const {
    return true;
  }
};

struct im_thermal {
  im_thermal() = delete;

  size_t idx;

  bool operator==(const im_thermal& rhs) const {
    return idx == rhs.idx;
  }
};

using im_object_variant = std::variant<im_airspace, im_waypoint, im_flarm, im_task_pt, im_oracle, im_weatherst,
                                       im_own_pos, im_team, im_thermal>;

static_assert(std::is_copy_constructible_v<im_object_variant>,
              "im_object_variant must be nothrow copy constructible");
static_assert(std::is_nothrow_move_constructible_v<im_object_variant>,
              "im_object_variant must be nothrow move constructible");

static_assert(std::is_copy_assignable_v<im_object_variant>,
              "im_object_variant must be nothrow copy assignable");
static_assert(std::is_nothrow_move_assignable_v<im_object_variant>,
              "im_object_variant must be nothrow move assignable");

#endif  // EVENT_OBJECT_IDENTIFIER_H
