/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Airspace.h
 */
#pragma once

#include <cstdint>
#include <optional>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include "tchar.h"
#include "Util/tstring.hpp"
#include "Screen/LKColor.h"

namespace Airspace {

/**
 * Airspace Type and Class value is maintained in same enum to simplify
 * compatibilitty handling (OpenAIR-V1 / OpenAIR-V2 / OpenAIR-Poaf / OpenAIP)
 */

enum class Type : uint8_t {
  OTHER, // used as unknown value for backward compatibility, !! must be the first !!

  // Airspace Class:  AC Field in OpenAir V2 Format
  A,      // Class A,
  B,      // Class B,
  C,      // Class C,
  D,      // Class D,
  E,      // Class E,
  F,      // Class F,
  G,      // Class G,
  UNC,    // Unclassified,


  // Airspace Type: AY Field in OpenAir V2 Format
  ACCSEC,   //	Airspace providing communication frequency in remote areas
  ADIZ,     //	Air Defence Ident Zone
  ALERT,    //	Alert Area
  ASRA,     //	Aerial Sporting Or Recreational Activity
  ATZ,      //	Aerodrome Traffic Zone
  AWY,      //	Airway
  CTA,      //	Controlled Traffic Area
  CTR,      //	Control Zone
  CUSTOM,   //	Custom or user defined airspace
  FIR,      //	Flight Information Region
  FIS,      //	Flight Information Service Sector
  GSEC,     //	Gliding Sector
  HTZ,      //	Helicopter Traffic Zone
  LTA,      //	Lower Traffic Area (Allows VFR Traffic in CTA)
  MATZ,     //	Military Airport Traffic Zone
  MTA,      //	Military Training Area
  MTR,      //	Military Training Route
  N,        //	NTOAM Affected Area
  OFR,      //	Overflight Restriction
  P,        //	Prohibited Area
  Q,        //	Danger Area
  R,        //	Restricted Area
  RMZ,      //	Radio Mandatory Zone
  TFR,      //	Temporary Flight Restriction
  TIA,      //	Traffic Information Area
  TIZ,      //	Traffic Information Zone
  TMA,      //	Terminal Manoeuvring Area
  TMZ,      //	Transponder Mandatory Zone
  TRA,      //	Temporary Reserved Area
  TRAFR,    //	TRA/TSA Feeding Route
  TRZ,      //	Transponder Recommended Zone
  TSA,      //	Temporary Segregated Area
  UIR,      //	Upper Flight Information Region
  UTA,      //	Upper Traffic Area (Allows VFR Traffic in CTA)
  VFRR,     //	Designated Route for VFR
  VFRSEC,   //	Visual Flying Rules Sector
  WARNING,  //	Warning Area

  MTMA,       // Military-Terminal-Zone
  AER,        // Radio-controlled-model-flying,
  AMA,        // Minimum-Altitude-Area,
  BAL,        // Hot-air-BALloons (BALLOON, BALLOONING),
  CBA,        // Cross-Border-Area,
  NATURE,     // Natural-reserve,
  OCA,        // Oceanic-Control-Area,
  PART,       // Part-of-an-airspace (used in airspace aggregations),
  PJE,        // Parachute-Jumping-Exercise,
  POLITICAL,  // Political/administrative area,
  PRN,        // Police-rescue-activity-or-nature-reserve-management-operations,
  PROTECT,    // Airspace-protected-from-specific-air-traffic,
  RAS,        // Regulated Airspace (not otherwise covered),
  RTBA,       // Reseau-Tres-Basse-Altitude,
  SECTOR,     // Control-Sector,
  SIV,        // Service-d'Information-de-Vol,
  SPORT,      // Aerial-sporting,
  SUR,        // Point-d'attention (Prisons)
  TRPLA,      // Treuil-Planeurs,
  TRVL,       // Treuil-Vol-Libre,
  ZIT,        // Zone-Interdite-Temporaire,
  ZRT,        // Zone-Règlementé-Temporaire,
  ZDT,        // Zone-Danger-Temporaire,
  ZSM,        // Zone-de-Sensibilité-Majeure
  MSZ,        // Major-Sensibility-Zone
  GP,         // Glider Prohibited,
  W,          // Wave Window
  FFVL,       // FFVL-Protocole (Paragliding/Hangliding/Kite),
  FFVP,       // FFVP-Protocole (Glider),

  TSKSEC,  // TASKSECTOR (internal, not part of OpenAir specification, used
           // to allow users to change task sector color)

  // this must be the last, it's used as sentinel for and of enum
  // and not available in airspace color/display/warning configuration.
  NONE,  // Airspace without type,
};

constexpr std::underlying_type_t<Type> to_underlying(Type type) {
  return static_cast<std::underlying_type_t<Type>>(type);
}

constexpr Type from_underlying(std::underlying_type_t<Type> value) {
  // This cast is safe because enum is contiguous with OTHER as 0 and NONE as
  // last value. If value is out of range, return OTHER as default unknown type.
  if (value < to_underlying(Type::OTHER) || value > to_underlying(Type::NONE)) {
    return Type::OTHER;
  }
  return static_cast<Type>(value);
}

template <Type value>
constexpr bool is_valid_enum() {
  // this is a compile-time check to ensure that enum values are defined as
  // expected. It relies on the fact that the function name will contain the
  // enum value when instantiated with a valid enum value. If the enum value is
  // not valid, the function name will not contain "Airspace::Type::" and the
  // check will fail.
  constexpr std::string_view name = __PRETTY_FUNCTION__;
  return name.find("Airspace::Type::") != std::string_view::npos;
}

template <size_t... types>
constexpr bool is_valid_enum(std::index_sequence<types...>) {
  return (is_valid_enum<static_cast<Type>(types)>() && ...);
}

constexpr bool is_valid_enum() {
  return is_valid_enum(std::make_index_sequence<to_underlying(Type::NONE)>{});
}

static_assert(to_underlying(Type::OTHER) == 0U,
              "Enum values must start at 0 with OTHER as unknown type");

static_assert(is_valid_enum(), "Enum values must be contiguous");

struct type_range {
  constexpr type_range() = default;

  using value_type = std::underlying_type_t<Type>;

  class iterator {
   public:
    constexpr iterator() = default;
    constexpr explicit iterator(value_type value) : value(value) {}

    constexpr Type operator*() const {
      return static_cast<Type>(value);
    }

    constexpr iterator& operator++() noexcept {
      ++value;
      return *this;
    }

    constexpr bool operator!=(const iterator& other) const noexcept {
      return value != other.value;
    }

   private:
    value_type value;
  };

  static constexpr iterator begin() {
    return iterator{to_underlying(Type::OTHER)};
  }

  static constexpr iterator end() {
    return iterator{to_underlying(Type::NONE)};
  }
};

struct type_info {
  Type type;
  const TCHAR* short_name;  // OpenAir Value
  const TCHAR* long_name;   // human readable name
  std::optional<LKColor> color = {};
  std::optional<size_t> pattern = {};
};

constexpr type_info type_info_table[] = {
    {Type::OTHER, _T("?"), _T("Unknown"), {{0x00,0xFF,0xFF}}, 2}, // Cyan

    {Type::A, _T("A"), _T("Class A"), {{0xFF,0x00,0x00}}, 3}, // Red
    {Type::B, _T("B"), _T("Class B"), {{0xFF,0x00,0x00}}, 3}, // Red
    {Type::C, _T("C"), _T("Class C"), {{0x7F,0x00,0x7F}}, 3}, // Purple
    {Type::D, _T("D"), _T("Class D"), {{0x00,0x00,0xFF}}, 3}, // Blue
    {Type::E, _T("E"), _T("Class E"), {{0x00,0x7F,0x00}}, 3}, // Green
    {Type::F, _T("F"), _T("Class F"), {{0x00,0x7F,0x00}}, 3}, // Green
    {Type::G, _T("G"), _T("Class G"), {{0x00,0x7F,0x00}}, 3}, // Green

    {Type::UNC, _T("UNC"), _T("Unclassified"), {{0x00,0xFF,0xFF}}}, // Cyan

    {Type::ACCSEC, _T("ACC"),
     _T("Airspace providing communication frequency in remote areas")},
    {Type::ADIZ, _T("ADIZ"), _T("Air-Defense-Identification-Zone")},
    {Type::ALERT, _T("Alert"), _T("Alert Area")},
    {Type::ASRA, _T("ASRA"), _T("Aerial Sporting Or Recreational Activity")},
    {Type::ATZ, _T("ATZ"), _T("Aerodrome-Traffic-Zone")},
    {Type::AWY, _T("AWY"), _T("Airway (corridor)")},
    {Type::CTA, _T("CTA"), _T("ConTrol-Area")},
    {Type::CTR, _T("CTR"), _T("Control-Traffic-Region"), {{0x7F, 0x00, 0x7F}}, 3},  // Purple
    {Type::CUSTOM, _T("Usr"), _T("Custom or user defined airspace")},
    {Type::FIR, _T("FIR"), _T("Flight-Information-Region")},
    {Type::FIS, _T("FIS"), _T("Flight-Information-Service")},
    {Type::GSEC, _T("GldSec"), _T("Gliding Sector"), {{0xFF, 0xFF, 0x00}}, 3},  // Yellow
    {Type::HTZ, _T("HTZ"), _T("Helicopter Traffic Zone")},
    {Type::LTA, _T("LTA"), _T("Lower-Traffic-Area")},
    {Type::MATZ, _T("MATZ"), _T("Military Airport Traffic Zone")},
    {Type::MTA, _T("MTA"), _T("Military Training Area")},
    {Type::MTR, _T("MTR"), _T("Military Training Route")},
    {Type::N, _T("Notam"), _T("NOTAM Affected Area"), {{0xFF, 0x00, 0x00}}},
    {Type::OFR, _T("OFR"), _T("Overflight Restriction"), {{0xFF, 0x00, 0xFF}}},  // Magenta
    {Type::P, _T("Prb"), _T("Prohibited"), {{0xFF, 0x00, 0x00}}, 0},
    {Type::Q, _T("Dgr"), _T("Danger"), {{0x7F, 0x00, 0x7F}}, 0},      // Purple
    {Type::R, _T("Res"), _T("Restricted"), {{0xFF, 0x00, 0x00}}, 0},  // red

    {Type::RMZ, _T("RMZ"), _T("Radio-Mandatory-Zone"), {{0xFF,0x00,0xFF}}, 3}, // Magenta
    {Type::TFR, _T("TFR"), _T("Temporary Flight Restriction"), {{0xFF,0x00,0xFF}}}, // Magenta
    {Type::TIA, _T("TIA"), _T("Traffic Information Area")},
    {Type::TIZ, _T("TIZ"), _T("Traffic Information Zone")},
    {Type::TMA, _T("TMA"), _T("Terminal-Manoeuvring-Area"), {{0x00,0x00,0xFF}}}, // BLue
    {Type::TMZ, _T("TMZ"), _T("Transponder-Mandatory-Zone"), {}, 3},
    {Type::TRA, _T("TRA"), _T("Temporary-Reserved-Area"), {{0xFF,0x00,0xFF}}}, // Magenta
    {Type::TRAFR, _T("TRAFR"), _T("TRA/TSA Feeding Route")},
    {Type::TRZ, _T("TRZ"), _T("Transponder Recommended Zone")},
    {Type::TSA, _T("TSA"), _T("Temporary-Segregated-Area"), {{0xFF,0x00,0xFF}}}, // Magenta
    {Type::UIR, _T("UIR"), _T("Upper-Flight-Information-Region")},
    {Type::UTA, _T("UTA"), _T("Upper-Control-Area")},
    {Type::VFRR, _T("VfrR"), _T("Designated Route for VFR")},
    {Type::VFRSEC, _T("VfrS"), _T("Visual Flying Rules Sector")},
    {Type::WARNING, _T("Warn"), _T("Warning Area"), {{0xFF,0x00,0xFF}}}, // Magenta

    {Type::MTMA, _T("MTMA"), _T("Military-Terminal-Zone"), {{0x7F,0x00,0x7F}}}, // Purple
    {Type::AER, _T("AER"), _T("Radio-controlled-model-flying")},
    {Type::AMA, _T("AMA"), _T("Minimum-Altitude-Area")},
    {Type::BAL, _T("BAL"), _T("Hot-air-BALloons (BALLOON, BALLOONING)")},
    {Type::CBA, _T("CBA"), _T("Cross-Border-Area")},
    {Type::NATURE, _T("Nat"), _T("Natural-reserve")},
    {Type::OCA, _T("OCA"), _T("Oceanic-Control-Area")},
    {Type::PART, _T("PART"), _T("Part-of-an-airspace (used in airspace aggregations)")},
    {Type::PJE, _T("PJE"), _T("Parachute-Jumping-Exercise")},
    {Type::POLITICAL, _T("Pol"), _T("Political/administrative area")},
    {Type::PRN, _T("PRN"),
     _T("Police-rescue-activity-or-nature-reserve-management-operations")},
    {Type::PROTECT, _T("Prot"), _T("Airspace-protected-from-specific-air-traffic")},
    {Type::RAS, _T("RAS"), _T("Regulated Airspace (not otherwise covered)")},
    {Type::RTBA, _T("RTBA"), _T("Reseau-Tres-Basse-Altitude"), {{0xFF, 0x00, 0x00}}},  // Red
    {Type::SECTOR, _T("SEC"), _T("Control-Sector")},
    {Type::SIV, _T("SIV"), _T("Service-d'Information-de-Vol")},
    {Type::SPORT, _T("Sprt"), _T("Aerial-sporting")},
    {Type::SUR, _T("SUR"), _T("Point-d'attention (Prisons)")},
    {Type::TRPLA, _T("TRPLA"), _T("Treuil-Planeurs")},
    {Type::TRVL, _T("TRVL"), _T("Treuil-Vol-Libre")},
    {Type::ZIT, _T("ZIT"), _T("Zone-Interdite-Temporaire"), {{0xFF, 0x00, 0x00}}},  // Red
    {Type::ZRT, _T("ZRT"), _T("Zone-Règlementé-Temporaire"), {{0xFF, 0x00, 0x00}}},  // Red
    {Type::ZDT, _T("ZDT"), _T("Zone-Danger-Temporaire"), {{0xFF, 0x00, 0x00}}},  // Red
    {Type::ZSM, _T("ZSM"), _T("Zone-de-Sensibilité-Majeure")},
    {Type::MSZ, _T("MSZ"), _T("Major-Sensibility-Zone")},
    {Type::GP, _T("GP"), _T("Glider Prohibited")},

    {Type::W, _T("Wav"), _T("Wave Window"), {}, 2},

    {Type::FFVL, _T("FFVL"), _T("FFVL-Protocole (Paragliding/Hangliding/Kite)")},
    {Type::FFVP, _T("FFVP"), _T("FFVP-Protocole (Glider)")},

    {Type::TSKSEC, _T("TSKSEC"), _T("Task Sector"), {{0xFF, 0xFF, 0x00}}, 3},  // Yellow
};

inline const type_info* lookup_type_info(Type type) {
  auto it = std::ranges::find(type_info_table, type, &type_info::type);
  return it != std::ranges::end(type_info_table) ? it : nullptr;
}

inline const TCHAR* to_short_name(Type type) {
  auto info = lookup_type_info(type);
  return info ? info->short_name : nullptr;
}

inline const TCHAR* to_long_name(Type type) {
  auto info = lookup_type_info(type);
  return info ? info->long_name : nullptr;
}

inline const std::optional<LKColor> to_color(Type type) {
  auto info = lookup_type_info(type);
  return info ? info->color : std::nullopt;
}

inline const std::optional<size_t> to_pattern(Type type) {
  auto info = lookup_type_info(type);
  return info ? info->pattern : std::nullopt;
}

template <typename Func, typename R = std::invoke_result_t<Func&, Type>>
inline R lookup(Type cls, Type type, Func&& func) noexcept(
    std::is_nothrow_invocable_v<Func&, Type>) {

  if (type == Airspace::Type::OTHER || type == Airspace::Type::NONE) {
    // OpenAir-V1 or OpenAip airspace source (OTHER)
    // OpenAir-V2 Airspace without type (NONE)
    return std::invoke(func, cls);
  }
  R result = std::invoke(func, type);
  if (result) {
    return result;
  }
  // not defined for type, use cls
  return std::invoke(func, cls);
}

#define CONCATE(name) Type::name
#define PAIR(name) { #name, CONCATE(name) }

struct type_pair {
  const char* name;
  Type type;
};


inline constexpr
type_pair type_table[] = {
    // Enum order
    {"OTHER", Type::OTHER}, // Unknown type
    PAIR(A),      // Class A,
    PAIR(B),      // Class B,
    PAIR(C),      // Class C,
    PAIR(D),      // Class D,
    PAIR(E),      // Class E,
    PAIR(F),      // Class F,
    PAIR(G),      // Class G,
    PAIR(UNC),    // Unclassified,

    PAIR(NONE),       // Airspace without type,

    PAIR(ACCSEC),     // Airspace providing communication frequency in remote areas
    PAIR(ADIZ),       // Air-Defense-Identification-Zone,
    PAIR(ALERT),      // Alert Area
    PAIR(ASRA),       // Aerial Sporting Or Recreational Activity
    PAIR(ATZ),        // Aerodrome-Traffic-Zone
    PAIR(AWY),        // Airway (corridor),
    PAIR(CTA),        // ConTrol-Area,
    PAIR(CTR),        // Control-Traffic-Region
    PAIR(CUSTOM),     // Custom or user defined airspace
    PAIR(FIR),        // Flight-Information-Region,
    PAIR(FIS),        // Flight-Information-Service,
    PAIR(GSEC),       // Gliding Sector
    {"GLIDING", Type::GSEC}, // Gliding Sector  
    PAIR(HTZ),        // Helicopter Traffic Zone
    PAIR(LTA),        // Lower-Trafic-Area,
    PAIR(MATZ),       // Military Airport Traffic Zone
    PAIR(MTA),        // Military Training Area
    PAIR(MTR),        // Military Training Route
    PAIR(N),          // NOTAM Affected Area
    {"NOTAM", Type::N}, // Notice-To-Airmen (alias for N),
    PAIR(OFR),        // Overflight Restriction
    PAIR(P),          // Prohibited,
    {"PROHIBITED", Type::P}, // Prohibited,
    PAIR(Q),          // Danger,
    {"DANGER", Type::Q}, // Danger,
    PAIR(R),          // Restricted,
    {"RESTRICTED", Type::R}, // Restricted,
    PAIR(RMZ),        // Radio-Mandatory-Zone,
    PAIR(TFR),        // Temporary Flight Restriction
    PAIR(TIA),        // Traffic Information Area
    PAIR(TIZ),        // Traffic Information Zone
    PAIR(TMA),        // Terminal-Manoeuvring-Area,
    PAIR(TMZ),        // Transponder-Mandatory-Zone,
    PAIR(TRA),        // Temporary-Reserved-Area,
    PAIR(TRAFR),      // TRA/TSA Feeding Route
    PAIR(TRZ),        // Transponder Recommended Zone
    PAIR(TSA),        // Temporary-Segregated-Area,
    PAIR(UIR),        // Upper-Flight-Information-Region,
    PAIR(UTA),        // Upper-Control-Area,
    PAIR(VFRR),       // Designated Route for VFR
    PAIR(VFRSEC),     // Visual Flying Rules Sector
    PAIR(WARNING),    // Warning Area

    // Secondary entries
    PAIR(MTMA),       // Military-Terminal-Zone
    PAIR(AER),        // Radio-controlled-model-flying,
    PAIR(AMA),        // Minimum-Altitude-Area,
    PAIR(BAL),        // Hot-air-BALloons (BALLOON, BALLOONING),
    PAIR(CBA),        // Cross-Boerder-Area,
    PAIR(NATURE),     // Natural-reserve,
    PAIR(OCA),        // Oceanic-Control-Area,
    PAIR(PART),       // Part-of-an-airspace (used in airspace aggregations),
    PAIR(PJE),        // Parachute-Jumping-Exercise,
    PAIR(POLITICAL),  // Political/administrative area,
    PAIR(PRN),        // Police-rescue-activity-or-nature-reserve-management-operations,
    PAIR(PROTECT),    // Airspace-protected-from-specific-air-traffic,
    PAIR(RAS),        // Regulated Airspace (not otherwise covered),
    PAIR(RTBA),       // Reseau-Tres-Basse-Altitude,
    PAIR(SECTOR),     // Control-Sector,
    PAIR(SIV),        // Service-d'Information-de-Vol,
    PAIR(SPORT),      // Aerial-sporting,
    PAIR(SUR),        // Point-d'attention (Prisons)
    PAIR(TRPLA),      // Treuil-Planeurs,
    PAIR(TRVL),       // Treuil-Vol-Libre,
    PAIR(ZIT),        // Zone-Interdite-Temporaire,
    PAIR(ZRT),        // Zone-Règlementé-Temporaire,
    PAIR(ZDT),        // Zone-Danger-Temporaire,
    PAIR(ZSM),        // Zone-de-Sensibilité-Majeure
    PAIR(MSZ),        // Major-Sensibility-Zone
    PAIR(GP),         // Glider Prohibited,
    PAIR(W),          // Wave Window
    {"WAVE", Type::W}, // Wave Window
    PAIR(FFVL),       // FFVL-Protocole (Paragliding/Hangliding/Kite),
    {"FFVL_Prot", Type::FFVL},
    PAIR(FFVP),       // FFVP-Protocole (Glider),
    {"FFVP_Prot", Type::FFVP},
    PAIR(TSKSEC),     // TASKSECTOR (internal, not part of OpenAir specification, used
                      // to allow users to change task sector color)
};

#undef PAIR
#undef CONCATE

inline Type from_string(std::string_view type) {
  trim_inplace(type); // trim whitespace for better compatibility with user input
  auto it = std::ranges::find(type_table, type, &type_pair::name);
  return it != std::ranges::end(type_table) ? it->type : Type::OTHER;
}

#ifdef _UNICODE
inline Type from_string(const wchar_t* type) {
  return from_string(to_utf8(type));
}
#endif

inline const char* to_string(Type type) {
  auto it = std::ranges::find(type_table, type, &type_pair::type);
  return it != std::ranges::end(type_table) ? it->name : type_table[0].name;
}

}  // namespace Airspace


constexpr auto AIRSPACECLASSCOUNT = Airspace::to_underlying(Airspace::Type::NONE);

#define ALLON 0
#define CLIP 1
#define AUTO 2
#define ALLBELOW 3
#define INSIDE 4
#define ALLOFF 5

#define OUTSIDE_CHECK_INTERVAL 4
