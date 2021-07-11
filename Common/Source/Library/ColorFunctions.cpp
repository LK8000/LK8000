/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


using std::min;

//
// Color conversions from http://www.cs.rit.edu/~ncs/color/t_convert.html
// based on Hue/Saturation/Value model created by A.R.Smith in 1978.
// These are cylindrical coordinate representations of RGB color models.
// Currently we dont use HSV.  -- paolo
// 
// HSV (hue, saturation, value) good for extending palettes
// HSL (hue, saturation, luminosity) good for changing whiteness
//
// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)
//

#if 0
void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v )
{
	float min, max, delta;
	min = std::min( std::min(r, g), b );
	max = std::max( std::max(r, g), b );
	*v = max;				// v
	delta = max - min;
	if( max != 0 )
		*s = delta / max;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		*s = 0;
		*h = -1;
		return;
	}
	if( r == max )
		*h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max )
		*h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		*h = 4 + ( r - g ) / delta;	// between magenta & cyan
	*h *= 60;				// degrees
	if( *h < 0 )
		*h += 360;
}
void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
	int i;
	float f, p, q, t;
	if( s == 0 ) {
		// achromatic (grey)
		*r = *g = *b = v;
		return;
	}
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:		// case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}
#endif


void RGBtoHSL( float r, float g, float b, float *H, float *S, float *L ) {

  r/=255; g/=255;b/=255;
  double cmax = max(r,max(g,b));
  double cmin = min(r,min(g,b));

  *L=(cmax+cmin)/2.0;

  if(cmax==cmin) {
     *S = 0;
     *H = 0; // it's really undefined
     return;
  } 

  if(*L < 0.5) 
     *S = (cmax-cmin)/(cmax+cmin);
  else
     *S = (cmax-cmin)/(2.0-cmax-cmin);

  double delta = cmax - cmin;
  if(r==cmax)
     *H = (g-b)/delta;
  else if(g==cmax)
     *H = 2.0 +(b-r)/delta;
  else
     *H=4.0+(r-g)/delta;

  *H /= 6.0;
  if(*H < 0.0) *H += 1;

}


static double HuetoRGB(double m1, double m2, double h )
{
  if( h < 0 ) h += 1.0;
  if( h > 1 ) h -= 1.0;
  if( 6.0*h < 1 ) return (m1+(m2-m1)*h*6.0);
  if( 2.0*h < 1 ) return m2;
  if( 3.0*h < 2.0 ) return (m1+(m2-m1)*((2.0/3.0)-h)*6.0);
  return m1;
}


void HSLtoRGB( float *r, float *g, float *b, float H, float S, float L ) {

  if(S==0) {
     L*=255; if (L>255) L=255;
     *r=*g=*b=L;
     return;
  } 

  double m1, m2;

  if(L <=0.5)
     m2 = L*(1.0+S);
  else
     m2 = L+S-L*S;

  m1 = 2.0*L-m2;

  *r = HuetoRGB(m1,m2,H+1.0/3.0)*255;
  *g = HuetoRGB(m1,m2,H)*255;
  *b = HuetoRGB(m1,m2,H-1.0/3.0)*255;

  if (*r>255) *r=255;
  if (*g>255) *g=255;
  if (*b>255) *b=255;

}



//
// Dimming applies to rgb triplets by changing brightness parameter "v"
// light range should be 0.5 to 1.5  . 
// Values below 1 will make darker colors. Over 1 lighter.
// pv
// 
void rgb_lightness( uint8_t &r, uint8_t &g, uint8_t &b, float light ) {

   LKASSERT(light>0 && light <= 1.8);
   if (light==1) return;

   float h, s, v, tr, tg, tb;
   tr=r; 
   tg=g; 
   tb=b; 

   RGBtoHSL( tr, tg, tb, &h, &s, &v );
   v*= light;
   HSLtoRGB( &tr, &tg, &tb, h, s, v );

   r=(uint8_t)tr;
   g=(uint8_t)tg;
   b=(uint8_t)tb;

}



