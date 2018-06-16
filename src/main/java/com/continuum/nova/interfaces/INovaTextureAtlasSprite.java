package com.continuum.nova.interfaces;

import net.minecraft.client.renderer.texture.PngSizeInfo;
import net.minecraft.util.ResourceLocation;

import java.io.IOException;

public interface INovaTextureAtlasSprite {
    ResourceLocation getLocation();
    void setLocation(ResourceLocation location);
    void setAndValidateSize(PngSizeInfo sizeInfo, boolean isAnimated) throws IOException;
}
