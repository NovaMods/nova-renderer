package com.continuum.nova.mixin.renderer;

import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

@Mixin(targets = "net.minecraft.client.renderer.GlStateManager$Profile$3")
public class MixinProfile_TRANSPARENT_MODEL {
    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public void apply() {
    }

    /**
     * @author Janrupf
     * @reason Cleared because we do not use OpenGL any more
     */
    @Overwrite
    public void clean() {
    }
}
