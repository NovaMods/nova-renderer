package net.minecraft.client.renderer.entity;

import net.minecraft.client.model.ModelShulkerBullet;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.entity.projectile.EntityShulkerBullet;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;

public class RenderShulkerBullet extends Render<EntityShulkerBullet>
{
    private static final ResourceLocation field_188348_a = new ResourceLocation("textures/entity/shulker/spark.png");
    private final ModelShulkerBullet field_188349_b = new ModelShulkerBullet();

    public RenderShulkerBullet(RenderManager p_i46551_1_)
    {
        super(p_i46551_1_);
    }

    private float func_188347_a(float p_188347_1_, float p_188347_2_, float p_188347_3_)
    {
        float f;

        for (f = p_188347_2_ - p_188347_1_; f < -180.0F; f += 360.0F)
        {
            ;
        }

        while (f >= 180.0F)
        {
            f -= 360.0F;
        }

        return p_188347_1_ + p_188347_3_ * f;
    }

    /**
     * Renders the desired {@code T} type Entity.
     */
    public void doRender(EntityShulkerBullet entity, double x, double y, double z, float entityYaw, float partialTicks)
    {
        GlStateManager.pushMatrix();
        float f = this.func_188347_a(entity.prevRotationYaw, entity.rotationYaw, partialTicks);
        float f1 = entity.prevRotationPitch + (entity.rotationPitch - entity.prevRotationPitch) * partialTicks;
        float f2 = (float)entity.ticksExisted + partialTicks;
        GlStateManager.translate((float)x, (float)y + 0.15F, (float)z);
        GlStateManager.rotate(MathHelper.sin(f2 * 0.1F) * 180.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager.rotate(MathHelper.cos(f2 * 0.1F) * 180.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate(MathHelper.sin(f2 * 0.15F) * 360.0F, 0.0F, 0.0F, 1.0F);
        float f3 = 0.03125F;
        GlStateManager.enableRescaleNormal();
        GlStateManager.scale(-1.0F, -1.0F, 1.0F);
        this.bindEntityTexture(entity);
        this.field_188349_b.render(entity, 0.0F, 0.0F, 0.0F, f, f1, f3);
        GlStateManager.enableBlend();
        GlStateManager.color(1.0F, 1.0F, 1.0F, 0.5F);
        GlStateManager.scale(1.5F, 1.5F, 1.5F);
        this.field_188349_b.render(entity, 0.0F, 0.0F, 0.0F, f, f1, f3);
        GlStateManager.disableBlend();
        GlStateManager.popMatrix();
        super.doRender(entity, x, y, z, entityYaw, partialTicks);
    }

    /**
     * Returns the location of an entity's texture. Doesn't seem to be called unless you call Render.bindEntityTexture.
     */
    protected ResourceLocation getEntityTexture(EntityShulkerBullet entity)
    {
        return field_188348_a;
    }
}
