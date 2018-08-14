#include <jni.h>
/* Header for class com_continuum_nova_system_NovaNative */

#ifndef _Included_com_continuum_nova_system_NovaNative
#define _Included_com_continuum_nova_system_NovaNative
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_initialize
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    execute_frame
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_execute_1frame
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    send_lightmap_texture
 * Signature: ([IIII)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_send_1lightmap_1texture
  (JNIEnv *, jobject, jintArray, jint, jint, jint);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    add_texture
 * Signature: (Lcom/continuum/nova/system/NovaNative/mc_atlas_texture;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_add_1texture
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    add_texture_location
 * Signature: (Lcom/continuum/nova/system/NovaNative/mc_texture_atlas_location;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_add_1texture_1location
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_max_texture_size
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_continuum_nova_system_NovaNative_get_1max_1texture_1size
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    reset_texture_manager
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_reset_1texture_1manager
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    add_chunk_geometry_for_filter
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/NovaNative/mc_chunk_render_object;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_add_1chunk_1geometry_1for_1filter
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    remove_chunk_geometry_for_filter
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/NovaNative/mc_chunk_render_object;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_remove_1chunk_1geometry_1for_1filter
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    should_close
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_continuum_nova_system_NovaNative_should_1close
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    add_gui_geometry
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/NovaNative/mc_gui_buffer;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_add_1gui_1geometry
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    clear_gui_buffers
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_clear_1gui_1buffers
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    set_mouse_grabbed
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_set_1mouse_1grabbed
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_next_mouse_button_event
 * Signature: ()Lcom/continuum/nova/system/NovaNative/mouse_button_event;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_get_1next_1mouse_1button_1event
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_next_mouse_position_event
 * Signature: ()Lcom/continuum/nova/system/NovaNative/mouse_position_event;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_get_1next_1mouse_1position_1event
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_next_mouse_scroll_event
 * Signature: ()Lcom/continuum/nova/system/NovaNative/mouse_scroll_event;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_get_1next_1mouse_1scroll_1event
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_next_key_press_event
 * Signature: ()Lcom/continuum/nova/system/NovaNative/key_press_event;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_get_1next_1key_1press_1event
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_next_key_char_event
 * Signature: ()Lcom/continuum/nova/system/NovaNative/key_char_event;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_get_1next_1key_1char_1event
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_window_size
 * Signature: ()Lcom/continuum/nova/system/NovaNative/window_size;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_get_1window_1size
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    set_fullscreen
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_set_1fullscreen
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    display_is_active
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_continuum_nova_system_NovaNative_display_1is_1active
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    set_string_setting
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_set_1string_1setting
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    set_float_setting
 * Signature: (Ljava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_set_1float_1setting
  (JNIEnv *, jobject, jstring, jfloat);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    set_player_camera_transform
 * Signature: (DDDFF)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_set_1player_1camera_1transform
  (JNIEnv *, jobject, jdouble, jdouble, jdouble, jfloat, jfloat);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    get_materials_and_filters
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_continuum_nova_system_NovaNative_get_1materials_1and_1filters
  (JNIEnv *, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    destruct
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_destruct
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
