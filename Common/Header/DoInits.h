/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

   PLEASE USE COMMENTS ALSO HERE TO DESCRIBE YOUR GLOBALS!
   YOU CAN INITIALIZE VALUES TO true,false,zero and NULL, 
   or you can do it also inside Globals_Init.

*/

#ifndef DOINITS_H
#define DOINITS_H

  #undef  GEXTERN
  #undef  GEXTTRUE
  #undef  GEXTFALSE
  #undef  GEXTNULL
  #undef  GEXTFONTNULL
  #undef  GEXTZERO

#if defined(STATIC_DOINITS)
  #define GEXTERN 
  #define GEXTTRUE       = true
  #define GEXTFALSE      = false
  #define GEXTNULL       = NULL
  #define GEXTZERO       = 0

#else
  #define GEXTERN extern
  #define GEXTTRUE  
  #define GEXTFALSE 
  #define GEXTNULL 
  #define GEXTZERO 

  extern void Init_DoInits(void);
  extern void Reset_All_DoInits(void);
  extern void Reset_Single_DoInits(int position);
  extern void Assign_DoInits(bool *location, int position);
#endif

 // Master of Do Inits
 typedef enum {
	MDI_MASTER,
	MDI_FIRST_DOINIT,
	MDI_DORANGEWAYPOINTLIST,
	MDI_DOCALCULATIONSSLOW,
	MDI_HEADING,
	MDI_AVERAGE30S,
	MDI_DOAUTOQNH,
	MDI_DOAIRSPACES,	// draw thread
	MDI_DOCOMMON,		// draw
	MDI_DOTRAFFIC,		// draw
	MDI_DETECTFREEFLYING,
	MDI_MCREADYCACHE,
	MDI_DOTHERMALHISTORY,	// draw 
	MDI_LAST_DOINIT,
 } MDI_t;

 GEXTERN bool *MasterDoInits[MDI_LAST_DOINIT+1];

#endif // DOINITS_H

