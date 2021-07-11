/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LKObjects.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(LKOBJECTS_H)
#define LKOBJECTS_H

#if defined(STATIC_LKOBJECTS)
  #define OEXTERN
  #undef  STATIC_LKOBJECTS

#else
  #undef  OEXTERN
  #define OEXTERN extern
  extern void LKObjects_Create(); 
  extern void LKObjects_Delete();

#endif

// Reusable objects for LK
#define LKBrush_White LK_WHITE_BRUSH
#define LKBrush_Black LK_BLACK_BRUSH
#define LKBrush_Hollow LK_HOLLOW_BRUSH

OEXTERN	LKBrush	LKBrush_Petrol;
OEXTERN	LKBrush	LKBrush_LightGreen;
OEXTERN	LKBrush	LKBrush_DarkGreen;
OEXTERN	LKBrush	LKBrush_Ndark;
OEXTERN	LKBrush LKBrush_Nlight;
OEXTERN	LKBrush	LKBrush_Mdark;
OEXTERN	LKBrush  LKBrush_Mlight;
OEXTERN	LKBrush	LKBrush_Red;
OEXTERN	LKBrush	LKBrush_Yellow;
OEXTERN	LKBrush	LKBrush_LightYellow;
OEXTERN	LKBrush	LKBrush_Green;
OEXTERN	LKBrush	LKBrush_DarkYellow2;
OEXTERN	LKBrush	LKBrush_Orange;
OEXTERN	LKBrush	LKBrush_Lake;
OEXTERN	LKBrush	LKBrush_Blue;
OEXTERN	LKBrush	LKBrush_Indigo;
OEXTERN	LKBrush	LKBrush_LightGrey;
OEXTERN	LKBrush	LKBrush_DarkGrey;
OEXTERN	LKBrush	LKBrush_LcdGreen;
OEXTERN	LKBrush	LKBrush_LcdDarkGreen;
OEXTERN	LKBrush	LKBrush_Grey;
OEXTERN	LKBrush	LKBrush_Emerald;
OEXTERN	LKBrush	LKBrush_DarkSlate;
OEXTERN	LKBrush	LKBrush_RifleGrey;
OEXTERN	LKBrush	LKBrush_LightCyan;

OEXTERN LKBrush	LKBrush_Vario_neg4 ;
OEXTERN LKBrush	LKBrush_Vario_neg3 ;
OEXTERN LKBrush	LKBrush_Vario_neg2 ;
OEXTERN LKBrush	LKBrush_Vario_neg1 ;
OEXTERN LKBrush	LKBrush_Vario_0    ;
OEXTERN LKBrush	LKBrush_Vario_pos1 ;
OEXTERN LKBrush	LKBrush_Vario_pos2 ;
OEXTERN LKBrush	LKBrush_Vario_pos3 ;
OEXTERN LKBrush	LKBrush_Vario_pos4 ;

// Contextual LKBrush
OEXTERN	LKBrush	LKBrush_Higlighted;
OEXTERN LKPen LKPen_Higlighted;
OEXTERN	LKBrush	LKBrush_FormBackGround; // default Form Background

OEXTERN	LKPen	LKPen_Black_N0;
OEXTERN	LKPen	LKPen_Black_N1;
OEXTERN	LKPen	LKPen_Black_N2;
OEXTERN	LKPen	LKPen_Black_N3;
OEXTERN	LKPen	LKPen_Black_N4;
OEXTERN	LKPen	LKPen_Black_N5;
OEXTERN	LKPen	LKPen_White_N0;
OEXTERN	LKPen	LKPen_White_N1;
OEXTERN	LKPen	LKPen_White_N2;
OEXTERN	LKPen	LKPen_White_N3;
OEXTERN	LKPen	LKPen_White_N4;
OEXTERN	LKPen	LKPen_White_N5;

OEXTERN	LKPen	LKPen_Green_N1;
OEXTERN	LKPen	LKPen_Red_N1;
OEXTERN	LKPen	LKPen_Blue_N1;
OEXTERN	LKPen	LKPen_Grey_N0;
OEXTERN	LKPen	LKPen_Grey_N1;
OEXTERN	LKPen	LKPen_Grey_N2;
OEXTERN	LKPen	LKPen_Petrol_C2;
OEXTERN	LKPen	LKPen_GABRG;






#endif

