package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.world.World;

public class EntityPortalFX extends EntityFX
{
    private float portalParticleScale;
    private double portalPosX;
    private double portalPosY;
    private double portalPosZ;

    protected EntityPortalFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        this.field_187129_i = xSpeedIn;
        this.field_187130_j = ySpeedIn;
        this.field_187131_k = zSpeedIn;
        this.portalPosX = this.field_187126_f = xCoordIn;
        this.portalPosY = this.field_187127_g = yCoordIn;
        this.portalPosZ = this.field_187128_h = zCoordIn;
        float f = this.field_187136_p.nextFloat() * 0.6F + 0.4F;
        this.portalParticleScale = this.particleScale = this.field_187136_p.nextFloat() * 0.2F + 0.5F;
        this.particleRed = this.particleGreen = this.particleBlue = 1.0F * f;
        this.particleGreen *= 0.3F;
        this.particleRed *= 0.9F;
        this.particleMaxAge = (int)(Math.random() * 10.0D) + 40;
        this.setParticleTextureIndex((int)(Math.random() * 8.0D));
    }

    public void func_187110_a(double p_187110_1_, double p_187110_3_, double p_187110_5_)
    {
        this.func_187108_a(this.func_187116_l().offset(p_187110_1_, p_187110_3_, p_187110_5_));
        this.func_187118_j();
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge;
        f = 1.0F - f;
        f = f * f;
        f = 1.0F - f;
        this.particleScale = this.portalParticleScale * f;
        super.renderParticle(worldRendererIn, entityIn, partialTicks, rotationX, rotationZ, rotationYZ, rotationXY, rotationXZ);
    }

    public int func_189214_a(float p_189214_1_)
    {
        int i = super.func_189214_a(p_189214_1_);
        float f = (float)this.particleAge / (float)this.particleMaxAge;
        f = f * f;
        f = f * f;
        int j = i & 255;
        int k = i >> 16 & 255;
        k = k + (int)(f * 15.0F * 16.0F);

        if (k > 240)
        {
            k = 240;
        }

        return j | k << 16;
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;
        float f = (float)this.particleAge / (float)this.particleMaxAge;
        float f1 = -f + f * f * 2.0F;
        float f2 = 1.0F - f1;
        this.field_187126_f = this.portalPosX + this.field_187129_i * (double)f2;
        this.field_187127_g = this.portalPosY + this.field_187130_j * (double)f2 + (double)(1.0F - f);
        this.field_187128_h = this.portalPosZ + this.field_187131_k * (double)f2;

        if (this.particleAge++ >= this.particleMaxAge)
        {
            this.func_187112_i();
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityPortalFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
