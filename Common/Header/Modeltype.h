/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Modeltype.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef MODELTYPE_H
#define MODELTYPE_H


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

#define MODELTYPE_UNKNOWN		0
#define MODELTYPE_GENERIC		0

#define MODELTYPE_EMBEDDED		 100	// generic embedded
#define MODELTYPE_ALTAIR		 101

#define MODELTYPE_PDA_PDA		1000	// generic PDA
#define MODELTYPE_PDA			1000

#define MODELTYPE_PNA_PNA		10000	// generic PNA
#define MODELTYPE_PNA			10000
#define MODELTYPE_PNA_GENERIC_BTKA	10011 // generic bt keyboard mode A
#define MODELTYPE_PNA_GENERIC_BTKB	10012 // generic bt keyboard mode B
#define MODELTYPE_PNA_GENERIC_BTKC	10013 // generic bt keyboard mode C
#define MODELTYPE_PNA_GENERIC_BTK1	10021 // generic bt keyboard type 1
#define MODELTYPE_PNA_GENERIC_BTK2	10022 // generic bt keyboard type 2
#define MODELTYPE_PNA_GENERIC_BTK3	10023 // generic bt keyboard type 3
#define MODELTYPE_PNA_HP		10200	// Generic HP
#define MODELTYPE_PNA_HP31X		10201	// HP310, 312, 314, 316

#define MODELTYPE_PNA_DAYTON	10400	// Generic VDO Dayton
#define MODELTYPE_PNA_PN6000	10401

#define MODELTYPE_PNA_MIO		10600	// Generic definitions
#define MODELTYPE_PNA_MIO520	10601
#define	MODELTYPE_PNA_MIOP350	10602

#define MODELTYPE_PNA_NAVIGON	10700	// Navigon
#define MODELTYPE_PNA_NVG4350	10701	// Navigon 4350

#define MODELTYPE_PNA_NAVMAN	10800
#define MODELTYPE_PNA_GARMIN	11000
#define MODELTYPE_PNA_CLARION	11200
#define MODELTYPE_PNA_MEDION	11400
#define MODELTYPE_PNA_MEDION_P5	11401	// clipping problems for P5430 and P5 family
#define MODELTYPE_PNA_SAMSUNG	11600
#define MODELTYPE_PNA_NAVIGO	11800
#define MODELTYPE_PNA_NOKIA	12000
#define MODELTYPE_PNA_NOKIA_500	12001 // 480x272
#define MODELTYPE_PNA_FUNTREK	14001 // 400x240 240x400
#define MODELTYPE_PNA_ROYALTEK3200	14101 // 320x240  aka Medion S3747
#define MODELTYPE_PNA_MINIMAP	15000


#endif
