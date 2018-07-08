package com.continuum.nova.mixin.renderer;

import net.minecraft.client.renderer.OpenGlHelper;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

@Mixin(OpenGlHelper.class)
public class MixinOpenGlHelper {

    @Overwrite
    public static void initializeTextures() {
    }

    @Overwrite
    public static boolean areShadersSupported() {
        return false;
    }

    @Overwrite
    public static String getLogText() {
        return "";
    }

    @Overwrite
    public static int glGetProgrami(int program, int pname) {
        return 0;
    }

    @Overwrite
    public static void glAttachShader(int program, int shaderIn) {
    }

    @Overwrite
    public static void glDeleteShader(int shaderIn) {
    }

    /**
     * creates a shader with the given mode and returns the GL id. params: mode
     */
    @Overwrite
    public static int glCreateShader(int type) {
        return 0;
    }

    @Overwrite
    public static void glShaderSource(int shaderIn, ByteBuffer string) {

    }

    @Overwrite
    public static void glCompileShader(int shaderIn) {

    }

    @Overwrite
    public static int glGetShaderi(int shaderIn, int pname) {
        return 0;
    }

    @Overwrite
    public static String glGetShaderInfoLog(int shaderIn, int maxLength) {
        return "";
    }

    @Overwrite
    public static String glGetProgramInfoLog(int program, int maxLength) {
        return "";
    }

    @Overwrite
    public static void glUseProgram(int program) {
    }

    @Overwrite
    public static int glCreateProgram() {
        return 0;
    }

    @Overwrite
    public static void glDeleteProgram(int program) {

    }

    @Overwrite
    public static void glLinkProgram(int program) {

    }

    @Overwrite
    public static int glGetUniformLocation(int programObj, CharSequence name) {
        return 0;
    }

    @Overwrite
    public static void glUniform1(int location, IntBuffer values) {

    }

    @Overwrite
    public static void glUniform1i(int location, int v0) {

    }

    @Overwrite
    public static void glUniform1(int location, FloatBuffer values) {

    }

    @Overwrite
    public static void glUniform2(int location, IntBuffer values) {

    }

    @Overwrite
    public static void glUniform2(int location, FloatBuffer values) {

    }

    @Overwrite
    public static void glUniform3(int location, IntBuffer values) {

    }

    @Overwrite
    public static void glUniform3(int location, FloatBuffer values) {

    }

    @Overwrite
    public static void glUniform4(int location, IntBuffer values) {

    }

    @Overwrite
    public static void glUniform4(int location, FloatBuffer values) {

    }

    @Overwrite
    public static void glUniformMatrix2(int location, boolean transpose, FloatBuffer matrices) {

    }

    @Overwrite
    public static void glUniformMatrix3(int location, boolean transpose, FloatBuffer matrices) {

    }

    @Overwrite
    public static void glUniformMatrix4(int location, boolean transpose, FloatBuffer matrices) {

    }

    @Overwrite
    public static int glGetAttribLocation(int program, CharSequence name) {
        return 0;
    }

    @Overwrite
    public static int glGenBuffers() {
        return 0;
    }

    @Overwrite
    public static void glBindBuffer(int target, int buffer) {

    }

    @Overwrite
    public static void glBufferData(int target, ByteBuffer data, int usage) {

    }

    @Overwrite
    public static void glDeleteBuffers(int buffer) {

    }

    @Overwrite
    public static boolean useVbo() {
        return false;
    }

    @Overwrite
    public static void glBindFramebuffer(int target, int framebufferIn) {

    }

    @Overwrite
    public static void glBindRenderbuffer(int target, int renderbuffer) {

    }

    @Overwrite
    public static void glDeleteRenderbuffers(int renderbuffer) {

    }

    @Overwrite
    public static void glDeleteFramebuffers(int framebufferIn) {

    }

    /**
     * Calls the appropriate glGenFramebuffers method and returns the newly created fbo, or returns -1 if not supported.
     */
    @Overwrite
    public static int glGenFramebuffers() {
        return 0;
    }

    @Overwrite
    public static int glGenRenderbuffers() {
        return 0;
    }

    @Overwrite
    public static void glRenderbufferStorage(int target, int internalFormat, int width, int height) {

    }

    @Overwrite
    public static void glFramebufferRenderbuffer(int target, int attachment, int renderBufferTarget, int renderBuffer) {

    }

    @Overwrite
    public static int glCheckFramebufferStatus(int target) {
        return 0;
    }

    @Overwrite
    public static void glFramebufferTexture2D(int target, int attachment, int textarget, int texture, int level) {

    }

    /**
     * Sets the current lightmap texture to the specified OpenGL constant
     */
    @Overwrite
    public static void setActiveTexture(int texture) {

    }

    /**
     * Sets the current lightmap texture to the specified OpenGL constant
     */
    @Overwrite
    public static void setClientActiveTexture(int texture) {

    }

    /**
     * Sets the current coordinates of the given lightmap texture
     */
    @Overwrite
    public static void setLightmapTextureCoords(int target, float p_77475_1_, float t) {

    }

    @Overwrite
    public static void glBlendFunc(int sFactorRGB, int dFactorRGB, int sfactorAlpha, int dfactorAlpha) {

    }

    @Overwrite
    public static boolean isFramebufferEnabled() {
        return false;
    }

    @Overwrite
    public static void renderDirections(int p_188785_0_) {
    }
}
