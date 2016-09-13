#ifndef _AYGSHELL_H
#define _AYGSHELL_H
#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef	__cplusplus
extern "C" {
#endif

#if _WIN32_WCE >= 400

#include <windows.h>
#include <basetyps.h>	/* Make sure we have a CLSID definition */
#include <shellapi.h>           /* for WINSHELLAPI */
#include <sipapi.h>
#include <prsht.h>

/*
 * Menu Bar
 */
typedef struct tagSHMENUBARINFO {
  DWORD cbSize;
  HWND hwndParent;
  DWORD dwFlags;
  UINT nToolBarId;
  HINSTANCE hInstRes;
  int nBmpId;
  int cBmpImages;
  HWND hwndMB;
  COLORREF clrBk;
} SHMENUBARINFO, *PSHMENUBARINFO;

/* Values for dwFlags */
#define	SHCMBF_EMPTYBAR		0x01
#define	SHCMBF_HIDDEN		0x02
#define	SHCMBF_HIDESIPBUTTON	0x04
#define	SHCMBF_COLORBK		0x08
#define	SHCMBF_HMENU		0x10

#define SHIDIM_FLAGS                0x0001

#define SHIDIF_DONEBUTTON           0x0001
#define SHIDIF_SIZEDLG              0x0002
#define SHIDIF_SIZEDLGFULLSCREEN    0x0004
#define SHIDIF_SIPDOWN              0x0008
#define SHIDIF_FULLSCREENNOMENUBAR  0x0010
#define SHIDIF_EMPTYMENU            0x0020

typedef struct tagSHACTIVATEINFO
{
  DWORD cbSize;
  HWND hwndLastFocus;
  UINT fSipUp :1;
  UINT fSipOnDeactivation :1;
  UINT fActive :1;
  UINT fReserved :29;
} SHACTIVATEINFO, *PSHACTIVATEINFO;

typedef struct tagSHINITDLGINFO
{
	DWORD	dwMask;
	HWND	hDlg;
	DWORD	dwFlags;
} SHINITDLGINFO, *PSHINITDLGINFO;

WINSHELLAPI BOOL WINAPI SHInitDialog(PSHINITDLGINFO);
WINSHELLAPI BOOL WINAPI SHCreateMenuBar(SHMENUBARINFO*);
WINSHELLAPI HWND WINAPI SHFindMenuBar(HWND);
WINSHELLAPI HRESULT WINAPI SHCreateNewItem(HWND,REFCLSID);
WINSHELLAPI BOOL WINAPI SHFullScreen(HWND,DWORD);
WINSHELLAPI BOOL WINAPI SHSipInfo(UINT,UINT,PVOID,UINT);
/* next exported by ordinal only: @84 */
WINSHELLAPI BOOL WINAPI SHHandleWMActivate(HWND,WPARAM,LPARAM,SHACTIVATEINFO*,DWORD);
/* next exported by ordinal only: @83 */
WINSHELLAPI BOOL WINAPI SHHandleWMSettingChange(HWND,WPARAM,LPARAM,SHACTIVATEINFO*);

/* The following are not in device ROMs. */
extern BOOL SHInvokeContextMenuCommand(HWND,UINT,HANDLE);

extern BOOL SHHandleWMSettingChange(HWND, WPARAM, LPARAM, SHACTIVATEINFO *);
extern BOOL SHHandleWMActivate(HWND, WPARAM, LPARAM, SHACTIVATEINFO *, DWORD);

#if (_WIN32_WCE >= 0x0300)
typedef enum
{
	SIP_UP = 0,
	SIP_DOWN,
	SIP_FORCEDOWN,
	SIP_UNCHANGED,
	SIP_INPUTDIALOG
} SIPSTATE;

WINSHELLAPI BOOL WINAPI SHSipPreference (HWND hWnd, SIPSTATE st);
#endif /* _WIN32_WCE >= 0x0300 */

/*
 * Work with the PocketPC "New" menu.
 */
typedef struct NMNEWMENU {
	NMHDR	hdr;
	TCHAR	szReg[80];
	HMENU	hMenu;
	CLSID	clsId;
} NMNEWMENU, *PNMNEWMENU;

#define	NMN_GETAPPREGKEY	1101
#define	NMN_NEWMENUDESTROY	1102
#define	NMN_INVOKECOMMAND	1103

#define	IDM_NEWMENUMAX		3000

/*
 * The Shared New menu
 *
 * See http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/html/_ceconnewbutton.asp
 *
 * Values from several sources are identical
 * 	ftp://ftp.berlios.de/pub/pocketsipmsg/PocketSM-0.7.5-src.tar.gz
 * 	http://www.huihoo.com/doxygen/vlc/html/newres_8h-source.html
 * 	http://www.ee.umd.edu/courses/enee408g.S2002/report/group4/Final/Video/PocketPC/NEWRES.H
 */
#define	IDM_SHAREDNEW		10
#define	IDM_SHAREDNEWDEFAULT	11

/*
 * http://msdn2.microsoft.com/en-us/library/aa457781.aspx
 * http://www.ee.umd.edu/courses/enee408g.S2002/report/group4/Final/Video/PocketPC/NEWRES.H
 *
 */
#define	NOMENU			0xFFFF

/*
 * These are in the Boling book
 */
#define	IDS_SHNEW	1
#define	IDS_SHEDIT	2
#define	IDS_SHTOOLS	3
#define	IDS_SHVIEW	4
#define	IDS_SHFILE	5
#define	IDS_SHGO	6
#define	IDS_SHFAVORITES	7
#define	IDS_SHOPEN	8

/* Values for npPriority */
typedef enum {
	SHNP_INFORM = 0x1b1,
	SHNP_ICONIC
} SHNP;

/*
 * PocketPC Notifications
 */
typedef struct SHNOTIFICATIONDATA {
	DWORD		cbStruct,
			dwID;
	SHNP		npPriority;
	DWORD		csDuration;
	HICON		hicon;
	DWORD		grfFlags;
	CLSID		clsid;
	HWND		hwndSink;
	LPCTSTR		pszHTML,
			pszTitle;
	LPARAM		lParam;
} SHNOTIFICATIONDATA, *PSHNOTIFICATIONDATA;

extern LRESULT SHNotificationGetData(const CLSID *, DWORD, SHNOTIFICATIONDATA *);
extern LRESULT SHNotificationUpdate(DWORD, SHNOTIFICATIONDATA *);
extern LRESULT SHNotificationRemove(const CLSID *, DWORD);

/* Values for grfFlags */
#define	SHNF_STRAIGHTTOTRAY	1
#define	SHNF_CRITICAL		2
#define	SHNF_FORCEMESSAGE	8
#define	SHNF_DISPLAYON		16
#define	SHNF_SILENT		32

/*
 * Fullscreen applications
 */
extern BOOL SHFullScreen(HWND, DWORD);

/* Values for the second parameter to SHFullScreen */
#define	SHFS_SHOWTASKBAR	1
#define	SHFS_HIDETASKBAR	2
#define	SHFS_SHOWSIPBUTTON	4
#define	SHFS_HIDESIPBUTTON	8
#define	SHFS_SHOWSTARTICON	16
#define	SHFS_HIDESTARTICON	32

/*
 * SIPPREF appears to be some magic control to automatically display the SIP.
 * Use with
 *	CONTROL  "",-1,WC_SIPPREF, NOT WS_VISIBLE,-10,-10,5,5
 * See http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/html/_cerefwc_sippref.asp
 */
#define	WC_SIPPREF	L"SIPPREF"

/*
 * Stuff for SHRecognizeGesture
 *
 * See
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/html/_cerefshrginfo.asp
 * and
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/html/_cerefshrecognizegesture.asp
 */
#if (_WIN32_WCE >= 0x0420)
typedef struct tagSHRGI {
	DWORD	cbSize;
	HWND	hwndClient;
	POINT	ptDown;
	DWORD	dwFlags;
} SHRGINFO, *PSHRGINFO;

WINSHELLAPI DWORD SHRecognizeGesture(SHRGINFO *shrg);
#define SHRG_RETURNCMD 0x0001
#endif

#if (_WIN32_WCE >= 0x0300)
/*
 * http://www.docjar.com/html/api/org/eclipse/swt/internal/win32/OS.java.html
 */
#define	SHCMBM_SETSUBMENU	0x0590
#define	SHCMBM_GETSUBMENU	0x0591
#define	SHCMBM_GETMENU		0x0592

/* from http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=1733046&SiteID=1 */
#define SHMBOF_NODEFAULT    0x00000001
#define SHMBOF_NOTIFY       0x00000002
#define SHCMBM_OVERRIDEKEY  (WM_USER + 403)
#define VK_TBACK VK_ESCAPE

void SHSendBackToFocusWindow(UINT,WPARAM,LPARAM);
#endif /* _WIN32_WCE */

#if (_WIN32_WCE >= 0x0400)
HBITMAP SHLoadImageFile(LPCTSTR pszFileName);
HBITMAP SHLoadImageResource(HINSTANCE hinst, UINT uIdImageFile);
#endif	/*  _WIN32_WCE >= 0x0300 */

#endif /* _WIN32_WCE >= 400 */

#if (_WIN32_WCE >= 0x0300)
BOOL SHInitExtraControls(void);

#define	WS_NONAVDONEBUTTON	WS_MINIMIZEBOX

BOOL SHDoneButton(HWND, DWORD);
#define	SHDB_SHOW	1
#define	SHDB_HIDE	2

BOOL SHSetAppKeyWndAssoc(BYTE bVk, HWND hwnd);
#endif	/* _WIN32_WCE >= 0x0300 */

/*googling*/
#define GN_CONTEXTMENU 1000

/*http://msdn.microsoft.com/en-us/library/bb416652.aspx*/
typedef struct tagNMRGINFO {
	NMHDR	hdr;
	POINT	ptAction;
	DWORD	dwItemSpec;
} NMRGINFO, *PNMRGINFO;

#ifdef	__cplusplus
}
#endif

#endif	/* _AYGSHELL_H */
