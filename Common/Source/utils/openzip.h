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
#include <tchar.h>
#include <assert.h>

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
#include <utility>

#ifdef WIN32
static inline
ZZIP_FILE * openzip(const TCHAR* szFile, const char *mode) {
	return zzip_fopen(szFile, mode);
}
#endif

class zzip_file_ptr {
public:
    zzip_file_ptr() : _fp() { }
    explicit zzip_file_ptr(ZZIP_FILE* fp) : _fp(fp) { }

	zzip_file_ptr(const zzip_file_ptr&) = delete;

	zzip_file_ptr(zzip_file_ptr&& src) {
		_fp = src._fp;
		src._fp = nullptr;
	}

	zzip_file_ptr& operator=(zzip_file_ptr&& src) {
		std::swap(_fp, src._fp);
	  return (*this);
	}

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
            _fp = nullptr;
        }
    }

    operator bool() const {
        return (_fp != nullptr); 
    }

    operator ZZIP_FILE*() const { 
        assert(_fp);
        return _fp; 
    }

protected:
    ZZIP_FILE* _fp;
};
#endif
#endif	/* OPENZIP_H */
