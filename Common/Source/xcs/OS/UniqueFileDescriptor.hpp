/*
 * Copyright (C) 2012 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XCSOAR_UNIQUE_FILE_DESCRIPTOR_HPP
#define XCSOAR_UNIQUE_FILE_DESCRIPTOR_HPP

#include "FileDescriptor.hpp"

#include <utility>

/**
 * An OO wrapper for a UNIX file descriptor.
 */
class UniqueFileDescriptor : protected FileDescriptor {
public:
  UniqueFileDescriptor():FileDescriptor(FileDescriptor::Undefined()) {}

protected:
  explicit UniqueFileDescriptor(int _fd):FileDescriptor(_fd) {
    assert(IsDefined());
  }

public:
  explicit UniqueFileDescriptor(FileDescriptor _fd)
    :FileDescriptor(_fd) {}

  UniqueFileDescriptor(UniqueFileDescriptor &&other)
    :FileDescriptor(other.Steal()) {}

  ~UniqueFileDescriptor() {
    Close();
  }

  UniqueFileDescriptor &operator=(UniqueFileDescriptor &&other) {
    std::swap(fd, other.fd);
    return *this;
  }

  /**
   * Convert this object to its #FileDescriptor base type.
   */
  const FileDescriptor &ToFileDescriptor() const {
    return *this;
  }

  using FileDescriptor::IsDefined;
  using FileDescriptor::Get;

protected:
  void Set(int _fd) {
    assert(!IsDefined());
    assert(_fd >= 0);

    FileDescriptor::Set(_fd);
  }

  using FileDescriptor::Steal;

public:
  using FileDescriptor::Open;
  using FileDescriptor::OpenReadOnly;

#ifdef HAVE_POSIX
  using FileDescriptor::OpenNonBlocking;

  static bool CreatePipe(UniqueFileDescriptor &r, UniqueFileDescriptor &w) {
    return FileDescriptor::CreatePipe(r, w);
  }

  using FileDescriptor::SetNonBlocking;
  using FileDescriptor::SetBlocking;
  using FileDescriptor::Duplicate;

  static bool CreatePipe(FileDescriptor &r, FileDescriptor &w);
#endif

#ifdef __linux__
  using FileDescriptor::CreateEventFD;
  using FileDescriptor::CreateSignalFD;
  using FileDescriptor::CreateInotify;
#endif

  void Close() {
    if (IsDefined())
      FileDescriptor::Close();
  }

  using FileDescriptor::Rewind;
  using FileDescriptor::GetSize;
  using FileDescriptor::Read;
  using FileDescriptor::Write;

#ifdef HAVE_POSIX
  using FileDescriptor::Poll;
  using FileDescriptor::WaitReadable;
  using FileDescriptor::WaitWritable;
#endif
};

#endif
