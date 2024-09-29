/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LocalPath.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 02, 2024
 */

#ifndef _LOCAL_PATH_H
#define _LOCAL_PATH_H

#include "Compiler.h"
#include "tchar.h"
#include <cstddef>

const TCHAR *LKGetLocalPath();
const TCHAR *LKGetSystemPath();

void LocalPath(TCHAR* buffer, size_t size, const TCHAR* file = _T("")) gcc_nonnull_all;

template<size_t size>
void LocalPath(TCHAR (&buffer)[size], const TCHAR* file = _T("")) {
   LocalPath(buffer, size, file);
}

void LocalPath(TCHAR* buffer, size_t size, const TCHAR* SubPath, const TCHAR* file) gcc_nonnull_all;

template<size_t size>
void LocalPath(TCHAR (&buffer)[size], const TCHAR* SubPath, const TCHAR* file) {
   LocalPath(buffer, size, SubPath, file);
}

void SystemPath(TCHAR* buffer, size_t size, const TCHAR* file = _T("")) gcc_nonnull_all;

template<size_t size>
void SystemPath(TCHAR (&buffer)[size], const TCHAR* file = _T("")) {
   SystemPath(buffer, size, file);
}

void SystemPath(TCHAR* buffer, size_t size, const TCHAR* SubPath, const TCHAR* file) gcc_nonnull_all;

template<size_t size>
void SystemPath(TCHAR (&buffer)[size], const TCHAR* SubPath, const TCHAR* file) {
   SystemPath(buffer, size, SubPath, file);
}

void RemoveFilePathPrefix(const TCHAR* szPrefix, TCHAR* szFilePath) gcc_nonnull_all;

#endif // _LOCAL_PATH_H
