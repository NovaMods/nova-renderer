//
// Created by David on 10-Apr-16.
//

#include <iostream>
#include "com_continuum_nova_nativethread_NativeInterface.h"
#include "nova_renderer.h"
#include "../jni/handle.h"


JNIEXPORT void JNICALL Java_com_continuum_nova_nativethread_NativeInterface_runVulkanMod(JNIEnv * env, jobject obj) {

    std::cout << "About to create the mod object";

    nova_renderer * mod = new nova_renderer();

    std::cout << "Mod object created";

    set_handle(env, obj, mod);
}