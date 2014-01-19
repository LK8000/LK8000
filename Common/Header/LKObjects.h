/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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
OEXTERN	HBRUSH	LKBrush_White;
OEXTERN	HBRUSH	LKBrush_Black;
OEXTERN	HBRUSH	LKBrush_Petrol;
OEXTERN	HBRUSH	LKBrush_LightGreen;
OEXTERN	HBRUSH	LKBrush_DarkGreen;
OEXTERN	HBRUSH	LKBrush_Ndark;
OEXTERN	HBRUSH  LKBrush_Nlight;
OEXTERN	HBRUSH	LKBrush_Mdark;
OEXTERN	HBRUSH  LKBrush_Mlight;
OEXTERN	HBRUSH	LKBrush_Red;
OEXTERN	HBRUSH	LKBrush_Yellow;
OEXTERN	HBRUSH	LKBrush_Green;
OEXTERN	HBRUSH	LKBrush_DarkYellow2;
OEXTERN	HBRUSH	LKBrush_Orange;
OEXTERN	HBRUSH	LKBrush_Lake;
OEXTERN	HBRUSH	LKBrush_Blue;
OEXTERN	HBRUSH	LKBrush_Indigo;
OEXTERN	HBRUSH	LKBrush_LightGrey;
OEXTERN	HBRUSH	LKBrush_DarkGrey;
OEXTERN	HBRUSH	LKBrush_LcdGreen;
OEXTERN	HBRUSH	LKBrush_LcdDarkGreen;
OEXTERN	HBRUSH	LKBrush_Grey;
OEXTERN	HBRUSH	LKBrush_Emerald;
OEXTERN	HBRUSH	LKBrush_DarkSlate;
OEXTERN	HBRUSH	LKBrush_RifleGrey;
OEXTERN	HBRUSH	LKBrush_LightCyan;
OEXTERN	HBRUSH	LKBrush_Viola;

OEXTERN HBRUSH	LKBrush_Vario_neg4 ;
OEXTERN HBRUSH	LKBrush_Vario_neg3 ;
OEXTERN HBRUSH	LKBrush_Vario_neg2 ;
OEXTERN HBRUSH	LKBrush_Vario_neg1 ;
OEXTERN HBRUSH	LKBrush_Vario_0    ;
OEXTERN HBRUSH	LKBrush_Vario_pos1 ;
OEXTERN HBRUSH	LKBrush_Vario_pos2 ;
OEXTERN HBRUSH	LKBrush_Vario_pos3 ;
OEXTERN HBRUSH	LKBrush_Vario_pos4 ;

OEXTERN	HPEN	LKPen_Black_N0;
OEXTERN	HPEN	LKPen_Black_N1;
OEXTERN	HPEN	LKPen_Black_N2;
OEXTERN	HPEN	LKPen_Black_N3;
OEXTERN	HPEN	LKPen_Black_N4;
OEXTERN	HPEN	LKPen_Black_N5;
OEXTERN	HPEN	LKPen_White_N0;
OEXTERN	HPEN	LKPen_White_N1;
OEXTERN	HPEN	LKPen_White_N2;
OEXTERN	HPEN	LKPen_White_N3;
OEXTERN	HPEN	LKPen_White_N4;
OEXTERN	HPEN	LKPen_White_N5;

OEXTERN	HPEN	LKPen_Green_N1;
OEXTERN	HPEN	LKPen_Red_N1;
OEXTERN	HPEN	LKPen_Blue_N1;
OEXTERN	HPEN	LKPen_Grey_N0;
OEXTERN	HPEN	LKPen_Grey_N1;
OEXTERN	HPEN	LKPen_Grey_N2;
OEXTERN	HPEN	LKPen_Petrol_C2;
OEXTERN	HPEN	LKPen_GABRG;
OEXTERN	HPEN	LKPen_Viola_N1;


#endif
