/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sound.h
 * Author: Bruno de Lacheisserie
 *
 * Created on January 29, 2015, 10:11 PM
 */

#ifndef Sms_H
#define	Sms_H

#include <tchar.h>


void LKTakeoffSms();
void LKSafeLandingSms();
void LKEmergencyLandingSms();

void LKSLandingSms();
void LKSms(const char *phoneNumber, const char *textMessage);



#endif	/* Sms_H */

