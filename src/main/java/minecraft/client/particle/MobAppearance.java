package net.minecraft.client.particle;

import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.OpenGlHelper;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.entity.RenderManager;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.monster.EntityGuardian;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class MobAppearance extends EntityFX
{
    private EntityLivingBase entity;

    protected MobAppearance(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.particleRed = this.particleGreen = this.particleBlue = 1.0F;
        this.field_187129_i = this.field_187130_j = this.field_187131_k = 0.0D;
        this.particleGravity = 0.0F;
        this.particleMaxAge = 30;
    }

    public int getFXLayer()
    {
        return 3;
    }

    public void func_189213_a()
    {
        super.func_189213_a();

        if (this.entity == null)
        {
            EntityGuardian entityguardian = new EntityGuardian(this.field_187122_b);
            entityguardian.setElder();
            this.entity = entityguardian;
        }
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        if (this.entity != null)
        {
            RenderManager rendermanager = Minecraft.getMinecraft().getRenderManager();
            rendermanager.setRenderPosition(EntityFX.interpPosX, EntityFX.interpPosY, EntityFX.interpPosZ);
            float f = 0.42553192F;
            float f1 = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge;
            GlStateManager.depthMask(true);
            GlStateManager.enableBlend();
            GlStateManager.enableDepth();
            GlStateManager.blendFunc(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA);
            float f2 = 240.0F;
            OpenGlHelper.setLightmapTextureCoords(OpenGlHelper.lightmapTexUnit, f2, f2);
            GlStateManager.pushMatrix();
            float f3 = 0.05F + 0.5F * MathHelper.sin(f1 * (float)Math.PI);
            GlStateManager.color(1.0F, 1.0F, 1.0F, f3);
            GlStateManager.translate(0.0F, 1.8F, 0.0F);
            GlStateManager.rotate(180.0F - entityIn.rotationYaw, 0.0F, 1.0F, 0.0F);
            GlStateManager.rotate(60.0F - 150.0F * f1 - entityIn.rotationPitch, 1.0F, 0.0F, 0.0F);
            GlStateManager.translate(0.0F, -0.4F, -1.5F);
            GlStateManager.scale(f, f, f);
            this.entity.rotationYaw = this.entity.prevRotationYaw = 0.0F;
            this.entity.rotationYawHead = this.entity.prevRotationYawHead = 0.0F;
            rendermanager.func_188391_a(this.entity, 0.0D, 0.0D, 0.0D, 0.0F, partialTicks, false);
            GlStateManager.popMatrix();
            GlStateManager.enableDepth();
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new MobAppearance(worldIn, xCoordIn, yCoordIn, zCoordIn);
        }
    }
}
