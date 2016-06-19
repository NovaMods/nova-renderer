package net.minecraft.entity.ai;

import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityCreature;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;

public class EntityAIPanic extends EntityAIBase
{
    private EntityCreature theEntityCreature;
    protected double speed;
    private double randPosX;
    private double randPosY;
    private double randPosZ;

    public EntityAIPanic(EntityCreature creature, double speedIn)
    {
        this.theEntityCreature = creature;
        this.speed = speedIn;
        this.setMutexBits(1);
    }

    /**
     * Returns whether the EntityAIBase should begin execution.
     */
    public boolean shouldExecute()
    {
        if (this.theEntityCreature.getAITarget() == null && !this.theEntityCreature.isBurning())
        {
            return false;
        }
        else
        {
            Vec3d vec3d = RandomPositionGenerator.findRandomTarget(this.theEntityCreature, 5, 4);

            if (vec3d == null)
            {
                return false;
            }
            else
            {
                this.randPosX = vec3d.xCoord;
                this.randPosY = vec3d.yCoord;
                this.randPosZ = vec3d.zCoord;

                if (this.theEntityCreature.isBurning())
                {
                    BlockPos blockpos = this.func_188497_a(this.theEntityCreature.worldObj, this.theEntityCreature, 5, 4);

                    if (blockpos != null)
                    {
                        this.randPosX = (double)blockpos.getX();
                        this.randPosY = (double)blockpos.getY();
                        this.randPosZ = (double)blockpos.getZ();
                    }
                }

                return true;
            }
        }
    }

    /**
     * Execute a one shot task or start executing a continuous task
     */
    public void startExecuting()
    {
        this.theEntityCreature.getNavigator().tryMoveToXYZ(this.randPosX, this.randPosY, this.randPosZ, this.speed);
    }

    /**
     * Returns whether an in-progress EntityAIBase should continue executing
     */
    public boolean continueExecuting()
    {
        return !this.theEntityCreature.getNavigator().noPath();
    }

    private BlockPos func_188497_a(World p_188497_1_, Entity p_188497_2_, int p_188497_3_, int p_188497_4_)
    {
        BlockPos blockpos = new BlockPos(p_188497_2_);
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos();
        int i = blockpos.getX();
        int j = blockpos.getY();
        int k = blockpos.getZ();
        float f = (float)(p_188497_3_ * p_188497_3_ * p_188497_4_ * 2);
        BlockPos blockpos1 = null;

        for (int l = i - p_188497_3_; l <= i + p_188497_3_; ++l)
        {
            for (int i1 = j - p_188497_4_; i1 <= j + p_188497_4_; ++i1)
            {
                for (int j1 = k - p_188497_3_; j1 <= k + p_188497_3_; ++j1)
                {
                    blockpos$mutableblockpos.set(l, i1, j1);
                    IBlockState iblockstate = p_188497_1_.getBlockState(blockpos$mutableblockpos);
                    Block block = iblockstate.getBlock();

                    if (block == Blocks.water || block == Blocks.flowing_water)
                    {
                        float f1 = (float)((l - i) * (l - i) + (i1 - j) * (i1 - j) + (j1 - k) * (j1 - k));

                        if (f1 < f)
                        {
                            f = f1;
                            blockpos1 = new BlockPos(blockpos$mutableblockpos);
                        }
                    }
                }
            }
        }

        return blockpos1;
    }
}
