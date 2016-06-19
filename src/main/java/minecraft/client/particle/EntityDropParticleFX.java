package net.minecraft.client.particle;

import net.minecraft.block.BlockLiquid;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityDropParticleFX extends EntityFX
{
    /** the material type for dropped items/blocks */
    private Material materialType;

    /** The height of the current bob */
    private int bobTimer;

    protected EntityDropParticleFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, Material p_i1203_8_)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.field_187129_i = this.field_187130_j = this.field_187131_k = 0.0D;

        if (p_i1203_8_ == Material.water)
        {
            this.particleRed = 0.0F;
            this.particleGreen = 0.0F;
            this.particleBlue = 1.0F;
        }
        else
        {
            this.particleRed = 1.0F;
            this.particleGreen = 0.0F;
            this.particleBlue = 0.0F;
        }

        this.setParticleTextureIndex(113);
        this.func_187115_a(0.01F, 0.01F);
        this.particleGravity = 0.06F;
        this.materialType = p_i1203_8_;
        this.bobTimer = 40;
        this.particleMaxAge = (int)(64.0D / (Math.random() * 0.8D + 0.2D));
        this.field_187129_i = this.field_187130_j = this.field_187131_k = 0.0D;
    }

    public int func_189214_a(float p_189214_1_)
    {
        return this.materialType == Material.water ? super.func_189214_a(p_189214_1_) : 257;
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;

        if (this.materialType == Material.water)
        {
            this.particleRed = 0.2F;
            this.particleGreen = 0.3F;
            this.particleBlue = 1.0F;
        }
        else
        {
            this.particleRed = 1.0F;
            this.particleGreen = 16.0F / (float)(40 - this.bobTimer + 16);
            this.particleBlue = 4.0F / (float)(40 - this.bobTimer + 8);
        }

        this.field_187130_j -= (double)this.particleGravity;

        if (this.bobTimer-- > 0)
        {
            this.field_187129_i *= 0.02D;
            this.field_187130_j *= 0.02D;
            this.field_187131_k *= 0.02D;
            this.setParticleTextureIndex(113);
        }
        else
        {
            this.setParticleTextureIndex(112);
        }

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
            if (this.materialType == Material.water)
            {
                this.func_187112_i();
                this.field_187122_b.spawnParticle(EnumParticleTypes.WATER_SPLASH, this.field_187126_f, this.field_187127_g, this.field_187128_h, 0.0D, 0.0D, 0.0D, new int[0]);
            }
            else
            {
                this.setParticleTextureIndex(114);
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
                d0 = (double)BlockLiquid.getLiquidHeightPercent(((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue());
            }

            double d1 = (double)(MathHelper.floor_double(this.field_187127_g) + 1) - d0;

            if (this.field_187127_g < d1)
            {
                this.func_187112_i();
            }
        }
    }

    public static class LavaFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityDropParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, Material.lava);
        }
    }

    public static class WaterFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityDropParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, Material.water);
        }
    }
}
