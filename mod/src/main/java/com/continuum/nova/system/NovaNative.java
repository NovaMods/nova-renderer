package com.continuum.nova.system;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class NovaNative {
    public static final Logger LOG = LogManager.getLogger(NovaNative.class);

    public static native void initialize();

    public static native void executeFrame();

    public static native void sendLightmapTexture(int[] data, int length, int width, int height);

    public static native void addTexture(MinecraftAtlasTexture texture);

    public static native void addTextureLocation(MinecraftTextureAtlasLocation location);

    public static native int getMaxTextureSize();

    public static native void resetTextureManager();

    public static native void addChunkGeometryForFilter(String filter_name, MinecraftChunkRenderObject render_object);

    public static native void removeChunkGeometryForFilter(String filter_name, MinecraftChunkRenderObject render_object);

    public static native boolean shouldClose();

    public static native void addGuiGeometry(String type, MinecraftGuiBuffer buffer);

    public static native void clearGuiBuffers();

    public static native void setMouseGrabbed(boolean grabbed);

    public static native MouseButtonEvent getNextMouseButtonEvent();

    public static native MousePositionEvent getNextMousePositionEvent();

    public static native MouseScrollEvent getNextMouseScrollEvent();

    public static native KeyPressEvent getNextKeyPressEvent();

    public static native KeyCharEvent getNextKeyCharEvent();

    public static native WindowSize getWindowSize();

    public static native void setFullscreen(boolean fullscreen);

    public static native boolean displayIsActive();

    public static native void setStringSetting(String setting, String value);

    public static native void setFloatSetting(String setting_name, float setting_value);

    public static native void setPlayerCameraTransform(double x, double y, double z, float yaw, float pitch);

    public static native String getMaterialsAndFilters();

    public static native void destruct();
}
