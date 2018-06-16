package com.continuum.nova.interfaces;

import net.minecraft.client.renderer.texture.DynamicTexture;

public interface INovaEntityRenderer {
    void updateLightmap(float partialTicks);
    boolean isLightmapUpdateNeeded();
    DynamicTexture getLightmapTexture();
}
