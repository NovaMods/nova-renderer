package net.minecraft.client.particle;

import net.minecraft.world.World;

public class EntityEnchantmentTableParticleFX extends EntityFX
{
    private float field_70565_a;
    private double coordX;
    private double coordY;
    private double coordZ;

    protected EntityEnchantmentTableParticleFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        this.field_187129_i = xSpeedIn;
        this.field_187130_j = ySpeedIn;
        this.field_187131_k = zSpeedIn;
        this.coordX = xCoordIn;
        this.coordY = yCoordIn;
        this.coordZ = zCoordIn;
        this.field_187126_f = this.field_187123_c = xCoordIn + xSpeedIn;
        this.field_187127_g = this.field_187124_d = yCoordIn + ySpeedIn;
        this.field_187128_h = this.field_187125_e = zCoordIn + zSpeedIn;
        float f = this.field_187136_p.nextFloat() * 0.6F + 0.4F;
        this.field_70565_a = this.particleScale = this.field_187136_p.nextFloat() * 0.5F + 0.2F;
        this.particleRed = this.particleGreen = this.particleBlue = 1.0F * f;
        this.particleGreen *= 0.9F;
        this.particleRed *= 0.9F;
        this.particleMaxAge = (int)(Math.random() * 10.0D) + 30;
        this.setParticleTextureIndex((int)(Math.random() * 26.0D + 1.0D + 224.0D));
    }

    public void func_187110_a(double p_187110_1_, double p_187110_3_, double p_187110_5_)
    {
        this.func_187108_a(this.func_187116_l().offset(p_187110_1_, p_187110_3_, p_187110_5_));
        this.func_187118_j();
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
        f = 1.0F - f;
        float f1 = 1.0F - f;
        f1 = f1 * f1;
        f1 = f1 * f1;
        this.field_187126_f = this.coordX + this.field_187129_i * (double)f;
        this.field_187127_g = this.coordY + this.field_187130_j * (double)f - (double)(f1 * 1.2F);
        this.field_187128_h = this.coordZ + this.field_187131_k * (double)f;

        if (this.particleAge++ >= this.particleMaxAge)
        {
            this.func_187112_i();
        }
    }

    public static class EnchantmentTable implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityEnchantmentTableParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
