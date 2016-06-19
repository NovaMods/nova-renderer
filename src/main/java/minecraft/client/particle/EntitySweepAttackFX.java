package net.minecraft.client.particle;

import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.RenderHelper;
import net.minecraft.client.renderer.Tessellator;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.client.renderer.vertex.VertexFormat;
import net.minecraft.entity.Entity;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.World;

public class EntitySweepAttackFX extends EntityFX
{
    private static final ResourceLocation field_187137_a = new ResourceLocation("textures/entity/sweep.png");
    private static final VertexFormat field_187138_G = (new VertexFormat()).addElement(DefaultVertexFormats.POSITION_3F).addElement(DefaultVertexFormats.TEX_2F).addElement(DefaultVertexFormats.COLOR_4UB).addElement(DefaultVertexFormats.TEX_2S).addElement(DefaultVertexFormats.NORMAL_3B).addElement(DefaultVertexFormats.PADDING_1B);
    private int field_187139_H;
    private int field_187140_I;
    private TextureManager field_187141_J;
    private float field_187142_K;

    protected EntitySweepAttackFX(TextureManager p_i46582_1_, World p_i46582_2_, double p_i46582_3_, double p_i46582_5_, double p_i46582_7_, double p_i46582_9_, double p_i46582_11_, double p_i46582_13_)
    {
        super(p_i46582_2_, p_i46582_3_, p_i46582_5_, p_i46582_7_, 0.0D, 0.0D, 0.0D);
        this.field_187141_J = p_i46582_1_;
        this.field_187140_I = 4;
        this.particleRed = this.particleGreen = this.particleBlue = this.field_187136_p.nextFloat() * 0.6F + 0.4F;
        this.field_187142_K = 1.0F - (float)p_i46582_9_ * 0.5F;
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        int i = (int)(((float)this.field_187139_H + partialTicks) * 3.0F / (float)this.field_187140_I);

        if (i <= 7)
        {
            this.field_187141_J.bindTexture(field_187137_a);
            float f = (float)(i % 4) / 4.0F;
            float f1 = f + 0.24975F;
            float f2 = (float)(i / 2) / 2.0F;
            float f3 = f2 + 0.4995F;
            float f4 = 1.0F * this.field_187142_K;
            float f5 = (float)(this.field_187123_c + (this.field_187126_f - this.field_187123_c) * (double)partialTicks - interpPosX);
            float f6 = (float)(this.field_187124_d + (this.field_187127_g - this.field_187124_d) * (double)partialTicks - interpPosY);
            float f7 = (float)(this.field_187125_e + (this.field_187128_h - this.field_187125_e) * (double)partialTicks - interpPosZ);
            GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
            GlStateManager.disableLighting();
            RenderHelper.disableStandardItemLighting();
            worldRendererIn.begin(7, field_187138_G);
            worldRendererIn.pos((double)(f5 - rotationX * f4 - rotationXY * f4), (double)(f6 - rotationZ * f4 * 0.5F), (double)(f7 - rotationYZ * f4 - rotationXZ * f4)).tex((double)f1, (double)f3).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(0, 240).normal(0.0F, 1.0F, 0.0F).endVertex();
            worldRendererIn.pos((double)(f5 - rotationX * f4 + rotationXY * f4), (double)(f6 + rotationZ * f4 * 0.5F), (double)(f7 - rotationYZ * f4 + rotationXZ * f4)).tex((double)f1, (double)f2).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(0, 240).normal(0.0F, 1.0F, 0.0F).endVertex();
            worldRendererIn.pos((double)(f5 + rotationX * f4 + rotationXY * f4), (double)(f6 + rotationZ * f4 * 0.5F), (double)(f7 + rotationYZ * f4 + rotationXZ * f4)).tex((double)f, (double)f2).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(0, 240).normal(0.0F, 1.0F, 0.0F).endVertex();
            worldRendererIn.pos((double)(f5 + rotationX * f4 - rotationXY * f4), (double)(f6 - rotationZ * f4 * 0.5F), (double)(f7 + rotationYZ * f4 - rotationXZ * f4)).tex((double)f, (double)f3).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(0, 240).normal(0.0F, 1.0F, 0.0F).endVertex();
            Tessellator.getInstance().draw();
            GlStateManager.enableLighting();
        }
    }

    public int func_189214_a(float p_189214_1_)
    {
        return 61680;
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;
        ++this.field_187139_H;

        if (this.field_187139_H == this.field_187140_I)
        {
            this.func_187112_i();
        }
    }

    public int getFXLayer()
    {
        return 3;
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntitySweepAttackFX(Minecraft.getMinecraft().getTextureManager(), worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
