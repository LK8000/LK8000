/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   openzip.h
 * Author: Bruno de Lacheisserie
 *
 * Created on December 20, 2014, 3:28 PM
 */

#ifndef OPENZIP_H
#define	OPENZIP_H

#ifdef __cplusplus
extern "C" {
#endif

ZZIP_FILE * openzip(const TCHAR* szFile, const char *mode);

#ifdef __cplusplus
}
#endif

#endif	/* OPENZIP_H */
