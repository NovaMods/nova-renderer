package net.minecraft.block;

import com.google.common.base.Objects;
import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.properties.PropertyDirection;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemStack;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockTripWireHook extends Block
{
    public static final PropertyDirection FACING = BlockHorizontal.FACING;
    public static final PropertyBool POWERED = PropertyBool.create("powered");
    public static final PropertyBool ATTACHED = PropertyBool.create("attached");
    protected static final AxisAlignedBB field_185743_d = new AxisAlignedBB(0.3125D, 0.0D, 0.625D, 0.6875D, 0.625D, 1.0D);
    protected static final AxisAlignedBB field_185744_e = new AxisAlignedBB(0.3125D, 0.0D, 0.0D, 0.6875D, 0.625D, 0.375D);
    protected static final AxisAlignedBB field_185745_f = new AxisAlignedBB(0.625D, 0.0D, 0.3125D, 1.0D, 0.625D, 0.6875D);
    protected static final AxisAlignedBB field_185746_g = new AxisAlignedBB(0.0D, 0.0D, 0.3125D, 0.375D, 0.625D, 0.6875D);

    public BlockTripWireHook()
    {
        super(Material.circuits);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH).withProperty(POWERED, Boolean.valueOf(false)).withProperty(ATTACHED, Boolean.valueOf(false)));
        this.setCreativeTab(CreativeTabs.tabRedstone);
        this.setTickRandomly(true);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        switch ((EnumFacing)state.getValue(FACING))
        {
            case EAST:
            default:
                return field_185746_g;

            case WEST:
                return field_185745_f;

            case SOUTH:
                return field_185744_e;

            case NORTH:
                return field_185743_d;
        }
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

    /**
     * Check whether this Block can be placed on the given side
     */
    public boolean canPlaceBlockOnSide(World worldIn, BlockPos pos, EnumFacing side)
    {
        return side.getAxis().isHorizontal() && worldIn.getBlockState(pos.offset(side.getOpposite())).isNormalCube();
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
        {
            if (worldIn.getBlockState(pos.offset(enumfacing)).isNormalCube())
            {
                return true;
            }
        }

        return false;
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        IBlockState iblockstate = this.getDefaultState().withProperty(POWERED, Boolean.valueOf(false)).withProperty(ATTACHED, Boolean.valueOf(false));

        if (facing.getAxis().isHorizontal())
        {
            iblockstate = iblockstate.withProperty(FACING, facing);
        }

        return iblockstate;
    }

    /**
     * Called by ItemBlocks after a block is set in the world, to allow post-place logic
     */
    public void onBlockPlacedBy(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase placer, ItemStack stack)
    {
        this.func_176260_a(worldIn, pos, state, false, false, -1, (IBlockState)null);
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (neighborBlock != this)
        {
            if (this.checkForDrop(worldIn, pos, state))
            {
                EnumFacing enumfacing = (EnumFacing)state.getValue(FACING);

                if (!worldIn.getBlockState(pos.offset(enumfacing.getOpposite())).isNormalCube())
                {
                    this.dropBlockAsItem(worldIn, pos, state, 0);
                    worldIn.setBlockToAir(pos);
                }
            }
        }
    }

    public void func_176260_a(World worldIn, BlockPos pos, IBlockState hookState, boolean p_176260_4_, boolean p_176260_5_, int p_176260_6_, IBlockState p_176260_7_)
    {
        EnumFacing enumfacing = (EnumFacing)hookState.getValue(FACING);
        boolean flag = ((Boolean)hookState.getValue(ATTACHED)).booleanValue();
        boolean flag1 = ((Boolean)hookState.getValue(POWERED)).booleanValue();
        boolean flag2 = !p_176260_4_;
        boolean flag3 = false;
        int i = 0;
        IBlockState[] aiblockstate = new IBlockState[42];

        for (int j = 1; j < 42; ++j)
        {
            BlockPos blockpos = pos.offset(enumfacing, j);
            IBlockState iblockstate = worldIn.getBlockState(blockpos);

            if (iblockstate.getBlock() == Blocks.tripwire_hook)
            {
                if (iblockstate.getValue(FACING) == enumfacing.getOpposite())
                {
                    i = j;
                }

                break;
            }

            if (iblockstate.getBlock() != Blocks.tripwire && j != p_176260_6_)
            {
                aiblockstate[j] = null;
                flag2 = false;
            }
            else
            {
                if (j == p_176260_6_)
                {
                    iblockstate = (IBlockState)Objects.firstNonNull(p_176260_7_, iblockstate);
                }

                boolean flag4 = !((Boolean)iblockstate.getValue(BlockTripWire.DISARMED)).booleanValue();
                boolean flag5 = ((Boolean)iblockstate.getValue(BlockTripWire.POWERED)).booleanValue();
                flag3 |= flag4 && flag5;
                aiblockstate[j] = iblockstate;

                if (j == p_176260_6_)
                {
                    worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn));
                    flag2 &= flag4;
                }
            }
        }

        flag2 = flag2 & i > 1;
        flag3 = flag3 & flag2;
        IBlockState iblockstate1 = this.getDefaultState().withProperty(ATTACHED, Boolean.valueOf(flag2)).withProperty(POWERED, Boolean.valueOf(flag3));

        if (i > 0)
        {
            BlockPos blockpos1 = pos.offset(enumfacing, i);
            EnumFacing enumfacing1 = enumfacing.getOpposite();
            worldIn.setBlockState(blockpos1, iblockstate1.withProperty(FACING, enumfacing1), 3);
            this.func_176262_b(worldIn, blockpos1, enumfacing1);
            this.func_180694_a(worldIn, blockpos1, flag2, flag3, flag, flag1);
        }

        this.func_180694_a(worldIn, pos, flag2, flag3, flag, flag1);

        if (!p_176260_4_)
        {
            worldIn.setBlockState(pos, iblockstate1.withProperty(FACING, enumfacing), 3);

            if (p_176260_5_)
            {
                this.func_176262_b(worldIn, pos, enumfacing);
            }
        }

        if (flag != flag2)
        {
            for (int k = 1; k < i; ++k)
            {
                BlockPos blockpos2 = pos.offset(enumfacing, k);
                IBlockState iblockstate2 = aiblockstate[k];

                if (iblockstate2 != null && worldIn.getBlockState(blockpos2).getBlock() != Blocks.air)
                {
                    worldIn.setBlockState(blockpos2, iblockstate2.withProperty(ATTACHED, Boolean.valueOf(flag2)), 3);
                }
            }
        }
    }

    /**
     * Called randomly when setTickRandomly is set to true (used by e.g. crops to grow, etc.)
     */
    public void randomTick(World worldIn, BlockPos pos, IBlockState state, Random random)
    {
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        this.func_176260_a(worldIn, pos, state, false, true, -1, (IBlockState)null);
    }

    private void func_180694_a(World worldIn, BlockPos pos, boolean p_180694_3_, boolean p_180694_4_, boolean p_180694_5_, boolean p_180694_6_)
    {
        if (p_180694_4_ && !p_180694_6_)
        {
            worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.block_tripwire_click_on, SoundCategory.BLOCKS, 0.4F, 0.6F);
        }
        else if (!p_180694_4_ && p_180694_6_)
        {
            worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.block_tripwire_click_off, SoundCategory.BLOCKS, 0.4F, 0.5F);
        }
        else if (p_180694_3_ && !p_180694_5_)
        {
            worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.block_tripwire_attach, SoundCategory.BLOCKS, 0.4F, 0.7F);
        }
        else if (!p_180694_3_ && p_180694_5_)
        {
            worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.block_tripwire_detach, SoundCategory.BLOCKS, 0.4F, 1.2F / (worldIn.rand.nextFloat() * 0.2F + 0.9F));
        }
    }

    private void func_176262_b(World worldIn, BlockPos p_176262_2_, EnumFacing p_176262_3_)
    {
        worldIn.notifyNeighborsOfStateChange(p_176262_2_, this);
        worldIn.notifyNeighborsOfStateChange(p_176262_2_.offset(p_176262_3_.getOpposite()), this);
    }

    private boolean checkForDrop(World worldIn, BlockPos pos, IBlockState state)
    {
        if (!this.canPlaceBlockAt(worldIn, pos))
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockToAir(pos);
            return false;
        }
        else
        {
            return true;
        }
    }

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        boolean flag = ((Boolean)state.getValue(ATTACHED)).booleanValue();
        boolean flag1 = ((Boolean)state.getValue(POWERED)).booleanValue();

        if (flag || flag1)
        {
            this.func_176260_a(worldIn, pos, state, true, false, -1, (IBlockState)null);
        }

        if (flag1)
        {
            worldIn.notifyNeighborsOfStateChange(pos, this);
            worldIn.notifyNeighborsOfStateChange(pos.offset(((EnumFacing)state.getValue(FACING)).getOpposite()), this);
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

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT_MIPPED;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(FACING, EnumFacing.getHorizontal(meta & 3)).withProperty(POWERED, Boolean.valueOf((meta & 8) > 0)).withProperty(ATTACHED, Boolean.valueOf((meta & 4) > 0));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i = 0;
        i = i | ((EnumFacing)state.getValue(FACING)).getHorizontalIndex();

        if (((Boolean)state.getValue(POWERED)).booleanValue())
        {
            i |= 8;
        }

        if (((Boolean)state.getValue(ATTACHED)).booleanValue())
        {
            i |= 4;
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
        return new BlockStateContainer(this, new IProperty[] {FACING, POWERED, ATTACHED});
    }
}
