package com.continuum.nova.mixin.renderer;

import net.minecraft.client.renderer.OpenGlHelper;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

@Mixin(OpenGlHelper.class)
public class MixinOpenGlHelper {

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void initializeTextures() {
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static boolean areShadersSupported() {
        return true;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static String getLogText() {
        return "";
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGetProgrami(int program, int pname) {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glAttachShader(int program, int shaderIn) {
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDeleteShader(int shaderIn) {
    }

    /**
     * creates a shader with the given mode and returns the GL id. params: mode
     *
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glCreateShader(int type) {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glShaderSource(int shaderIn, ByteBuffer string) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glCompileShader(int shaderIn) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGetShaderi(int shaderIn, int pname) {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static String glGetShaderInfoLog(int shaderIn, int maxLength) {
        return "";
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static String glGetProgramInfoLog(int program, int maxLength) {
        return "";
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUseProgram(int program) {
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glCreateProgram() {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDeleteProgram(int program) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glLinkProgram(int program) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGetUniformLocation(int programObj, CharSequence name) {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform1(int location, IntBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform1i(int location, int v0) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform1(int location, FloatBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform2(int location, IntBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform2(int location, FloatBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform3(int location, IntBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform3(int location, FloatBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform4(int location, IntBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniform4(int location, FloatBuffer values) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniformMatrix2(int location, boolean transpose, FloatBuffer matrices) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniformMatrix3(int location, boolean transpose, FloatBuffer matrices) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glUniformMatrix4(int location, boolean transpose, FloatBuffer matrices) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGetAttribLocation(int program, CharSequence name) {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGenBuffers() {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glBindBuffer(int target, int buffer) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glBufferData(int target, ByteBuffer data, int usage) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDeleteBuffers(int buffer) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static boolean useVbo() {
        return false;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glBindFramebuffer(int target, int framebufferIn) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glBindRenderbuffer(int target, int renderbuffer) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDeleteRenderbuffers(int renderbuffer) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDeleteFramebuffers(int framebufferIn) {

    }

    /**
     * Calls the appropriate glGenFramebuffers method and returns the newly created fbo, or returns -1 if not supported.
     *
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGenFramebuffers() {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGenRenderbuffers() {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glRenderbufferStorage(int target, int internalFormat, int width, int height) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glFramebufferRenderbuffer(int target, int attachment, int renderBufferTarget, int renderBuffer) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glCheckFramebufferStatus(int target) {
        return 0;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glFramebufferTexture2D(int target, int attachment, int textarget, int texture, int level) {

    }

    /**
     * Sets the current lightmap texture to the specified OpenGL constant
     *
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void setActiveTexture(int texture) {

    }

    /**
     * Sets the current lightmap texture to the specified OpenGL constant
     *
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void setClientActiveTexture(int texture) {

    }

    /**
     * Sets the current coordinates of the given lightmap texture
     *
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void setLightmapTextureCoords(int target, float p_77475_1_, float t) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glBlendFunc(int sFactorRGB, int dFactorRGB, int sfactorAlpha, int dfactorAlpha) {

    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static boolean isFramebufferEnabled() {
        return false;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void renderDirections(int p_188785_0_) {
    }
}
