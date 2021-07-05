/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   filesystem.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 juillet 2014, 13:45
 */

#ifndef FILESYSTEM_H
#define	FILESYSTEM_H
#include "tchar.h"
#include <string.h>

namespace lk {
    namespace filesystem {
        bool exist(const TCHAR* szPath);
        bool isDirectory(const TCHAR* szPath);
        bool isFile(const TCHAR* szPath);

        bool createDirectory(const TCHAR* szPath);

        bool copyFile(const TCHAR* szSrc, const TCHAR* szDst, bool overwrite);
        bool moveFile(const TCHAR* szSrc, const TCHAR* szDst);
        bool deleteFile(const TCHAR* szPath);

        size_t getFileSize(const TCHAR* szPath);

        inline bool isDots(const TCHAR* szName) {
            return ((_tcscmp(szName, _T(".")) == 0) || (_tcscmp(szName, _T("..")) == 0));
        }


		class directory_iterator_impl;

        class directory_iterator {
        public:
            directory_iterator(const TCHAR* szPath);
            ~directory_iterator();

            directory_iterator& operator++();
            operator bool();

            bool isDirectory() const;
            const TCHAR* getName() const;

        private:
            directory_iterator_impl* _impl;
        };


        // name of current executable file
        bool getExeName(TCHAR* szName, size_t MaxSize);

        // full Path of current executable file, included trail path separator
        bool getExePath(TCHAR* szPath, size_t MaxSize);

        // root Path of current executable file name
        bool getBasePath(TCHAR* szPath, size_t MaxSize);

        // user directory ( ".../MyDocument/" on Windows, "/home/username/" on linux )
        bool getUserPath(TCHAR* szPath, size_t MaxSize);

        // fix directory separator
        void fixPath(TCHAR* szPath);


    }
}

#ifndef _FILESYSTEM_IMPL_
#undef CreateDirectory
#undef CopyFile
#undef MoveFile
#undef DeleteFile
#undef GetFileAttributes
#undef FindFirstFile
#undef FindNextFile
#undef FindClose
#undef GetModuleFileName
#undef SHGetSpecialFolderPath
#endif
#endif	/* FILESYSTEM_H */
