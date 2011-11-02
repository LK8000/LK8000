/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils2.h,v 8.3 2010/12/15 12:40:49 root Exp root $
*/
#if !defined(LK8000_UTILS2_H_)
#define LK8000_UTILS2_H_


bool	InitLDRotary(ldrotary_s *buf);
void	InitWindRotary(windrotary_s *wbuf);

void	SetOverColorRef();
TCHAR*  GetSizeSuffix(void);
void	LKRunStartEnd(bool);

bool	LockMode(short lmode);
double	GetMacCready(int wpindex, short wpmode);

extern bool CheckClubVersion(void);
extern void ClubForbiddenMsg(void);

#endif // LK8000_UTILS2_H_
