/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   resource_sound.cpp
 * Author: Jack
 *
 * Created on April 2015
 * 
 */

#ifndef RESOURCE_SOUND_H
#define	RESOURCE_SOUND_H

#include "Util/tstring.hpp"
#include "sound_table.h"
#include "utils/EnumString.h"


typedef enum resource_sound {
	IDR_WAV_HIGHCLICK=sound_code_t::LK_CLICKH,
	IDR_WAV_CLICK=sound_code_t::LK_SHORTERCLICKM,
	IDR_WAV_OVERTONE0=sound_code_t::LK_S0,
	IDR_WAV_OVERTONE1=sound_code_t::LK_S1,
	IDR_WAV_OVERTONE2=sound_code_t::LK_S2,
	IDR_WAV_OVERTONE3=sound_code_t::LK_S3,
	IDR_WAV_OVERTONE4=sound_code_t::LK_S4,
	IDR_WAV_OVERTONE5=sound_code_t::LK_S5,
	IDR_WAV_OVERTONE6=sound_code_t::LK_S6,
	IDR_WAV_OVERTONE7=sound_code_t::LK_S6b,
	IDR_WAV_TONE1=sound_code_t::LK_T1,
	IDR_WAV_TONE2=sound_code_t::LK_T2,
	IDR_WAV_TONE3=sound_code_t::LK_T3,
	IDR_WAV_TONE4=sound_code_t::LK_T4,
	IDR_WAV_TONE7=sound_code_t::LK_T8,
	IDR_WAV_BTONE2=sound_code_t::LK_B2b,
	IDR_WAV_BTONE4=sound_code_t::LK_B4,
	IDR_WAV_BTONE5=sound_code_t::LK_B5,
	IDR_WAV_BTONE6=sound_code_t::LK_B5b,
	IDR_WAV_BTONE7=sound_code_t::LK_B7,
	IDR_WAV_MM0=sound_code_t::MM0,
	IDR_WAV_MM1=sound_code_t::MM1,
	IDR_WAV_MM2=sound_code_t::MM2,
	IDR_WAV_MM3=sound_code_t::MM3,
	IDR_WAV_MM4=sound_code_t::MM4,
	IDR_WAV_MM5=sound_code_t::MM5,
	IDR_WAV_MM6=sound_code_t::MM6,
} resource_sound_t;

Begin_Enum_String( resource_sound )
{
    Enum_String( IDR_WAV_HIGHCLICK );
    Enum_String( IDR_WAV_CLICK );
    Enum_String( IDR_WAV_OVERTONE0 );
    Enum_String( IDR_WAV_OVERTONE1 );
    Enum_String( IDR_WAV_OVERTONE2 );
    Enum_String( IDR_WAV_OVERTONE3 );
    Enum_String( IDR_WAV_OVERTONE4 );
    Enum_String( IDR_WAV_OVERTONE5 );
    Enum_String( IDR_WAV_OVERTONE6 );
    Enum_String( IDR_WAV_OVERTONE7 );
    Enum_String( IDR_WAV_TONE1 );
    Enum_String( IDR_WAV_TONE2 );
    Enum_String( IDR_WAV_TONE3 );
    Enum_String( IDR_WAV_TONE4 );
    Enum_String( IDR_WAV_TONE7 );
    Enum_String( IDR_WAV_BTONE2 );
    Enum_String( IDR_WAV_BTONE4 );
    Enum_String( IDR_WAV_BTONE5 );
    Enum_String( IDR_WAV_BTONE6 );
    Enum_String( IDR_WAV_BTONE7 );
    Enum_String( IDR_WAV_MM0 );
    Enum_String( IDR_WAV_MM1 );
    Enum_String( IDR_WAV_MM2 );
    Enum_String( IDR_WAV_MM3 );
    Enum_String( IDR_WAV_MM4 );
    Enum_String( IDR_WAV_MM5 );
    Enum_String( IDR_WAV_MM6 );
}
End_Enum_String;

#endif	/* RESOURCE_SOUND_H */

