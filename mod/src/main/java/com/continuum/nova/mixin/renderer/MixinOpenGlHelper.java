package com.continuum.nova.mixin.renderer;

import net.minecraft.client.renderer.OpenGlHelper;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

@Mixin(OpenGlHelper.class)
public class MixinOpenGlHelper {

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more, NovaClassTransformer returns wrong value.
     */
    @Overwrite
    public static boolean areShadersSupported() {
        return true;
    }

    /**
     * @author Barteks2x
     * @reason Cleared because we do not use OpenGL any more, NovaClassTransformer returns wrong value.
     */
    @Overwrite
    public static String getLogText() {
        return "";
    }
}
