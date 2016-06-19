package net.minecraft.client.particle;

import net.minecraft.world.World;

public class EntityFishWakeFX extends EntityFX
{
    protected EntityFishWakeFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i45073_8_, double p_i45073_10_, double p_i45073_12_)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.field_187129_i *= 0.30000001192092896D;
        this.field_187130_j = Math.random() * 0.20000000298023224D + 0.10000000149011612D;
        this.field_187131_k *= 0.30000001192092896D;
        this.particleRed = 1.0F;
        this.particleGreen = 1.0F;
        this.particleBlue = 1.0F;
        this.setParticleTextureIndex(19);
        this.func_187115_a(0.01F, 0.01F);
        this.particleMaxAge = (int)(8.0D / (Math.random() * 0.8D + 0.2D));
        this.particleGravity = 0.0F;
        this.field_187129_i = p_i45073_8_;
        this.field_187130_j = p_i45073_10_;
        this.field_187131_k = p_i45073_12_;
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;
        this.field_187130_j -= (double)this.particleGravity;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.9800000190734863D;
        this.field_187130_j *= 0.9800000190734863D;
        this.field_187131_k *= 0.9800000190734863D;
        int i = 60 - this.particleMaxAge;
        float f = (float)i * 0.001F;
        this.func_187115_a(f, f);
        this.setParticleTextureIndex(19 + i % 4);

        if (this.particleMaxAge-- <= 0)
        {
            this.func_187112_i();
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityFishWakeFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
