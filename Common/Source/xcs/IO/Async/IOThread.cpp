/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "IOThread.hpp"
#include "FileEventHandler.hpp"

bool
IOThread::Start()
{
  assert(!IsDefined());
  assert(io_loop.IsEmpty());

  quit = false;

  if (!pipe.Create())
    return false;

  io_loop.Add(pipe.GetReadFD(), READ, *this);

  return Thread::Start();
}

void
IOThread::Stop()
{
  /* set the "quit" flag and wake up the thread */
  io_loop.Lock();
  quit = true;
  io_loop.Unlock();
  pipe.Signal();

  /* wait for the thread to finish */
  Join();

  io_loop.Remove(pipe.GetReadFD());
}

void
IOThread::LockAdd(FileDescriptor fd, unsigned mask, FileEventHandler &handler)
{
  io_loop.Lock();
  const bool old_modified = io_loop.IsModified();
  Add(fd, mask, handler);
  const bool new_modified = io_loop.IsModified();
  io_loop.Unlock();

  if (!old_modified && new_modified)
    pipe.Signal();
}

void
IOThread::LockRemove(FileDescriptor fd)
{
  io_loop.Lock();
  const bool old_modified = io_loop.IsModified();
  Remove(fd);
  const bool new_modified = io_loop.IsModified();

  if (new_modified && !IsInside()) {
    /* this method is synchronous: after returning, all handlers must
       be finished */

    io_loop.WaitUntilNotRunning();
  }

  io_loop.Unlock();

  if (!old_modified && new_modified)
    pipe.Signal();
}

void
IOThread::Run()
{
  io_loop.Lock();

  while (!quit) {
    io_loop.Wait();

    if (quit)
      break;

    io_loop.Dispatch();
  }

  io_loop.Unlock();
}

bool
IOThread::OnFileEvent(FileDescriptor fd, unsigned mask)
{
  assert(fd == pipe.GetReadFD());

  pipe.Read();
  return true;
}

