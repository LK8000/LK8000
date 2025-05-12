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

#include "Screen/SubCanvas.hpp"
#include "Globals.hpp"
#include <algorithm>

#ifdef USE_GLSL
#include "Shaders.hpp"
#endif

SubCanvas::SubCanvas(Canvas &canvas, RasterPoint _offset, PixelSize _size)
  : relative(_offset)
{
  pen = canvas.pen;
  brush = canvas.brush;
  font = canvas.font;
  text_color = canvas.text_color;
  background_color = canvas.background_color;
  background_mode = canvas.background_mode;
  
  assert(canvas.offset == OpenGL::translate);
  offset = canvas.offset + _offset;
  
  /* sub canvas bottom right limits can't be outside "parent" canvas. */
  size.cx = std::min<PixelScalar>(_size.cx, canvas.GetSize().cx - _offset.x) ;
  size.cy = std::min<PixelScalar>(_size.cy, canvas.GetSize().cy - _offset.y) ;

  if (relative.x != 0 || relative.y != 0) {
    OpenGL::translate += _offset;

#ifdef USE_GLSL
    OpenGL::UpdateShaderTranslate();
#else
    glPushMatrix();
#ifdef HAVE_GLES
    glTranslatex((GLfixed)relative.x << 16, (GLfixed)relative.y << 16, 0);
#else
    glTranslatef(relative.x, relative.y, 0);
#endif
#endif /* !USE_GLSL */
  }
  
  if ((relative.x + size.cx < canvas.GetSize().cx) 
          || (relative.y + size.cy < canvas.GetSize().cy)) {

    /* Enable Clipping */
    push_scissor = std::make_unique<GLPushScissor>();
    scissor = std::make_unique<GLCanvasScissor>(*this);
  }
}

SubCanvas::~SubCanvas()
{
  assert(offset == OpenGL::translate);

  if (relative.x != 0 || relative.y != 0) {
    OpenGL::translate -= relative;

#ifdef USE_GLSL
    OpenGL::UpdateShaderTranslate();
#else
    glPopMatrix();
#endif
  }
  
  if(scissor) {
    scissor = nullptr;
    push_scissor = nullptr;
  }
}
