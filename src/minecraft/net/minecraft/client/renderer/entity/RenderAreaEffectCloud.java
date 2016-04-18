package net.minecraft.client.renderer.entity;

import net.minecraft.entity.EntityAreaEffectCloud;
import net.minecraft.util.ResourceLocation;

public class RenderAreaEffectCloud extends Render<EntityAreaEffectCloud>
{
    public RenderAreaEffectCloud(RenderManager p_i46554_1_)
    {
        super(p_i46554_1_);
    }

    /**
     * Renders the desired {@code T} type Entity.
     */
    public void doRender(EntityAreaEffectCloud entity, double x, double y, double z, float entityYaw, float partialTicks)
    {
        super.doRender(entity, x, y, z, entityYaw, partialTicks);
    }

    /**
     * Returns the location of an entity's texture. Doesn't seem to be called unless you call Render.bindEntityTexture.
     */
    protected ResourceLocation getEntityTexture(EntityAreaEffectCloud entity)
    {
        return null;
    }
}
