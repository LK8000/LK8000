/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: XShape.cpp,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#include "XShape.h"

XShape::XShape() : hide(false) {
  msInitShape(&shape);
}

XShape::~XShape() {
  clear();
}

void XShape::clear() {
  msFreeShape(&shape);
}

void XShape::load(shapefileObj* shpfile, int i) {
  msSHPReadShape(shpfile->hSHP, i, &shape);
}
