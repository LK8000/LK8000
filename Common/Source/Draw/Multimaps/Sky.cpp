/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"
#include "LKObjects.h"
#include "Screen/Point.hpp"

void RenderSky(LKSurface& Surface, const RECT& rci, const LKColor& Col1, const LKColor& Col2, int iSteps) {

    RECT rcd = rci;
    if (iSteps == 1) {
        iSteps++;
    }
    double fdy = (double) (rci.top - rci.bottom) / (double) (iSteps - 1);

    double fTop = (double) rcd.bottom - fdy;
    for (int i = 0; i < iSteps; i++) {
        rcd.bottom = rcd.top;
        fTop += fdy;
        rcd.top = (PixelScalar)fTop;

        LKBrush hbHorizon(Col2.MixColors(Col1, (double) i / (double) iSteps));

        Surface.FillRect(&rcd, hbHorizon);
    }
    if(rcd.top != rci.top) {
        rcd.bottom = rcd.top;
        rcd.top = rci.top;
        LKBrush hbHorizon(Col2);
        Surface.FillRect(&rcd, hbHorizon);
    }
}
