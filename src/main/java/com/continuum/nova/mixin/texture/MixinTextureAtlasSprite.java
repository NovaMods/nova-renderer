package com.continuum.nova.mixin.texture;

import com.continuum.nova.interfaces.INovaTextureAtlasSprite;
import net.minecraft.client.renderer.texture.PngSizeInfo;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.util.ResourceLocation;
import org.spongepowered.asm.mixin.Implements;
import org.spongepowered.asm.mixin.Interface;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;

import java.io.IOException;

@Mixin(TextureAtlasSprite.class)
@Implements(@Interface(iface = INovaTextureAtlasSprite.class, prefix = "nova$"))
public abstract class MixinTextureAtlasSprite {
    @Shadow protected abstract void resetSprite();

    @Shadow protected int width;
    @Shadow protected int height;
    private ResourceLocation location;

    public ResourceLocation nova$getLocation() {
        return location;
    }

    public void nova$setLocation(ResourceLocation location) {
        this.location = location;
    }

    public void nova$setAndValidateSize(PngSizeInfo sizeInfo, boolean isAnimated) throws IOException {
        this.resetSprite();
        this.width = sizeInfo.pngWidth;
        this.height = sizeInfo.pngHeight;

        if(isAnimated) {
            this.height = this.width;
        } else if(sizeInfo.pngHeight != sizeInfo.pngWidth) {
            throw new IOException("broken aspect ratio and not an animation");
        }
    }
}
