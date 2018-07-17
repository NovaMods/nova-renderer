package com.continuum.nova.mixin.renderer;

import net.minecraft.client.renderer.GlStateManager;
import org.spongepowered.asm.mixin.*;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

import javax.annotation.Nullable;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

@Mixin(value = GlStateManager.class)
public class MixinGlStateManager {
    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void pushAttrib() {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void popAttrib() {
    }

    @Inject(
            method = "alphaFunc",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glAlphaFunc(IF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelAlphaFunc(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "colorMaterial",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glColorMaterial(II)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelColorMaterial(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glLight(int light, int pname, FloatBuffer params) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glLightModel(int pname, FloatBuffer params) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glNormal3f(float nx, float ny, float nz) {
    }

    @Inject(
            method = "depthFunc",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glDepthFunc(I)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelDepthFunc(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "depthMask",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glDepthMask(Z)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelDepthMasg(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "blendFunc(II)V",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glBlendFunc(II)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelBlendFunc(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glBlendEquation(int blendEquation) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void enableOutlineMode(int obfuscated_p_187431_0_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void disableOutlineMode() {
    }

    @Inject(
            method = "setFog(I)V",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glFogi(II)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelSetFog(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "setFogDensity",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glFogf(IF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelSetFogDensity(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "setFogStart",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glFogf(IF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelSetFogStart(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "setFogEnd",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glFogf(IF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelSetFogEnd(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glFog(int pname, FloatBuffer param) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glFogi(int pname, int param) {
    }

    @Inject(
            method = "cullFace(I)V",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glCullFace(I)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelCullFace(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glPolygonMode(int face, int mode) {
    }

    @Inject(
            method = "doPolygonOffset",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glPolygonOffset(FF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelDoPolygonOffset(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "colorLogicOp(I)V",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glLogicOp(I)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelColorLogicOp(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "texGen(Lnet/minecraft/client/renderer/GlStateManager$TexGen;I)V",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glTexGeni(III)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelTextGen_1(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void texGen(GlStateManager.TexGen texGen, int pname, FloatBuffer params) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexEnv(int p_187448_0_, int p_187448_1_, FloatBuffer p_187448_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexEnvi(int p_187399_0_, int p_187399_1_, int p_187399_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexEnvf(int p_187436_0_, int p_187436_1_, float p_187436_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexParameterf(int p_187403_0_, int p_187403_1_, float p_187403_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexParameteri(int p_187421_0_, int p_187421_1_, int p_187421_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGetTexLevelParameteri(int p_187411_0_, int p_187411_1_, int p_187411_2_) {
        return 0;
    }

    /**
     * @author Janrupf
     * @reason Overwritten due to change that cannot be done with Mixins (FIXME: We should not remove the entire code!)
     */
    @Overwrite
    public static void deleteTexture(int texture) {
    }

    @Inject(
            method = "bindTexture",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glBindTexture(II)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelBindTexture(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexImage2D(int p_187419_0_, int p_187419_1_, int p_187419_2_, int p_187419_3_, int p_187419_4_, int p_187419_5_, int p_187419_6_, int p_187419_7_, @Nullable IntBuffer p_187419_8_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexSubImage2D(int target, int level, int xoffset, int yOffset, int width, int height, int format, int type, IntBuffer data) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glCopyTexSubImage2D(int p_187443_0_, int p_187443_1_, int p_187443_2_, int p_187443_3_, int p_187443_4_, int p_187443_5_, int p_187443_6_, int p_187443_7_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glGetTexImage(int p_187433_0_, int p_187433_1_, int p_187433_2_, int p_187433_3_, IntBuffer p_187433_4_) {
    }

    @Inject(
            method = "shadeModel",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glShadeModel(I)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelShadeModel(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void viewport(int x, int y, int width, int height) {
    }

    @Inject(
            method = "colorMask",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glColorMask(ZZZZ)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelColorMask(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "clearDepth",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glClearDepth(D)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelClearDepth(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    @Inject(
            method = "clearColor",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glClearColor(FFFF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelClearColor(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void clear(int mask) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void matrixMode(int mode) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void loadIdentity() {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void pushMatrix() {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void popMatrix() {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void getFloat(int pname, FloatBuffer params) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void ortho(double left, double right, double bottom, double top, double zNear, double zFar) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void rotate(float angle, float x, float y, float z) {
        //GL11.glRotatef(angle, x, y, z);
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void scale(float x, float y, float z) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void scale(double x, double y, double z) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void translate(float x, float y, float z) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void translate(double x, double y, double z) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void multMatrix(FloatBuffer matrix) {
    }

    @Inject(
            method = "color(FFFF)V",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glColor4f(FFFF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private static void cancelColor(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexCoord2f(float p_187426_0_, float p_187426_1_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glVertex3f(float p_187435_0_, float p_187435_1_, float p_187435_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glNormalPointer(int p_187446_0_, int p_187446_1_, ByteBuffer p_187446_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexCoordPointer(int p_187405_0_, int p_187405_1_, int p_187405_2_, int p_187405_3_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glTexCoordPointer(int p_187404_0_, int p_187404_1_, int p_187404_2_, ByteBuffer p_187404_3_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glVertexPointer(int p_187420_0_, int p_187420_1_, int p_187420_2_, int p_187420_3_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glVertexPointer(int p_187427_0_, int p_187427_1_, int p_187427_2_, ByteBuffer p_187427_3_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glColorPointer(int p_187406_0_, int p_187406_1_, int p_187406_2_, int p_187406_3_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glColorPointer(int p_187400_0_, int p_187400_1_, int p_187400_2_, ByteBuffer p_187400_3_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDisableClientState(int p_187429_0_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glEnableClientState(int p_187410_0_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glBegin(int p_187447_0_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glEnd() {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDrawArrays(int p_187439_0_, int p_187439_1_, int p_187439_2_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glLineWidth(float p_187441_0_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void callList(int list) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glDeleteLists(int p_187449_0_, int p_187449_1_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glNewList(int p_187423_0_, int p_187423_1_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glEndList() {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGenLists(int p_187442_0_) {
        return 1;
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glPixelStorei(int p_187425_0_, int p_187425_1_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glReadPixels(int p_187413_0_, int p_187413_1_, int p_187413_2_, int p_187413_3_, int p_187413_4_, int p_187413_5_, IntBuffer p_187413_6_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGetError() {
        return 0;
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static String glGetString(int p_187416_0_) {
        return "";
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static void glGetInteger(int p_187445_0_, IntBuffer p_187445_1_) {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public static int glGetInteger(int p_187397_0_) {
        return 0;
    }

}