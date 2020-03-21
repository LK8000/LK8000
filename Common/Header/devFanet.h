/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   devFanet.h
 *  Author: Gerald Eichler
 *
 *  Created on 13 march 2020, 14:45
 */

#ifndef DEVFANET_H
#define	DEVFANET_H

BOOL FanetRegister(void);
BOOL FanetParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

#endif	/* DEVFANETH */
