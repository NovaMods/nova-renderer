package com.continuum.nova.gui;

import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.client.resources.IResource;

import java.io.IOException;

/**
 * Represents a TextureAtlasSprite whose value comes not from the disk, but from memory. This texture is not loaded
 * from disk ever
 *
 * @author ddubois
 */
public class MemoryTextureAtlasSprite extends TextureAtlasSprite {
    public MemoryTextureAtlasSprite(String spriteName) {
        super(spriteName);
    }

    @Override
    public void loadSpriteFrames(IResource resource, int mipmaplevels) throws IOException {
        // Don't want to do anything because we don't need to load any frames from disk
    }
}
