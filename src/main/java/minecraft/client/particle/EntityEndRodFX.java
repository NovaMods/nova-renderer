package net.minecraft.client.particle;

import net.minecraft.world.World;

public class EntityEndRodFX extends EntityAnimatedFX
{
    public EntityEndRodFX(World p_i46580_1_, double p_i46580_2_, double p_i46580_4_, double p_i46580_6_, double p_i46580_8_, double p_i46580_10_, double p_i46580_12_)
    {
        super(p_i46580_1_, p_i46580_2_, p_i46580_4_, p_i46580_6_, 176, 8, -5.0E-4F);
        this.field_187129_i = p_i46580_8_;
        this.field_187130_j = p_i46580_10_;
        this.field_187131_k = p_i46580_12_;
        this.particleScale *= 0.75F;
        this.particleMaxAge = 60 + this.field_187136_p.nextInt(12);
        this.func_187145_d(15916745);
    }

    public void func_187110_a(double p_187110_1_, double p_187110_3_, double p_187110_5_)
    {
        this.func_187108_a(this.func_187116_l().offset(p_187110_1_, p_187110_3_, p_187110_5_));
        this.func_187118_j();
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityEndRodFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
