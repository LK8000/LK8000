/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */
#include "externs.h"
#include "MapWindow.h"
#include "Multimap.h"
#include "Sideview.h"

/**
 * @brief Draw simple thin black line separator between side and topview, if needed
 * @param Surface : Surface to draw
 * @param rci : TopView RECT, line is draw on bottom side of this RECT
 */
void MapWindow::DrawMultimap_SideTopSeparator(LKSurface& Surface, const RECT& rci) {
    if (rci.bottom > 0 && Current_Multimap_SizeY < SIZE4) {
        const POINT line[2] = {
            { rci.left, rci.bottom - 1},
            { rci.right, rci.bottom - 1}
        };
        Surface.DrawLine(PEN_SOLID, ScreenThinSize, line[0], line[1], RGB_BLACK, rci);
    }
}
