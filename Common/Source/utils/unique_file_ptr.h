// Copyright (c) 2021-2023, Bruno de Lacheisserie
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of  nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef _UNIQUE_FILE_PTR_H_
#define _UNIQUE_FILE_PTR_H_

#include <cstdio>
#include <memory>
#include <cassert>
#include <cstdarg>

struct file_deleter {
	void operator()(FILE* pFile) {
		if (pFile) {
			std::fclose(pFile);
		}
	}
};

struct unique_file_ptr : public std::unique_ptr<FILE, file_deleter> {
	using std::unique_ptr<FILE, file_deleter>::unique_ptr;

 	int fseek(long _Offset, int _Origin) const {
		assert(*this);
		return std::fseek(get(), _Offset, _Origin);
	}

	size_t fwrite(const void* __restrict__ _Str, size_t _Size, size_t _Count) const {
		assert(*this);
		return std::fwrite(_Str, _Size, _Count, get());
	}

	size_t fread(void* __restrict__ _Str, size_t _Size, size_t _Count) const {
		assert(*this);
		return std::fread(_Str, _Size, _Count, get());
	}

	char* fgets(char* __restrict__ __dst, int __n) const {
		assert(*this);
		return std::fgets(__dst, __n, get());
	}

	int fputs(const char* __restrict__ _Str) const {
		assert(*this);
		return std::fputs(_Str, get());
	}

	int fputc(int _Ch) const {
		assert(*this);
		return std::fputc(_Ch, get());
	}

	int fflush() const {
		assert(*this);
		return std::fflush(get());
	}

	int vfprintf (const char* __format, va_list __local_argv) const {
		assert(*this);
		return std::vfprintf(get(), __format, __local_argv);
	}

	int fprintf (const char* __format, ...) const {
		va_list local_argv;
		va_start(local_argv, __format);
		int retval = vfprintf(__format, local_argv);
		va_end(local_argv);
		return retval;
	}

	long ftell() const {
		assert(*this);
		return std::ftell(get());
	}
};

inline
unique_file_ptr make_unique_file_ptr(const char* filename, const char* flags) {
	return unique_file_ptr(std::fopen(filename, flags));
}

#ifdef WIN32

inline
unique_file_ptr make_unique_file_ptr(const wchar_t* filename, const wchar_t* flags) {
	return unique_file_ptr(_wfopen(filename, flags));
}

#endif


#endif // _UNIQUE_FILE_PTR_H_
