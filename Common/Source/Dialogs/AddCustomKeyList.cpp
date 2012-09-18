/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgCustomKeys.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"


void AddCustomKeyList( DataFieldEnum* dfe) {

	// Careful, order must respect the enum list in lk8000.h CustomKeyMode_t

	// TOKEN 239 = "Disabled" 
    dfe->addEnumText(MsgToken(239));
	// TOKEN 435 = "Menu" 
    dfe->addEnumText(MsgToken(435));
	// TOKEN 517 = "Page Back" 
    dfe->addEnumText(MsgToken(517));
	// TOKEN 725 = "Toggle Map<>current page" 
    dfe->addEnumText(MsgToken(725));
	// TOKEN 723 = "Toggle Map<>Landables" 
    dfe->addEnumText(MsgToken(723));
	// TOKEN 385 = "Landables" 
    dfe->addEnumText(MsgToken(385));
	// TOKEN 722 = "Toggle Map<>Commons" 
    dfe->addEnumText(MsgToken(722));
	// TOKEN 192 = "Commons" 
    dfe->addEnumText(MsgToken(192));
	// TOKEN 724 = "Toggle Map<>Traffic" 
    dfe->addEnumText(MsgToken(724));
	// TOKEN 738 = "Traffic" 
    dfe->addEnumText(MsgToken(738));
	// TOKEN 363 = "Invert colors" 
    dfe->addEnumText(MsgToken(363));
    dfe->addEnumText(TEXT("TrueWind"));
	// TOKEN 726 = "Toggle overlays" 
    dfe->addEnumText(MsgToken(726));
    dfe->addEnumText(TEXT("AutoZoom On/Off"));
    dfe->addEnumText(TEXT("ActiveMap On/Off"));
	// TOKEN 426 = "Mark Location" 
    dfe->addEnumText(MsgToken(426));
    dfe->addEnumText(TEXT("PG/HG Time Gates"));
    dfe->addEnumText(TEXT("Thermal Booster"));
	// TOKEN 329 = "Goto Home" 
    dfe->addEnumText(MsgToken(329));
	// TOKEN 519 = "Panorama trigger" 
    dfe->addEnumText(MsgToken(519));
	// TOKEN 448 = "Multitarget rotate" 
    dfe->addEnumText(MsgToken(448));
	// TOKEN 447 = "Multitarget menu" 
    dfe->addEnumText(MsgToken(447));
	// TOKEN 700 = "Team code" 
    dfe->addEnumText(MsgToken(700));
	// TOKEN 767 = "Use HBar on/off" 
    dfe->addEnumText(MsgToken(767));
	// TOKEN 130 = "Basic Setup menu" 
    dfe->addEnumText(MsgToken(130));
    dfe->addEnumText(TEXT("SIMulation menu"));
    dfe->addEnumText(MsgToken(1652)); // Airspace Analysis
    dfe->addEnumText(MsgToken(1653)); // toggle map Airspace 
    dfe->addEnumText(MsgToken(1657)); // zoom in
    dfe->addEnumText(MsgToken(1658)); // zoom out
    dfe->addEnumText(MsgToken(1659)); // zoom in more
    dfe->addEnumText(MsgToken(1660)); // zoom out more
    dfe->addEnumText(MsgToken(1687)); // toggle optimize route
    dfe->addEnumText(MsgToken(1688)); // screen lock
    dfe->addEnumText(MsgToken(1689)); // where am I
    dfe->addEnumText(MsgToken(1666)); // Toggle Total Energy
    dfe->addEnumText(MsgToken(2063)); // Nodepad
    dfe->addEnumText(MsgToken(1693)); // Change+ Terrain Colors
    dfe->addEnumText(MsgToken(871));  // Nearest Airspace
    dfe->addEnumText(MsgToken(1740)); // OLC Analysis
    dfe->addEnumText(MsgToken(1774)); // Change- Terrain colors
    dfe->addEnumText(MsgToken(1754)); // free flight
    dfe->addEnumText(MsgToken(1787)); // custom menu
    dfe->addEnumText(MsgToken(685));  // Task Calculator
    dfe->addEnumText(MsgToken(684));  // Target (task..)
    dfe->addEnumText(MsgToken(1791)); // Arm toggle advance
    dfe->addEnumTextNoLF(MsgToken(2064)); // Repeat Message
    dfe->addEnumTextNoLF(MsgToken(2015)); // Waypoint lookup
    dfe->addEnumText(MsgToken(2082)); // PAN
    dfe->addEnumTextNoLF(MsgToken(2227)); // Toggle WindRose
    dfe->addEnumTextNoLF(MsgToken(2228)); // Flarm Radar
    dfe->addEnumTextNoLF(MsgToken(2143)); // Device A
    dfe->addEnumTextNoLF(MsgToken(2144)); // Device B
    dfe->addEnumTextNoLF(MsgToken(2229)); // ResetOdometer
    dfe->addEnumTextNoLF(MsgToken(2230)); // Force Landing

    dfe->Sort(0);

}



