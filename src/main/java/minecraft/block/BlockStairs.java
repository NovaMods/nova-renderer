package net.minecraft.block;

import com.google.common.collect.Lists;
import java.util.List;
import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyDirection;
import net.minecraft.block.properties.PropertyEnum;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.item.ItemStack;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.IStringSerializable;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.Explosion;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockStairs extends Block
{
    public static final PropertyDirection FACING = BlockHorizontal.FACING;
    public static final PropertyEnum<BlockStairs.EnumHalf> HALF = PropertyEnum.<BlockStairs.EnumHalf>create("half", BlockStairs.EnumHalf.class);
    public static final PropertyEnum<BlockStairs.EnumShape> SHAPE = PropertyEnum.<BlockStairs.EnumShape>create("shape", BlockStairs.EnumShape.class);
    protected static final AxisAlignedBB field_185712_d = new AxisAlignedBB(0.0D, 0.5D, 0.0D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185714_e = new AxisAlignedBB(0.0D, 0.5D, 0.0D, 0.5D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185716_f = new AxisAlignedBB(0.5D, 0.5D, 0.0D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185718_g = new AxisAlignedBB(0.0D, 0.5D, 0.0D, 1.0D, 1.0D, 0.5D);
    protected static final AxisAlignedBB field_185710_B = new AxisAlignedBB(0.0D, 0.5D, 0.5D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185711_C = new AxisAlignedBB(0.0D, 0.5D, 0.0D, 0.5D, 1.0D, 0.5D);
    protected static final AxisAlignedBB field_185713_D = new AxisAlignedBB(0.5D, 0.5D, 0.0D, 1.0D, 1.0D, 0.5D);
    protected static final AxisAlignedBB field_185715_E = new AxisAlignedBB(0.0D, 0.5D, 0.5D, 0.5D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185717_F = new AxisAlignedBB(0.5D, 0.5D, 0.5D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185719_G = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.5D, 1.0D);
    protected static final AxisAlignedBB field_185720_H = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.5D, 0.5D, 1.0D);
    protected static final AxisAlignedBB field_185721_I = new AxisAlignedBB(0.5D, 0.0D, 0.0D, 1.0D, 0.5D, 1.0D);
    protected static final AxisAlignedBB field_185722_J = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.5D, 0.5D);
    protected static final AxisAlignedBB field_185723_K = new AxisAlignedBB(0.0D, 0.0D, 0.5D, 1.0D, 0.5D, 1.0D);
    protected static final AxisAlignedBB field_185724_L = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.5D, 0.5D, 0.5D);
    protected static final AxisAlignedBB field_185725_M = new AxisAlignedBB(0.5D, 0.0D, 0.0D, 1.0D, 0.5D, 0.5D);
    protected static final AxisAlignedBB field_185726_N = new AxisAlignedBB(0.0D, 0.0D, 0.5D, 0.5D, 0.5D, 1.0D);
    protected static final AxisAlignedBB field_185727_O = new AxisAlignedBB(0.5D, 0.0D, 0.5D, 1.0D, 0.5D, 1.0D);
    private final Block modelBlock;
    private final IBlockState modelState;

    protected BlockStairs(IBlockState modelState)
    {
        super(modelState.getBlock().blockMaterial);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH).withProperty(HALF, BlockStairs.EnumHalf.BOTTOM).withProperty(SHAPE, BlockStairs.EnumShape.STRAIGHT));
        this.modelBlock = modelState.getBlock();
        this.modelState = modelState;
        this.setHardness(this.modelBlock.blockHardness);
        this.setResistance(this.modelBlock.blockResistance / 3.0F);
        this.setStepSound(this.modelBlock.stepSound);
        this.setLightOpacity(255);
        this.setCreativeTab(CreativeTabs.tabBlock);
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
        state = this.getActualState(state, worldIn, pos);

        for (AxisAlignedBB axisalignedbb : func_185708_x(state))
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, axisalignedbb);
        }
    }

    private static List<AxisAlignedBB> func_185708_x(IBlockState p_185708_0_)
    {
        List<AxisAlignedBB> list = Lists.<AxisAlignedBB>newArrayList();
        boolean flag = p_185708_0_.getValue(HALF) == BlockStairs.EnumHalf.TOP;
        list.add(flag ? field_185712_d : field_185719_G);
        BlockStairs.EnumShape blockstairs$enumshape = (BlockStairs.EnumShape)p_185708_0_.getValue(SHAPE);

        if (blockstairs$enumshape == BlockStairs.EnumShape.STRAIGHT || blockstairs$enumshape == BlockStairs.EnumShape.INNER_LEFT || blockstairs$enumshape == BlockStairs.EnumShape.INNER_RIGHT)
        {
            list.add(func_185707_y(p_185708_0_));
        }

        if (blockstairs$enumshape != BlockStairs.EnumShape.STRAIGHT)
        {
            list.add(func_185705_z(p_185708_0_));
        }

        return list;
    }

    private static AxisAlignedBB func_185707_y(IBlockState p_185707_0_)
    {
        boolean flag = p_185707_0_.getValue(HALF) == BlockStairs.EnumHalf.TOP;

        switch ((EnumFacing)p_185707_0_.getValue(FACING))
        {
            case NORTH:
            default:
                return flag ? field_185722_J : field_185718_g;

            case SOUTH:
                return flag ? field_185723_K : field_185710_B;

            case WEST:
                return flag ? field_185720_H : field_185714_e;

            case EAST:
                return flag ? field_185721_I : field_185716_f;
        }
    }

    private static AxisAlignedBB func_185705_z(IBlockState p_185705_0_)
    {
        EnumFacing enumfacing = (EnumFacing)p_185705_0_.getValue(FACING);
        EnumFacing enumfacing1;

        switch ((BlockStairs.EnumShape)p_185705_0_.getValue(SHAPE))
        {
            case OUTER_LEFT:
            default:
                enumfacing1 = enumfacing;
                break;

            case OUTER_RIGHT:
                enumfacing1 = enumfacing.rotateY();
                break;

            case INNER_RIGHT:
                enumfacing1 = enumfacing.getOpposite();
                break;

            case INNER_LEFT:
                enumfacing1 = enumfacing.rotateYCCW();
        }

        boolean flag = p_185705_0_.getValue(HALF) == BlockStairs.EnumHalf.TOP;

        switch (enumfacing1)
        {
            case NORTH:
            default:
                return flag ? field_185724_L : field_185711_C;

            case SOUTH:
                return flag ? field_185727_O : field_185717_F;

            case WEST:
                return flag ? field_185726_N : field_185715_E;

            case EAST:
                return flag ? field_185725_M : field_185713_D;
        }
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

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        this.modelBlock.randomDisplayTick(worldIn, pos, state, rand);
    }

    public void onBlockClicked(World worldIn, BlockPos pos, EntityPlayer playerIn)
    {
        this.modelBlock.onBlockClicked(worldIn, pos, playerIn);
    }

    /**
     * Called when a player destroys this Block
     */
    public void onBlockDestroyedByPlayer(World worldIn, BlockPos pos, IBlockState state)
    {
        this.modelBlock.onBlockDestroyedByPlayer(worldIn, pos, state);
    }

    public int getPackedLightmapCoords(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return this.modelState.getPackedLightmapCoords(source, pos);
    }

    /**
     * Returns how much this block can resist explosions from the passed in entity.
     */
    public float getExplosionResistance(Entity exploder)
    {
        return this.modelBlock.getExplosionResistance(exploder);
    }

    public BlockRenderLayer getBlockLayer()
    {
        return this.modelBlock.getBlockLayer();
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return this.modelBlock.tickRate(worldIn);
    }

    public AxisAlignedBB getCollisionBoundingBox(IBlockState worldIn, World pos, BlockPos state)
    {
        return this.modelState.getCollisionBoundingBox(pos, state);
    }

    public Vec3d modifyAcceleration(World worldIn, BlockPos pos, Entity entityIn, Vec3d motion)
    {
        return this.modelBlock.modifyAcceleration(worldIn, pos, entityIn, motion);
    }

    /**
     * Returns if this block is collidable (only used by Fire). Args: x, y, z
     */
    public boolean isCollidable()
    {
        return this.modelBlock.isCollidable();
    }

    public boolean canCollideCheck(IBlockState state, boolean hitIfLiquid)
    {
        return this.modelBlock.canCollideCheck(state, hitIfLiquid);
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return this.modelBlock.canPlaceBlockAt(worldIn, pos);
    }

    public void onBlockAdded(World worldIn, BlockPos pos, IBlockState state)
    {
        this.onNeighborBlockChange(worldIn, pos, this.modelState, Blocks.air);
        this.modelBlock.onBlockAdded(worldIn, pos, this.modelState);
    }

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        this.modelBlock.breakBlock(worldIn, pos, this.modelState);
    }

    /**
     * Triggered whenever an entity collides with this block (enters into the block)
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, Entity entityIn)
    {
        this.modelBlock.onEntityCollidedWithBlock(worldIn, pos, entityIn);
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        this.modelBlock.updateTick(worldIn, pos, state, rand);
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        return this.modelBlock.onBlockActivated(worldIn, pos, this.modelState, playerIn, side, hitX, EnumFacing.DOWN, 0.0F, 0.0F, 0.0F);
    }

    /**
     * Called when this Block is destroyed by an Explosion
     */
    public void onBlockDestroyedByExplosion(World worldIn, BlockPos pos, Explosion explosionIn)
    {
        this.modelBlock.onBlockDestroyedByExplosion(worldIn, pos, explosionIn);
    }

    /**
     * Checks if an IBlockState represents a block that is opaque and a full cube.
     *  
     * @param state The block state to check.
     */
    public boolean isFullyOpaque(IBlockState state)
    {
        return state.getValue(HALF) == BlockStairs.EnumHalf.TOP;
    }

    /**
     * Get the MapColor for this Block and the given BlockState
     */
    public MapColor getMapColor(IBlockState state)
    {
        return this.modelBlock.getMapColor(this.modelState);
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        IBlockState iblockstate = super.onBlockPlaced(worldIn, pos, facing, hitX, hitY, hitZ, meta, placer);
        iblockstate = iblockstate.withProperty(FACING, placer.getHorizontalFacing()).withProperty(SHAPE, BlockStairs.EnumShape.STRAIGHT);
        return facing != EnumFacing.DOWN && (facing == EnumFacing.UP || (double)hitY <= 0.5D) ? iblockstate.withProperty(HALF, BlockStairs.EnumHalf.BOTTOM) : iblockstate.withProperty(HALF, BlockStairs.EnumHalf.TOP);
    }

    /**
     * Ray traces through the blocks collision from start vector to end vector returning a ray trace hit.
     */
    public RayTraceResult collisionRayTrace(IBlockState worldIn, World pos, BlockPos start, Vec3d end, Vec3d p_180636_5_)
    {
        List<RayTraceResult> list = Lists.<RayTraceResult>newArrayList();

        for (AxisAlignedBB axisalignedbb : func_185708_x(this.getActualState(worldIn, pos, start)))
        {
            list.add(this.func_185503_a(start, end, p_180636_5_, axisalignedbb));
        }

        RayTraceResult raytraceresult1 = null;
        double d1 = 0.0D;

        for (RayTraceResult raytraceresult : list)
        {
            if (raytraceresult != null)
            {
                double d0 = raytraceresult.hitVec.squareDistanceTo(p_180636_5_);

                if (d0 > d1)
                {
                    raytraceresult1 = raytraceresult;
                    d1 = d0;
                }
            }
        }

        return raytraceresult1;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        IBlockState iblockstate = this.getDefaultState().withProperty(HALF, (meta & 4) > 0 ? BlockStairs.EnumHalf.TOP : BlockStairs.EnumHalf.BOTTOM);
        iblockstate = iblockstate.withProperty(FACING, EnumFacing.getFront(5 - (meta & 3)));
        return iblockstate;
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i = 0;

        if (state.getValue(HALF) == BlockStairs.EnumHalf.TOP)
        {
            i |= 4;
        }

        i = i | 5 - ((EnumFacing)state.getValue(FACING)).getIndex();
        return i;
    }

    /**
     * Get the actual Block state of this Block at the given position. This applies properties not visible in the
     * metadata, such as fence connections.
     */
    public IBlockState getActualState(IBlockState state, IBlockAccess worldIn, BlockPos pos)
    {
        return state.withProperty(SHAPE, func_185706_d(state, worldIn, pos));
    }

    private static BlockStairs.EnumShape func_185706_d(IBlockState p_185706_0_, IBlockAccess p_185706_1_, BlockPos p_185706_2_)
    {
        EnumFacing enumfacing = (EnumFacing)p_185706_0_.getValue(FACING);
        IBlockState iblockstate = p_185706_1_.getBlockState(p_185706_2_.offset(enumfacing));

        if (func_185709_i(iblockstate) && p_185706_0_.getValue(HALF) == iblockstate.getValue(HALF))
        {
            EnumFacing enumfacing1 = (EnumFacing)iblockstate.getValue(FACING);

            if (enumfacing1.getAxis() != ((EnumFacing)p_185706_0_.getValue(FACING)).getAxis() && func_185704_d(p_185706_0_, p_185706_1_, p_185706_2_, enumfacing1.getOpposite()))
            {
                if (enumfacing1 == enumfacing.rotateYCCW())
                {
                    return BlockStairs.EnumShape.OUTER_LEFT;
                }

                return BlockStairs.EnumShape.OUTER_RIGHT;
            }
        }

        IBlockState iblockstate1 = p_185706_1_.getBlockState(p_185706_2_.offset(enumfacing.getOpposite()));

        if (func_185709_i(iblockstate1) && p_185706_0_.getValue(HALF) == iblockstate1.getValue(HALF))
        {
            EnumFacing enumfacing2 = (EnumFacing)iblockstate1.getValue(FACING);

            if (enumfacing2.getAxis() != ((EnumFacing)p_185706_0_.getValue(FACING)).getAxis() && func_185704_d(p_185706_0_, p_185706_1_, p_185706_2_, enumfacing2))
            {
                if (enumfacing2 == enumfacing.rotateYCCW())
                {
                    return BlockStairs.EnumShape.INNER_LEFT;
                }

                return BlockStairs.EnumShape.INNER_RIGHT;
            }
        }

        return BlockStairs.EnumShape.STRAIGHT;
    }

    private static boolean func_185704_d(IBlockState p_185704_0_, IBlockAccess p_185704_1_, BlockPos p_185704_2_, EnumFacing p_185704_3_)
    {
        IBlockState iblockstate = p_185704_1_.getBlockState(p_185704_2_.offset(p_185704_3_));
        return !func_185709_i(iblockstate) || iblockstate.getValue(FACING) != p_185704_0_.getValue(FACING) || iblockstate.getValue(HALF) != p_185704_0_.getValue(HALF);
    }

    public static boolean func_185709_i(IBlockState p_185709_0_)
    {
        return p_185709_0_.getBlock() instanceof BlockStairs;
    }

    /**
     * Returns the blockstate with the given rotation from the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withRotation(IBlockState state, Rotation rot)
    {
        return state.withProperty(FACING, rot.func_185831_a((EnumFacing)state.getValue(FACING)));
    }

    @SuppressWarnings("incomplete-switch")

    /**
     * Returns the blockstate with the given mirror of the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
    {
        EnumFacing enumfacing = (EnumFacing)state.getValue(FACING);
        BlockStairs.EnumShape blockstairs$enumshape = (BlockStairs.EnumShape)state.getValue(SHAPE);

        switch (mirrorIn)
        {
            case LEFT_RIGHT:
                if (enumfacing.getAxis() == EnumFacing.Axis.Z)
                {
                    switch (blockstairs$enumshape)
                    {
                        case OUTER_LEFT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.OUTER_RIGHT);

                        case OUTER_RIGHT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.OUTER_LEFT);

                        case INNER_RIGHT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.INNER_LEFT);

                        case INNER_LEFT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.INNER_RIGHT);

                        default:
                            return state.withRotation(Rotation.CLOCKWISE_180);
                    }
                }

                break;

            case FRONT_BACK:
                if (enumfacing.getAxis() == EnumFacing.Axis.X)
                {
                    switch (blockstairs$enumshape)
                    {
                        case OUTER_LEFT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.OUTER_RIGHT);

                        case OUTER_RIGHT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.OUTER_LEFT);

                        case INNER_RIGHT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.INNER_RIGHT);

                        case INNER_LEFT:
                            return state.withRotation(Rotation.CLOCKWISE_180).withProperty(SHAPE, BlockStairs.EnumShape.INNER_LEFT);

                        case STRAIGHT:
                            return state.withRotation(Rotation.CLOCKWISE_180);
                    }
                }
        }

        return super.withMirror(state, mirrorIn);
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {FACING, HALF, SHAPE});
    }

    public static enum EnumHalf implements IStringSerializable
    {
        TOP("top"),
        BOTTOM("bottom");

        private final String name;

        private EnumHalf(String name)
        {
            this.name = name;
        }

        public String toString()
        {
            return this.name;
        }

        public String getName()
        {
            return this.name;
        }
    }

    public static enum EnumShape implements IStringSerializable
    {
        STRAIGHT("straight"),
        INNER_LEFT("inner_left"),
        INNER_RIGHT("inner_right"),
        OUTER_LEFT("outer_left"),
        OUTER_RIGHT("outer_right");

        private final String name;

        private EnumShape(String name)
        {
            this.name = name;
        }

        public String toString()
        {
            return this.name;
        }

        public String getName()
        {
            return this.name;
        }
    }
}
