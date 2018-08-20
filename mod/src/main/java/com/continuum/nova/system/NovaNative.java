package com.continuum.nova.system;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class NovaNative {
    public static final Logger LOG = LogManager.getLogger(NovaNative.class);

    public enum NovaVertexFormat {
        POS,
        POS_UV,
        POS_UV_LIGHTMAPUV_NORMAL_TANGENT,
        POS_UV_COLOR
    }

    public enum GeometryType {
        BLOCK,
        ENTITY,
        FALLING_BLOCK,
        GUI,
        TEXT,
        GUI_BACKGROUND,
        CLOUD,
        SKY_DECORATION,
        SELECTION_BOX,
        GLINT,
        WEATHER,
        HAND,
        FULLSCREEN_QUAD,
        PARTICLE,
        LIT_PARTICLE,
        EYES
    }

    public enum NativeBoolean {
        FALSE,
        TRUE
    }

    public native void initialize();

    public native void executeFrame();

    public native void sendLightmapTexture(int[] data, int length, int width, int height);

    public native void addTexture(MinecraftAtlasTexture texture);

    public native void addTextureLocation(MinecraftTextureAtlasLocation location);

    public native int getMaxTextureSize();

    public native void resetTextureManager();

    public native  void addChunkGeometryForFilter(String filter_name, MinecraftChunkRenderObject render_object);

    public native void removeChunkGeometryForFilter(String filter_name, MinecraftChunkRenderObject render_object);

    public native boolean shouldClose();

    public native void addGuiGeometry(String type, MinecraftGuiBuffer buffer);

    public native void clearGuiBuffers();

    public native void setMouseGrabbed(boolean grabbed);

    public native MouseButtonEvent getNextMouseButtonEvent();

    public native MousePositionEvent getNextMousePositionEvent();

    public native MouseScrollEvent getNextMouseScrollEvent();

    public native KeyPressEvent getNextKeyPressEvent();

    public native KeyCharEvent getNextKeyCharEvent();

    public native WindowSize getWindowSize();

    public native  void setFullscreen(int fullscreen);

    public native boolean displayIsActive();

    public native void setStringSetting(String setting, String value);

    public native void setFloatSetting(String setting_name, float setting_value);

    public native void setPlayerCameraTransform(double x, double y, double z, float yaw, float pitch);

    public native String getMaterialsAndFilters();

    public native void destruct();
}
