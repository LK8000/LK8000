/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Modeltype.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef MODELTYPE_H
#define MODELTYPE_H

#include <tuple>
#include "Util/tstring.hpp"


namespace settings {
    class writer;
}

namespace ModelType {

    /*
        Here we declare Model Types for embedded custom versions. Initially for PNAs only.
        We don't need a "type" and a "model" such as "pna" and "hp310". Instead we use a
        single int value with subsets made of ranges.
        We use modeltypes currently for extraclipping, hardware key transcoding, and we should
        also handle embedded gps com ports and adjustments (TODO)

        types     0 -    99 are reserved and 0 is generic/unknown
        types   100 -   999 are special devices running embedded LK8000
        types  1000 -  9999 are PDAs
        types 10000 - 99999 are PNAs, each brand with 200 units slots for inner types
                                    (initially we try to stay below 32767 within a short var)
        types over 100000	are reserved and should not be used
    */

    enum Type_t : int {
        GENERIC      = 0,

        BTKA         = 10011, // generic bt keyboard mode A
        BTKB         = 10012, // generic bt keyboard mode B
        BTKC         = 10013, // generic bt keyboard mode C

        BTK1         = 10021, // generic bt keyboard type 1
        BTK2         = 10022, // generic bt keyboard type 2
        BTK3         = 10023, // generic bt keyboard type 3

        XCREMOTE     = 10024, // XCRemote stick

        QWERTY		 = 10025, // VR-PC QWERTY Keyboard

        HP31X        = 10201,	// HP310, 312, 314, 316

        PN6000       = 10401,

        PNA_MIO      = 10600,	// Generic definitions

        PNA_NAVIGON  = 10700,	// Navigon

        MEDION_P5    = 11401,	// clipping problems for P5430 and P5 family

        NOKIA_500    = 12001, // 480x272

        FUNTREK      = 14001, // 400x240 240x400
        ROYALTEK3200 = 14101, // 320x240  aka Medion S3747

        LX_MINI_MAP  = 15000,
    };

    struct list {
        using value_type = const std::tuple<Type_t, const TCHAR*>;
        using iterator = value_type*;

        static iterator begin();
        static iterator end();

        static iterator find(Type_t id);
        static iterator find(const tstring_view& name);
    };

    unsigned get_index(Type_t id);

    Type_t get_id(unsigned index);
    const TCHAR* get_name(Type_t id);

    /**
     * to Set/Get GlobalModelType and GlobalModelName
     */
    void Set(Type_t id);
    Type_t Get();
    const TCHAR* GetName();

    /**
     * @param name Model type name
     * @return true if name is valid model type
     */
    bool Set(const TCHAR* name);


    /**
     * to Save/Load GlobalModelType to profile
     */
    void ResetSettings();
    bool LoadSettings(const char *key, const char *value);
    void SaveSettings(settings::writer& writer_settings);
};

#ifdef PNA
bool LoadModelFromProfile();
bool SetModelType();
#endif

#endif
