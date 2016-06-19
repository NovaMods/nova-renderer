package net.minecraft.pathfinding;

import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.EntityLiving;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.IBlockAccess;

public class SwimNodeProcessor extends NodeProcessor
{
    public void func_186315_a(IBlockAccess p_186315_1_, EntityLiving p_186315_2_)
    {
        super.func_186315_a(p_186315_1_, p_186315_2_);
    }

    /**
     * This method is called when all nodes have been processed and PathEntity is created.
     *  {@link net.minecraft.world.pathfinder.WalkNodeProcessor WalkNodeProcessor} uses this to change its field {@link
     * net.minecraft.world.pathfinder.WalkNodeProcessor#avoidsWater avoidsWater}
     */
    public void postProcess()
    {
        super.postProcess();
    }

    public PathPoint func_186318_b()
    {
        return this.openPoint(MathHelper.floor_double(this.field_186326_b.getEntityBoundingBox().minX), MathHelper.floor_double(this.field_186326_b.getEntityBoundingBox().minY + 0.5D), MathHelper.floor_double(this.field_186326_b.getEntityBoundingBox().minZ));
    }

    public PathPoint func_186325_a(double p_186325_1_, double p_186325_3_, double p_186325_5_)
    {
        return this.openPoint(MathHelper.floor_double(p_186325_1_ - (double)(this.field_186326_b.width / 2.0F)), MathHelper.floor_double(p_186325_3_ + 0.5D), MathHelper.floor_double(p_186325_5_ - (double)(this.field_186326_b.width / 2.0F)));
    }

    public int func_186320_a(PathPoint[] p_186320_1_, PathPoint p_186320_2_, PathPoint p_186320_3_, float p_186320_4_)
    {
        int i = 0;

        for (EnumFacing enumfacing : EnumFacing.values())
        {
            PathPoint pathpoint = this.func_186328_b(p_186320_2_.xCoord + enumfacing.getFrontOffsetX(), p_186320_2_.yCoord + enumfacing.getFrontOffsetY(), p_186320_2_.zCoord + enumfacing.getFrontOffsetZ());

            if (pathpoint != null && !pathpoint.visited && pathpoint.distanceTo(p_186320_3_) < p_186320_4_)
            {
                p_186320_1_[i++] = pathpoint;
            }
        }

        return i;
    }

    public PathNodeType func_186319_a(IBlockAccess p_186319_1_, int p_186319_2_, int p_186319_3_, int p_186319_4_, EntityLiving p_186319_5_, int p_186319_6_, int p_186319_7_, int p_186319_8_, boolean p_186319_9_, boolean p_186319_10_)
    {
        return PathNodeType.WATER;
    }

    private PathPoint func_186328_b(int p_186328_1_, int p_186328_2_, int p_186328_3_)
    {
        PathNodeType pathnodetype = this.func_186327_c(p_186328_1_, p_186328_2_, p_186328_3_);
        return pathnodetype == PathNodeType.WATER ? this.openPoint(p_186328_1_, p_186328_2_, p_186328_3_) : null;
    }

    private PathNodeType func_186327_c(int p_186327_1_, int p_186327_2_, int p_186327_3_)
    {
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos();

        for (int i = p_186327_1_; i < p_186327_1_ + this.entitySizeX; ++i)
        {
            for (int j = p_186327_2_; j < p_186327_2_ + this.entitySizeY; ++j)
            {
                for (int k = p_186327_3_; k < p_186327_3_ + this.entitySizeZ; ++k)
                {
                    IBlockState iblockstate = this.blockaccess.getBlockState(blockpos$mutableblockpos.set(i, j, k));

                    if (iblockstate.getMaterial() != Material.water)
                    {
                        return PathNodeType.BLOCKED;
                    }
                }
            }
        }

        return PathNodeType.WATER;
    }
}
