package net.minecraft.client.particle;

import net.minecraft.block.BlockLiquid;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityRainFX extends EntityFX
{
    protected EntityRainFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.field_187129_i *= 0.30000001192092896D;
        this.field_187130_j = Math.random() * 0.20000000298023224D + 0.10000000149011612D;
        this.field_187131_k *= 0.30000001192092896D;
        this.particleRed = 1.0F;
        this.particleGreen = 1.0F;
        this.particleBlue = 1.0F;
        this.setParticleTextureIndex(19 + this.field_187136_p.nextInt(4));
        this.func_187115_a(0.01F, 0.01F);
        this.particleGravity = 0.06F;
        this.particleMaxAge = (int)(8.0D / (Math.random() * 0.8D + 0.2D));
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

        if (this.particleMaxAge-- <= 0)
        {
            this.func_187112_i();
        }

        if (this.field_187132_l)
        {
            if (Math.random() < 0.5D)
            {
                this.func_187112_i();
            }

            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }

        BlockPos blockpos = new BlockPos(this.field_187126_f, this.field_187127_g, this.field_187128_h);
        IBlockState iblockstate = this.field_187122_b.getBlockState(blockpos);
        Material material = iblockstate.getMaterial();

        if (material.isLiquid() || material.isSolid())
        {
            double d0 = 0.0D;

            if (iblockstate.getBlock() instanceof BlockLiquid)
            {
                d0 = (double)(1.0F - BlockLiquid.getLiquidHeightPercent(((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue()));
            }
            else
            {
                d0 = iblockstate.func_185900_c(this.field_187122_b, blockpos).maxY;
            }

            double d1 = (double)MathHelper.floor_double(this.field_187127_g) + d0;

            if (this.field_187127_g < d1)
            {
                this.func_187112_i();
            }
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityRainFX(worldIn, xCoordIn, yCoordIn, zCoordIn);
        }
    }
}
