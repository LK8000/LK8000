//
// Created by BLC on 14/10/2022.
//

#include "PowerManagerUtils.h"
#include "Android/Context.hpp"
#include "Android/Main.hpp"
#include "Java/Class.hxx"

namespace {

Java::Class GetClass(JNIEnv * env) {
    Java::Class cls = { env, "org/LK8000/Air3/PowerManagerUtils"};
    Java::RethrowException(env); // rethrow ClassNotFoundException
    return cls;
}

} // namespace

bool PowerManagerUtils::openModuleFanet() noexcept {
    JNIEnv * env = Java::GetEnv();
    try {
        Java::Class cls = GetClass(env);

        jmethodID method = env->GetStaticMethodID(cls.Get(),
                                                  "openModuleFanet",
                                                  "(Landroid/content/Context;)Z");
        Java::RethrowException(env); // rethrow NoSuchMethodError
        if (!method) {
            return false;
        }
        env->CallStaticBooleanMethod(cls, method, context->Get());
    } catch(std::exception& e) {
        return false;
    }
    return true;
}

void PowerManagerUtils::closeModuleFanet() noexcept {
    JNIEnv * env = Java::GetEnv();
    try {
        Java::Class cls = GetClass(env);
        jmethodID method = env->GetStaticMethodID(cls.Get(),
                                                  "closeModuleFanet",
                                                  "(Landroid/content/Context;)V");
        Java::RethrowException(env); // rethrow NoSuchMethodError
        if (method) {
            env->CallStaticVoidMethod(cls, method, context->Get());
        }
    }
    catch (...) {}
}
