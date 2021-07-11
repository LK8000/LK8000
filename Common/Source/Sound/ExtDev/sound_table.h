/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   sound_table.cpp
 * Author: Jack
 *
 * Created on April 2015
 * 
 * This class is for associating file sound with nmea string sent for external device
 * 
 * When adding a new sound, you have too:
 * - adding filename (without WAV extension) entry in enum sound_code, BEFORE "last" enum entry
 * - adding filename (without WAV extension) entry in Enum_String( sound_code ) block
 * - add the corresponding entry in the SOUND_TABLE.TXT _Sytem configuration file
 */

#ifndef SOUND_TABLE_H
#define	SOUND_TABLE_H

#include "Util/tstring.hpp"
#include "tchar.h"
#include "utils/EnumString.h"
#include <array>

typedef enum sound_code {
    DEFAULT,
	CABINCHIME12F,
	CABINCHIME12,
	CABINCHIME1B,
	CABINCHIME1C,
	CABINCHIME1,
	CABINCHIME21,
	CABINCHIME2B,
	CABINCHIME2C,
	CABINCHIME2,
	DROPMARKER,
	LK_3HITONES,
	LK_AIRSPACEV3,
	LK_AIRSPACE,
	LK_ALARM_ALT1,
	LK_ALARM_ALT2,
	LK_ALARM_ALT3,
	LK_ALERT_DOUBLE2,
	LK_ALERT_SHORT,
	LK_ALERT_SKULLED,
	LK_ALERT_WARPED,
	LK_BEEP0,
	LK_BEEP1,
	LK_BEEP4,
	LK_BEEP6,
	LK_BEEP9,
	LK_BELL,
	LK_BLIP,
	LK_BOOSTER,
	LK_CHIME,
	LK_CLICKH,
	LK_CLICKL,
	LK_CLICKM,
	LK_CONNECTV2,
	LK_CONNECTV3,
	LK_CONNECT,
	LK_DINGDONG,
	LK_DISCONNECTV2,
	LK_DISCONNECTV3,
	LK_DISCONNECT,
	LK_FREEFLIGHT,
	LK_GATEOPEN,
	LK_GEARWARNING,
	LK_GPSNOCOM,
	LK_GPSNOFIX,
	LK_GREEN,
	LK_HITONE,
	LK_LANDING,
	LK_LONGHITONE,
	LK_ORBITER1,
	LK_ORBITER2,
	LK_ORBITER3,
	LK_OVERTARGETH,
	LK_OVERTARGET,
	LK_PROBLEM,
	LK_RED,
	LK_S0,
	LK_S1,
	LK_S2,
	LK_S3,
	LK_S4d,
	LK_S4,
	LK_S5d,
	LK_S5,
	LK_S6b,
	LK_S6,
	LK_S7,
	LK_SAFETAKEOFF,
	LK_SDHITONE,
	LK_SHORTERCLICKM,
	LK_SHORTHITONE,
	LK_SLIDE,
	LK_SONAR_H1,
	LK_SONAR_H2,
	LK_SONAR_H3,
	LK_SONAR_H4,
	LK_SONAR_H5,
	LK_T1,
	LK_T2,
	LK_T3,
	LK_T4,
	LK_T5,
	LK_T8,
	LK_TAKEOFF,
	LK_TASKFINISH,
	LK_TASKPOINT,
	LK_TASKSTART,
	LK_TICK,
	LK_TOCK,
	LK_TONEDOWN,
	LK_TONEUP,
	LK_WARNING,
	LK_WHISTLE,
	MM0,
	MM1,
	MM2,
	MM3,
	MM4,
	MM5,
	MM6,
	TARGVISIBLE,
	UNLOCKMAP,
	VOICEDEMO1,
	LK_FREEFLIGHT_LOW,
	LK_LANDING_LOW,
	LK_SAFETAKEOFF_LOW,
	LK_B2b,
	LK_B4,
	LK_B5,
	LK_B5b,
	LK_B7,
    last,
} sound_code_t;

Begin_Enum_String( sound_code )
{
    Enum_String( DEFAULT );
    Enum_String( CABINCHIME12F );
    Enum_String( CABINCHIME12 );
    Enum_String( CABINCHIME1B );
    Enum_String( CABINCHIME1C );
    Enum_String( CABINCHIME1 );
    Enum_String( CABINCHIME21 );
    Enum_String( CABINCHIME2B );
    Enum_String( CABINCHIME2C );
    Enum_String( CABINCHIME2 );
    Enum_String( DROPMARKER );
    Enum_String( LK_3HITONES );
    Enum_String( LK_AIRSPACEV3 );
    Enum_String( LK_AIRSPACE );
    Enum_String( LK_ALARM_ALT1 );
    Enum_String( LK_ALARM_ALT2 );
    Enum_String( LK_ALARM_ALT3 );
    Enum_String( LK_ALERT_DOUBLE2 );
    Enum_String( LK_ALERT_SHORT );
    Enum_String( LK_ALERT_SKULLED );
    Enum_String( LK_ALERT_WARPED );
    Enum_String( LK_BEEP0 );
    Enum_String( LK_BEEP1 );
    Enum_String( LK_BEEP4 );
    Enum_String( LK_BEEP6 );
    Enum_String( LK_BEEP9 );
    Enum_String( LK_BELL );
    Enum_String( LK_BLIP );
    Enum_String( LK_BOOSTER );
    Enum_String( LK_CHIME );
    Enum_String( LK_CLICKH );
    Enum_String( LK_CLICKL );
    Enum_String( LK_CLICKM );
    Enum_String( LK_CONNECTV2 );
    Enum_String( LK_CONNECTV3 );
    Enum_String( LK_CONNECT );
    Enum_String( LK_DINGDONG );
    Enum_String( LK_DISCONNECTV2 );
    Enum_String( LK_DISCONNECTV3 );
    Enum_String( LK_DISCONNECT );
    Enum_String( LK_FREEFLIGHT );
    Enum_String( LK_GATEOPEN );
    Enum_String( LK_GEARWARNING );
    Enum_String( LK_GPSNOCOM );
    Enum_String( LK_GPSNOFIX );
    Enum_String( LK_GREEN );
    Enum_String( LK_HITONE );
    Enum_String( LK_LANDING );
    Enum_String( LK_LONGHITONE );
    Enum_String( LK_ORBITER1 );
    Enum_String( LK_ORBITER2 );
    Enum_String( LK_ORBITER3 );
    Enum_String( LK_OVERTARGETH );
    Enum_String( LK_OVERTARGET );
    Enum_String( LK_PROBLEM );
    Enum_String( LK_RED );
    Enum_String( LK_S0 );
    Enum_String( LK_S1 );
    Enum_String( LK_S2 );
    Enum_String( LK_S3 );
    Enum_String( LK_S4d );
    Enum_String( LK_S4 );
    Enum_String( LK_S5d );
    Enum_String( LK_S5 );
    Enum_String( LK_S6b );
    Enum_String( LK_S6 );
    Enum_String( LK_S7 );
    Enum_String( LK_SAFETAKEOFF );
    Enum_String( LK_SDHITONE );
    Enum_String( LK_SHORTERCLICKM );
    Enum_String( LK_SHORTHITONE );
    Enum_String( LK_SLIDE );
    Enum_String( LK_SONAR_H1 );
    Enum_String( LK_SONAR_H2 );
    Enum_String( LK_SONAR_H3 );
    Enum_String( LK_SONAR_H4 );
    Enum_String( LK_SONAR_H5 );
    Enum_String( LK_T1 );
    Enum_String( LK_T2 );
    Enum_String( LK_T3 );
    Enum_String( LK_T4 );
    Enum_String( LK_T5 );
    Enum_String( LK_T8 );
    Enum_String( LK_TAKEOFF );
    Enum_String( LK_TASKFINISH );
    Enum_String( LK_TASKPOINT );
    Enum_String( LK_TASKSTART );
    Enum_String( LK_TICK );
    Enum_String( LK_TOCK );
    Enum_String( LK_TONEDOWN );
    Enum_String( LK_TONEUP );
    Enum_String( LK_WARNING );
    Enum_String( LK_WHISTLE );
    Enum_String( MM0 );
    Enum_String( MM1 );
    Enum_String( MM2 );
    Enum_String( MM3 );
    Enum_String( MM4 );
    Enum_String( MM5 );
    Enum_String( MM6 );
    Enum_String( TARGVISIBLE );
    Enum_String( UNLOCKMAP );
    Enum_String( VOICEDEMO1 );
    Enum_String( LK_FREEFLIGHT_LOW );
    Enum_String( LK_LANDING_LOW );
    Enum_String( LK_SAFETAKEOFF_LOW );
    Enum_String( LK_B2b );
    Enum_String( LK_B4 );
    Enum_String( LK_B5 );
    Enum_String( LK_B5b );
    Enum_String( LK_B7 );
}
End_Enum_String;

class sound_table final {
    friend class SoundGlobalInit; 
private:
    void set(sound_code_t code, const TCHAR * nmeaStr);
   
    // This array is loaded at init phase and contain association between enum sound code and nmea string
    std::array<tstring, sound_code_t::last> table;

public:
    sound_table() {}
    ~sound_table() {}

    const tstring& getNmeaStr(sound_code_t code) const;

protected:   
    // this 2 method can only b used by SoundGlobalInit !
    // SoundGlobalInit is called by main() before start of any thread, so no need to use lock
    bool init();
    void reset();
};
#endif	/* SOUND_TABLE_H */

