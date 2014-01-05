/*
 * LKGeneralAviation.h
 *
 *  Created on: Nov 24, 2010
 *      Author: orenc
 */

#ifndef LKGENERALAVIATION_H_
#define LKGENERALAVIATION_H_

void DrawHSIarc(HDC hdc, POINT Orig, RECT rc );
int DrawCompassArc(HDC hdc, long x, long y, int radius, RECT rc,
	    double bearing);


#endif /* LKGENERALAVIATION_H_ */
