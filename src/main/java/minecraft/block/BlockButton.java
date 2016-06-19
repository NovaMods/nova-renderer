package net.minecraft.block;

import java.util.List;
import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public abstract class BlockButton extends BlockDirectional
{
    public static final PropertyBool POWERED = PropertyBool.create("powered");
    protected static final AxisAlignedBB field_185618_b = new AxisAlignedBB(0.3125D, 0.875D, 0.375D, 0.6875D, 1.0D, 0.625D);
    protected static final AxisAlignedBB field_185620_c = new AxisAlignedBB(0.3125D, 0.0D, 0.375D, 0.6875D, 0.125D, 0.625D);
    protected static final AxisAlignedBB field_185622_d = new AxisAlignedBB(0.3125D, 0.375D, 0.875D, 0.6875D, 0.625D, 1.0D);
    protected static final AxisAlignedBB field_185624_e = new AxisAlignedBB(0.3125D, 0.375D, 0.0D, 0.6875D, 0.625D, 0.125D);
    protected static final AxisAlignedBB field_185626_f = new AxisAlignedBB(0.875D, 0.375D, 0.3125D, 1.0D, 0.625D, 0.6875D);
    protected static final AxisAlignedBB field_185628_g = new AxisAlignedBB(0.0D, 0.375D, 0.3125D, 0.125D, 0.625D, 0.6875D);
    protected static final AxisAlignedBB field_185619_B = new AxisAlignedBB(0.3125D, 0.9375D, 0.375D, 0.6875D, 1.0D, 0.625D);
    protected static final AxisAlignedBB field_185621_C = new AxisAlignedBB(0.3125D, 0.0D, 0.375D, 0.6875D, 0.0625D, 0.625D);
    protected static final AxisAlignedBB field_185623_D = new AxisAlignedBB(0.3125D, 0.375D, 0.9375D, 0.6875D, 0.625D, 1.0D);
    protected static final AxisAlignedBB field_185625_E = new AxisAlignedBB(0.3125D, 0.375D, 0.0D, 0.6875D, 0.625D, 0.0625D);
    protected static final AxisAlignedBB field_185627_F = new AxisAlignedBB(0.9375D, 0.375D, 0.3125D, 1.0D, 0.625D, 0.6875D);
    protected static final AxisAlignedBB field_185629_G = new AxisAlignedBB(0.0D, 0.375D, 0.3125D, 0.0625D, 0.625D, 0.6875D);
    private final boolean wooden;

    protected BlockButton(boolean wooden)
    {
        super(Material.circuits);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH).withProperty(POWERED, Boolean.valueOf(false)));
        this.setTickRandomly(true);
        this.setCreativeTab(CreativeTabs.tabRedstone);
        this.wooden = wooden;
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return NULL_AABB;
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return this.wooden ? 30 : 20;
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

    /**
     * Check whether this Block can be placed on the given side
     */
    public boolean canPlaceBlockOnSide(World worldIn, BlockPos pos, EnumFacing side)
    {
        return func_181088_a(worldIn, pos, side.getOpposite());
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        for (EnumFacing enumfacing : EnumFacing.values())
        {
            if (func_181088_a(worldIn, pos, enumfacing))
            {
                return true;
            }
        }

        return false;
    }

    protected static boolean func_181088_a(World p_181088_0_, BlockPos p_181088_1_, EnumFacing p_181088_2_)
    {
        BlockPos blockpos = p_181088_1_.offset(p_181088_2_);
        return p_181088_2_ == EnumFacing.DOWN ? p_181088_0_.getBlockState(blockpos).func_185896_q() : p_181088_0_.getBlockState(blockpos).isNormalCube();
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        return func_181088_a(worldIn, pos, facing.getOpposite()) ? this.getDefaultState().withProperty(FACING, facing).withProperty(POWERED, Boolean.valueOf(false)) : this.getDefaultState().withProperty(FACING, EnumFacing.DOWN).withProperty(POWERED, Boolean.valueOf(false));
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (this.checkForDrop(worldIn, pos, state) && !func_181088_a(worldIn, pos, ((EnumFacing)state.getValue(FACING)).getOpposite()))
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockToAir(pos);
        }
    }

    private boolean checkForDrop(World worldIn, BlockPos pos, IBlockState state)
    {
        if (this.canPlaceBlockAt(worldIn, pos))
        {
            return true;
        }
        else
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockToAir(pos);
            return false;
        }
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        EnumFacing enumfacing = (EnumFacing)state.getValue(FACING);
        boolean flag = ((Boolean)state.getValue(POWERED)).booleanValue();

        switch (enumfacing)
        {
            case EAST:
                return flag ? field_185629_G : field_185628_g;

            case WEST:
                return flag ? field_185627_F : field_185626_f;

            case SOUTH:
                return flag ? field_185625_E : field_185624_e;

            case NORTH:
            default:
                return flag ? field_185623_D : field_185622_d;

            case UP:
                return flag ? field_185621_C : field_185620_c;

            case DOWN:
                return flag ? field_185619_B : field_185618_b;
        }
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (((Boolean)state.getValue(POWERED)).booleanValue())
        {
            return true;
        }
        else
        {
            worldIn.setBlockState(pos, state.withProperty(POWERED, Boolean.valueOf(true)), 3);
            worldIn.markBlockRangeForRenderUpdate(pos, pos);
            this.func_185615_a(playerIn, worldIn, pos);
            this.notifyNeighbors(worldIn, pos, (EnumFacing)state.getValue(FACING));
            worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn));
            return true;
        }
    }

    protected abstract void func_185615_a(EntityPlayer p_185615_1_, World p_185615_2_, BlockPos p_185615_3_);

    protected abstract void func_185617_b(World p_185617_1_, BlockPos p_185617_2_);

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        if (((Boolean)state.getValue(POWERED)).booleanValue())
        {
            this.notifyNeighbors(worldIn, pos, (EnumFacing)state.getValue(FACING));
        }

        super.breakBlock(worldIn, pos, state);
    }

    public int getWeakPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return ((Boolean)worldIn.getValue(POWERED)).booleanValue() ? 15 : 0;
    }

    public int getStrongPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return !((Boolean)worldIn.getValue(POWERED)).booleanValue() ? 0 : (worldIn.getValue(FACING) == side ? 15 : 0);
    }

    /**
     * Can this block provide power. Only wire currently seems to have this change based on its state.
     */
    public boolean canProvidePower(IBlockState state)
    {
        return true;
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
            if (((Boolean)state.getValue(POWERED)).booleanValue())
            {
                if (this.wooden)
                {
                    this.func_185616_e(state, worldIn, pos);
                }
                else
                {
                    worldIn.setBlockState(pos, state.withProperty(POWERED, Boolean.valueOf(false)));
                    this.notifyNeighbors(worldIn, pos, (EnumFacing)state.getValue(FACING));
                    this.func_185617_b(worldIn, pos);
                    worldIn.markBlockRangeForRenderUpdate(pos, pos);
                }
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
            if (this.wooden)
            {
                if (!((Boolean)state.getValue(POWERED)).booleanValue())
                {
                    this.func_185616_e(state, worldIn, pos);
                }
            }
        }
    }

    private void func_185616_e(IBlockState p_185616_1_, World p_185616_2_, BlockPos p_185616_3_)
    {
        List <? extends Entity > list = p_185616_2_.<Entity>getEntitiesWithinAABB(EntityArrow.class, p_185616_1_.func_185900_c(p_185616_2_, p_185616_3_).offset(p_185616_3_));
        boolean flag = !list.isEmpty();
        boolean flag1 = ((Boolean)p_185616_1_.getValue(POWERED)).booleanValue();

        if (flag && !flag1)
        {
            p_185616_2_.setBlockState(p_185616_3_, p_185616_1_.withProperty(POWERED, Boolean.valueOf(true)));
            this.notifyNeighbors(p_185616_2_, p_185616_3_, (EnumFacing)p_185616_1_.getValue(FACING));
            p_185616_2_.markBlockRangeForRenderUpdate(p_185616_3_, p_185616_3_);
            this.func_185615_a((EntityPlayer)null, p_185616_2_, p_185616_3_);
        }

        if (!flag && flag1)
        {
            p_185616_2_.setBlockState(p_185616_3_, p_185616_1_.withProperty(POWERED, Boolean.valueOf(false)));
            this.notifyNeighbors(p_185616_2_, p_185616_3_, (EnumFacing)p_185616_1_.getValue(FACING));
            p_185616_2_.markBlockRangeForRenderUpdate(p_185616_3_, p_185616_3_);
            this.func_185617_b(p_185616_2_, p_185616_3_);
        }

        if (flag)
        {
            p_185616_2_.scheduleUpdate(new BlockPos(p_185616_3_), this, this.tickRate(p_185616_2_));
        }
    }

    private void notifyNeighbors(World worldIn, BlockPos pos, EnumFacing facing)
    {
        worldIn.notifyNeighborsOfStateChange(pos, this);
        worldIn.notifyNeighborsOfStateChange(pos.offset(facing.getOpposite()), this);
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        EnumFacing enumfacing;

        switch (meta & 7)
        {
            case 0:
                enumfacing = EnumFacing.DOWN;
                break;

            case 1:
                enumfacing = EnumFacing.EAST;
                break;

            case 2:
                enumfacing = EnumFacing.WEST;
                break;

            case 3:
                enumfacing = EnumFacing.SOUTH;
                break;

            case 4:
                enumfacing = EnumFacing.NORTH;
                break;

            case 5:
            default:
                enumfacing = EnumFacing.UP;
        }

        return this.getDefaultState().withProperty(FACING, enumfacing).withProperty(POWERED, Boolean.valueOf((meta & 8) > 0));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i;

        switch ((EnumFacing)state.getValue(FACING))
        {
            case EAST:
                i = 1;
                break;

            case WEST:
                i = 2;
                break;

            case SOUTH:
                i = 3;
                break;

            case NORTH:
                i = 4;
                break;

            case UP:
            default:
                i = 5;
                break;

            case DOWN:
                i = 0;
        }

        if (((Boolean)state.getValue(POWERED)).booleanValue())
        {
            i |= 8;
        }

        return i;
    }

    /**
     * Returns the blockstate with the given rotation from the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withRotation(IBlockState state, Rotation rot)
    {
        return state.withProperty(FACING, rot.func_185831_a((EnumFacing)state.getValue(FACING)));
    }

    /**
     * Returns the blockstate with the given mirror of the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
    {
        return state.withRotation(mirrorIn.func_185800_a((EnumFacing)state.getValue(FACING)));
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {FACING, POWERED});
    }
}
