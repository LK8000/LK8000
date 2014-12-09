/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Window.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 december 2014, 21:34
 */

#include "WndMainBase.h"
#include "Event/Shared/Event.hpp"
#include "Event/Console/Loop.hpp"
#include "Event/Queue.hpp"
#include "Event/Globals.hpp"

#ifdef DRAW_MOUSE_CURSOR
#include "Util/Macros.hpp"
#include "Screen/Layout.hpp"
#endif

#ifdef KOBO
#include "Screen/Memory/Canvas.hpp"
#endif


void WndMainBase::FullScreen() {
    
}

void WndMainBase::Refresh() {
    
}

bool WndMainBase::OnEvent(const Event &event) {
    switch (event.type) {
        Window *w;

        case Event::NOP:
        case Event::QUIT:
        case Event::TIMER:
        case Event::USER:
        case Event::CALLBACK:
            break;

        case Event::CLOSE:
            OnClose();
            break;

        case Event::KEY_DOWN:
            w = GetFocus();
            if (w == nullptr)
                w = this;

            return w->OnKeyDown(event.param);

        case Event::KEY_UP:
            w = GetFocus();
            if (w == nullptr)
                w = this;

            return w->OnKeyUp(event.param);

        case Event::MOUSE_MOTION:
#ifdef DRAW_MOUSE_CURSOR
            /* redraw to update the mouse cursor position */
            Invalidate();
#endif

            // XXX keys
            return OnMouseMove((POINT){event.point.x, event.point.y});

        case Event::MOUSE_DOWN:
            return double_click.Check(event.point)
                    ? OnLButtonDblClick((POINT){event.point.x, event.point.y})
                    : OnLButtonDown((POINT){event.point.x, event.point.y});

        case Event::MOUSE_UP:
            double_click.Moved(event.point);

            return OnLButtonUp((POINT){event.point.x, event.point.y});

        case Event::MOUSE_WHEEL:
/*            
            return OnMouseWheel((POINT){event.point.x, event.point.y}, (int) event.param);
 */
            return false;
    }

    return false;
}

bool WndMainBase::FilterEvent(const Event &event, Window *allowed) const {
  assert(allowed != nullptr);

  switch (event.type) {
  case Event::MOUSE_MOTION:
  case Event::MOUSE_DOWN:
  case Event::MOUSE_UP:
    return FilterMouseEvent(event.point, allowed);

  default:
    return true;
  }
}

gcc_pure
static const Window* IsAncestor(const Window *maybe_ancestor, const Window *w)
{
  while (true) {
    const Window *parent = w->GetOwner();
    if (parent == nullptr)
      return nullptr;

    if (parent == maybe_ancestor)
      return parent;

    w = parent;
  }
}

bool WndMainBase::FilterMouseEvent(RasterPoint pt, Window *allowed) const {
    const Window *container = this;
    while (true) {
        const Window *child = container->EventChildAt(pt.x, pt.y);
        if (child == nullptr)
            /* no receiver for the event */
            return false;

        if (child == allowed)
            /* the event reaches an allowed window: success */
            return true;

        const Window *next = IsAncestor(allowed, child);
        if (next == nullptr)
            return false;

        container = next;
    }
}



