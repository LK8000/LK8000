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

#include "Canvas.hpp"
#include "Triangulate.hpp"
#include "Globals.hpp"
#include "Texture.hpp"
#include "Scope.hpp"
#include "VertexArray.hpp"
#include "Shapes.hpp"
#include "FallbackBuffer.hpp"
#include "Features.hpp"
#include "VertexPointer.hpp"
#include "Screen/Custom/Cache.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Util.hpp"
#include "Util/AllocatedArray.hpp"

#ifdef USE_GLSL
#include "Shaders.hpp"
#include "Program.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include "Compatibility.hpp"
#endif

#ifdef UNICODE
#include "Util/ConvertString.hpp"
#endif

#ifndef NDEBUG
#include "Util/UTF8.hpp"
#endif

#include <memory>
#include <assert.h>
#include "utils/stl_utils.h"
#include "utils/array_adaptor.h"
#include "Screen/PolygonRenderer.h"


AllocatedArray<RasterPoint> Canvas::vertex_buffer;
AllocatedArray<FloatPoint> Canvas::vertex_buffer_float;

void
Canvas::DrawFilledRectangle(int left, int top, int right, int bottom,
                            const Color color)
{
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  color.Bind();

  std::unique_ptr<const ScopeAlphaBlend> blend;
  if(!color.IsOpaque()) {
      blend = std::make_unique<const ScopeAlphaBlend>();
  }

#ifdef HAVE_GLES
  const RasterPoint vertices[] = {
    { left, top },
    { right, top },
    { left, bottom },
    { right, bottom },
  };

  const ScopeVertexPointer vp(vertices);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#else
  glRecti(left, top, right, bottom);
#endif
}

void
Canvas::OutlineRectangleGL(int left, int top, int right, int bottom)
{
    const GLexact penWidth = std::max(1U,pen.GetWidth());
    
    if(penWidth <= 1) {
        // use line loop only for line with = 1, otherwise few pixel are missing on each corner.
        const ExactRasterPoint vertices[] = {
            RasterPoint{left, top},
            RasterPoint{right-1, top},
            RasterPoint{right-1, bottom-1},
            RasterPoint{left, bottom-1},
        };

        const ScopeVertexPointer vp(vertices);
        glDrawArrays(GL_LINE_LOOP, 0, std::size(vertices));
    } else {

#if defined(HAVE_GLES) && !defined(HAVE_GLES2)
        glLineWidthx(1 << 16);
#else
        glLineWidth(1);
#endif
        const ExactRasterPoint vertices[] = {
            RasterPoint{left, top},
            RasterPoint{left + penWidth, top + penWidth},
            RasterPoint{right, top},
            RasterPoint{right - penWidth, top + penWidth},
            RasterPoint{right, bottom},
            RasterPoint{right- penWidth, bottom - penWidth},
            RasterPoint{left, bottom},
            RasterPoint{left + penWidth, bottom - penWidth},
            RasterPoint{left, top},
            RasterPoint{left + penWidth, top + penWidth}
        };

        const ScopeVertexPointer vp(vertices);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, std::size(vertices));        
    }
}

void
Canvas::FadeToWhite(GLubyte alpha)
{
  const ScopeAlphaBlend blend;
  const Color color(0xff, 0xff, 0xff, alpha);
  Clear(color);
}

void
Canvas::FadeToWhite(PixelRect rc, GLubyte alpha)
{
  const ScopeAlphaBlend blend;
  const Color color(0xff, 0xff, 0xff, alpha);
  DrawFilledRectangle(rc.left, rc.right, rc.right, rc.bottom, color);
}

void
Canvas::DrawRaisedEdge(PixelRect &rc)
{
  Pen bright(1, Color(240, 240, 240));
  Select(bright);
  DrawTwoLinesExact(rc.left, rc.bottom - 2, rc.left, rc.top,
                    rc.right - 2, rc.top);

  Pen dark(1, Color(128, 128, 128));
  Select(dark);
  DrawTwoLinesExact(rc.left + 1, rc.bottom - 1, rc.right - 1, rc.bottom - 1,
                    rc.right - 1, rc.top + 1);

  ++rc.left;
  ++rc.top;
  --rc.right;
  --rc.bottom;
}

void
Canvas::DrawPolyline(const RasterPoint *points, unsigned num_points)
{
#ifdef USE_GLSL
    glm::mat4 matrix = glm::translate(glm::mat4(1),glm::vec3(1, 1, 0));
    glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(matrix));
#else
    glPushMatrix();

#ifdef HAVE_GLES
    glTranslatex((GLfixed)1 << 16, (GLfixed)1 << 16, 0);
#else
    glTranslatef(1, 1, 0.);
#endif
#endif
    
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  pen.Bind();

  if (pen.GetWidth() <= OpenGL::max_line_width) {
    const ScopeVertexPointer vp(points);
    glDrawArrays(GL_LINE_STRIP, 0, num_points);
  } else {
    const unsigned vertices = LineToTriangles(points, num_points, vertex_buffer,
                                        pen.GetWidth(), false);
    if (vertices > 0) {
      const ScopeVertexPointer vp(vertex_buffer.begin());        
      glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices);
    }
  }

  pen.Unbind();
  
#ifdef USE_GLSL
    glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1)));
#else
    glPopMatrix();
#endif
    
}

void
Canvas::DrawPolyline(const FloatPoint *points, unsigned num_points) {
#ifdef USE_GLSL
  glm::mat4 matrix = glm::translate(glm::mat4(1),glm::vec3(1, 1, 0));
    glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(matrix));
#else
  glPushMatrix();

#ifdef HAVE_GLES
  glTranslatex((GLfixed)1 << 16, (GLfixed)1 << 16, 0);
#else
  glTranslatef(1, 1, 0.);
#endif
#endif

#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  pen.Bind();

  if (pen.GetWidth() <= OpenGL::max_line_width) {
    const ScopeVertexPointer vp(points);
    glDrawArrays(GL_LINE_STRIP, 0, num_points);
  } else {
    const unsigned vertices = LineToTriangles(points, num_points, vertex_buffer_float,
                                              pen.GetWidth(), false);
    if (vertices > 0) {
      const ScopeVertexPointer vp(vertex_buffer_float.begin());
      glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices);
    }
  }

  pen.Unbind();

#ifdef USE_GLSL
  glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1)));
#else
  glPopMatrix();
#endif

}


void
Canvas::DrawPolygon(const RasterPoint *points, unsigned num_points)
{
  if (brush.IsHollow() && !pen.IsDefined())
    return;

#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  if (!brush.IsHollow() && num_points >= 3) {
    brush.Bind();

    std::unique_ptr<const ScopeAlphaBlend> blend;
    if(!brush.IsOpaque()) {
      blend = std::make_unique<const ScopeAlphaBlend>();
    }

    PolygonRenderer renderer(PolygonDrawCallback{});
    renderer.BeginPolygon();
    renderer.BeginContour();

    for(const auto& point : make_array(points, num_points)) {
      renderer.AddVertex(point.x, point.y);
    }

    renderer.EndContour();
    renderer.EndPolygon();
  }

  if (IsPenOverBrush()) {
    pen.Bind();

    if (pen.GetWidth() <= OpenGL::max_line_width) {
      ScopeVertexPointer vp(points);
      glDrawArrays(GL_LINE_LOOP, 0, num_points);
    } else {
      unsigned vertices = LineToTriangles(points, num_points, vertex_buffer,
                                          pen.GetWidth(), true);
      if (vertices > 0) {
        ScopeVertexPointer vp(vertex_buffer.begin());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices);
      }
    }

    pen.Unbind();
  }
}

void
Canvas::DrawTriangleFan(const RasterPoint *points, unsigned num_points)
{
  if (brush.IsHollow() && !pen.IsDefined())
    return;

#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  std::unique_ptr<const ScopeAlphaBlend> blend;
  if(!brush.IsOpaque()) {
    blend = std::make_unique<const ScopeAlphaBlend>();
  }

  ScopeVertexPointer vp(points);

  if (!brush.IsHollow() && num_points >= 3) {
    brush.Bind();

    std::unique_ptr<const ScopeAlphaBlend> blend;
    if(!brush.IsOpaque()) {
      blend = std::make_unique<const ScopeAlphaBlend>();
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, num_points);
  }

  if (IsPenOverBrush()) {
    pen.Bind();

    if (pen.GetWidth() <= OpenGL::max_line_width) {
      glDrawArrays(GL_LINE_LOOP, 0, num_points);
    } else {
      unsigned vertices = LineToTriangles(points, num_points, vertex_buffer,
                                          pen.GetWidth(), true);
      if (vertices > 0) {
        vp.Update(vertex_buffer.begin());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices);
      }
    }

    pen.Unbind();
  }
}

void
Canvas::DrawTriangleFan(const FloatPoint *points, unsigned num_points)
{
  if (!brush.IsHollow() && num_points >= 3) {

#ifdef USE_GLSL
    OpenGL::solid_shader->Use();
#endif

    ScopeVertexPointer vp(points);

    brush.Bind();
    std::unique_ptr<const ScopeAlphaBlend> blend;
    if(!brush.IsOpaque()) {
      blend = std::make_unique<const ScopeAlphaBlend>();
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, num_points);
  }
}

void
Canvas::DrawHLine(int x1, int x2, int y, Color color)
{
  color.Bind();

  const RasterPoint v[] = {
    { GLvalue(x1), GLvalue(y) },
    { GLvalue(x2), GLvalue(y) },
  };

  const ScopeVertexPointer vp(v);
  glDrawArrays(GL_LINE_STRIP, 0, std::size(v));
}

void
Canvas::DrawLine(int ax, int ay, int bx, int by)
{
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  pen.Bind();

  const RasterPoint v[] = {
    { GLvalue(ax), GLvalue(ay) },
    { GLvalue(bx), GLvalue(by) },
  };

  const ScopeVertexPointer vp(v);
  glDrawArrays(GL_LINE_STRIP, 0, std::size(v));

  pen.Unbind();
}

void
Canvas::DrawExactLine(int ax, int ay, int bx, int by)
{
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  pen.Bind();

  const ExactRasterPoint v[] = {
    { ToGLexact(ax), ToGLexact(ay) },
    { ToGLexact(bx), ToGLexact(by) },
  };

  const ScopeVertexPointer vp(v);
  glDrawArrays(GL_LINE_STRIP, 0, std::size(v));

  pen.Unbind();
}

/**
 * Draw a line from a to b, using triangle caps if pen-size > 2 to hide
 * gaps between consecutive lines.
 */
void
Canvas::DrawLinePiece(const RasterPoint a, const RasterPoint b)
{
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  pen.Bind();

  const RasterPoint v[] = { {a.x, a.y}, {b.x, b.y} };
  if (pen.GetWidth() > 2) {
    unsigned strip_len = LineToTriangles(v, 2, vertex_buffer, pen.GetWidth(),
                                         false, true);
    if (strip_len > 0) {
      const ScopeVertexPointer vp(vertex_buffer.begin());
      glDrawArrays(GL_TRIANGLE_STRIP, 0, strip_len);
    }
  } else {
    const ScopeVertexPointer vp(v);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
  }

  pen.Unbind();
}

void
Canvas::DrawTwoLines(int ax, int ay, int bx, int by, int cx, int cy)
{
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  pen.Bind();

  const RasterPoint v[] = {
    { GLvalue(ax), GLvalue(ay) },
    { GLvalue(bx), GLvalue(by) },
    { GLvalue(cx), GLvalue(cy) },
  };

  const ScopeVertexPointer vp(v);
  glDrawArrays(GL_LINE_STRIP, 0, std::size(v));

  pen.Unbind();
}

void
Canvas::DrawTwoLinesExact(int ax, int ay, int bx, int by, int cx, int cy)
{
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  pen.Bind();

  const ExactRasterPoint v[] = {
    { ToGLexact(ax), ToGLexact(ay) },
    { ToGLexact(bx), ToGLexact(by) },
    { ToGLexact(cx), ToGLexact(cy) },
  };

  const ScopeVertexPointer vp(v);
  glDrawArrays(GL_LINE_STRIP, 0, std::size(v));

  pen.Unbind();
}

void
Canvas::DrawCircle(int x, int y, unsigned radius)
{
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  if (IsPenOverBrush() && pen.GetWidth() > 2) {
    ScopeVertexPointer vp;
    GLDonutVertices vertices(x, y,
                             radius - pen.GetWidth() / 2,
                             radius + pen.GetWidth() / 2);
    if (!brush.IsHollow()) {
      vertices.BindInnerCircle(vp);
      brush.Bind();
      glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.CIRCLE_SIZE);
    }
    vertices.Bind(vp);
    pen.Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.SIZE);
    pen.Unbind();
  } else {
    GLFallbackArrayBuffer &buffer = radius < 16
      ? *OpenGL::small_circle_buffer
      : *OpenGL::circle_buffer;
    const unsigned n = radius < 16
      ? OpenGL::SMALL_CIRCLE_SIZE
      : OpenGL::CIRCLE_SIZE;

    const FloatPoint *const points = (const FloatPoint *)buffer.BeginRead();
    const ScopeVertexPointer vp(points);

#ifdef USE_GLSL
    glm::mat4 matrix2 = glm::scale(glm::translate(glm::mat4(1),
                                                  glm::vec3(x, y, 0)),
                                   glm::vec3(GLfloat(radius), GLfloat(radius),
                                             1.));
    glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE,
                       glm::value_ptr(matrix2));
#else
    glPushMatrix();

#ifdef HAVE_GLES
    glTranslatex((GLfixed)x << 16, (GLfixed)y << 16, 0);
    glScalex((GLfixed)radius << 16, (GLfixed)radius << 16, (GLfixed)1 << 16);
#else
    glTranslatef(x, y, 0.);
    glScalef(radius, radius, 1.);
#endif
#endif

    if (!brush.IsHollow()) {
      brush.Bind();
      glDrawArrays(GL_TRIANGLE_FAN, 0, n);
    }

    if (IsPenOverBrush()) {
      pen.Bind();
      glDrawArrays(GL_LINE_LOOP, 0, n);
      pen.Unbind();
    }

#ifdef USE_GLSL
    glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1)));
#else
    glPopMatrix();
#endif

    buffer.EndRead();
  }
}

void
Canvas::DrawSegment(int x, int y, unsigned radius,
                    Angle start, Angle end, bool horizon)
{
  ::Segment(*this, x, y, radius, start, end, horizon);
}

gcc_const
static unsigned
AngleToDonutVertex(Angle angle)
{
  return GLDonutVertices::ImportAngle(NATIVE_TO_INT(angle.Native())
                                      + std::size(ISINETABLE) * 3u / 4u,
                                      std::size(ISINETABLE));
}

gcc_const
static std::pair<unsigned,unsigned>
AngleToDonutVertices(Angle start, Angle end)
{
  static constexpr Angle epsilon = Angle::FullCircle()
    / int(GLDonutVertices::CIRCLE_SIZE * 4u);

  const Angle delta = end - start;

  if (fabs(delta.AsDelta().Native()) <= epsilon.Native())
    /* full circle */
    return std::make_pair(0u, unsigned(GLDonutVertices::MAX_ANGLE));

  const unsigned istart = AngleToDonutVertex(start);
  unsigned iend = AngleToDonutVertex(end);

  if (istart == iend && delta > epsilon) {
    if (end - start >= Angle::HalfCircle())
      /* nearly full circle, round down the end */
      iend = GLDonutVertices::PreviousAngle(iend);
    else
      /* slightly larger than epsilon: draw at least two indices */
      iend = GLDonutVertices::NextAngle(iend);
  }

  return std::make_pair(istart, iend);
}

void
Canvas::DrawAnnulus(int x, int y,
                    unsigned small_radius, unsigned big_radius,
                    Angle start, Angle end)
{
  if (1 == 1) {
    /* TODO: switched to the unoptimised generic implementation due to
       TRAC #2221, caused by rounding error of start/end radial;
       should reimplement GLDonutVertices to use the exact start/end
       radial */
    ::Annulus(*this, x, y, big_radius, start, end, small_radius);
    return;
  }

  ScopeVertexPointer vp;
  GLDonutVertices vertices(x, y, small_radius, big_radius);

  const std::pair<unsigned,unsigned> i = AngleToDonutVertices(start, end);
  const unsigned istart = i.first;
  const unsigned iend = i.second;

  if (!brush.IsHollow()) {
    brush.Bind();
    vertices.Bind(vp);

    if (istart > iend) {
      glDrawArrays(GL_TRIANGLE_STRIP, istart,
                   GLDonutVertices::MAX_ANGLE - istart + 2);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, iend + 2);
    } else {
      glDrawArrays(GL_TRIANGLE_STRIP, istart, iend - istart + 2);
    }
  }

  if (IsPenOverBrush()) {
    pen.Bind();

    if (istart != iend && iend != GLDonutVertices::MAX_ANGLE) {
      if (brush.IsHollow())
        vertices.Bind(vp);

      glDrawArrays(GL_LINE_STRIP, istart, 2);
      glDrawArrays(GL_LINE_STRIP, iend, 2);
    }

    const unsigned pstart = istart / 2;
    const unsigned pend = iend / 2;

    vertices.BindInnerCircle(vp);
    if (pstart < pend) {
      glDrawArrays(GL_LINE_STRIP, pstart, pend - pstart + 1);
    } else {
      glDrawArrays(GL_LINE_STRIP, pstart,
                   GLDonutVertices::CIRCLE_SIZE - pstart + 1);
      glDrawArrays(GL_LINE_STRIP, 0, pend + 1);
    }

    vertices.BindOuterCircle(vp);
    if (pstart < pend) {
      glDrawArrays(GL_LINE_STRIP, pstart, pend - pstart + 1);
    } else {
      glDrawArrays(GL_LINE_STRIP, pstart,
                   GLDonutVertices::CIRCLE_SIZE - pstart + 1);
      glDrawArrays(GL_LINE_STRIP, 0, pend + 1);
    }

    pen.Unbind();
  }
}

void
Canvas::DrawKeyhole(int x, int y,
                    unsigned small_radius, unsigned big_radius,
                    Angle start, Angle end)
{
  ::KeyHole(*this, x, y, big_radius, start, end, small_radius);
}

void
Canvas::DrawFocusRectangle(PixelRect rc)
{
  DrawOutlineRectangle(rc.left, rc.top, rc.right, rc.bottom, COLOR_DARK_GRAY);
}

const PixelSize
Canvas::CalcTextSize(const TCHAR *text) const
{
  assert(text != nullptr);
#ifdef UNICODE
  const WideToUTF8Converter text2(text);
#else
  const char* text2 = text;
  assert(ValidateUTF8(text));
#endif

  PixelSize size = { 0, 0 };
  assert(font);
  if (font == nullptr)
    return size;

  /* see if the TextCache can handle this request */
  size = TextCache::LookupSize(*font, text2);
  if (size.cy > 0)
    return size;

  return TextCache::GetSize(*font, text2);
}

/**
 * Prepare drawing a GL_ALPHA texture with the specified color.
 */
static void
PrepareColoredAlphaTexture(Color color)
{
#ifdef USE_GLSL
  OpenGL::alpha_shader->Use();
  color.Bind();
#else
  color.Bind();

  if (color == COLOR_BLACK) {
    /* GL_ALPHA textures have black RGB - this is easy */

    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  } else {
    /* use GL_COMBINE to replace the texture color (black) with the
       specified one */
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

    /* replace the texture color with the selected text color */
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

    /* use the texture alpha */
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
  }
#endif
}

void
Canvas::DrawText(int x, int y, const TCHAR *text)
{
  assert(text != nullptr);
#ifdef UNICODE
  const WideToUTF8Converter text2(text);
#else
  const char* text2 = text;
  assert(ValidateUTF8(text));
#endif

#ifdef HAVE_GLES
  assert(offset == OpenGL::translate);
#endif

  if (font == nullptr)
    return;

  GLTexture *texture = TextCache::Get(*font, text2);
  if (texture == nullptr)
    return;

  if (background_mode == OPAQUE)
    DrawFilledRectangle(x, y,
                        x + texture->GetWidth(), y + texture->GetHeight(),
                        background_color);

  PrepareColoredAlphaTexture(text_color);

#ifndef USE_GLSL
  const GLEnable<GL_TEXTURE_2D> scope;
#endif

  const ScopeAlphaBlend blend;

  texture->Bind();
  texture->Draw(x, y);
}

void
Canvas::DrawTransparentText(int x, int y, const TCHAR *text)
{
  assert(text != nullptr);
#ifdef UNICODE
  const WideToUTF8Converter text2(text);
#else
  const char* text2 = text;
  assert(ValidateUTF8(text));
#endif

#ifdef HAVE_GLES
  assert(offset == OpenGL::translate);
#endif

  if (font == nullptr)
    return;

  GLTexture *texture = TextCache::Get(*font, text2);
  if (texture == nullptr)
    return;

  PrepareColoredAlphaTexture(text_color);

#ifndef USE_GLSL
  const GLEnable<GL_TEXTURE_2D> scope;
#endif

  const ScopeAlphaBlend blend;

  texture->Bind();
  texture->Draw(x, y);
}

void
Canvas::DrawClippedText(int x, int y,
                        unsigned width, unsigned height,
                        const TCHAR *text)
{
  assert(text != nullptr);
#ifdef UNICODE
  const WideToUTF8Converter text2(text);
#else
  const char* text2 = text;
  assert(ValidateUTF8(text));
#endif

#ifdef HAVE_GLES
  assert(offset == OpenGL::translate);
#endif

  if (font == nullptr)
    return;

  GLTexture *texture = TextCache::Get(*font, text2);
  if (texture == nullptr)
    return;

  if (texture->GetHeight() < height)
    height = texture->GetHeight();
  if (texture->GetWidth() < width)
    width = texture->GetWidth();

  PrepareColoredAlphaTexture(text_color);

#ifndef USE_GLSL
  const GLEnable<GL_TEXTURE_2D> scope;
#endif

  const ScopeAlphaBlend blend;

  texture->Bind();
  texture->Draw(x, y, width, height, 0, 0, width, height);
}

void
Canvas::Stretch(int dest_x, int dest_y,
                unsigned dest_width, unsigned dest_height,
                const GLTexture &texture,
                int src_x, int src_y,
                unsigned src_width, unsigned src_height)
{
#ifdef HAVE_GLES
  assert(offset == OpenGL::translate);
#endif

#ifdef USE_GLSL
  OpenGL::texture_shader->Use();
#endif

  texture.Draw(dest_x, dest_y, dest_width, dest_height,
               src_x, src_y, src_width, src_height);
}

void
Canvas::Stretch(int dest_x, int dest_y,
                unsigned dest_width, unsigned dest_height,
                const GLTexture &texture)
{
  Stretch(dest_x, dest_y, dest_width, dest_height,
          texture, 0, 0, texture.GetWidth(), texture.GetHeight());
}

void
Canvas::Copy(int dest_x, int dest_y,
             unsigned dest_width, unsigned dest_height,
             const Bitmap &src, int src_x, int src_y)
{
  Stretch(dest_x, dest_y, dest_width, dest_height,
          src, src_x, src_y, dest_width, dest_height);
}

void
Canvas::Copy(const Bitmap &src)
{
  Copy(0, 0, src.GetWidth(), src.GetHeight(), src, 0, 0);
}

void
Canvas::StretchNot(const Bitmap &src)
{
  assert(src.IsDefined());

#ifdef USE_GLSL
  OpenGL::invert_shader->Use();
#else
  const GLEnable<GL_TEXTURE_2D> scope;

  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

  /* invert the texture color */
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);

  /* copy the texture alpha */
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
#endif

  GLTexture &texture = *src.GetNative();
  texture.Bind();
  texture.Draw(0, 0, GetWidth(), GetHeight(),
               0, 0, src.GetWidth(), src.GetHeight());
}

void
Canvas::Stretch(int dest_x, int dest_y,
                unsigned dest_width, unsigned dest_height,
                const Bitmap &src, int src_x, int src_y,
                unsigned src_width, unsigned src_height)
{
#ifdef HAVE_GLES
  assert(offset == OpenGL::translate);
#endif
  assert(src.IsDefined());

#ifdef USE_GLSL
  OpenGL::texture_shader->Use();
#else
  const GLEnable<GL_TEXTURE_2D> scope;
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#endif

  GLTexture &texture = *src.GetNative();
  texture.Bind();
  texture.Draw(dest_x, dest_y, dest_width, dest_height,
               src_x, src_y, src_width, src_height);
}

void
Canvas::Stretch(int dest_x, int dest_y,
                unsigned dest_width, unsigned dest_height,
                const Bitmap &src)
{
#ifdef HAVE_GLES
  assert(offset == OpenGL::translate);
#endif
  assert(src.IsDefined());

#ifdef USE_GLSL
  OpenGL::texture_shader->Use();
#else
  const GLEnable<GL_TEXTURE_2D> scope;
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#endif

  GLTexture &texture = *src.GetNative();
  texture.Bind();
  texture.Draw(dest_x, dest_y, dest_width, dest_height,
               0, 0, src.GetWidth(), src.GetHeight());
}

void
Canvas::StretchMono(int dest_x, int dest_y,
                    unsigned dest_width, unsigned dest_height,
                    const Bitmap &src,
                    int src_x, int src_y,
                    unsigned src_width, unsigned src_height,
                    Color fg_color, Color bg_color)
{
  /* note that this implementation ignores the background color; it is
     not mandatory, and we can assume that the background is already
     set; it is only being passed to this function because the GDI
     implementation will be faster when erasing the background
     again */

#ifdef USE_GLSL
  OpenGL::alpha_shader->Use();
  fg_color.Bind();
#else
  const GLEnable<GL_TEXTURE_2D> scope;

  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

  /* replace the texture color with the selected text color */
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

  /* invert texture alpha (our bitmaps have black text on white
     background) */
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

  const ScopeAlphaBlend blend;

  GLTexture &texture = *src.GetNative();
  texture.Bind();
  texture.Draw(dest_x, dest_y, dest_width, dest_height,
               src_x, src_y, src_width, src_height);
}

void
Canvas::CopyToTexture(GLTexture &texture, PixelRect src_rc) const
{
#ifdef HAVE_GLES
  assert(offset == OpenGL::translate);
#endif

  texture.Bind();
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                      OpenGL::translate.x + src_rc.left,
                      OpenGL::viewport_size.y - OpenGL::translate.y - src_rc.bottom,
                      src_rc.right - src_rc.left,
                      src_rc.bottom - src_rc.top);

}

void
Canvas::DrawRoundRectangle(int left, int top, int right, int bottom,
                           unsigned ellipse_width,
                           unsigned ellipse_height)
{
  unsigned radius = std::min(std::min(ellipse_width, ellipse_height),
                             unsigned(std::min(bottom - top,
                                               right - left))) / 2u;
  ::RoundRect(*this, left, top, right, bottom, radius);
}
