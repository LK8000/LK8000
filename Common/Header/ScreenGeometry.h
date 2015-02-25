/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   PLEASE USE COMMENTS ALSO HERE TO DESCRIBE YOUR GLOBALS!
   YOU CAN INITIALIZE VALUES TO true,false,zero and NULL, 
   or you can do it also inside Globals_Init.

*/

#ifndef SCREENGEO_H
#define SCREENGEO_H

#define SCREEN_GEOMETRY_INVALID  0

#define SCREEN_GEOMETRY_SQUARED  1
#define SCREEN_GEOMETRY_43       2
#define SCREEN_GEOMETRY_53       3
#define SCREEN_GEOMETRY_169      4
#define SCREEN_GEOMETRY_21       5

#define SCREEN_GEOMETRY_COUNT    6

extern unsigned short GetScreenGeometry(unsigned int x, unsigned int y);
extern double GetScreen0Ratio(void);


#endif
