// Copyright (c) 2024, Bruno de Lacheisserie
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

#include "UUID.h"
#include "Java/Class.hxx"
#include "Java/Env.hxx"

namespace {

Java::Class uuid_cls(JNIEnv* env) {
  return Java::GetClassRethrow(env, "java/util/UUID");
}

jmethodID GetMethodID(JNIEnv* env, const char* name, const char* sig) {
  return env->GetMethodID(uuid_cls(env), name, sig);
}

uint64_t getLeastSignificantBits(JNIEnv* env, jobject object) {
  static jmethodID method = GetMethodID(env, "getLeastSignificantBits", "()J");
  return env->CallLongMethod(object, method);
}

uint64_t getMostSignificantBits(JNIEnv* env, jobject object) {
  static jmethodID method = GetMethodID(env, "getMostSignificantBits", "()J");
  return env->CallLongMethod(object, method);
}

} // namespace

uuid_t Java::UUID::to_uuid_t(JNIEnv* env, jobject object) {
  return { getMostSignificantBits(env, object), getLeastSignificantBits(env, object) };
}

Java::LocalObject Java::UUID::from_uuid_t(JNIEnv* env, uuid_t uuid) {
  static jmethodID ctor = GetMethodID(env, "<init>", "(JJ)V");
  return NewObjectRethrow(env, uuid_cls(env), ctor, uuid.msb(), uuid.lsb());
}
