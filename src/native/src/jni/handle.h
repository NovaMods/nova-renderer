//
// Created by David on 01-Apr-16.
//

#ifndef RENDERER_HANDLE_H_H
#define RENDERER_HANDLE_H_H

#include <jni.h>

/*!
 * \brief Allows us to get and set the nativeHandle field that hangs out in each of the JNI classes we're making.
 *
 * From http://thebreakfastpost.com/2012/01/26/wrapping-a-c-library-with-jni-part-2/
 */

jfieldID get_handle_field(JNIEnv *env, jobject obj) {
    jclass c = env->GetObjectClass(obj);

    // J is the type signature for long
    return env->GetFieldID(c, "nativeHandle", "J");
}

template <typename T>
T * get_handle(JNIEnv *env, jobject obj) {
    jlong handle = env->GetLongField(obj, get_handle_field(env, obj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
void set_handle(JNIEnv *env, jobject obj, T *t) {
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, get_handle_field(env, obj), handle);
}

#endif //RENDERER_HANDLE_H_H
