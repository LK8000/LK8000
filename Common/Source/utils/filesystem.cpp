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
#define _FILESYSTEM_IMPL_

#include "filesystem.h"

#include "LocalPath.h"

#include <cassert>
#include <span>
#include <zzip/lib.h>

namespace lk {
    namespace filesystem {
        // private class declaration

        class directory_iterator_impl {
        public:

            directory_iterator_impl() {
            }

            virtual ~directory_iterator_impl() {
            }

            virtual void operator++() = 0;
            virtual operator bool() = 0;

            virtual bool isDirectory() const = 0;
            virtual const TCHAR* getName() const = 0;

        };

    }  // namespace filesystem
} // namespace lk

#ifdef _WIN32
#include "filesystem_WIN32.cpp"
#else
#include "filesystem_POSIX.cpp"
#endif

lk::filesystem::directory_iterator::~directory_iterator() {
    delete _impl;
}

lk::filesystem::directory_iterator& lk::filesystem::directory_iterator::operator++() {
    ++(*_impl);
    return (*this);
}

lk::filesystem::directory_iterator::directory_iterator::operator bool() {
    return (bool)(*_impl);
}

bool lk::filesystem::directory_iterator::isDirectory() const {
    return _impl->isDirectory();
}

const TCHAR* lk::filesystem::directory_iterator::getName() const {
    return _impl->getName();
}

namespace {

    bool checkFilter(const TCHAR* filename,  const TCHAR **suffix_filters, size_t filter_count) {
        if (!suffix_filters || filter_count == 0) {
            return true;
        }
        size_t filename_size = _tcslen(filename);
        for (const TCHAR* suffix : std::span(suffix_filters, filter_count)) {
            if(!suffix || !suffix[0]) {
                continue;  // skip invalid/empty filter entries
            }
            size_t suffix_size = _tcslen(suffix);
            if(filename_size < suffix_size) {
                continue;
            }
            const TCHAR* filename_suffix = &filename[filename_size - suffix_size];
            if (_tcsicmp(filename_suffix, suffix) == 0) {
                return true;
            }
        }
        return false;
    }

}  // namespace

bool lk::filesystem::ScanDirectories(const TCHAR* sPath, const TCHAR* subdir,
                                     const TCHAR** suffix_filters,
                                     size_t filter_count,
                                     const scan_file_callable_t& on_file) {
  assert(sPath);
  assert(subdir);
  assert(on_file);

  using tstring = std::basic_string<TCHAR>;

  tstring FileName = sPath;
  if (!FileName.empty() && FileName.back() != _T('/')) {
    FileName += _T('/');
  }
  if (_tcslen(subdir) > 0) {
    FileName += subdir;
    if (FileName.back() != _T('/')) {
      FileName += _T('/');
    }
  }

  FileName += _T("*");
  lk::filesystem::fixPath(FileName.data());

  for (lk::filesystem::directory_iterator It(FileName.c_str()); It; ++It) {
    if (It.isDirectory()) {
      FileName = subdir;
      if (FileName.size() > 0) {
        FileName += _T("/");
      }
      FileName += It.getName();
      if (!ScanDirectories(sPath, FileName.c_str(), suffix_filters, filter_count, on_file)) {
        return false;
      }
    }
    else if (checkFilter(It.getName(), suffix_filters, filter_count)) {
      tstring RelativePath = subdir;
      if (RelativePath.size() > 0) {
        RelativePath += _T("/");
      }
      RelativePath += It.getName();
      if (!on_file(It.getName(), RelativePath.c_str())) {
        return false;
      }
    }
  }
  return true;
}

#ifndef UNICODE

void lk::filesystem::ScanZipDirectory(const TCHAR* subdir,
                                      const TCHAR** suffix_filters,
                                      size_t filter_count,
                                      const scan_file_callable_t& on_file) {
  assert(subdir);
  assert(on_file);

  const zzip_strings_t ext[] = {".zip", ".ZIP", "", 0};
  zzip_error_t zzipError;

  using tstring = std::basic_string<TCHAR>;

  tstring sRootPath = LKGetSystemPath();
  if(sRootPath.empty()) {
    return;
  }
  sRootPath.pop_back(); // remove trailing directory separator

  ZZIP_DIR* dir = zzip_dir_open_ext_io(sRootPath.c_str(), &zzipError, ext, nullptr);
  if (!dir) {
    return;
  }

  const size_t subdir_size = _tcslen(subdir);

  ZZIP_DIRENT dirent;
  while (zzip_dir_read(dir, &dirent)) {
    if (_tcsnicmp(subdir, dirent.d_name, subdir_size) != 0) {
      continue;
    }
    // Ensure we matched a complete directory component, not just a prefix
    if (subdir_size > 0 && dirent.d_name[subdir_size] != _T('/') &&
        dirent.d_name[subdir_size] != _T('\\') &&
        dirent.d_name[subdir_size] != _T('\0')) {
      continue;
    }

    if (!checkFilter(dirent.d_name, suffix_filters, filter_count)) {
      continue;
    }

    // Path separator in zip files is always '/', even on Windows
    TCHAR* fileName = _tcsrchr(dirent.d_name, _T('/'));
    fileName = fileName ? fileName + 1 : dirent.d_name;

    TCHAR* relativePath = dirent.d_name + _tcslen(subdir);
    while ((*relativePath) == _T('/') || (*relativePath) == _T('\\')) {
      ++relativePath;
    }

    if (!on_file(fileName, relativePath)) {
      break;
    }
  }

  zzip_dir_close(dir);
}

#endif  // !UNICODE
