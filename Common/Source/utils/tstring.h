/* 
 * File:   tstring.h
 * Author: Bruno
 *
 * Created on 21 août 2013, 23:13
 */
#ifndef _TSTRING_H
#define	_TSTRING_H

#include <string>


namespace std {
#ifdef UNICODE
	typedef wstring tstring;
#else
	typedef string tstring;
#endif
}

#endif	/* TSTRING_H */

