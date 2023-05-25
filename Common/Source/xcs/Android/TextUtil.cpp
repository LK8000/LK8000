/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
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

#include "TextUtil.hpp"
#include "Java/Class.hxx"
#include "Java/String.hxx"
#include "Java/Exception.hxx"
#include "Screen/Point.hpp"
#include "Asset.hpp"
#include "Android/Context.hpp"
#include "Android/Main.hpp"
#include <array>

JNIEnv *TextUtil::env;
static Java::TrivialClass cls;
jmethodID TextUtil::midTextUtil;
jmethodID TextUtil::midGetFontMetrics;
jmethodID TextUtil::midGetTextBounds;
jmethodID TextUtil::midGetTextTextureGL;

void
TextUtil::Initialise(JNIEnv *_env)
{
  env = _env;

  cls.Find(env, "org/LK8000/TextUtil");

  midTextUtil = env->GetMethodID(cls, "<init>","(Landroid/content/Context;Ljava/lang/String;IIIZ)V");
  midGetFontMetrics = env->GetMethodID(cls, "getFontMetrics", "([I)V");
  midGetTextBounds = env->GetMethodID(cls, "getTextBounds",
                                      "(Ljava/lang/String;)[I");
  midGetTextTextureGL = env->GetMethodID(cls, "getTextTextureGL",
                                         "(Ljava/lang/String;)[I");
}

void
TextUtil::Deinitialise(JNIEnv *env)
{
  cls.Clear(env);
}

TextUtil::TextUtil(jobject _obj)
  :Java::GlobalObject(env, _obj) {
  // get height, ascent_height and capital_height
  assert(midGetFontMetrics);
  Java::LocalRef<jintArray> metricsArray = { env, env->NewIntArray(5) };
  env->CallVoidMethod(Get(), midGetFontMetrics, metricsArray.Get());

  std::array<jint, 5> metrics;
  env->GetIntArrayRegion(metricsArray, 0, metrics.size(), metrics.data());

  height = metrics[0];
  style = metrics[1];
  ascent_height = metrics[2];
  capital_height = metrics[3];
  line_spacing = metrics[4];
}

TextUtil *
TextUtil::create(const char *facename, int height, bool bold, bool italic)
{
  jint paramStyle, paramTextSize;

  Java::String paramFamilyName(env, facename);
  paramStyle = 0;
  if (bold)
    paramStyle |= 1;
  if (italic)
    paramStyle |= 2;
  paramTextSize = height;

  int paint_flags = 0;
  if (!IsDithered())
    /* 1 = Paint.ANTI_ALIAS_FLAG */
    paint_flags |= 1;

  // construct org.xcsoar.TextUtil object
  Java::LocalObject localObject = {
          env, env->NewObject(cls, midTextUtil, context->Get(), paramFamilyName.Get(),
                              paramStyle, paramTextSize,
                              paint_flags, false)
  };

  if (!localObject)
    return nullptr;

  return new TextUtil(localObject);
}

PixelSize
TextUtil::getTextBounds(const char *text) const
{

  Java::String text2(env, text);
  Java::LocalRef<jintArray> paramExtent = {
          env, (jintArray)env->CallObjectMethod(Get(), midGetTextBounds, text2.Get())
  };

  std::array<jint, 2> extent;
  if (!Java::DiscardException(env)) {
    env->GetIntArrayRegion(paramExtent, 0, extent.size(), extent.data());
  } else {
    /* Java exception has occurred; return zeroes */
    extent[0] = 0;
    extent[1] = 0;
  }

  return { extent[0], extent[1] };
}

TextUtil::Texture
TextUtil::getTextTextureGL(const char *text) const
{
  Java::String text2(env, text);

  Java::LocalRef<jintArray> jresult = {
          env, (jintArray)env->CallObjectMethod(Get(), midGetTextTextureGL, text2.Get())
  };
  std::array<jint, 5> result;
  if (!Java::DiscardException(env) && jresult != nullptr) {
    env->GetIntArrayRegion(jresult, 0, result.size(), result.data());
  } else {
    result[0] = result[1] = result[2] = result[3] = result[4] = 0;
  }

  return Texture(result[0], result[1], result[2], result[3], result[4]);
}
