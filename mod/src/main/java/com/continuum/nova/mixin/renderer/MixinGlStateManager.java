package com.continuum.nova.mixin.renderer;

import static com.continuum.nova.gui.NovaDraw.matrixStack;

import com.continuum.nova.gui.NovaDraw;
import glm.mat._4.Mat4;
import net.minecraft.client.renderer.GlStateManager;
import org.spongepowered.asm.mixin.*;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

import javax.annotation.Nullable;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.Stack;

@Mixin(value = GlStateManager.class)
public class MixinGlStateManager {

    // TODO: manage matrix mode too?
    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void loadIdentity() {
        matrixStack.peek().set(1);
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void pushMatrix() {
        matrixStack.push(new Mat4(matrixStack.peek()));
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void popMatrix() {
        matrixStack.pop();
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void ortho(double left, double right, double bottom, double top, double zNear, double zFar) {
        matrixStack.peek().ortho((float) left, (float) right, (float) bottom, (float) top, (float) zNear, (float) zFar);
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void rotate(float angle, float x, float y, float z) {
        matrixStack.peek().rotate((float) Math.toRadians(angle), x, y, z);
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void scale(float x, float y, float z) {
        matrixStack.peek().scale(x, y, 1); // do not scale Z because it breaks GUI z index, TODO: detect GUI rendering and only do that for GUI?
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void scale(double x, double y, double z) {
        matrixStack.peek().scale((float) x, (float) y, (float) 1);
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void translate(float x, float y, float z) {
        matrixStack.peek().translate(x, y, 0);
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void translate(double x, double y, double z) {
        matrixStack.peek().translate((float) x, (float) y, (float) 0);
    }

    /**
     * @author Janrupf, Barteks2x
     * @reason Manage those matrices to get some of the GL-based rendering working
     */
    @Overwrite
    public static void multMatrix(FloatBuffer matrix) {
        float[] f = new float[16];
        for (int i = 0; i < f.length; i++) {
            f[i] = matrix.get();
        }
        Mat4 mat = new Mat4(f);
        matrixStack.peek().mul(mat);
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more. Not done properly by NovaClassTransformer because returning 0 breaks other code
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
    public static String glGetString(int p_187416_0_) {
        return "";
    }
}