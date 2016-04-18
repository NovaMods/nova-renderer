package net.minecraft.client.particle;

import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.Tessellator;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.entity.Entity;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class EntityFootStepFX extends EntityFX
{
    private static final ResourceLocation FOOTPRINT_TEXTURE = new ResourceLocation("textures/particle/footprint.png");
    private int footstepAge;
    private int footstepMaxAge;
    private TextureManager currentFootSteps;

    protected EntityFootStepFX(TextureManager currentFootStepsIn, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.currentFootSteps = currentFootStepsIn;
        this.field_187129_i = this.field_187130_j = this.field_187131_k = 0.0D;
        this.footstepMaxAge = 200;
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.footstepAge + partialTicks) / (float)this.footstepMaxAge;
        f = f * f;
        float f1 = 2.0F - f * 2.0F;

        if (f1 > 1.0F)
        {
            f1 = 1.0F;
        }

        f1 = f1 * 0.2F;
        GlStateManager.disableLighting();
        float f2 = 0.125F;
        float f3 = (float)(this.field_187126_f - interpPosX);
        float f4 = (float)(this.field_187127_g - interpPosY);
        float f5 = (float)(this.field_187128_h - interpPosZ);
        float f6 = this.field_187122_b.getLightBrightness(new BlockPos(this.field_187126_f, this.field_187127_g, this.field_187128_h));
        this.currentFootSteps.bindTexture(FOOTPRINT_TEXTURE);
        GlStateManager.enableBlend();
        GlStateManager.blendFunc(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA);
        worldRendererIn.begin(7, DefaultVertexFormats.POSITION_TEX_COLOR);
        worldRendererIn.pos((double)(f3 - 0.125F), (double)f4, (double)(f5 + 0.125F)).tex(0.0D, 1.0D).color(f6, f6, f6, f1).endVertex();
        worldRendererIn.pos((double)(f3 + 0.125F), (double)f4, (double)(f5 + 0.125F)).tex(1.0D, 1.0D).color(f6, f6, f6, f1).endVertex();
        worldRendererIn.pos((double)(f3 + 0.125F), (double)f4, (double)(f5 - 0.125F)).tex(1.0D, 0.0D).color(f6, f6, f6, f1).endVertex();
        worldRendererIn.pos((double)(f3 - 0.125F), (double)f4, (double)(f5 - 0.125F)).tex(0.0D, 0.0D).color(f6, f6, f6, f1).endVertex();
        Tessellator.getInstance().draw();
        GlStateManager.disableBlend();
        GlStateManager.enableLighting();
    }

    public void func_189213_a()
    {
        ++this.footstepAge;

        if (this.footstepAge == this.footstepMaxAge)
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
            return new EntityFootStepFX(Minecraft.getMinecraft().getTextureManager(), worldIn, xCoordIn, yCoordIn, zCoordIn);
        }
    }
}
