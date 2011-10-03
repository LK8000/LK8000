#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include        "magfield.h"

int main() 
{

 double lat_deg,lon_deg,h,var;
 int yy,mm,dd;

 lat_deg=45.0;
 lon_deg=9.0;

 h=      1; // altitude in km
 mm=     10;
 dd=     1;
 yy=     11;


 var=rad_to_deg(SGMagVar(deg_to_rad(lat_deg),deg_to_rad(lon_deg),h, yymmdd_to_julian_days(yy,mm,dd)));

 fprintf(stdout,"var= %4.2f \n",var);

}
  
  

