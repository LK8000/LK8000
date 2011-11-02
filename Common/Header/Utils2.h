/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils2.h,v 8.3 2010/12/15 12:40:49 root Exp root $
*/
#if !defined(LK8000_UTILS2_H_)
#define LK8000_UTILS2_H_

#if !defined(AFX_CALCULATIONS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#include "Calculations.h"
#endif

#include "MapWindow.h"

bool	InitLDRotary(ldrotary_s *buf);
void	InsertLDRotary(ldrotary_s *buf, int distance, int altitude);
double	CalculateLDRotary(ldrotary_s *buf, DERIVED_INFO *Calculated);

// TrueWind functions
void	InitWindRotary(windrotary_s *wbuf);
void	InsertWindRotary(windrotary_s *wbuf, double speed, double track, double altitude);
int	CalculateWindRotary(windrotary_s *wbuf, double iaspeed, double *wfrom, double *wspeed, int windcalctime, int wmode);

void	SetOverColorRef();
void	SetMapScales(void);
bool	LoadModelFromProfile(void);
TCHAR*  GetSizeSuffix(void);
void	LKRunStartEnd(bool);

int   GetFontRenderer();
int	roundupdivision(int a, int b);
void	Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);
bool	LockMode(short lmode);
double	GetMacCready(int wpindex, short wpmode);
void	unicodetoascii(TCHAR *text, int tsize, char *atext);

extern bool CheckClubVersion(void);
extern void ClubForbiddenMsg(void);

extern void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

#endif // LK8000_UTILS2_H_
