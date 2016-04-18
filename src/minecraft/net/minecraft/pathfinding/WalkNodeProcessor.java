package net.minecraft.pathfinding;

import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.block.BlockDoor;
import net.minecraft.block.BlockFence;
import net.minecraft.block.BlockFenceGate;
import net.minecraft.block.BlockRailBase;
import net.minecraft.block.BlockWall;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.EntityLiving;
import net.minecraft.init.Blocks;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.IBlockAccess;

public class WalkNodeProcessor extends NodeProcessor
{
    private float avoidsWater;

    public void func_186315_a(IBlockAccess p_186315_1_, EntityLiving p_186315_2_)
    {
        super.func_186315_a(p_186315_1_, p_186315_2_);
        this.avoidsWater = p_186315_2_.func_184643_a(PathNodeType.WATER);
    }

    /**
     * This method is called when all nodes have been processed and PathEntity is created.
     *  {@link net.minecraft.world.pathfinder.WalkNodeProcessor WalkNodeProcessor} uses this to change its field {@link
     * net.minecraft.world.pathfinder.WalkNodeProcessor#avoidsWater avoidsWater}
     */
    public void postProcess()
    {
        super.postProcess();
        this.field_186326_b.func_184644_a(PathNodeType.WATER, this.avoidsWater);
    }

    public PathPoint func_186318_b()
    {
        int i;

        if (this.func_186322_e() && this.field_186326_b.isInWater())
        {
            i = (int)this.field_186326_b.getEntityBoundingBox().minY;
            BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos(MathHelper.floor_double(this.field_186326_b.posX), i, MathHelper.floor_double(this.field_186326_b.posZ));

            for (Block block = this.blockaccess.getBlockState(blockpos$mutableblockpos).getBlock(); block == Blocks.flowing_water || block == Blocks.water; block = this.blockaccess.getBlockState(blockpos$mutableblockpos).getBlock())
            {
                ++i;
                blockpos$mutableblockpos.set(MathHelper.floor_double(this.field_186326_b.posX), i, MathHelper.floor_double(this.field_186326_b.posZ));
            }
        }
        else if (!this.field_186326_b.onGround)
        {
            BlockPos blockpos;

            for (blockpos = new BlockPos(this.field_186326_b); (this.blockaccess.getBlockState(blockpos).getMaterial() == Material.air || this.blockaccess.getBlockState(blockpos).getBlock().isPassable(this.blockaccess, blockpos)) && blockpos.getY() > 0; blockpos = blockpos.down())
            {
                ;
            }

            i = blockpos.up().getY();
        }
        else
        {
            i = MathHelper.floor_double(this.field_186326_b.getEntityBoundingBox().minY + 0.5D);
        }

        BlockPos blockpos2 = new BlockPos(this.field_186326_b);
        PathNodeType pathnodetype1 = this.func_186331_a(this.field_186326_b, blockpos2.getX(), i, blockpos2.getZ());

        if (this.field_186326_b.func_184643_a(pathnodetype1) < 0.0F)
        {
            Set<BlockPos> set = new HashSet();
            set.add(new BlockPos(this.field_186326_b.getEntityBoundingBox().minX, (double)i, this.field_186326_b.getEntityBoundingBox().minZ));
            set.add(new BlockPos(this.field_186326_b.getEntityBoundingBox().minX, (double)i, this.field_186326_b.getEntityBoundingBox().maxZ));
            set.add(new BlockPos(this.field_186326_b.getEntityBoundingBox().maxX, (double)i, this.field_186326_b.getEntityBoundingBox().minZ));
            set.add(new BlockPos(this.field_186326_b.getEntityBoundingBox().maxX, (double)i, this.field_186326_b.getEntityBoundingBox().maxZ));

            for (BlockPos blockpos1 : set)
            {
                PathNodeType pathnodetype = this.func_186329_a(this.field_186326_b, blockpos1);

                if (this.field_186326_b.func_184643_a(pathnodetype) >= 0.0F)
                {
                    return this.openPoint(blockpos1.getX(), blockpos1.getY(), blockpos1.getZ());
                }
            }
        }

        return this.openPoint(blockpos2.getX(), i, blockpos2.getZ());
    }

    public PathPoint func_186325_a(double p_186325_1_, double p_186325_3_, double p_186325_5_)
    {
        return this.openPoint(MathHelper.floor_double(p_186325_1_ - (double)(this.field_186326_b.width / 2.0F)), MathHelper.floor_double(p_186325_3_), MathHelper.floor_double(p_186325_5_ - (double)(this.field_186326_b.width / 2.0F)));
    }

    public int func_186320_a(PathPoint[] p_186320_1_, PathPoint p_186320_2_, PathPoint p_186320_3_, float p_186320_4_)
    {
        int i = 0;
        int j = 0;
        PathNodeType pathnodetype = this.func_186331_a(this.field_186326_b, p_186320_2_.xCoord, p_186320_2_.yCoord + 1, p_186320_2_.zCoord);

        if (this.field_186326_b.func_184643_a(pathnodetype) >= 0.0F)
        {
            j = 1;
        }

        BlockPos blockpos = (new BlockPos(p_186320_2_.xCoord, p_186320_2_.yCoord, p_186320_2_.zCoord)).down();
        double d0 = (double)p_186320_2_.yCoord - (1.0D - this.blockaccess.getBlockState(blockpos).func_185900_c(this.blockaccess, blockpos).maxY);
        PathPoint pathpoint = this.func_186332_a(p_186320_2_.xCoord, p_186320_2_.yCoord, p_186320_2_.zCoord + 1, j, d0, EnumFacing.SOUTH);
        PathPoint pathpoint1 = this.func_186332_a(p_186320_2_.xCoord - 1, p_186320_2_.yCoord, p_186320_2_.zCoord, j, d0, EnumFacing.WEST);
        PathPoint pathpoint2 = this.func_186332_a(p_186320_2_.xCoord + 1, p_186320_2_.yCoord, p_186320_2_.zCoord, j, d0, EnumFacing.EAST);
        PathPoint pathpoint3 = this.func_186332_a(p_186320_2_.xCoord, p_186320_2_.yCoord, p_186320_2_.zCoord - 1, j, d0, EnumFacing.NORTH);

        if (pathpoint != null && !pathpoint.visited && pathpoint.distanceTo(p_186320_3_) < p_186320_4_)
        {
            p_186320_1_[i++] = pathpoint;
        }

        if (pathpoint1 != null && !pathpoint1.visited && pathpoint1.distanceTo(p_186320_3_) < p_186320_4_)
        {
            p_186320_1_[i++] = pathpoint1;
        }

        if (pathpoint2 != null && !pathpoint2.visited && pathpoint2.distanceTo(p_186320_3_) < p_186320_4_)
        {
            p_186320_1_[i++] = pathpoint2;
        }

        if (pathpoint3 != null && !pathpoint3.visited && pathpoint3.distanceTo(p_186320_3_) < p_186320_4_)
        {
            p_186320_1_[i++] = pathpoint3;
        }

        boolean flag = pathpoint3 == null || pathpoint3.field_186287_m == PathNodeType.OPEN || pathpoint3.field_186286_l != 0.0F;
        boolean flag1 = pathpoint == null || pathpoint.field_186287_m == PathNodeType.OPEN || pathpoint.field_186286_l != 0.0F;
        boolean flag2 = pathpoint2 == null || pathpoint2.field_186287_m == PathNodeType.OPEN || pathpoint2.field_186286_l != 0.0F;
        boolean flag3 = pathpoint1 == null || pathpoint1.field_186287_m == PathNodeType.OPEN || pathpoint1.field_186286_l != 0.0F;

        if (flag && flag3)
        {
            PathPoint pathpoint4 = this.func_186332_a(p_186320_2_.xCoord - 1, p_186320_2_.yCoord, p_186320_2_.zCoord - 1, j, d0, EnumFacing.NORTH);

            if (pathpoint4 != null && !pathpoint4.visited && pathpoint4.distanceTo(p_186320_3_) < p_186320_4_)
            {
                p_186320_1_[i++] = pathpoint4;
            }
        }

        if (flag && flag2)
        {
            PathPoint pathpoint5 = this.func_186332_a(p_186320_2_.xCoord + 1, p_186320_2_.yCoord, p_186320_2_.zCoord - 1, j, d0, EnumFacing.NORTH);

            if (pathpoint5 != null && !pathpoint5.visited && pathpoint5.distanceTo(p_186320_3_) < p_186320_4_)
            {
                p_186320_1_[i++] = pathpoint5;
            }
        }

        if (flag1 && flag3)
        {
            PathPoint pathpoint6 = this.func_186332_a(p_186320_2_.xCoord - 1, p_186320_2_.yCoord, p_186320_2_.zCoord + 1, j, d0, EnumFacing.SOUTH);

            if (pathpoint6 != null && !pathpoint6.visited && pathpoint6.distanceTo(p_186320_3_) < p_186320_4_)
            {
                p_186320_1_[i++] = pathpoint6;
            }
        }

        if (flag1 && flag2)
        {
            PathPoint pathpoint7 = this.func_186332_a(p_186320_2_.xCoord + 1, p_186320_2_.yCoord, p_186320_2_.zCoord + 1, j, d0, EnumFacing.SOUTH);

            if (pathpoint7 != null && !pathpoint7.visited && pathpoint7.distanceTo(p_186320_3_) < p_186320_4_)
            {
                p_186320_1_[i++] = pathpoint7;
            }
        }

        return i;
    }

    private PathPoint func_186332_a(int p_186332_1_, int p_186332_2_, int p_186332_3_, int p_186332_4_, double p_186332_5_, EnumFacing p_186332_7_)
    {
        PathPoint pathpoint = null;
        BlockPos blockpos = new BlockPos(p_186332_1_, p_186332_2_, p_186332_3_);
        BlockPos blockpos1 = blockpos.down();
        double d0 = (double)p_186332_2_ - (1.0D - this.blockaccess.getBlockState(blockpos1).func_185900_c(this.blockaccess, blockpos1).maxY);

        if (d0 - p_186332_5_ > 1.0D)
        {
            return null;
        }
        else
        {
            PathNodeType pathnodetype = this.func_186331_a(this.field_186326_b, p_186332_1_, p_186332_2_, p_186332_3_);
            float f = this.field_186326_b.func_184643_a(pathnodetype);
            double d1 = (double)this.field_186326_b.width / 2.0D;

            if (f >= 0.0F)
            {
                pathpoint = this.openPoint(p_186332_1_, p_186332_2_, p_186332_3_);
                pathpoint.field_186287_m = pathnodetype;
                pathpoint.field_186286_l = Math.max(pathpoint.field_186286_l, f);
            }

            if (pathnodetype == PathNodeType.WALKABLE)
            {
                return pathpoint;
            }
            else
            {
                if (pathpoint == null && p_186332_4_ > 0 && pathnodetype != PathNodeType.FENCE && pathnodetype != PathNodeType.TRAPDOOR)
                {
                    pathpoint = this.func_186332_a(p_186332_1_, p_186332_2_ + 1, p_186332_3_, p_186332_4_ - 1, p_186332_5_, p_186332_7_);

                    if (pathpoint != null && (pathpoint.field_186287_m == PathNodeType.OPEN || pathpoint.field_186287_m == PathNodeType.WALKABLE))
                    {
                        double d2 = (double)(p_186332_1_ - p_186332_7_.getFrontOffsetX()) + 0.5D;
                        double d3 = (double)(p_186332_3_ - p_186332_7_.getFrontOffsetZ()) + 0.5D;
                        AxisAlignedBB axisalignedbb = new AxisAlignedBB(d2 - d1, (double)p_186332_2_ + 0.001D, d3 - d1, d2 + d1, (double)((float)p_186332_2_ + this.field_186326_b.height), d3 + d1);
                        AxisAlignedBB axisalignedbb1 = this.blockaccess.getBlockState(blockpos).func_185900_c(this.blockaccess, blockpos);
                        AxisAlignedBB axisalignedbb2 = axisalignedbb.addCoord(0.0D, axisalignedbb1.maxY - 0.002D, 0.0D);

                        if (this.field_186326_b.worldObj.func_184143_b(axisalignedbb2))
                        {
                            pathpoint = null;
                        }
                    }
                }

                if (pathnodetype == PathNodeType.OPEN)
                {
                    AxisAlignedBB axisalignedbb3 = new AxisAlignedBB((double)p_186332_1_ - d1 + 0.5D, (double)p_186332_2_ + 0.001D, (double)p_186332_3_ - d1 + 0.5D, (double)p_186332_1_ + d1 + 0.5D, (double)((float)p_186332_2_ + this.field_186326_b.height), (double)p_186332_3_ + d1 + 0.5D);

                    if (this.field_186326_b.worldObj.func_184143_b(axisalignedbb3))
                    {
                        return null;
                    }

                    int i = 0;

                    while (p_186332_2_ > 0 && pathnodetype == PathNodeType.OPEN)
                    {
                        --p_186332_2_;

                        if (i++ >= this.field_186326_b.getMaxFallHeight())
                        {
                            return null;
                        }

                        pathnodetype = this.func_186331_a(this.field_186326_b, p_186332_1_, p_186332_2_, p_186332_3_);
                        f = this.field_186326_b.func_184643_a(pathnodetype);

                        if (pathnodetype != PathNodeType.OPEN && f >= 0.0F)
                        {
                            pathpoint = this.openPoint(p_186332_1_, p_186332_2_, p_186332_3_);
                            pathpoint.field_186287_m = pathnodetype;
                            pathpoint.field_186286_l = Math.max(pathpoint.field_186286_l, f);
                            break;
                        }

                        if (f < 0.0F)
                        {
                            return null;
                        }
                    }
                }

                return pathpoint;
            }
        }
    }

    public PathNodeType func_186319_a(IBlockAccess p_186319_1_, int p_186319_2_, int p_186319_3_, int p_186319_4_, EntityLiving p_186319_5_, int p_186319_6_, int p_186319_7_, int p_186319_8_, boolean p_186319_9_, boolean p_186319_10_)
    {
        EnumSet<PathNodeType> enumset = EnumSet.<PathNodeType>noneOf(PathNodeType.class);
        PathNodeType pathnodetype = PathNodeType.BLOCKED;
        double d0 = (double)p_186319_5_.width / 2.0D;
        BlockPos blockpos = new BlockPos(p_186319_5_);

        for (int i = p_186319_2_; i < p_186319_2_ + p_186319_6_; ++i)
        {
            for (int j = p_186319_3_; j < p_186319_3_ + p_186319_7_; ++j)
            {
                for (int k = p_186319_4_; k < p_186319_4_ + p_186319_8_; ++k)
                {
                    PathNodeType pathnodetype1 = func_186330_a(p_186319_1_, i, j, k);

                    if (pathnodetype1 == PathNodeType.DOOR_WOOD_CLOSED && p_186319_9_ && p_186319_10_)
                    {
                        pathnodetype1 = PathNodeType.WALKABLE;
                    }

                    if (pathnodetype1 == PathNodeType.DOOR_OPEN && !p_186319_10_)
                    {
                        pathnodetype1 = PathNodeType.BLOCKED;
                    }

                    if (pathnodetype1 == PathNodeType.RAIL && !(p_186319_1_.getBlockState(blockpos).getBlock() instanceof BlockRailBase) && !(p_186319_1_.getBlockState(blockpos.down()).getBlock() instanceof BlockRailBase))
                    {
                        pathnodetype1 = PathNodeType.FENCE;
                    }

                    if (i == p_186319_2_ && j == p_186319_3_ && k == p_186319_4_)
                    {
                        pathnodetype = pathnodetype1;
                    }

                    if (j > p_186319_3_ && pathnodetype1 != PathNodeType.OPEN)
                    {
                        AxisAlignedBB axisalignedbb = new AxisAlignedBB((double)i - d0 + 0.5D, (double)p_186319_3_ + 0.001D, (double)k - d0 + 0.5D, (double)i + d0 + 0.5D, (double)((float)p_186319_3_ + p_186319_5_.height), (double)k + d0 + 0.5D);

                        if (!p_186319_5_.worldObj.func_184143_b(axisalignedbb))
                        {
                            pathnodetype1 = PathNodeType.OPEN;
                        }
                    }

                    enumset.add(pathnodetype1);
                }
            }
        }

        if (enumset.contains(PathNodeType.FENCE))
        {
            return PathNodeType.FENCE;
        }
        else
        {
            PathNodeType pathnodetype2 = PathNodeType.BLOCKED;

            for (PathNodeType pathnodetype3 : enumset)
            {
                if (p_186319_5_.func_184643_a(pathnodetype3) < 0.0F)
                {
                    return pathnodetype3;
                }

                if (p_186319_5_.func_184643_a(pathnodetype3) >= p_186319_5_.func_184643_a(pathnodetype2))
                {
                    pathnodetype2 = pathnodetype3;
                }
            }

            if (pathnodetype == PathNodeType.OPEN && p_186319_5_.func_184643_a(pathnodetype2) == 0.0F)
            {
                return PathNodeType.OPEN;
            }
            else
            {
                return pathnodetype2;
            }
        }
    }

    private PathNodeType func_186329_a(EntityLiving p_186329_1_, BlockPos p_186329_2_)
    {
        return this.func_186319_a(this.blockaccess, p_186329_2_.getX(), p_186329_2_.getY(), p_186329_2_.getZ(), p_186329_1_, this.entitySizeX, this.entitySizeY, this.entitySizeZ, this.func_186324_d(), this.func_186323_c());
    }

    private PathNodeType func_186331_a(EntityLiving p_186331_1_, int p_186331_2_, int p_186331_3_, int p_186331_4_)
    {
        return this.func_186319_a(this.blockaccess, p_186331_2_, p_186331_3_, p_186331_4_, p_186331_1_, this.entitySizeX, this.entitySizeY, this.entitySizeZ, this.func_186324_d(), this.func_186323_c());
    }

    public static PathNodeType func_186330_a(IBlockAccess p_186330_0_, int p_186330_1_, int p_186330_2_, int p_186330_3_)
    {
        BlockPos blockpos = new BlockPos(p_186330_1_, p_186330_2_, p_186330_3_);
        IBlockState iblockstate = p_186330_0_.getBlockState(blockpos);
        Block block = iblockstate.getBlock();
        Material material = iblockstate.getMaterial();
        PathNodeType pathnodetype = PathNodeType.BLOCKED;

        if (block != Blocks.trapdoor && block != Blocks.iron_trapdoor && block != Blocks.waterlily)
        {
            if (block == Blocks.fire)
            {
                return PathNodeType.DAMAGE_FIRE;
            }
            else if (block == Blocks.cactus)
            {
                return PathNodeType.DAMAGE_CACTUS;
            }
            else if (block instanceof BlockDoor && material == Material.wood && !((Boolean)iblockstate.getValue(BlockDoor.OPEN)).booleanValue())
            {
                return PathNodeType.DOOR_WOOD_CLOSED;
            }
            else if (block instanceof BlockDoor && material == Material.iron && !((Boolean)iblockstate.getValue(BlockDoor.OPEN)).booleanValue())
            {
                return PathNodeType.DOOR_IRON_CLOSED;
            }
            else if (block instanceof BlockDoor && ((Boolean)iblockstate.getValue(BlockDoor.OPEN)).booleanValue())
            {
                return PathNodeType.DOOR_OPEN;
            }
            else if (block instanceof BlockRailBase)
            {
                return PathNodeType.RAIL;
            }
            else if (!(block instanceof BlockFence) && !(block instanceof BlockWall) && (!(block instanceof BlockFenceGate) || ((Boolean)iblockstate.getValue(BlockFenceGate.OPEN)).booleanValue()))
            {
                if (material == Material.air)
                {
                    pathnodetype = PathNodeType.OPEN;
                }
                else
                {
                    if (material == Material.water)
                    {
                        return PathNodeType.WATER;
                    }

                    if (material == Material.lava)
                    {
                        return PathNodeType.LAVA;
                    }
                }

                if (block.isPassable(p_186330_0_, blockpos) && pathnodetype == PathNodeType.BLOCKED)
                {
                    pathnodetype = PathNodeType.OPEN;
                }

                if (pathnodetype == PathNodeType.OPEN && p_186330_2_ >= 1)
                {
                    PathNodeType pathnodetype1 = func_186330_a(p_186330_0_, p_186330_1_, p_186330_2_ - 1, p_186330_3_);
                    pathnodetype = pathnodetype1 != PathNodeType.WALKABLE && pathnodetype1 != PathNodeType.OPEN && pathnodetype1 != PathNodeType.WATER && pathnodetype1 != PathNodeType.LAVA ? PathNodeType.WALKABLE : PathNodeType.OPEN;
                }

                if (pathnodetype == PathNodeType.WALKABLE)
                {
                    for (int j = p_186330_1_ - 1; j <= p_186330_1_ + 1; ++j)
                    {
                        for (int i = p_186330_3_ - 1; i <= p_186330_3_ + 1; ++i)
                        {
                            if (j != p_186330_1_ || i != p_186330_3_)
                            {
                                Block block1 = p_186330_0_.getBlockState(new BlockPos(j, p_186330_2_, i)).getBlock();

                                if (block1 == Blocks.cactus)
                                {
                                    pathnodetype = PathNodeType.DANGER_CACTUS;
                                }
                                else if (block1 == Blocks.fire)
                                {
                                    pathnodetype = PathNodeType.DANGER_FIRE;
                                }
                            }
                        }
                    }
                }

                return pathnodetype;
            }
            else
            {
                return PathNodeType.FENCE;
            }
        }
        else
        {
            return PathNodeType.TRAPDOOR;
        }
    }
}
