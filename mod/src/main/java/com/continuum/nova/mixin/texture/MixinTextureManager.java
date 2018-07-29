package com.continuum.nova.mixin.texture;

import com.continuum.nova.texture.INovaTextureManager;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.util.ResourceLocation;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

@Mixin(TextureManager.class)
public class MixinTextureManager implements INovaTextureManager {

    private ResourceLocation currentTexture;

    /**
     * @author Barteks2x
     * @reason see {@link INovaTextureManager}
     */
    @Overwrite
    public void bindTexture(ResourceLocation resource) {
        this.currentTexture = resource;
    }

    @Override public ResourceLocation getCurrentTexture() {
        return currentTexture;
    }
}
