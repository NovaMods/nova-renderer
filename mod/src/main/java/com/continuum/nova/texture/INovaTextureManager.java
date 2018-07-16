package com.continuum.nova.texture;

import net.minecraft.util.ResourceLocation;

/**
 * This class will be implemented by vanilla texture manager,
 * Nova changes the way textures are used, there is no real texture binding the way it's done in MC.
 * So nova needs to keep track of which texture would be bound at what point,
 * and render everything rendered with given texture at it's own time.
 *
 * This interface allows to access currently bound texture from MC texture manager.
 */
public interface INovaTextureManager {

    /**
     * Returns the ResourceLocation of the currently used texture.
     */
    ResourceLocation getCurrentTexture();
}
