package net.minecraft.client.particle;

import net.minecraft.world.World;

public class EntityAuraFX extends EntityFX
{
    protected EntityAuraFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double speedIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, speedIn);
        float f = this.field_187136_p.nextFloat() * 0.1F + 0.2F;
        this.particleRed = f;
        this.particleGreen = f;
        this.particleBlue = f;
        this.setParticleTextureIndex(0);
        this.func_187115_a(0.02F, 0.02F);
        this.particleScale *= this.field_187136_p.nextFloat() * 0.6F + 0.5F;
        this.field_187129_i *= 0.019999999552965164D;
        this.field_187130_j *= 0.019999999552965164D;
        this.field_187131_k *= 0.019999999552965164D;
        this.particleMaxAge = (int)(20.0D / (Math.random() * 0.8D + 0.2D));
    }

    public void func_187110_a(double p_187110_1_, double p_187110_3_, double p_187110_5_)
    {
        this.func_187108_a(this.func_187116_l().offset(p_187110_1_, p_187110_3_, p_187110_5_));
        this.func_187118_j();
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.99D;
        this.field_187130_j *= 0.99D;
        this.field_187131_k *= 0.99D;

        if (this.particleMaxAge-- <= 0)
        {
            this.func_187112_i();
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityAuraFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }

    public static class HappyVillagerFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntityAuraFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
            entityfx.setParticleTextureIndex(82);
            entityfx.setRBGColorF(1.0F, 1.0F, 1.0F);
            return entityfx;
        }
    }
}
