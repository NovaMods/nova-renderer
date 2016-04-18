package net.minecraft.client.renderer.entity;

import net.minecraft.client.model.ModelRenderer;
import net.minecraft.client.model.ModelShulker;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.culling.ICamera;
import net.minecraft.client.renderer.entity.layers.LayerRenderer;
import net.minecraft.entity.monster.EntityShulker;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;

public class RenderShulker extends RenderLiving<EntityShulker>
{
    private static final ResourceLocation field_188342_a = new ResourceLocation("textures/entity/shulker/endergolem.png");
    private int field_188343_b;

    public RenderShulker(RenderManager p_i46550_1_, ModelShulker p_i46550_2_)
    {
        super(p_i46550_1_, p_i46550_2_, 0.0F);
        this.addLayer(new RenderShulker.HeadLayer());
        this.field_188343_b = p_i46550_2_.func_187065_a();
        this.shadowSize = 0.0F;
    }

    /**
     * Renders the desired {@code T} type Entity.
     */
    public void doRender(EntityShulker entity, double x, double y, double z, float entityYaw, float partialTicks)
    {
        if (this.field_188343_b != ((ModelShulker)this.mainModel).func_187065_a())
        {
            this.mainModel = new ModelShulker();
            this.field_188343_b = ((ModelShulker)this.mainModel).func_187065_a();
        }

        int i = entity.func_184693_dc();

        if (i > 0 && entity.func_184697_de())
        {
            BlockPos blockpos = entity.func_184699_da();
            BlockPos blockpos1 = entity.func_184692_dd();
            double d0 = (double)((float)i - partialTicks) / 6.0D;
            d0 = d0 * d0;
            double d1 = (double)(blockpos.getX() - blockpos1.getX()) * d0;
            double d2 = (double)(blockpos.getY() - blockpos1.getY()) * d0;
            double d3 = (double)(blockpos.getZ() - blockpos1.getZ()) * d0;
            super.doRender(entity, x - d1, y - d2, z - d3, entityYaw, partialTicks);
        }
        else
        {
            super.doRender(entity, x, y, z, entityYaw, partialTicks);
        }
    }

    public boolean shouldRender(EntityShulker livingEntity, ICamera camera, double camX, double camY, double camZ)
    {
        if (super.shouldRender(livingEntity, camera, camX, camY, camZ))
        {
            return true;
        }
        else
        {
            if (livingEntity.func_184693_dc() > 0 && livingEntity.func_184697_de())
            {
                BlockPos blockpos = livingEntity.func_184692_dd();
                BlockPos blockpos1 = livingEntity.func_184699_da();
                Vec3d vec3d = new Vec3d((double)blockpos1.getX(), (double)blockpos1.getY(), (double)blockpos1.getZ());
                Vec3d vec3d1 = new Vec3d((double)blockpos.getX(), (double)blockpos.getY(), (double)blockpos.getZ());

                if (camera.isBoundingBoxInFrustum(new AxisAlignedBB(vec3d1.xCoord, vec3d1.yCoord, vec3d1.zCoord, vec3d.xCoord, vec3d.yCoord, vec3d.zCoord)))
                {
                    return true;
                }
            }

            return false;
        }
    }

    /**
     * Returns the location of an entity's texture. Doesn't seem to be called unless you call Render.bindEntityTexture.
     */
    protected ResourceLocation getEntityTexture(EntityShulker entity)
    {
        return field_188342_a;
    }

    protected void rotateCorpse(EntityShulker bat, float p_77043_2_, float p_77043_3_, float partialTicks)
    {
        super.rotateCorpse(bat, p_77043_2_, p_77043_3_, partialTicks);

        switch (bat.func_184696_cZ())
        {
            case DOWN:
            default:
                break;

            case EAST:
                GlStateManager.translate(0.5F, 0.5F, 0.0F);
                GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                GlStateManager.rotate(90.0F, 0.0F, 0.0F, 1.0F);
                break;

            case WEST:
                GlStateManager.translate(-0.5F, 0.5F, 0.0F);
                GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                GlStateManager.rotate(-90.0F, 0.0F, 0.0F, 1.0F);
                break;

            case NORTH:
                GlStateManager.translate(0.0F, 0.5F, -0.5F);
                GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                break;

            case SOUTH:
                GlStateManager.translate(0.0F, 0.5F, 0.5F);
                GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                GlStateManager.rotate(180.0F, 0.0F, 0.0F, 1.0F);
                break;

            case UP:
                GlStateManager.translate(0.0F, 1.0F, 0.0F);
                GlStateManager.rotate(180.0F, 1.0F, 0.0F, 0.0F);
        }
    }

    /**
     * Allows the render to do any OpenGL state modifications necessary before the model is rendered. Args:
     * entityLiving, partialTickTime
     */
    protected void preRenderCallback(EntityShulker entitylivingbaseIn, float partialTickTime)
    {
        float f = 0.999F;
        GlStateManager.scale(0.999F, 0.999F, 0.999F);
    }

    class HeadLayer implements LayerRenderer<EntityShulker>
    {
        private HeadLayer()
        {
        }

        public void doRenderLayer(EntityShulker entitylivingbaseIn, float p_177141_2_, float p_177141_3_, float partialTicks, float p_177141_5_, float p_177141_6_, float p_177141_7_, float scale)
        {
            GlStateManager.pushMatrix();

            switch (entitylivingbaseIn.func_184696_cZ())
            {
                case DOWN:
                default:
                    break;

                case EAST:
                    GlStateManager.rotate(90.0F, 0.0F, 0.0F, 1.0F);
                    GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                    GlStateManager.translate(1.0F, -1.0F, 0.0F);
                    GlStateManager.rotate(180.0F, 0.0F, 1.0F, 0.0F);
                    break;

                case WEST:
                    GlStateManager.rotate(-90.0F, 0.0F, 0.0F, 1.0F);
                    GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                    GlStateManager.translate(-1.0F, -1.0F, 0.0F);
                    GlStateManager.rotate(180.0F, 0.0F, 1.0F, 0.0F);
                    break;

                case NORTH:
                    GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                    GlStateManager.translate(0.0F, -1.0F, -1.0F);
                    break;

                case SOUTH:
                    GlStateManager.rotate(180.0F, 0.0F, 0.0F, 1.0F);
                    GlStateManager.rotate(90.0F, 1.0F, 0.0F, 0.0F);
                    GlStateManager.translate(0.0F, -1.0F, 1.0F);
                    break;

                case UP:
                    GlStateManager.rotate(180.0F, 1.0F, 0.0F, 0.0F);
                    GlStateManager.translate(0.0F, -2.0F, 0.0F);
            }

            ModelRenderer modelrenderer = ((ModelShulker)RenderShulker.this.getMainModel()).field_187066_a;
            modelrenderer.rotateAngleY = p_177141_6_ * 0.017453292F;
            modelrenderer.rotateAngleX = p_177141_7_ * 0.017453292F;
            RenderShulker.this.bindTexture(RenderShulker.field_188342_a);
            modelrenderer.render(scale);
            GlStateManager.popMatrix();
        }

        public boolean shouldCombineTextures()
        {
            return false;
        }
    }
}
