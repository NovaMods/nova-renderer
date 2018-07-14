package com.continuum.nova.mixin.texture;

import net.minecraft.client.renderer.texture.TextureUtil;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

@Mixin(TextureUtil.class)
public class MixinTextureUtil {
    /**
     * @author Janrupf
     * @reason // TODO: #Placeholder -> because I have no idea why this has to be done [Janrupf]
     * @inheritDoc
     */
    @Overwrite
    public static int glGenTextures() {
        return 0;
    }
}
