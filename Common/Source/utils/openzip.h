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

#include <zzip/zzip.h>

#ifdef __cplusplus
extern "C" {
#endif

ZZIP_FILE * openzip(const TCHAR* szFile, const char *mode);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class zzip_file_ptr {
public:
    zzip_file_ptr() : _fp() { }
    explicit zzip_file_ptr(ZZIP_FILE* fp) : _fp(fp) { }

    zzip_file_ptr& operator=(ZZIP_FILE* fp) {
        if(_fp != fp) {
            close();
        }
        _fp = fp;
        return *this;
    }

    ~zzip_file_ptr() {
        close();
    }

    void close() {
        if(_fp) {
            zzip_close(_fp);
        }
    }

    operator bool() { return !!(_fp); }

    operator ZZIP_FILE*() { return _fp; }

private:
    ZZIP_FILE* _fp;
};
#endif
#endif	/* OPENZIP_H */
