package net.minecraft.block;

import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.properties.PropertyEnum;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.IStringSerializable;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockLever extends Block
{
    public static final PropertyEnum<BlockLever.EnumOrientation> FACING = PropertyEnum.<BlockLever.EnumOrientation>create("facing", BlockLever.EnumOrientation.class);
    public static final PropertyBool POWERED = PropertyBool.create("powered");
    protected static final AxisAlignedBB field_185692_c = new AxisAlignedBB(0.3125D, 0.20000000298023224D, 0.625D, 0.6875D, 0.800000011920929D, 1.0D);
    protected static final AxisAlignedBB field_185693_d = new AxisAlignedBB(0.3125D, 0.20000000298023224D, 0.0D, 0.6875D, 0.800000011920929D, 0.375D);
    protected static final AxisAlignedBB field_185694_e = new AxisAlignedBB(0.625D, 0.20000000298023224D, 0.3125D, 1.0D, 0.800000011920929D, 0.6875D);
    protected static final AxisAlignedBB field_185695_f = new AxisAlignedBB(0.0D, 0.20000000298023224D, 0.3125D, 0.375D, 0.800000011920929D, 0.6875D);
    protected static final AxisAlignedBB field_185696_g = new AxisAlignedBB(0.25D, 0.0D, 0.25D, 0.75D, 0.6000000238418579D, 0.75D);
    protected static final AxisAlignedBB field_185691_B = new AxisAlignedBB(0.25D, 0.4000000059604645D, 0.25D, 0.75D, 1.0D, 0.75D);

    protected BlockLever()
    {
        super(Material.circuits);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, BlockLever.EnumOrientation.NORTH).withProperty(POWERED, Boolean.valueOf(false)));
        this.setCreativeTab(CreativeTabs.tabRedstone);
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
        return func_181090_a(worldIn, pos, side.getOpposite());
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        for (EnumFacing enumfacing : EnumFacing.values())
        {
            if (func_181090_a(worldIn, pos, enumfacing))
            {
                return true;
            }
        }

        return false;
    }

    protected static boolean func_181090_a(World p_181090_0_, BlockPos p_181090_1_, EnumFacing p_181090_2_)
    {
        return BlockButton.func_181088_a(p_181090_0_, p_181090_1_, p_181090_2_);
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        IBlockState iblockstate = this.getDefaultState().withProperty(POWERED, Boolean.valueOf(false));

        if (func_181090_a(worldIn, pos, facing.getOpposite()))
        {
            return iblockstate.withProperty(FACING, BlockLever.EnumOrientation.forFacings(facing, placer.getHorizontalFacing()));
        }
        else
        {
            for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
            {
                if (enumfacing != facing && func_181090_a(worldIn, pos, enumfacing.getOpposite()))
                {
                    return iblockstate.withProperty(FACING, BlockLever.EnumOrientation.forFacings(enumfacing, placer.getHorizontalFacing()));
                }
            }

            if (worldIn.getBlockState(pos.down()).func_185896_q())
            {
                return iblockstate.withProperty(FACING, BlockLever.EnumOrientation.forFacings(EnumFacing.UP, placer.getHorizontalFacing()));
            }
            else
            {
                return iblockstate;
            }
        }
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (this.func_181091_e(worldIn, pos, state) && !func_181090_a(worldIn, pos, ((BlockLever.EnumOrientation)state.getValue(FACING)).getFacing().getOpposite()))
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockToAir(pos);
        }
    }

    private boolean func_181091_e(World p_181091_1_, BlockPos p_181091_2_, IBlockState p_181091_3_)
    {
        if (this.canPlaceBlockAt(p_181091_1_, p_181091_2_))
        {
            return true;
        }
        else
        {
            this.dropBlockAsItem(p_181091_1_, p_181091_2_, p_181091_3_, 0);
            p_181091_1_.setBlockToAir(p_181091_2_);
            return false;
        }
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        switch ((BlockLever.EnumOrientation)state.getValue(FACING))
        {
            case EAST:
            default:
                return field_185695_f;

            case WEST:
                return field_185694_e;

            case SOUTH:
                return field_185693_d;

            case NORTH:
                return field_185692_c;

            case UP_Z:
            case UP_X:
                return field_185696_g;

            case DOWN_X:
            case DOWN_Z:
                return field_185691_B;
        }
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (worldIn.isRemote)
        {
            return true;
        }
        else
        {
            state = state.cycleProperty(POWERED);
            worldIn.setBlockState(pos, state, 3);
            float f = ((Boolean)state.getValue(POWERED)).booleanValue() ? 0.6F : 0.5F;
            worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.block_lever_click, SoundCategory.BLOCKS, 0.3F, f);
            worldIn.notifyNeighborsOfStateChange(pos, this);
            EnumFacing enumfacing = ((BlockLever.EnumOrientation)state.getValue(FACING)).getFacing();
            worldIn.notifyNeighborsOfStateChange(pos.offset(enumfacing.getOpposite()), this);
            return true;
        }
    }

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        if (((Boolean)state.getValue(POWERED)).booleanValue())
        {
            worldIn.notifyNeighborsOfStateChange(pos, this);
            EnumFacing enumfacing = ((BlockLever.EnumOrientation)state.getValue(FACING)).getFacing();
            worldIn.notifyNeighborsOfStateChange(pos.offset(enumfacing.getOpposite()), this);
        }

        super.breakBlock(worldIn, pos, state);
    }

    public int getWeakPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return ((Boolean)worldIn.getValue(POWERED)).booleanValue() ? 15 : 0;
    }

    public int getStrongPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return !((Boolean)worldIn.getValue(POWERED)).booleanValue() ? 0 : (((BlockLever.EnumOrientation)worldIn.getValue(FACING)).getFacing() == side ? 15 : 0);
    }

    /**
     * Can this block provide power. Only wire currently seems to have this change based on its state.
     */
    public boolean canProvidePower(IBlockState state)
    {
        return true;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(FACING, BlockLever.EnumOrientation.byMetadata(meta & 7)).withProperty(POWERED, Boolean.valueOf((meta & 8) > 0));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i = 0;
        i = i | ((BlockLever.EnumOrientation)state.getValue(FACING)).getMetadata();

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
        switch (rot)
        {
            case CLOCKWISE_180:
                switch ((BlockLever.EnumOrientation)state.getValue(FACING))
                {
                    case EAST:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.WEST);

                    case WEST:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.EAST);

                    case SOUTH:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.NORTH);

                    case NORTH:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.SOUTH);

                    default:
                        return state;
                }

            case COUNTERCLOCKWISE_90:
                switch ((BlockLever.EnumOrientation)state.getValue(FACING))
                {
                    case EAST:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.NORTH);

                    case WEST:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.SOUTH);

                    case SOUTH:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.EAST);

                    case NORTH:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.WEST);

                    case UP_Z:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.UP_X);

                    case UP_X:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.UP_Z);

                    case DOWN_X:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.DOWN_Z);

                    case DOWN_Z:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.DOWN_X);
                }

            case CLOCKWISE_90:
                switch ((BlockLever.EnumOrientation)state.getValue(FACING))
                {
                    case EAST:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.SOUTH);

                    case WEST:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.NORTH);

                    case SOUTH:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.WEST);

                    case NORTH:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.EAST);

                    case UP_Z:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.UP_X);

                    case UP_X:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.UP_Z);

                    case DOWN_X:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.DOWN_Z);

                    case DOWN_Z:
                        return state.withProperty(FACING, BlockLever.EnumOrientation.DOWN_X);
                }

            default:
                return state;
        }
    }

    /**
     * Returns the blockstate with the given mirror of the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
    {
        return state.withRotation(mirrorIn.func_185800_a(((BlockLever.EnumOrientation)state.getValue(FACING)).getFacing()));
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {FACING, POWERED});
    }

    public static enum EnumOrientation implements IStringSerializable
    {
        DOWN_X(0, "down_x", EnumFacing.DOWN),
        EAST(1, "east", EnumFacing.EAST),
        WEST(2, "west", EnumFacing.WEST),
        SOUTH(3, "south", EnumFacing.SOUTH),
        NORTH(4, "north", EnumFacing.NORTH),
        UP_Z(5, "up_z", EnumFacing.UP),
        UP_X(6, "up_x", EnumFacing.UP),
        DOWN_Z(7, "down_z", EnumFacing.DOWN);

        private static final BlockLever.EnumOrientation[] META_LOOKUP = new BlockLever.EnumOrientation[values().length];
        private final int meta;
        private final String name;
        private final EnumFacing facing;

        private EnumOrientation(int meta, String name, EnumFacing facing)
        {
            this.meta = meta;
            this.name = name;
            this.facing = facing;
        }

        public int getMetadata()
        {
            return this.meta;
        }

        public EnumFacing getFacing()
        {
            return this.facing;
        }

        public String toString()
        {
            return this.name;
        }

        public static BlockLever.EnumOrientation byMetadata(int meta)
        {
            if (meta < 0 || meta >= META_LOOKUP.length)
            {
                meta = 0;
            }

            return META_LOOKUP[meta];
        }

        public static BlockLever.EnumOrientation forFacings(EnumFacing clickedSide, EnumFacing entityFacing)
        {
            switch (clickedSide)
            {
                case DOWN:
                    switch (entityFacing.getAxis())
                    {
                        case X:
                            return DOWN_X;

                        case Z:
                            return DOWN_Z;

                        default:
                            throw new IllegalArgumentException("Invalid entityFacing " + entityFacing + " for facing " + clickedSide);
                    }

                case UP:
                    switch (entityFacing.getAxis())
                    {
                        case X:
                            return UP_X;

                        case Z:
                            return UP_Z;

                        default:
                            throw new IllegalArgumentException("Invalid entityFacing " + entityFacing + " for facing " + clickedSide);
                    }

                case NORTH:
                    return NORTH;

                case SOUTH:
                    return SOUTH;

                case WEST:
                    return WEST;

                case EAST:
                    return EAST;

                default:
                    throw new IllegalArgumentException("Invalid facing: " + clickedSide);
            }
        }

        public String getName()
        {
            return this.name;
        }

        static {
            for (BlockLever.EnumOrientation blocklever$enumorientation : values())
            {
                META_LOOKUP[blocklever$enumorientation.getMetadata()] = blocklever$enumorientation;
            }
        }
    }
}
