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

/*
 * using custom deleter to check null file ptr is useless for unique_ptr but mandatory for shared_ptr
 *   shared_ptr can be instantiated from unique_ptr, so to avoid mistake it's better to always use this custom deleter...
 */
struct file_deleter {
	void operator()(FILE* pFile) {
		if (pFile) {
			std::fclose(pFile);
		}
	}
};

using unique_file_ptr = std::unique_ptr<FILE, file_deleter>;

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
