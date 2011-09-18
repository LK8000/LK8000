/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#if !defined(SWITCHES_H)
#define SWITCHES_H

#if defined(STATIC_SWITCHES)

  #undef  EXTMODE
  #undef  EXTTRUE
  #undef  EXTFALSE

  #define EXTMODE 
  #define EXTTRUE	=true
  #define EXTFALSE	=false

  #undef  STATIC_SWITCHES

#else

  #undef  EXTMODE
  #undef  EXTTRUE
  #undef  EXTFALSE

  #define EXTTRUE
  #define EXTFALSE
  #define EXTMODE extern

#endif


  EXTMODE bool LKSW_ReloadProfileBitmaps EXTFALSE;





#endif
