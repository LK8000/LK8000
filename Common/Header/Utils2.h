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

#if 0
int	FilterFast(ifilter_s *buf, int minvalue, int maxvalue);
bool	InitFilterBuffer(ifilter_s *buf, short bsize);
int	FilterRotary(ifilter_s *buf, int minvalue, int maxvalue);
void	InsertRotaryBuffer(ifilter_s *buf, int value);
#endif

bool	InitLDRotary(ldrotary_s *buf);
void	InsertLDRotary(ldrotary_s *buf, int distance, int altitude);
double	CalculateLDRotary(ldrotary_s *buf, DERIVED_INFO *Calculated);

// TrueWind functions
void	InitWindRotary(windrotary_s *wbuf);
void	InsertWindRotary(windrotary_s *wbuf, double speed, double track, double altitude);
int	CalculateWindRotary(windrotary_s *wbuf, double iaspeed, double *wfrom, double *wspeed, int windcalctime, int wmode);

void	SetOverColorRef();
bool	CustomKeyHandler(const int key);
void	SetMapScales(void);
bool	LoadModelFromProfile(void);
TCHAR*  GetSizeSuffix(void);
void	LKRunStartEnd(bool);

void	InitLKFonts();
void	InitLKScreen();
void	InitLK8000();
int   GetFontRenderer();
bool	LockMode(short lmode);
void	BottomBarChange(bool advance);
void	InfoPageChange(bool advance);
int	roundupdivision(int a, int b);
void	Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);
void	InitModeTable();
void	SetModeType(short modeindex, short modetype);
void	NextModeType();
void	PreviousModeType();
void	NextModeIndex();
void	PreviousModeIndex();
void	SetModeIndex();
void	SoundModeIndex();
void	SelectMapSpace(short i);
void	UnselectMapSpace(short i);
int	GetInfoboxType(int i);
int	GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode);
double	GetMacCready(int wpindex, short wpmode);
void	unicodetoascii(TCHAR *text, int tsize, char *atext);

int ProcessVirtualKey(int x, int y, long keytime, short vkmode);

extern int GetOvertargetIndex(void);
extern void GetOvertargetName(TCHAR *overtargetname);
extern TCHAR * GetOvertargetHeader(void);
extern void RotateOvertarget(void);
extern void ToggleOverlays(void);
extern bool CheckClubVersion(void);
extern void ClubForbiddenMsg(void);

extern void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

/* REMOVE
extern HFONT                                   LK8UnitFont;
extern HFONT					LK8TitleFont;
extern HFONT					LK8TitleNavboxFont;
extern HFONT					LK8MapFont;
extern HFONT                                   LK8ValueFont;
extern HFONT                                   LK8TargetFont;
extern HFONT                                   LK8BigFont;
extern HFONT                                   LK8SmallFont;
extern HFONT                                   LK8MediumFont;
extern HFONT                                   LK8InfoBigFont;
extern HFONT                                   LK8InfoBigItalicFont;
extern HFONT                                   LK8InfoNormalFont;
extern HFONT					LK8InfoSmallFont;
extern HFONT					LK8PanelBigFont;
extern HFONT					LK8PanelMediumFont;
extern HFONT					LK8PanelSmallFont;
extern HFONT					LK8PanelUnitFont;
*/


#endif // LK8000_UTILS2_H_
