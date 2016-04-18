package net.minecraft.client.particle;

import net.minecraft.block.material.Material;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class EntitySuspendFX extends EntityFX
{
    protected EntitySuspendFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn)
    {
        super(worldIn, xCoordIn, yCoordIn - 0.125D, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        this.particleRed = 0.4F;
        this.particleGreen = 0.4F;
        this.particleBlue = 0.7F;
        this.setParticleTextureIndex(0);
        this.func_187115_a(0.01F, 0.01F);
        this.particleScale *= this.field_187136_p.nextFloat() * 0.6F + 0.2F;
        this.field_187129_i = xSpeedIn * 0.0D;
        this.field_187130_j = ySpeedIn * 0.0D;
        this.field_187131_k = zSpeedIn * 0.0D;
        this.particleMaxAge = (int)(16.0D / (Math.random() * 0.8D + 0.2D));
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);

        if (this.field_187122_b.getBlockState(new BlockPos(this.field_187126_f, this.field_187127_g, this.field_187128_h)).getMaterial() != Material.water)
        {
            this.func_187112_i();
        }

        if (this.particleMaxAge-- <= 0)
        {
            this.func_187112_i();
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntitySuspendFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
