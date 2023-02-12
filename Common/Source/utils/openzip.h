/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   openzip.h
 * Author: Bruno de Lacheisserie
 *
 * Created on December 20, 2014, 3:28 PM
 */

#ifndef OPENZIP_H
#define	OPENZIP_H

#include <zzip/zzip.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * on Win32 platform, this function is used only for topology.
 */
ZZIP_FILE * openzip(const char* szFile, const char *mode);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <memory>

#ifdef UNICODE
#include "utils/charset_helper.h"

static inline
ZZIP_FILE * openzip(const wchar_t* szFile, const char *mode) {
    std::string uname = to_utf8(szFile);
	return openzip(uname.c_str(), mode);
}

#endif

struct zzip_file_ptr_delete {
    void operator()(ZZIP_FILE *__ptr) {
        zzip_close(__ptr);
    }
};

using zzip_file_ptr = std::unique_ptr<ZZIP_FILE, zzip_file_ptr_delete>;


#endif /* __cplusplus */
#endif /* OPENZIP_H */
