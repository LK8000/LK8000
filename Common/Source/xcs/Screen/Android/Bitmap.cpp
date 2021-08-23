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

#include "Screen/Bitmap.hpp"
#include "Screen/OpenGL/Texture.hpp"
#include "Android/Bitmap.hpp"
#include "Android/NativeView.hpp"
#include "Android/Main.hpp"
#include "Android/android_drawable.h"
#include "ResourceId.hpp"

Bitmap::Bitmap(ResourceId id)
{
  Load(id);
}

static const char *
find_resource_name(unsigned id)
{
  for (unsigned i = 0; DrawableNames[i].name != nullptr; ++i)
    if (DrawableNames[i].id == id)
      return DrawableNames[i].name;

  return nullptr;
}

static Java::LocalObject
LoadResourceBitmap(ResourceId id)
{
  const char *name = find_resource_name((unsigned)id);
  if (name == nullptr)
    return nullptr;

  return native_view->loadResourceBitmap(name);
}

bool
Bitmap::Set(const Java::LocalObject &bmp, Type type)
{
  assert(bmp);

  JNIEnv *env = bmp.GetEnv();

  size.cx = AndroidBitmap::GetWidth(env, bmp);
  size.cy = AndroidBitmap::GetHeight(env, bmp);

  if (!MakeTexture(bmp, type)) {
    Reset();
    return false;
  }

  return true;
}

bool
Bitmap::MakeTexture(const Java::LocalObject &bmp, Type type)
{
  assert(bmp != nullptr);

  jint result[5];
  if (!native_view->bitmapToTexture(bmp, type == Bitmap::Type::MONO, result))
    return false;

  texture = new GLTexture(result[0], result[1], result[2], result[3], result[4]);
  if (interpolation) {
    texture->Bind();
    texture->EnableInterpolation();
  }

  return true;
}

bool
Bitmap::Load(ResourceId id, Type _type)
{
  assert(id.IsDefined());

  Reset();

  Java::LocalObject new_bmp = LoadResourceBitmap(id);
  if (new_bmp == nullptr)
    return false;

  return Set(new_bmp, _type);
}

bool
Bitmap::LoadStretch(ResourceId id, unsigned zoom)
{
  assert(zoom > 0);

  // XXX
  return Load(id);
}

bool
Bitmap::LoadAssetsFile(const TCHAR *name) {
  assert(name != nullptr && *name != _T('\0'));

  Reset();

  auto new_bmp = native_view->loadAssetsBitmap(name);
  if (new_bmp == nullptr)
    return false;

  return Set(new_bmp, Type::STANDARD);
}

bool
Bitmap::LoadFile(const TCHAR *path)
{
  assert(path != nullptr && *path != _T('\0'));

  Reset();

  auto new_bmp = native_view->loadFileBitmap(path);
  if (new_bmp == nullptr)
    return false;

  return Set(new_bmp, Type::STANDARD);
}
