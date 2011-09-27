/*
 * $Id$
 */

#if !defined(LKMAPWINDOW_H)
#define LKMAPWINDOW_H

/* REMOVE
#define ISGLIDER (AircraftCategory == (AircraftCategory_t)umGlider)
#define ISPARAGLIDER (AircraftCategory == (AircraftCategory_t)umParaglider)
#define ISCAR (AircraftCategory == (AircraftCategory_t)umCar)
#define ISGAAIRCRAFT (AircraftCategory == (AircraftCategory_t)umGAaircraft)

#define CURTYPE ModeType[ModeIndex]
#define INVERTCOLORS  (Appearance.InverseInfoBox)

#define LKINFOFONT      LK8SmallFont            // was InfoWindowFont
// km for distance, kmh for speed etc.  in map overlay
#define LKMAPFONT       LK8MapFont              // was MapWindowFont
*/

// Maximum number of characters for waypoint name. Should be dynamically calculated?
// CAUTION a line should be adjusted by hand! Not a reference value!!
#define MAXNLNAME 12


#define PANELCOLUMNS    4
#define PANELROWS       4
#define RIGHTLIMITER    NIBLSCALE(3)
#define LEFTLIMITER     NIBLSCALE(3)
#define TOPLIMITER      NIBLSCALE(1)
#define BOTTOMLIMITER   NIBLSCALE(2)


#endif
