/*
 * LKGeneralAviation.h
 *
 *  Created on: Nov 24, 2010
 *      Author: orenc
 */

#ifndef LKGENERALAVIATION_H_
#define LKGENERALAVIATION_H_

void DrawHSIarc(LKSurface& Surface, const POINT& Orig, const RECT& rc );
int DrawCompassArc(LKSurface& Surface, long x, long y, int radius, const RECT& rc, double bearing);


#endif /* LKGENERALAVIATION_H_ */
