#include "memory_mapped_file.hpp"
#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

namespace memory_mapped_file
{
    unsigned int mmf_granularity()
    {
    #if defined(_WIN32)
        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        return SystemInfo.dwAllocationGranularity;
    #else
        return sysconf(_SC_PAGE_SIZE);
    #endif
    }

    base_mmf::base_mmf():
        data_(0),
        offset_(0),
        mapped_size_(0),
        file_size_(0),
        granularity_(mmf_granularity()),
    #if defined(_WIN32)
        file_handle_(INVALID_HANDLE_VALUE),
        file_mapping_handle_(INVALID_HANDLE_VALUE)
    #else
        file_handle_(-1)
    #endif
    {
    }

    base_mmf::~base_mmf()
    {
        close();
    }

    void base_mmf::close()
    {
        unmap();
    #if defined(_WIN32)
        ::CloseHandle(file_handle_);
        file_handle_ = (void*)-1;
    #else
        ::close(file_handle_);
        file_handle_ = -1;
    #endif
        file_size_ = 0;
    }

    void base_mmf::unmap()
    {
        if (data_)
        {
            char* real_data = data_
                - (offset_ - offset_ / granularity_ * granularity_);
    #if defined(_WIN32)
            ::UnmapViewOfFile(real_data);
            ::CloseHandle(file_mapping_handle_);
            file_mapping_handle_ = INVALID_HANDLE_VALUE;
    #else
            size_t real_mapped_size = mapped_size_ + (data_ - real_data);
            ::munmap(const_cast<char*>(real_data), real_mapped_size);
    #endif
        }
        data_ = 0;
        offset_ = 0;
        mapped_size_ = 0;
    }

    size_t base_mmf::query_file_size_()
    {
    #if defined(_WIN32)
        DWORD high_size;
        DWORD low_size = GetFileSize(file_handle_, &high_size);
        return (size_t(high_size) << 32) | low_size;
    #else
        struct stat sbuf;
        if (::fstat(file_handle_, &sbuf) == -1) return 0;
        return sbuf.st_size;
    #endif
    }

    read_only_mmf::read_only_mmf(TCHAR const* pathname, bool map_all)
    {
        open(pathname, map_all);
    }

    void read_only_mmf::open(TCHAR const* pathname, bool map_all)
    {
        if (! pathname) return;
        if (is_open()) close();
    #if defined(_WIN32)
        file_handle_ = ::CreateFile(pathname, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (file_handle_ == INVALID_HANDLE_VALUE) return;
    #else
        file_handle_ = ::open(pathname, O_RDONLY);
        if (file_handle_ == -1) return;        
    #endif
        file_size_ = query_file_size_();
        if (map_all) map();
    }
    
    void read_only_mmf::map(
        size_t offset, size_t requested_size)
    {
        unmap();
        if (offset >= file_size_) return;
        size_t mapping_size = requested_size && offset + requested_size
            < file_size_ ? requested_size : file_size_ - offset;
        if (mapping_size <= 0) return;
        size_t real_offset = offset / granularity_ * granularity_;
    #if defined(_WIN32)
        file_mapping_handle_ = ::CreateFileMapping(
            file_handle_, 0, PAGE_READONLY, (offset + mapping_size) >> 32,
            (offset + mapping_size) & 0xFFFFFFFF, 0);
        if (file_mapping_handle_ == INVALID_HANDLE_VALUE) return;
        char* real_data = static_cast<char*>(::MapViewOfFile(
            file_mapping_handle_, FILE_MAP_READ, real_offset >> 32,
            real_offset & 0xFFFFFFFF, offset - real_offset + mapping_size));
        if (! real_data) return;
    #else
        char* real_data = static_cast<char*>(::mmap(
            0, offset - real_offset + mapping_size, PROT_READ, MAP_SHARED,
            file_handle_, real_offset));
        if (real_data == MAP_FAILED) return;
    #endif
        data_ = real_data + (offset - real_offset);
        mapped_size_ = mapping_size;
        offset_ = offset;
    }

    writable_mmf::writable_mmf(TCHAR const* pathname,
        memory_mapped_file::mmf_exists_mode exists_mode,
        memory_mapped_file::mmf_doesnt_exist_mode doesnt_exist_mode)
    {
        open(pathname, exists_mode, doesnt_exist_mode);
    }

    void writable_mmf::open(TCHAR const* pathname,
        memory_mapped_file::mmf_exists_mode exists_mode,
        memory_mapped_file::mmf_doesnt_exist_mode doesnt_exist_mode)
    {
        if (! pathname) return;
        if (is_open()) close();
    #if defined(_WIN32)
        int win_open_mode;
        
        switch (exists_mode)
        {
        case if_exists_just_open:
        case if_exists_map_all:
            win_open_mode = doesnt_exist_mode == if_doesnt_exist_create ?
                OPEN_ALWAYS : OPEN_EXISTING;
            break;
        case if_exists_truncate:
            win_open_mode = doesnt_exist_mode == if_doesnt_exist_create ?
                CREATE_ALWAYS : TRUNCATE_EXISTING;
            break;
        default:
            if (doesnt_exist_mode == if_doesnt_exist_create)
            {
                win_open_mode = CREATE_NEW;
            }
            else return;
        }

        file_handle_ = ::CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
            win_open_mode, FILE_ATTRIBUTE_NORMAL, 0);
        if (file_handle_ == INVALID_HANDLE_VALUE) return;
    #else
        int posix_open_mode = O_RDWR;
        switch (exists_mode)
        {
        case if_exists_just_open:
        case if_exists_map_all:
            posix_open_mode |= doesnt_exist_mode == if_doesnt_exist_create ?
                O_CREAT : 0;
            break;
        case if_exists_truncate:
            posix_open_mode |= doesnt_exist_mode == if_doesnt_exist_create ?
                O_TRUNC | O_CREAT : O_TRUNC;
            break;
        default:
            if (doesnt_exist_mode == if_doesnt_exist_create)
                posix_open_mode |= O_EXCL | O_CREAT;
            else return;
        }
        file_handle_ = ::open(pathname, posix_open_mode,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_handle_ == -1) return;
    #endif
        file_size_ = query_file_size_();
        if (exists_mode == if_exists_map_all && file_size_ > 0) map();
    }

    void writable_mmf::map(size_t offset, size_t requested_size)
    {
        unmap();
        if (offset > file_size_) return;
        size_t mapping_size = requested_size ?
            requested_size : file_size_ - offset;
        size_t real_offset = offset / granularity_ * granularity_;
    #if defined(_WIN32)
        file_mapping_handle_ = ::CreateFileMapping(
            file_handle_, 0, PAGE_READWRITE, (offset + mapping_size) >> 32,
            (offset + mapping_size) & 0xFFFFFFFF, 0);
        if (file_mapping_handle_ == INVALID_HANDLE_VALUE) return;
        char* real_data = static_cast<char*>(::MapViewOfFile(
            file_mapping_handle_, FILE_MAP_WRITE, real_offset >> 32,
            real_offset & 0xFFFFFFFF, offset - real_offset + mapping_size));
        if (! real_data) return;    
    #else
        if (offset + mapping_size > file_size_)
        {
            if (-1 == ftruncate(file_handle_, offset + mapping_size)) return;
            file_size_ = offset + mapping_size;
        }
        char* real_data = static_cast<char*>(::mmap(
            0, offset - real_offset + mapping_size, PROT_READ | PROT_WRITE, MAP_SHARED,
            file_handle_, real_offset));
        if (data_ == MAP_FAILED) return;
    #endif
        if (offset + mapping_size > file_size_)
        {
            file_size_ = offset + mapping_size;
        }
        data_ = real_data + (offset - real_offset);
        mapped_size_ = mapping_size;
        offset_ = offset;
    }

    bool writable_mmf::flush()
    {
        if (data_)
        {
            char* real_data = data_
                - (offset_ - offset_ / granularity_ * granularity_);
            size_t real_mapped_size = mapped_size_ + (data_ - real_data);
    #if defined(_WIN32)
            return ::FlushViewOfFile(real_data, real_mapped_size) != 0
                && FlushFileBuffers(file_handle_) != 0;
            if (::FlushViewOfFile(real_data, real_mapped_size) == 0) return false;
    #else
            if (::msync(real_data, real_mapped_size, MS_SYNC) != 0) return false;
    #endif
        }
    #if defined(_WIN32)
        return FlushFileBuffers(file_handle_) != 0;
    #else
        return true;
    #endif
    }
}
