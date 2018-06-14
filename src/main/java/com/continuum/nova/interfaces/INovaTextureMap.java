package com.continuum.nova.interfaces;

import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.util.ResourceLocation;

import java.util.Optional;

public interface INovaTextureMap {
    void createWhiteTexture(ResourceLocation location);
    Optional<TextureAtlasSprite> getWhiteImage();
    int getWidth();
    int getHeight();
}
