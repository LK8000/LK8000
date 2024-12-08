// Copyright (c) 2021, Bruno de Lacheisserie
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of  nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef _MATH_POINT3D_H_
#define _MATH_POINT3D_H_

#include <cmath>

class Point3D {
 public:
  constexpr Point3D() = default;

  constexpr Point3D(const Point3D&) = default;
  constexpr Point3D& operator=(const Point3D&) = default;

  constexpr Point3D(Point3D&&) = default;
  constexpr Point3D& operator=(Point3D&&) = default;

  constexpr Point3D operator+(const Point3D& other) const {
    return { x + other.x, y + other.y, z + other.z };
  }

  constexpr Point3D operator*(const Point3D& other) const {
    return { x * other.x, y * other.y, z * other.z };
  }

  constexpr Point3D operator/(double v) const {
    return { x / v, y / v, z / v };
  }

  constexpr double length() const {
    Point3D tmp = (*this) * (*this);
    return std::sqrt(tmp.x + tmp.y + tmp.z);
  }

  double x = {};
  double y = {};
  double z = {};
};

#endif // _MATH_POINT3D_H_
