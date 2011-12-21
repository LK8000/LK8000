/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKInterface.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
#if !defined(LK8000_LKINTERFACE_H)
#define LK8000_LKINTERFACE_H

void    InitLKFonts();
void    InitLKScreen();
void    InitLK8000();
void    InitModeTable();

bool	CustomKeyHandler(const int key);
void	BottomBarChange(bool advance);
void	InfoPageChange(bool advance);
void	SetModeType(short modeindex, short modetype);
void	NextModeType();
void	PreviousModeType();
void	NextModeIndex();
void	PreviousModeIndex();
void	SetModeIndex(short i);
void	SoundModeIndex();
void	SelectMapSpace(short i);
void	UnselectMapSpace(short i);
int	GetInfoboxType(int i);
int	GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode);

extern int GetOvertargetIndex(void);
extern void GetOvertargetName(TCHAR *overtargetname);
extern TCHAR * GetOvertargetHeader(void);
extern void RotateOvertarget(void);
extern void ToggleOverlays(void);

int ProcessVirtualKey(int x, int y, long keytime, short vkmode);


#endif // LK8000_LKINTERFACE_H
