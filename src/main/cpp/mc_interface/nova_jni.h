/*! 
 * \author gold1 
 * \date 19-Jul-17.
 */

#ifndef RENDERER_NOVA_JNI_H
#define RENDERER_NOVA_JNI_H


#include "../../../../../../Program Files/Java/jdk1.8.0_121/include/jni.h"
#include "../utils/export.h"

#if defined(_WIN32)
    #define JNICALL __stdcall
#else
    #if defined(__GNUC__)
        #define JNICALL
    #endif
#endif

extern "C" {
NOVA_API jobject JNICALL Java_getSharedBuffer(JNIEnv *env, jobject caller);
}

#endif //RENDERER_NOVA_JNI_H
