package com.continuum.nova.mixin.texture;

import com.continuum.nova.interfaces.INovaDynamicTexture;
import net.minecraft.client.renderer.texture.DynamicTexture;
import org.spongepowered.asm.mixin.*;

@Mixin(DynamicTexture.class)
@Implements(@Interface(iface = INovaDynamicTexture.class, prefix = "nova$"))
public class MixinDynamicTexture {
    @Shadow @Final private int width;

    @Shadow @Final private int height;

    public int nova$getWidth() {
        return width;
    }

    public int nova$getHeight() {
        return height;
    }
}
