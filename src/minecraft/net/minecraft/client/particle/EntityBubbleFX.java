package net.minecraft.client.particle;

import net.minecraft.block.material.Material;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class EntityBubbleFX extends EntityFX
{
    protected EntityBubbleFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        this.particleRed = 1.0F;
        this.particleGreen = 1.0F;
        this.particleBlue = 1.0F;
        this.setParticleTextureIndex(32);
        this.func_187115_a(0.02F, 0.02F);
        this.particleScale *= this.field_187136_p.nextFloat() * 0.6F + 0.2F;
        this.field_187129_i = xSpeedIn * 0.20000000298023224D + (Math.random() * 2.0D - 1.0D) * 0.019999999552965164D;
        this.field_187130_j = ySpeedIn * 0.20000000298023224D + (Math.random() * 2.0D - 1.0D) * 0.019999999552965164D;
        this.field_187131_k = zSpeedIn * 0.20000000298023224D + (Math.random() * 2.0D - 1.0D) * 0.019999999552965164D;
        this.particleMaxAge = (int)(8.0D / (Math.random() * 0.8D + 0.2D));
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;
        this.field_187130_j += 0.002D;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.8500000238418579D;
        this.field_187130_j *= 0.8500000238418579D;
        this.field_187131_k *= 0.8500000238418579D;

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
            return new EntityBubbleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
