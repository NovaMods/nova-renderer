package net.minecraft.client.renderer.tileentity;

import net.minecraft.client.model.ModelBase;
import net.minecraft.client.model.ModelEnderCrystal;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.culling.ICamera;
import net.minecraft.client.renderer.entity.Render;
import net.minecraft.client.renderer.entity.RenderDragon;
import net.minecraft.client.renderer.entity.RenderManager;
import net.minecraft.entity.item.EntityEnderCrystal;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;

public class RenderEnderCrystal extends Render<EntityEnderCrystal>
{
    private static final ResourceLocation enderCrystalTextures = new ResourceLocation("textures/entity/endercrystal/endercrystal.png");
    private final ModelBase modelEnderCrystal = new ModelEnderCrystal(0.0F, true);
    private final ModelBase field_188316_g = new ModelEnderCrystal(0.0F, false);

    public RenderEnderCrystal(RenderManager renderManagerIn)
    {
        super(renderManagerIn);
        this.shadowSize = 0.5F;
    }

    /**
     * Renders the desired {@code T} type Entity.
     */
    public void doRender(EntityEnderCrystal entity, double x, double y, double z, float entityYaw, float partialTicks)
    {
        float f = (float)entity.innerRotation + partialTicks;
        GlStateManager.pushMatrix();
        GlStateManager.translate((float)x, (float)y, (float)z);
        this.bindTexture(enderCrystalTextures);
        float f1 = MathHelper.sin(f * 0.2F) / 2.0F + 0.5F;
        f1 = f1 * f1 + f1;

        if (this.field_188301_f)
        {
            GlStateManager.enableColorMaterial();
            GlStateManager.func_187431_e(this.func_188298_c(entity));
        }

        if (entity.shouldShowBottom())
        {
            this.modelEnderCrystal.render(entity, 0.0F, f * 3.0F, f1 * 0.2F, 0.0F, 0.0F, 0.0625F);
        }
        else
        {
            this.field_188316_g.render(entity, 0.0F, f * 3.0F, f1 * 0.2F, 0.0F, 0.0F, 0.0625F);
        }

        if (this.field_188301_f)
        {
            GlStateManager.func_187417_n();
            GlStateManager.disableColorMaterial();
        }

        GlStateManager.popMatrix();
        BlockPos blockpos = entity.getBeamTarget();

        if (blockpos != null)
        {
            this.bindTexture(RenderDragon.enderDragonCrystalBeamTextures);
            float f2 = (float)blockpos.getX() + 0.5F;
            float f3 = (float)blockpos.getY() + 0.5F;
            float f4 = (float)blockpos.getZ() + 0.5F;
            RenderDragon.func_188325_a(x, y - 1.2999999523162842D + (double)(f1 * 0.4F), z, partialTicks, entity.posX, entity.posY, entity.posZ, entity.innerRotation, (double)f2, (double)f3, (double)f4);
        }

        super.doRender(entity, x, y, z, entityYaw, partialTicks);
    }

    /**
     * Returns the location of an entity's texture. Doesn't seem to be called unless you call Render.bindEntityTexture.
     */
    protected ResourceLocation getEntityTexture(EntityEnderCrystal entity)
    {
        return enderCrystalTextures;
    }

    public boolean shouldRender(EntityEnderCrystal livingEntity, ICamera camera, double camX, double camY, double camZ)
    {
        return super.shouldRender(livingEntity, camera, camX, camY, camZ) || livingEntity.getBeamTarget() != null;
    }
}
