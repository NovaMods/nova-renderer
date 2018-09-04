/*!
 * \brief Implements all the functions exposed as part of the MC/Nova interface
 *
 * \author ddubois 
 * \date 14-Aug-18.
 */

#define STRING_VOID_SIGNATURE "(Ljava/lang/String;)V"
#define JSTRING(std_string) env->NewStringUTF(std_string.c_str())

#include <iostream>
#include "jni/com_continuum_nova_system_NovaNative.h"
#include "../src/nova_renderer.hpp"
#include "../src/util/logger.hpp"

#include "../src/platform.hpp"

#if SUPPORT_DX12
    #include "../src/render_engine/dx12/dx12_render_engine.hpp"
    #define RenderEngineType nova::dx12_render_engine

    #ifdef ERROR
        // So glad Windows defines ERROR, a super specific and not commonly used token
        #undef ERROR
    #endif
#elif SUPPORT_VULKAN
#include "../src/render_engine/vulkan/vulkan_render_engine.hpp"
#define RenderEngineType nova::vulkan_render_engine
#endif

/*!
 * \brief Initializes Nova
 *
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    initialize
 * Signature: (Lorg/apache/logging/log4j/Logger;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_initialize
        (JNIEnv *env, jclass nova_native_class, jobject log4j_logger) {
    jclass log4j_logger_class = env->GetObjectClass(log4j_logger);

    jmethodID info_method = env->GetMethodID(log4j_logger_class, "info", STRING_VOID_SIGNATURE);
    if(info_method) {
        nova::logger::instance.add_log_handler(nova::log_level::INFO, [log4j_logger, info_method, env](auto msg) {
            env->CallVoidMethod(log4j_logger, info_method, JSTRING(msg));
        });
    } else {
        std::cerr << "Failed to find method id for org/apache/logging/log4j/Logger#info(Ljava/lang/String;)V" << std::endl;
        nova::logger::instance.add_log_handler(nova::log_level::INFO, [](auto msg) {
           std::cout << "[NOVA JNI FALLBACK LOGGER/INFO] " << msg << std::endl;
        });
    }

    jmethodID warn_method = env->GetMethodID(log4j_logger_class, "warn", STRING_VOID_SIGNATURE);
    if(warn_method) {
        nova::logger::instance.add_log_handler(nova::log_level::WARN, [log4j_logger, warn_method, env](auto msg) {
            env->CallVoidMethod(log4j_logger, warn_method, JSTRING(msg));
        });
    } else {
        std::cerr << "Failed to find method id for org/apache/logging/log4j/Logger#warn(Ljava/lang/String;)V" << std::endl;
        nova::logger::instance.add_log_handler(nova::log_level::WARN, [](auto msg) {
            std::cerr << "[NOVA JNI FALLBACK LOGGER/WARN] " << msg << std::endl;
        });
    }

    jmethodID error_method = env->GetMethodID(log4j_logger_class, "error", STRING_VOID_SIGNATURE);
    if(error_method) {
        nova::logger::instance.add_log_handler(nova::log_level::ERROR, [log4j_logger, error_method, env](auto msg) {
            env->CallVoidMethod(log4j_logger, error_method, JSTRING(msg));
        });
    } else {
        std::cerr << "Failed to find method id for org/apache/logging/log4j/Logger#error(Ljava/lang/String;)V" << std::endl;
        nova::logger::instance.add_log_handler(nova::log_level::ERROR, [](auto msg) {
            std::cerr << "[NOVA JNI FALLBACK LOGGER/ERROR] " << msg << std::endl;
        });
    }

    jmethodID fatal_method = env->GetMethodID(log4j_logger_class, "fatal", STRING_VOID_SIGNATURE);
    if(fatal_method) {
        nova::logger::instance.add_log_handler(nova::log_level::FATAL, [log4j_logger, fatal_method, env](auto msg) {
            env->CallVoidMethod(log4j_logger, fatal_method, JSTRING(msg));
        });
    } else {
        std::cerr << "Failed to find method id for org/apache/logging/log4j/Logger#fatal(Ljava/lang/String;)V" << std::endl;
        nova::logger::instance.add_log_handler(nova::log_level::FATAL, [](auto msg) {
            std::cerr << "[NOVA JNI FALLBACK LOGGER/FATAL] " << msg << std::endl;
        });
    }

    jmethodID debug_method = env->GetMethodID(log4j_logger_class, "debug", STRING_VOID_SIGNATURE);
    if(debug_method) {
        nova::logger::instance.add_log_handler(nova::log_level::DEBUG, [log4j_logger, debug_method, env](auto msg) {
            env->CallVoidMethod(log4j_logger, debug_method, JSTRING(msg));
        });
    } else {
        std::cerr << "Failed to find method id for org/apache/logging/log4j/Logger#debug(Ljava/lang/String;)V" << std::endl;
        nova::logger::instance.add_log_handler(nova::log_level::DEBUG, [](auto msg) {
            std::cout << "[NOVA JNI FALLBACK LOGGER/DEBUG] " << msg << std::endl;
        });
    }

    jmethodID trace_method = env->GetMethodID(log4j_logger_class, "trace", STRING_VOID_SIGNATURE);
    if(trace_method) {
        nova::logger::instance.add_log_handler(nova::log_level::TRACE, [log4j_logger, trace_method, env](auto msg) {
            env->CallVoidMethod(log4j_logger, trace_method, JSTRING(msg));
        });

        // MAX_LEVEL can be interpreted as TRACE for log4j
        nova::logger::instance.add_log_handler(nova::log_level::MAX_LEVEL, [log4j_logger, trace_method, env](auto msg) {
            env->CallVoidMethod(log4j_logger, trace_method, JSTRING(("(MAX_LEVEL) " + msg)));
        });
    } else {
        std::cerr << "Failed to find method id for org/apache/logging/log4j/Logger#trace(Ljava/lang/String;)V" << std::endl;
        nova::logger::instance.add_log_handler(nova::log_level::TRACE, [](auto msg) {
            std::cout << "[NOVA JNI FALLBACK LOGGER/TRACE] " << msg << std::endl;
        });

        nova::logger::instance.add_log_handler(nova::log_level::MAX_LEVEL, [](auto msg) {
            std::cout << "[NOVA JNI FALLBACK LOGGER/MAX] " << msg << std::endl;
        });
    }

    nova::nova_renderer<RenderEngineType>::initialize();
}

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    executeFrame
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_executeFrame
        (JNIEnv *, jclass) {
    nova::nova_renderer<RenderEngineType>::get_instance()->execute_frame();
}

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    sendLightmapTexture
 * Signature: ([IIII)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_sendLightmapTexture
        (JNIEnv *, jclass, jintArray, jint, jint, jint);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addTexture
 * Signature: (Lcom/continuum/nova/system/MinecraftAtlasTexture;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addTexture
        (JNIEnv *, jclass, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addTextureLocation
 * Signature: (Lcom/continuum/nova/system/MinecraftTextureAtlasLocation;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addTextureLocation
        (JNIEnv *, jclass, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getMaxTextureSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_continuum_nova_system_NovaNative_getMaxTextureSize
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    resetTextureManager
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_resetTextureManager
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addChunkGeometryForFilter
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/MinecraftChunkRenderObject;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addChunkGeometryForFilter
        (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    removeChunkGeometryForFilter
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/MinecraftChunkRenderObject;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_removeChunkGeometryForFilter
        (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    shouldClose
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_continuum_nova_system_NovaNative_shouldClose
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addGuiGeometry
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/MinecraftGuiBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addGuiGeometry
        (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    clearGuiBuffers
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_clearGuiBuffers
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setMouseGrabbed
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setMouseGrabbed
        (JNIEnv *, jclass, jboolean);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextMouseButtonEvent
 * Signature: ()Lcom/continuum/nova/system/MouseButtonEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextMouseButtonEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextMousePositionEvent
 * Signature: ()Lcom/continuum/nova/system/MousePositionEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextMousePositionEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextMouseScrollEvent
 * Signature: ()Lcom/continuum/nova/system/MouseScrollEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextMouseScrollEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextKeyPressEvent
 * Signature: ()Lcom/continuum/nova/system/KeyPressEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextKeyPressEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextKeyCharEvent
 * Signature: ()Lcom/continuum/nova/system/KeyCharEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextKeyCharEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getWindowSize
 * Signature: ()Lcom/continuum/nova/system/WindowSize;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getWindowSize
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setFullscreen
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setFullscreen
        (JNIEnv *, jclass, jboolean);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    displayIsActive
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_continuum_nova_system_NovaNative_displayIsActive
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setStringSetting
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setStringSetting
        (JNIEnv *, jclass, jstring, jstring);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setFloatSetting
 * Signature: (Ljava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setFloatSetting
        (JNIEnv *, jclass, jstring, jfloat);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setPlayerCameraTransform
 * Signature: (DDDFF)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setPlayerCameraTransform
        (JNIEnv *, jclass, jdouble, jdouble, jdouble, jfloat, jfloat);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getMaterialsAndFilters
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_continuum_nova_system_NovaNative_getMaterialsAndFilters
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    destruct
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_destruct
        (JNIEnv *, jclass) {
    nova::nova_renderer<RenderEngineType>::deinitialize();
}
