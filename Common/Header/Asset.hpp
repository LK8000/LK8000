/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Asset.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 8 décembre 2014, 00:01
 */

#ifndef ASSET_HPP
#define	ASSET_HPP

constexpr
bool IsAltair() {
    return false;
}

static inline
bool IsDithered() {
#ifdef DITHER
    return true;
#elif defined(ANDROID) && defined(__arm__)
    return is_dithered;
#else
    return false;
#endif
}

#endif	/* ASSET_HPP */

