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

#include "Screen/TopWindow.hpp"
#include "Screen/Custom/TopCanvas.hpp"
#include "Event/Globals.hpp"
#include "Event/Poll/Queue.hpp"

#include <X11/Xatom.h>

void
TopWindow::CreateNative(const TCHAR *text, PixelSize size,
                        TopWindowStyle style)
{
  x_display = event_queue->GetDisplay();
  assert(x_display != nullptr);

  const X11Window x_root = DefaultRootWindow(x_display);
  if (x_root == 0) {
    fprintf(stderr, "DefaultRootWindow() failed\n");
    exit(EXIT_FAILURE);
  }

  XSetWindowAttributes swa;
  swa.event_mask = KeyPressMask | KeyReleaseMask | KeymapStateMask |
    ButtonPressMask | ButtonReleaseMask |
    PointerMotionMask |
    VisibilityChangeMask |
    ExposureMask | StructureNotifyMask;

  x_window = XCreateWindow(x_display, x_root,
                           0, 0, size.cx, size.cy, 0,
                           CopyFromParent, InputOutput,
                           CopyFromParent, CWEventMask,
                           &swa);
  if (x_window == 0) {
    fprintf(stderr, "XCreateWindow() failed\n");
    exit(EXIT_FAILURE);
  }

  XMapWindow(x_display, x_window);
  XStoreName(x_display, x_window, text);

  if (style.GetFullScreen()) {
    /* tell the window manager we want full-screen */
    const Atom atoms[] = {
      XInternAtom(x_display, "_NET_WM_STATE_FULLSCREEN", false),
    };
    XChangeProperty(x_display, x_window,
                    XInternAtom(x_display, "_NET_WM_STATE", false),
                    XA_ATOM, 32, PropModeReplace,
                    (const unsigned char *)atoms, std::size(atoms));
  }

  /* receive "Close" button clicks from the window manager */
  auto wm_delete_window = XInternAtom(x_display, "WM_DELETE_WINDOW", false);
  XSetWMProtocols(x_display, x_window, &wm_delete_window, 1);
}

bool
TopWindow::IsVisible() const
{
  return event_queue->IsVisible();
}

void
TopWindow::EnableCapture()
{
  XGrabPointer(x_display, x_window, true,
               ButtonPressMask |
               ButtonReleaseMask |
               PointerMotionMask,
               GrabModeAsync, GrabModeAsync,
               0, 0, CurrentTime);
}

void
TopWindow::DisableCapture()
{
  XUngrabPointer(x_display, CurrentTime);
}
