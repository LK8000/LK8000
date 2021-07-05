/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   filesystem.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 juillet 2014, 13:45
 */

#include <windows.h>
#include <shlobj.h>
#include "Util/tstring.hpp"

bool lk::filesystem::exist(const TCHAR* szPath) {
    DWORD dwAttribut = GetFileAttributes(szPath);
    return (dwAttribut != INVALID_FILE_ATTRIBUTES);
}

bool lk::filesystem::isDirectory(const TCHAR* szPath) {
    DWORD dwAttribut = GetFileAttributes(szPath);
    if (dwAttribut != INVALID_FILE_ATTRIBUTES) {
        return (dwAttribut & FILE_ATTRIBUTE_DIRECTORY);
    }
    return false;
}

bool lk::filesystem::isFile(const TCHAR* szPath) {
    DWORD dwAttribut = GetFileAttributes(szPath);
    if (dwAttribut != INVALID_FILE_ATTRIBUTES) {
        return !(dwAttribut & FILE_ATTRIBUTE_DIRECTORY);
    }
    return false;
}

bool lk::filesystem::createDirectory(const TCHAR* szPath) {
    if (!szPath) {
        return false;
    }

    if (!isDirectory(szPath)) {
        // if directory not Exists, try to create parent directory before create this !
        tstring sPath(szPath);
        if ((*sPath.rbegin()) == _T('\\')) {
            // remove trailing directory separators.
            sPath.erase(sPath.length() - 1);
        }
        const tstring::size_type found = sPath.find_last_of(_T("\\"));
        if (found != sPath.npos) {
            // make recursive call with parent directory path !
            if (!createDirectory(sPath.substr(0, found).c_str())) {
                return false;
            }
        }
        if (!CreateDirectory(szPath, NULL)) {
            // if Fails because Already Exists return true !
            return (GetLastError() != ERROR_ALREADY_EXISTS);
        }
    }
    return true;
}

bool lk::filesystem::copyFile(const TCHAR* szSrc, const TCHAR* szDst, bool overwrite) {
    return CopyFile(szSrc, szDst, !overwrite);
}

bool lk::filesystem::moveFile(const TCHAR* szSrc, const TCHAR* szDst) {
    return MoveFile(szSrc, szDst);
}

bool lk::filesystem::deleteFile(const TCHAR* szPath) {
    return DeleteFile(szPath);
}



namespace lk {
    namespace filesystem {

        class directory_iterator_impl_WIN32 : public directory_iterator_impl {
        public:

            directory_iterator_impl_WIN32(const TCHAR* szPath) {
                _hFind = FindFirstFile(szPath, &_FindData);
                if (_hFind != INVALID_HANDLE_VALUE) {
                    if (isDots(getName())) {
                        ++(*this);
                    }
                }
            }

            virtual ~directory_iterator_impl_WIN32() {
                if (_hFind != INVALID_HANDLE_VALUE) {
                    FindClose(_hFind);
                }
            }

            virtual void operator++() {
                do {
                    if (!FindNextFile(_hFind, &_FindData)) {
                        FindClose(_hFind);
                        _hFind = INVALID_HANDLE_VALUE;
                    }
                } while ((*this) && isDots(getName()));
            }

            virtual operator bool() {
                return (_hFind != INVALID_HANDLE_VALUE);
            }

            virtual bool isDirectory() const {
                return ((_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
            }

            virtual const TCHAR * getName() const {
                return _FindData.cFileName;
            }

        private:
            HANDLE _hFind;
            WIN32_FIND_DATA _FindData;
        };
    };
};

lk::filesystem::directory_iterator::directory_iterator(const TCHAR* szPath) : _impl(new directory_iterator_impl_WIN32(szPath)) {

}

bool lk::filesystem::getExeName(TCHAR* szName, size_t MaxSize) {
    DWORD nSize = GetModuleFileName(NULL, szName, MaxSize);
    if (nSize >= MaxSize) {
        // insuficient buffer spaces;
        return false;
    }

    TCHAR* szSep = _tcsrchr(szName, _T('\\'));
    if (!szSep) {
        _tcscpy(szName, szSep);
    }

    return true;
}

bool lk::filesystem::getExePath(TCHAR* szPath, size_t MaxSize) {
    DWORD nSize = GetModuleFileName(NULL, szPath, MaxSize);
    if (nSize >= MaxSize) {
        // insuficient buffer spaces;
        return false;
    }

    TCHAR* szSep = _tcsrchr(szPath, _T('\\'));
    if (!szSep) {
        szSep = szPath;
    } else {
        (*szSep++) = '\\';
    }
    (*szSep) = _T('\0');

    return true;
}

bool lk::filesystem::getBasePath(TCHAR* szPath, size_t MaxSize) {
    DWORD nSize = GetModuleFileName(NULL, szPath, MaxSize);
    if (nSize >= MaxSize) {
        // insuficient buffer spaces;
        return false;
    }

    TCHAR* szTmp = szPath;
    if (_tcsncmp(szTmp, _T("\\\\?\\"), 4)) {
        szTmp += 4;
    } else if ((*szTmp) == _T('\\')) {
        ++szTmp;
    }

    szTmp = _tcschr(szTmp, _T('\\'));
    if (szTmp) {
        (*(++szTmp)) = _T('\0');
    }

    return true;
}

bool lk::filesystem::getUserPath(TCHAR* szPath, size_t MaxSize) {
    if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_PERSONAL, false)) {
        if (szPath[0] != _T('\0')) {
            size_t n = _tcslen(szPath);
            if (szPath[n - 1] != _T('\\')) {
                _tcscat(szPath, _T("\\"));
            }
            return true;
        }
    }
    return false;
}

void lk::filesystem::fixPath(TCHAR* szPath) {
    TCHAR * sz = _tcsstr(szPath, _T("/"));
    while(sz) {
        (*sz) = _T('\\');
        sz = _tcsstr(sz, _T("/"));
    }
}

size_t lk::filesystem::getFileSize(const TCHAR* szPath) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesEx(szPath, GetFileExInfoStandard, &data) ||
            (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return 0;
    }

    return data.nFileSizeLow | (uint64_t(data.nFileSizeHigh) << 32);
}
