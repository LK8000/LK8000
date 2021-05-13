
#ifndef	_TCHAR_H_
#define _TCHAR_H_

/*
 * NOTE: This tests _UNICODE, which is different from the UNICODE define
 *       used to differentiate Win32 API calls.
 */
#ifndef	_UNICODE

/*
 * TCHAR, the type you should use instead of char.
 */
#ifndef _TCHAR_DEFINED
#ifndef RC_INVOKED
typedef char	TCHAR;
typedef char	_TCHAR;
#endif
#define _TCHAR_DEFINED
#endif

/*
 * _TEOF, the constant you should use instead of EOF.
 */
#define _TEOF (TCHAR)EOF

/*
 * __TEXT is a private macro whose specific use is to force the expansion of a
 * macro passed as an argument to the macros _T or _TEXT.  DO NOT use this
 * macro within your programs.  It's name and function could change without
 * notice.
 */
#define	__TEXT(q)	q

/*  for porting from other Windows compilers */
#define _tmain      main
#define _tWinMain   WinMain
#define _tenviron  _environ
#define __targv     __argv

/*
 * Non-unicode (standard) functions
 */

#define	_tprintf	printf
#define _ftprintf	fprintf
#define	_stprintf	sprintf
#define	_sntprintf	snprintf
#define	_vtprintf	vprintf
#define	_vftprintf	vfprintf
#define _vstprintf	vsprintf
#define	_vsntprintf	vsnprintf
#define	_tscanf		scanf
#define	_ftscanf	fscanf
#define	_stscanf	sscanf
#define	_fgettc		fgetc
#define	_fgettchar	_fgetchar
#define	_fgetts		fgets
#define	_fputtc		fputc
#define	_fputtchar	_fputchar
#define	_fputts		fputs
#define _tfdopen	_fdopen
#define	_tfopen		fopen
#define _tfreopen	freopen
#define	_tfsopen	_fsopen
#define	_tgetenv	getenv
#define	_tputenv	_putenv
#define	_tsearchenv	_searchenv
#define	_tmakepath	_makepath
#define	_tsplitpath	_splitpath
#define	_tfullpath	_fullpath
#define	_gettc		getc
#define	_getts		gets
#define	_puttc		putc
#define _puttchar       putchar
#define	_putts		puts
#define	_ungettc	ungetc
#define	_tcstod		strtod
#define	_tcstol		strtol
#define _tcstoul	strtoul
#define _tcstoull	strtoull
#define	_itot		_itoa
#define	_ltot		_ltoa
#define	_ultot		_ultoa
#define	_ttoi		atoi
#define	_ttol		atol
#define	_tcscat		strcat
#define _tcschr		strchr
#define _tcscmp		strcmp
#define _tcscpy		strcpy
#define _tcscspn	strcspn
#define	_tcslen		strlen
#define	_tcsncat	strncat
#define	_tcsncmp	strncmp
#define	_tcsncpy	strncpy
#define	_tcspbrk	strpbrk
#define	_tcsrchr	strrchr
#define _tcsspn		strspn
#define	_tcsstr		strstr
#define _tcstok		strtok
#define	_tcsdup		strdup
#define	_tcsicmp	strcasecmp
#define	_tcsnicmp	strncasecmp
#define	_tcsnset	_strnset
#define	_tcsrev		_strrev
#define _tcsset		_strset
#define	_tcslwr		_strlwr
#define	_tcsupr		_strupr
#define	_tcsxfrm	strxfrm
#define	_tcscoll	strcoll
#define	_tcsicoll	_stricoll
#define	_istalpha	isalpha
#define	_istupper	isupper
#define	_istlower	islower
#define	_istdigit	isdigit
#define	_istxdigit	isxdigit
#define	_istspace	isspace
#define	_istpunct	ispunct
#define	_istalnum	isalnum
#define	_istprint	isprint
#define	_istgraph	isgraph
#define	_istcntrl	iscntrl
#define	_istascii	isascii
#define _totupper	toupper
#define	_totlower	tolower
#define	_tasctime	asctime
#define	_tctime		ctime
#define	_tstrdate	_strdate
#define	_tstrtime	_strtime
#define	_tutime		_utime
#define _tcsftime	strftime
/* Macro functions */ 
#define _tcsdec     _strdec
#define _tcsinc     _strinc
#define _tcsnbcnt   _strncnt
#define _tcsnccnt   _strncnt
#define _tcsnextc   _strnextc
#define _tcsninc    _strninc
#define _tcsspnp    _strspnp
#define _strdec(_str1, _str2) ((_str1)>=(_str2) ? NULL : (_str2)-1)
#define _strinc(_str)  ((_str)+1)
#define _strnextc(_str) ((unsigned int) *(_str))
#define _strninc(_str, _inc) (((_str)+(_inc)))
#define _strncnt(_str, _cnt) ((strlen(_str)>_cnt) ? _count : strlen(_str))
#define _strspnp(_str1, _str2) ((*((_str1)+strspn(_str1,_str2))) ? ((_str1)+strspn(_str1,_str2)) : NULL)

#define _tchmod     _chmod
#define _tcreat     _creat
#define _tfindfirst _findfirst
#define _tfindnext  _findnext
#define _tmktemp    _mktemp
#define _topen      open
#define _taccess    _access
#define _tremove    remove
#define _trename    rename
#define _tsopen     _sopen
#define _tsetlocale setlocale
#define _tunlink    _unlink
#define _tfinddata_t    _finddata_t
#define _tchdir	    _chdir
#define _tgetcwd    _getcwd
#define _tgetdcwd   _getdcwd
#define _tmkdir	    _mkdir
#define _trmdir	    _rmdir
#define _tstat      _stat

#if 1  /* defined __MSVCRT__ */
/* Not in crtdll.dll. Define macros anyway? */
#define _ttoi64     _atoi64
#define _i64tot     _i64toa
#define _ui64tot    _ui64toa
#define _tcsnccoll  _strncoll
#define _tcsncoll   _strncoll
#define _tcsncicoll _strnicoll
#define _tcsnicoll  _strnicoll
#define _tfindfirsti64  _findfirsti64
#define _tfindnexti64   _findnexti64
#define _tfinddatai64_t _finddatai64_t
#define _tstati64   _stati64
#define _tstat64    _stat64
#endif  /* __MSVCRT__ */

/* dirent structures and functions */
#define _tdirent	dirent
#define _TDIR 		DIR
#define _topendir	opendir
#define _tclosedir	closedir
#define _treaddir	readdir
#define _trewinddir	rewinddir
#define _ttelldir	telldir
#define _tseekdir	seekdir

#endif	/* Not _UNICODE */

/*
 * UNICODE a constant string when _UNICODE is defined else returns the string
 * unmodified.  Also defined in w32api/winnt.h.
 */
#define TEXT(x)	__TEXT(x)
#define _TEXT(x)	__TEXT(x)
#define	_T(x)		__TEXT(x)

#endif	/* Not _TCHAR_H_ */

