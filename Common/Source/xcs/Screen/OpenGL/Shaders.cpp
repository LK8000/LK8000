/*
Copyright_License {

  XCSoar Glide Compute5r - http://www.xcsoar.org/
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

#include "Shaders.hpp"
#include "Program.hpp"
#include "Globals.hpp"
#include "Screen/OpenGL/Point.hpp"

#include <stdio.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace OpenGL {
  program_unique_ptr solid_shader;
  GLint solid_projection, solid_modelview, solid_translate;

  program_unique_ptr texture_shader;
  GLint texture_projection, texture_texture, texture_translate;

  program_unique_ptr invert_shader;
  GLint invert_projection, invert_texture, invert_translate;

  program_unique_ptr alpha_shader;
  GLint alpha_projection, alpha_texture, alpha_translate;
}

#ifdef HAVE_GLES
#define GLSL_VERSION "#version 100\n"
#define GLSL_PRECISION "precision mediump float;\n"
#else 
#define GLSL_VERSION "#version 120\n"
#define GLSL_PRECISION
#endif

namespace {

constexpr char solid_vertex_shader[] =
  GLSL_VERSION
  R"glsl(
    uniform mat4 projection;
    uniform mat4 modelview;
    uniform vec2 translate;
    attribute vec4 position;
    attribute vec4 color;
    varying vec4 colorvar;
    void main() {
      gl_Position = modelview * position;
      gl_Position.xy += translate;
      gl_Position = projection * gl_Position;
      colorvar = color;
    }
  )glsl";

constexpr char solid_fragment_shader[] =
  GLSL_VERSION
  GLSL_PRECISION
  R"glsl(
    varying vec4 colorvar;
    void main() {
      gl_FragColor = colorvar;
    }
  )glsl";

constexpr char texture_vertex_shader[] =
  GLSL_VERSION
  R"glsl(
    uniform mat4 projection;
    uniform vec2 translate;
    attribute vec4 position;
    attribute vec2 texcoord;
    varying vec2 texcoordvar;
    attribute vec4 color;
    varying vec4 colorvar;
    void main() {
      gl_Position = position;
      gl_Position.xy += translate;
      gl_Position = projection * gl_Position;
      texcoordvar = texcoord;
      colorvar = color;
    }
  )glsl";

constexpr char texture_fragment_shader[] =
  GLSL_VERSION
  GLSL_PRECISION
  R"glsl(
    uniform sampler2D texture;
    varying vec2 texcoordvar;
    void main() {
      gl_FragColor = texture2D(texture, texcoordvar);
    }
  )glsl";

const char *const invert_vertex_shader = texture_vertex_shader;
constexpr char invert_fragment_shader[] =
  GLSL_VERSION
  GLSL_PRECISION
  R"glsl(
    uniform sampler2D texture;
    varying vec2 texcoordvar;
    void main() {
      vec4 color = texture2D(texture, texcoordvar);
      gl_FragColor = vec4(vec3(1) - color.rgb, color.a);
    }
  )glsl";

const char *const alpha_vertex_shader = texture_vertex_shader;
constexpr char alpha_fragment_shader[] =
  GLSL_VERSION
  GLSL_PRECISION
  R"glsl(
    uniform sampler2D texture;
    varying vec4 colorvar;
    varying vec2 texcoordvar;
    void main() {
      gl_FragColor = vec4(colorvar.rgb, texture2D(texture, texcoordvar).a);
    }
  )glsl";

void CompileAttachShader(GLProgram& program, GLenum type, const char* code)
{
  GLShader shader(type);
  shader.Source(code);
  shader.Compile();

  if (shader.GetCompileStatus() != GL_TRUE) {
    char log[4096];
    shader.GetInfoLog(log, sizeof(log));
    fprintf(stderr, "Shader compiler failed: %s\n", log);
  }

  program.AttachShader(shader);
}

inline
OpenGL::program_unique_ptr make_program_unique_ptr() {
  return std::make_unique<GLProgram>();
}

OpenGL::program_unique_ptr CompileProgram(const char* vertex_shader, const char* fragment_shader) {
  auto program = make_program_unique_ptr();
  CompileAttachShader(*program, GL_VERTEX_SHADER, vertex_shader);
  CompileAttachShader(*program, GL_FRAGMENT_SHADER, fragment_shader);
  return program;
}

void LinkProgram(GLProgram& program) {
  program.Link();

  if (program.GetLinkStatus() != GL_TRUE) {
    char log[4096];
    program.GetInfoLog(log, sizeof(log));
    fprintf(stderr, "Shader linker failed: %s\n", log);
  }
}

}  // namespace

void
OpenGL::InitShaders()
{
  DeinitShaders();

  solid_shader = CompileProgram(solid_vertex_shader, solid_fragment_shader);
  solid_shader->BindAttribLocation(Attribute::POSITION, "position");
  solid_shader->BindAttribLocation(Attribute::COLOR, "color");
  LinkProgram(*solid_shader);

  solid_projection = solid_shader->GetUniformLocation("projection");
  solid_modelview = solid_shader->GetUniformLocation("modelview");
  solid_translate = solid_shader->GetUniformLocation("translate");

  solid_shader->Use();
  glUniformMatrix4fv(solid_modelview, 1, GL_FALSE,
                     glm::value_ptr(glm::mat4(1)));

  texture_shader = CompileProgram(texture_vertex_shader, texture_fragment_shader);
  texture_shader->BindAttribLocation(Attribute::POSITION, "position");
  texture_shader->BindAttribLocation(Attribute::TEXCOORD, "texcoord");
  LinkProgram(*texture_shader);

  texture_projection = texture_shader->GetUniformLocation("projection");
  texture_texture = texture_shader->GetUniformLocation("texture");
  texture_translate = texture_shader->GetUniformLocation("translate");

  texture_shader->Use();
  glUniform1i(texture_texture, 0);

  invert_shader = CompileProgram(invert_vertex_shader, invert_fragment_shader);
  invert_shader->BindAttribLocation(Attribute::POSITION, "position");
  invert_shader->BindAttribLocation(Attribute::TEXCOORD, "texcoord");
  LinkProgram(*invert_shader);

  invert_projection = invert_shader->GetUniformLocation("projection");
  invert_texture = invert_shader->GetUniformLocation("texture");
  invert_translate = invert_shader->GetUniformLocation("translate");

  invert_shader->Use();
  glUniform1i(invert_texture, 0);

  alpha_shader = CompileProgram(alpha_vertex_shader, alpha_fragment_shader);
  alpha_shader->BindAttribLocation(Attribute::POSITION, "position");
  alpha_shader->BindAttribLocation(Attribute::TEXCOORD, "texcoord");
  alpha_shader->BindAttribLocation(Attribute::COLOR, "color");
  LinkProgram(*alpha_shader);

  alpha_projection = alpha_shader->GetUniformLocation("projection");
  alpha_texture = alpha_shader->GetUniformLocation("texture");
  alpha_translate = alpha_shader->GetUniformLocation("translate");

  alpha_shader->Use();
  glUniform1i(alpha_texture, 0);
}

void
OpenGL::DeinitShaders() noexcept
{
  solid_shader = nullptr;
  texture_shader =nullptr;
  invert_shader = nullptr;
  alpha_shader = nullptr;  
}

void
OpenGL::UpdateShaderProjectionMatrix() noexcept
{
  alpha_shader->Use();
  glUniformMatrix4fv(alpha_projection, 1, GL_FALSE,
                     glm::value_ptr(projection_matrix));

  invert_shader->Use();
  glUniformMatrix4fv(invert_projection, 1, GL_FALSE,
                     glm::value_ptr(projection_matrix));

  texture_shader->Use();
  glUniformMatrix4fv(texture_projection, 1, GL_FALSE,
                     glm::value_ptr(projection_matrix));

  solid_shader->Use();
  glUniformMatrix4fv(solid_projection, 1, GL_FALSE,
                     glm::value_ptr(projection_matrix));
}

void
OpenGL::UpdateShaderTranslate() noexcept
{
  const FloatPoint t(translate.x, translate.y);

  solid_shader->Use();
  glUniform2f(solid_translate, t.x, t.y);

  texture_shader->Use();
  glUniform2f(texture_translate, t.x, t.y);

  invert_shader->Use();
  glUniform2f(invert_translate, t.x, t.y);

  alpha_shader->Use();
  glUniform2f(alpha_translate, t.x, t.y);
}
