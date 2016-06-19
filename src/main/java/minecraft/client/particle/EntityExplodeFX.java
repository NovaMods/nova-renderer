package net.minecraft.client.particle;

import net.minecraft.world.World;

public class EntityExplodeFX extends EntityFX
{
    protected EntityExplodeFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        this.field_187129_i = xSpeedIn + (Math.random() * 2.0D - 1.0D) * 0.05000000074505806D;
        this.field_187130_j = ySpeedIn + (Math.random() * 2.0D - 1.0D) * 0.05000000074505806D;
        this.field_187131_k = zSpeedIn + (Math.random() * 2.0D - 1.0D) * 0.05000000074505806D;
        this.particleRed = this.particleGreen = this.particleBlue = this.field_187136_p.nextFloat() * 0.3F + 0.7F;
        this.particleScale = this.field_187136_p.nextFloat() * this.field_187136_p.nextFloat() * 6.0F + 1.0F;
        this.particleMaxAge = (int)(16.0D / ((double)this.field_187136_p.nextFloat() * 0.8D + 0.2D)) + 2;
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;

        if (this.particleAge++ >= this.particleMaxAge)
        {
            this.func_187112_i();
        }

        this.setParticleTextureIndex(7 - this.particleAge * 8 / this.particleMaxAge);
        this.field_187130_j += 0.004D;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.8999999761581421D;
        this.field_187130_j *= 0.8999999761581421D;
        this.field_187131_k *= 0.8999999761581421D;

        if (this.field_187132_l)
        {
            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityExplodeFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
