/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   filesystem.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 juillet 2014, 13:45
 */

#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <string>

#ifdef UNICODE
#error "POSIX & UNICODE is unsupported"
#endif

bool lk::filesystem::exist(const TCHAR* szPath) {
    struct stat path_stat = {};
    return !::stat(szPath, &path_stat);
}

bool lk::filesystem::isDirectory(const TCHAR* szPath) {
    struct stat path_stat = {};
    return (!::stat(szPath, &path_stat) && S_ISDIR(path_stat.st_mode));
}

bool lk::filesystem::isFile(const TCHAR* szPath) {
    struct stat path_stat = {};
    return (!::stat(szPath, &path_stat) && !(S_ISDIR(path_stat.st_mode)));
}

bool lk::filesystem::createDirectory(const TCHAR* szPath) {
    if (!szPath) {
        return false;
    }

    if (!isDirectory(szPath)) {
        // if directory not Exists, try to create parent directory before create this !
        std::string sPath(szPath);
        if ((*sPath.rbegin()) == _T('/')) {
            // remove trailing directory separators.
            sPath.erase(sPath.length() - 1);
        }
        const std::string::size_type found = sPath.find_last_of(_T("/"));
        if (found != sPath.npos) {
            // make recursive call with parent directory path !
            if (!createDirectory(sPath.substr(0, found).c_str())) {
                return false;
            }
        }
        if (mkdir(szPath, S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST) {
            // if Fails because Already Exists return true !
            return false;
        }
    }
    return true;
}

bool lk::filesystem::copyFile(const TCHAR* szSrc, const TCHAR* szDst, bool overwrite) {
    int source = open(szSrc, O_RDONLY, 0);
    if (source == -1) {
        // source can't be read;
        return false;
    }
    // get size of source
    struct stat stat_source = {};
    fstat(source, &stat_source);

    int dest = open(szDst, O_WRONLY | O_CREAT | (overwrite ? O_TRUNC : 0), stat_source.st_mode);
    if (dest == -1) {
        // dest can't be write;
        close(source);
        return false;
    }


    // copy file content
    off_t offset = 0;
    ssize_t dest_size = sendfile(dest, source, &offset, stat_source.st_size);
    if (dest_size == -1 && errno == EINVAL) {
        // compat for Linux kernel before 2.6.33
        unsigned char Buffer[1024];
        size_t nRead = -1;
        dest_size = 0;
        while (nRead > 0) {
            nRead = read(source, Buffer, sizeof (Buffer));
            if (nRead > 0) {
                dest_size += write(dest, Buffer, nRead);
            }
        }
    }

    close(source);
    close(dest);

    if (dest_size != stat_source.st_size) {
        // if copy fail remove dest file;
        unlink(szDst);
        return false;
    }
    return true;
}

bool lk::filesystem::moveFile(const TCHAR* szSrc, const TCHAR* szDst) {
    return !rename(szSrc, szDst);
}

bool lk::filesystem::deleteFile(const TCHAR* szPath) {
    return !remove(szPath);
}


namespace lk {
    namespace filesystem {

        class directory_iterator_impl_POSIX : public directory_iterator_impl {
        public:

            directory_iterator_impl_POSIX(const TCHAR* szPath) {

                std::string strPath(szPath);
                const std::string::size_type found = strPath.find_last_of("/\\");
                _sPattern = strPath.substr(found + 1);
                strPath.erase(found + 1);

                dirp = opendir(strPath.c_str());
                entry = nullptr;
                if (dirp) {
                    ++(*this); // advance to first
                }
            }

            virtual ~directory_iterator_impl_POSIX() {
                if(dirp) {
                    closedir(dirp);
                }
            }

            virtual void operator++() {
                do {
                    entry = readdir(dirp);
                    if (!entry) {
                        // no Next close directory
                        closedir(dirp);
                        dirp = nullptr;
                        entry = nullptr;
                    }
                } while ((*this) && (isDots(getName()) || ((fnmatch(_sPattern.c_str(), getName(), FNM_CASEFOLD)) != 0)));
            }

            virtual operator bool() {
                return (dirp);
            }

            virtual bool isDirectory() const {
#if defined(_DIRENT_HAVE_D_TYPE) && defined(DTTOIF)
                return (entry && S_ISDIR(DTTOIF(entry->d_type)));
#else
                struct stat st;
                if (fstatat(dirfd(dirp), getName(), &st, 0) == 0) {
                    return S_ISDIR(st.st_mode);
                }
                return false;
#endif
            }

            virtual const TCHAR * getName() const {
                return entry ? entry->d_name : "";
            }

        private:
            DIR *dirp;
            struct dirent *entry;
            std::string _sPattern;
        };
    };
};

lk::filesystem::directory_iterator::directory_iterator(const TCHAR* szPath) : _impl(new directory_iterator_impl_POSIX(szPath)) {

}

bool lk::filesystem::getExeName(TCHAR* szName, size_t MaxSize) {
    ssize_t ret = readlink("/proc/self/exe", szName, MaxSize);
    if (-1 == ret) {
        return false;
    }
    szName[std::min((ssize_t)MaxSize, ret)] = '\0';

    TCHAR* szSep = _tcsrchr(szName, '/');
    if (szSep) {
        _tcscpy(szName, szSep);
    }

    return true;
}

bool lk::filesystem::getExePath(TCHAR* szPath, size_t MaxSize) {
    ssize_t ret = readlink("/proc/self/exe", szPath, MaxSize);
    if (-1 == ret) {
        return false;
    }
    szPath[std::min((ssize_t)MaxSize, ret)] = '\0';

    TCHAR* szSep = _tcsrchr(szPath, '/');
    if (!szSep) {
        szSep = szPath;
    } else {
        (*szSep++) = '/';
    }
    (*szSep) = '\0';

    return true;
}

bool lk::filesystem::getBasePath(TCHAR* szPath, size_t MaxSize) {
    ssize_t ret = readlink("/proc/self/exe", szPath, MaxSize);
    if (-1 == ret) {
        return false;
    }
    szPath[std::min((ssize_t)MaxSize, ret)] = '\0';

    TCHAR* szTmp = szPath;
    if ((*szTmp) == '/') {
        ++szTmp;
    }

    szTmp = strchr(szTmp, '/');
    if (szTmp) {
        (*(++szTmp)) = '\0';
    }

    return true;
}

#ifndef ANDROID
bool lk::filesystem::getUserPath(TCHAR* szPath, size_t MaxSize) {

    szPath[0] = '\0';
    char* szHome = getenv("HOME");
    if (szHome) {
        strncpy(szPath, szHome, MaxSize);
    }

    if (strlen(szPath) == 0) {
        struct passwd pwd;
        struct passwd *result;

        int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (bufsize == -1) /* Value was indeterminate */
            bufsize = 16384; /* Should be more than enough */

        char* buf = (char*)malloc(bufsize);
        int s = getpwuid_r(getuid(), &pwd, buf, bufsize, &result);
        if (s != 0 || result == NULL) {
            free(buf);
            return false;
        }
    }

    if (szPath[0] != '\0') {
        size_t n = _tcslen(szPath);
        if (szPath[n - 1] != '/') {
            _tcscat(szPath, "/");
        }
        return true;
    }

    return false;
}
#endif

void lk::filesystem::fixPath(TCHAR* szPath) {
    TCHAR * sz = _tcsstr(szPath, _T("\\"));
    while(sz) {
        (*sz) = _T('/');
        sz = _tcsstr(sz, _T("\\"));
    }
}

size_t lk::filesystem::getFileSize(const TCHAR* szPath) {
    struct stat st;
    if (stat(szPath, &st) < 0 || !S_ISREG(st.st_mode)) {
        return 0;
    }

    return st.st_size;
}
