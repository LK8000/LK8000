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

#ifndef XCSOAR_SCREEN_OPENGL_SCOPE_HPP
#define XCSOAR_SCREEN_OPENGL_SCOPE_HPP

#include "Features.hpp"
#include "System.hpp"
#include "Globals.hpp"
#include <assert.h>
/**
 * Enables and auto-disables an OpenGL capability.
 */
template<GLenum cap>
class GLEnable {
public:
  GLEnable() {
    ::glEnable(cap);
  }

  ~GLEnable() {
    ::glDisable(cap);
  }
  
  GLEnable(const GLEnable &) = delete;
  GLEnable &operator=(const GLEnable &) = delete;  
};

class GLBlend : public GLEnable<GL_BLEND> {
public:
  GLBlend(GLenum sfactor, GLenum dfactor) {
    ::glBlendFunc(sfactor, dfactor);
  }

#ifndef HAVE_GLES
  GLBlend(GLclampf alpha) {
    ::glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    ::glBlendColor(0, 0, 0, alpha);
  }
#endif
};

class GLScissor : public GLEnable<GL_SCISSOR_TEST> {
public:
  GLScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    assert(width > 0);
    assert(height > 0);
    ::glScissor(x, y, width, height);
#if HAVE_GLES
    OpenGL::scissor_test = true;
#endif
    
  }
  ~GLScissor() {
#if HAVE_GLES      
    OpenGL::scissor_test = false;
#endif
  }
};

#ifndef HAVE_GLES

/**
 * Save and auto-restore an OpenGL attributes state
 * see #glPushAttrib() documentation for list of attributes
 */
template<GLbitfield mask>
class GLPushAttrib {
public:
    GLPushAttrib() {
        GLint depth;  
        ::glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
        assert(depth < OpenGL::max_attrib_stack_depth); // Error GL_ATTRIB_STACK is full !!

        if(depth < OpenGL::max_attrib_stack_depth) {
            ::glPushAttrib(mask); 
            stack = true;
        } else {
            stack = false;
        }
    }

    ~GLPushAttrib() {
        if(stack) {
#ifndef NDEBUG
            GLint depth; 
            ::glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
            assert(depth < OpenGL::max_attrib_stack_depth); // Error GL_ATTRIB_STACK is empty !!
#endif
            ::glPopAttrib();
        }
    }
private:
    bool stack;
};

/**
 * Save and auto-restore an OpenGL scissor state 
 */
typedef GLPushAttrib<GL_SCISSOR_BIT> GLPushScissor;
#else

class GLPushScissor {
public:
    GLPushScissor() {
        ::glGetIntegerv(GL_SCISSOR_BOX, scissor_box);
        enabled = OpenGL::scissor_test;
    }
    ~GLPushScissor() {
        if(enabled) {
            ::glEnable(GL_SCISSOR_TEST);
        } else {
            ::glDisable(GL_SCISSOR_TEST);
        }
        ::glScissor(scissor_box[0],scissor_box[1],scissor_box[2],scissor_box[3]);

        OpenGL::scissor_test = enabled;
    }
private:
    GLint scissor_box[4];
    GLboolean enabled;
};
#endif

#endif
