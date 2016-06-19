package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public abstract class BlockBasePressurePlate extends Block
{
    protected static final AxisAlignedBB field_185509_a = new AxisAlignedBB(0.0625D, 0.0D, 0.0625D, 0.9375D, 0.03125D, 0.9375D);
    protected static final AxisAlignedBB field_185510_b = new AxisAlignedBB(0.0625D, 0.0D, 0.0625D, 0.9375D, 0.0625D, 0.9375D);
    protected static final AxisAlignedBB field_185511_c = new AxisAlignedBB(0.125D, 0.0D, 0.125D, 0.875D, 0.25D, 0.875D);

    protected BlockBasePressurePlate(Material materialIn)
    {
        this(materialIn, materialIn.getMaterialMapColor());
    }

    protected BlockBasePressurePlate(Material p_i46401_1_, MapColor p_i46401_2_)
    {
        super(p_i46401_1_, p_i46401_2_);
        this.setCreativeTab(CreativeTabs.tabRedstone);
        this.setTickRandomly(true);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        boolean flag = this.getRedstoneStrength(state) > 0;
        return flag ? field_185509_a : field_185510_b;
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return 20;
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return NULL_AABB;
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public boolean isPassable(IBlockAccess worldIn, BlockPos pos)
    {
        return true;
    }

    /**
     * Return true if an entity can be spawned inside the block (used to get the player's bed spawn location)
     */
    public boolean canSpawnInBlock()
    {
        return true;
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return this.canBePlacedOn(worldIn, pos.down());
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (!this.canBePlacedOn(worldIn, pos.down()))
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockToAir(pos);
        }
    }

    private boolean canBePlacedOn(World worldIn, BlockPos pos)
    {
        return worldIn.getBlockState(pos).func_185896_q() || worldIn.getBlockState(pos).getBlock() instanceof BlockFence;
    }

    /**
     * Called randomly when setTickRandomly is set to true (used by e.g. crops to grow, etc.)
     */
    public void randomTick(World worldIn, BlockPos pos, IBlockState state, Random random)
    {
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (!worldIn.isRemote)
        {
            int i = this.getRedstoneStrength(state);

            if (i > 0)
            {
                this.updateState(worldIn, pos, state, i);
            }
        }
    }

    /**
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
        if (!worldIn.isRemote)
        {
            int i = this.getRedstoneStrength(state);

            if (i == 0)
            {
                this.updateState(worldIn, pos, state, i);
            }
        }
    }

    /**
     * Updates the pressure plate when stepped on
     */
    protected void updateState(World worldIn, BlockPos pos, IBlockState state, int oldRedstoneStrength)
    {
        int i = this.computeRedstoneStrength(worldIn, pos);
        boolean flag = oldRedstoneStrength > 0;
        boolean flag1 = i > 0;

        if (oldRedstoneStrength != i)
        {
            state = this.setRedstoneStrength(state, i);
            worldIn.setBlockState(pos, state, 2);
            this.updateNeighbors(worldIn, pos);
            worldIn.markBlockRangeForRenderUpdate(pos, pos);
        }

        if (!flag1 && flag)
        {
            this.func_185508_c(worldIn, pos);
        }
        else if (flag1 && !flag)
        {
            this.func_185507_b(worldIn, pos);
        }

        if (flag1)
        {
            worldIn.scheduleUpdate(new BlockPos(pos), this, this.tickRate(worldIn));
        }
    }

    protected abstract void func_185507_b(World p_185507_1_, BlockPos p_185507_2_);

    protected abstract void func_185508_c(World p_185508_1_, BlockPos p_185508_2_);

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        if (this.getRedstoneStrength(state) > 0)
        {
            this.updateNeighbors(worldIn, pos);
        }

        super.breakBlock(worldIn, pos, state);
    }

    /**
     * Notify block and block below of changes
     */
    protected void updateNeighbors(World worldIn, BlockPos pos)
    {
        worldIn.notifyNeighborsOfStateChange(pos, this);
        worldIn.notifyNeighborsOfStateChange(pos.down(), this);
    }

    public int getWeakPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return this.getRedstoneStrength(worldIn);
    }

    public int getStrongPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return side == EnumFacing.UP ? this.getRedstoneStrength(worldIn) : 0;
    }

    /**
     * Can this block provide power. Only wire currently seems to have this change based on its state.
     */
    public boolean canProvidePower(IBlockState state)
    {
        return true;
    }

    public EnumPushReaction getMobilityFlag(IBlockState state)
    {
        return EnumPushReaction.DESTROY;
    }

    protected abstract int computeRedstoneStrength(World worldIn, BlockPos pos);

    protected abstract int getRedstoneStrength(IBlockState state);

    protected abstract IBlockState setRedstoneStrength(IBlockState state, int strength);
}
