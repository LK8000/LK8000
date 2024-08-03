/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   payload.h
 *  Author: Gerald Eichler
 */
#ifndef _devices_fanet_payload_h_
#define _devices_fanet_payload_h_
#include <cstdint>
#include <vector>

struct payload_t : public std::vector<uint8_t> {
    using base_class = std::vector<uint8_t>;
    using base_class::base_class;
    
    using back_insert_iterator = std::back_insert_iterator<payload_t>;

    back_insert_iterator back_inserter() {
        return std::back_inserter(*this);
    }
};

#endif // _devices_fanet_payload_h_
