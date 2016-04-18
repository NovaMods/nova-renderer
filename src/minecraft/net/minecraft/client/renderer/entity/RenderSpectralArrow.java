package net.minecraft.client.renderer.entity;

import net.minecraft.entity.projectile.EntitySpectralArrow;
import net.minecraft.util.ResourceLocation;

public class RenderSpectralArrow extends RenderArrow<EntitySpectralArrow>
{
    public static final ResourceLocation field_188303_a = new ResourceLocation("textures/entity/projectiles/spectral_arrow.png");

    public RenderSpectralArrow(RenderManager p_i46549_1_)
    {
        super(p_i46549_1_);
    }

    /**
     * Returns the location of an entity's texture. Doesn't seem to be called unless you call Render.bindEntityTexture.
     */
    protected ResourceLocation getEntityTexture(EntitySpectralArrow entity)
    {
        return field_188303_a;
    }
}
