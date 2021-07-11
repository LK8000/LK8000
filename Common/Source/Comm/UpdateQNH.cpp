/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"


//
// Change the QNH pressure. This is NOT an altitude, it is a real hP pressure.
// We return true if we did change the qnh, or false if not.
// Normally we should assume we did not change it because IT WAS THE SAME AS OLD ONE.
//
// Do all the functions required when QNH has changed.
//
// Notice> this function is not really thread safe
//
bool UpdateQNH( const double newqnh )
{

    static double oldqnh=0;

    // minimal check for acceptable qnh value, on planet Earth
    if (newqnh>100 && newqnh<1500) {
        QNH = newqnh; // we force it just in case
        if (oldqnh!=newqnh) {
            CAirspaceManager::Instance().QnhChangeNotify(QNH);
            oldqnh=newqnh;
            return true;
        }
    }

    return false;
}

