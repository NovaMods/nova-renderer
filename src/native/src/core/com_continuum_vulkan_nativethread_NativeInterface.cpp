//
// Created by David on 10-Apr-16.
//

#include "com_continuum_vulkan_nativethread_NativeInterface.h"
#include "nova_renderer.h"

JNIEXPORT jlong JNICALL Java_com_continuum_vulkan_nativethread_NativeInterface_runVulkanMod(JNIEnv *, jobject) {
    return run_vulkan_mod();
}