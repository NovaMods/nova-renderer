package net.minecraft.block;

import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.state.BlockPistonStructureHelper;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemStack;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityPiston;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockPistonBase extends BlockDirectional
{
    public static final PropertyBool EXTENDED = PropertyBool.create("extended");
    protected static final AxisAlignedBB field_185648_b = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.75D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185649_c = new AxisAlignedBB(0.25D, 0.0D, 0.0D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185650_d = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 1.0D, 0.75D);
    protected static final AxisAlignedBB field_185651_e = new AxisAlignedBB(0.0D, 0.0D, 0.25D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185652_f = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.75D, 1.0D);
    protected static final AxisAlignedBB field_185653_g = new AxisAlignedBB(0.0D, 0.25D, 0.0D, 1.0D, 1.0D, 1.0D);

    /** This piston is the sticky one? */
    private final boolean isSticky;

    public BlockPistonBase(boolean isSticky)
    {
        super(Material.piston);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH).withProperty(EXTENDED, Boolean.valueOf(false)));
        this.isSticky = isSticky;
        this.setStepSound(SoundType.STONE);
        this.setHardness(0.5F);
        this.setCreativeTab(CreativeTabs.tabRedstone);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        if (((Boolean)state.getValue(EXTENDED)).booleanValue())
        {
            switch ((EnumFacing)state.getValue(FACING))
            {
                case DOWN:
                    return field_185653_g;

                case UP:
                default:
                    return field_185652_f;

                case NORTH:
                    return field_185651_e;

                case SOUTH:
                    return field_185650_d;

                case WEST:
                    return field_185649_c;

                case EAST:
                    return field_185648_b;
            }
        }
        else
        {
            return FULL_BLOCK_AABB;
        }
    }

    /**
     * Checks if an IBlockState represents a block that is opaque and a full cube.
     *  
     * @param state The block state to check.
     */
    public boolean isFullyOpaque(IBlockState state)
    {
        return !((Boolean)state.getValue(EXTENDED)).booleanValue() || state.getValue(FACING) == EnumFacing.DOWN;
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
        func_185492_a(pos, p_185477_4_, p_185477_5_, state.func_185900_c(worldIn, pos));
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    /**
     * Called by ItemBlocks after a block is set in the world, to allow post-place logic
     */
    public void onBlockPlacedBy(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase placer, ItemStack stack)
    {
        worldIn.setBlockState(pos, state.withProperty(FACING, func_185647_a(pos, placer)), 2);

        if (!worldIn.isRemote)
        {
            this.checkForMove(worldIn, pos, state);
        }
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (!worldIn.isRemote)
        {
            this.checkForMove(worldIn, pos, state);
        }
    }

    public void onBlockAdded(World worldIn, BlockPos pos, IBlockState state)
    {
        if (!worldIn.isRemote && worldIn.getTileEntity(pos) == null)
        {
            this.checkForMove(worldIn, pos, state);
        }
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        return this.getDefaultState().withProperty(FACING, func_185647_a(pos, placer)).withProperty(EXTENDED, Boolean.valueOf(false));
    }

    private void checkForMove(World worldIn, BlockPos pos, IBlockState state)
    {
        EnumFacing enumfacing = (EnumFacing)state.getValue(FACING);
        boolean flag = this.shouldBeExtended(worldIn, pos, enumfacing);

        if (flag && !((Boolean)state.getValue(EXTENDED)).booleanValue())
        {
            if ((new BlockPistonStructureHelper(worldIn, pos, enumfacing, true)).canMove())
            {
                worldIn.addBlockEvent(pos, this, 0, enumfacing.getIndex());
            }
        }
        else if (!flag && ((Boolean)state.getValue(EXTENDED)).booleanValue())
        {
            worldIn.addBlockEvent(pos, this, 1, enumfacing.getIndex());
        }
    }

    private boolean shouldBeExtended(World worldIn, BlockPos pos, EnumFacing facing)
    {
        for (EnumFacing enumfacing : EnumFacing.values())
        {
            if (enumfacing != facing && worldIn.isSidePowered(pos.offset(enumfacing), enumfacing))
            {
                return true;
            }
        }

        if (worldIn.isSidePowered(pos, EnumFacing.DOWN))
        {
            return true;
        }
        else
        {
            BlockPos blockpos = pos.up();

            for (EnumFacing enumfacing1 : EnumFacing.values())
            {
                if (enumfacing1 != EnumFacing.DOWN && worldIn.isSidePowered(blockpos.offset(enumfacing1), enumfacing1))
                {
                    return true;
                }
            }

            return false;
        }
    }

    /**
     * Called on both Client and Server when World#addBlockEvent is called
     */
    public boolean onBlockEventReceived(World worldIn, BlockPos pos, IBlockState state, int eventID, int eventParam)
    {
        EnumFacing enumfacing = (EnumFacing)state.getValue(FACING);

        if (!worldIn.isRemote)
        {
            boolean flag = this.shouldBeExtended(worldIn, pos, enumfacing);

            if (flag && eventID == 1)
            {
                worldIn.setBlockState(pos, state.withProperty(EXTENDED, Boolean.valueOf(true)), 2);
                return false;
            }

            if (!flag && eventID == 0)
            {
                return false;
            }
        }

        if (eventID == 0)
        {
            if (!this.doMove(worldIn, pos, enumfacing, true))
            {
                return false;
            }

            worldIn.setBlockState(pos, state.withProperty(EXTENDED, Boolean.valueOf(true)), 2);
            worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.block_piston_extend, SoundCategory.BLOCKS, 0.5F, worldIn.rand.nextFloat() * 0.25F + 0.6F);
        }
        else if (eventID == 1)
        {
            TileEntity tileentity1 = worldIn.getTileEntity(pos.offset(enumfacing));

            if (tileentity1 instanceof TileEntityPiston)
            {
                ((TileEntityPiston)tileentity1).clearPistonTileEntity();
            }

            worldIn.setBlockState(pos, Blocks.piston_extension.getDefaultState().withProperty(BlockPistonMoving.FACING, enumfacing).withProperty(BlockPistonMoving.TYPE, this.isSticky ? BlockPistonExtension.EnumPistonType.STICKY : BlockPistonExtension.EnumPistonType.DEFAULT), 3);
            worldIn.setTileEntity(pos, BlockPistonMoving.func_185588_a(this.getStateFromMeta(eventParam), enumfacing, false, true));

            if (this.isSticky)
            {
                BlockPos blockpos = pos.add(enumfacing.getFrontOffsetX() * 2, enumfacing.getFrontOffsetY() * 2, enumfacing.getFrontOffsetZ() * 2);
                IBlockState iblockstate = worldIn.getBlockState(blockpos);
                Block block = iblockstate.getBlock();
                boolean flag1 = false;

                if (block == Blocks.piston_extension)
                {
                    TileEntity tileentity = worldIn.getTileEntity(blockpos);

                    if (tileentity instanceof TileEntityPiston)
                    {
                        TileEntityPiston tileentitypiston = (TileEntityPiston)tileentity;

                        if (tileentitypiston.getFacing() == enumfacing && tileentitypiston.isExtending())
                        {
                            tileentitypiston.clearPistonTileEntity();
                            flag1 = true;
                        }
                    }
                }

                if (!flag1 && iblockstate.getMaterial() != Material.air && func_185646_a(iblockstate, worldIn, blockpos, enumfacing.getOpposite(), false) && (iblockstate.getMobilityFlag() == EnumPushReaction.NORMAL || block == Blocks.piston || block == Blocks.sticky_piston))
                {
                    this.doMove(worldIn, pos, enumfacing, false);
                }
            }
            else
            {
                worldIn.setBlockToAir(pos.offset(enumfacing));
            }

            worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.block_piston_contract, SoundCategory.BLOCKS, 0.5F, worldIn.rand.nextFloat() * 0.15F + 0.6F);
        }

        return true;
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public static EnumFacing getFacing(int meta)
    {
        int i = meta & 7;
        return i > 5 ? null : EnumFacing.getFront(i);
    }

    public static EnumFacing func_185647_a(BlockPos p_185647_0_, EntityLivingBase p_185647_1_)
    {
        if (MathHelper.abs((float)p_185647_1_.posX - (float)p_185647_0_.getX()) < 2.0F && MathHelper.abs((float)p_185647_1_.posZ - (float)p_185647_0_.getZ()) < 2.0F)
        {
            double d0 = p_185647_1_.posY + (double)p_185647_1_.getEyeHeight();

            if (d0 - (double)p_185647_0_.getY() > 2.0D)
            {
                return EnumFacing.UP;
            }

            if ((double)p_185647_0_.getY() - d0 > 0.0D)
            {
                return EnumFacing.DOWN;
            }
        }

        return p_185647_1_.getHorizontalFacing().getOpposite();
    }

    public static boolean func_185646_a(IBlockState p_185646_0_, World p_185646_1_, BlockPos p_185646_2_, EnumFacing p_185646_3_, boolean p_185646_4_)
    {
        Block block = p_185646_0_.getBlock();

        if (block == Blocks.obsidian)
        {
            return false;
        }
        else if (!p_185646_1_.getWorldBorder().contains(p_185646_2_))
        {
            return false;
        }
        else if (p_185646_2_.getY() >= 0 && (p_185646_3_ != EnumFacing.DOWN || p_185646_2_.getY() != 0))
        {
            if (p_185646_2_.getY() <= p_185646_1_.getHeight() - 1 && (p_185646_3_ != EnumFacing.UP || p_185646_2_.getY() != p_185646_1_.getHeight() - 1))
            {
                if (block != Blocks.piston && block != Blocks.sticky_piston)
                {
                    if (p_185646_0_.getBlockHardness(p_185646_1_, p_185646_2_) == -1.0F)
                    {
                        return false;
                    }

                    if (p_185646_0_.getMobilityFlag() == EnumPushReaction.BLOCK)
                    {
                        return false;
                    }

                    if (p_185646_0_.getMobilityFlag() == EnumPushReaction.DESTROY)
                    {
                        return p_185646_4_;
                    }
                }
                else if (((Boolean)p_185646_0_.getValue(EXTENDED)).booleanValue())
                {
                    return false;
                }

                return !block.hasTileEntity();
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    private boolean doMove(World worldIn, BlockPos pos, EnumFacing direction, boolean extending)
    {
        if (!extending)
        {
            worldIn.setBlockToAir(pos.offset(direction));
        }

        BlockPistonStructureHelper blockpistonstructurehelper = new BlockPistonStructureHelper(worldIn, pos, direction, extending);

        if (!blockpistonstructurehelper.canMove())
        {
            return false;
        }
        else
        {
            List<BlockPos> list = blockpistonstructurehelper.getBlocksToMove();
            List<IBlockState> list1 = Lists.<IBlockState>newArrayList();

            for (int i = 0; i < list.size(); ++i)
            {
                BlockPos blockpos = (BlockPos)list.get(i);
                list1.add(worldIn.getBlockState(blockpos).getActualState(worldIn, blockpos));
            }

            List<BlockPos> list2 = blockpistonstructurehelper.getBlocksToDestroy();
            int k = list.size() + list2.size();
            IBlockState[] aiblockstate = new IBlockState[k];
            EnumFacing enumfacing = extending ? direction : direction.getOpposite();

            for (int j = list2.size() - 1; j >= 0; --j)
            {
                BlockPos blockpos1 = (BlockPos)list2.get(j);
                IBlockState iblockstate = worldIn.getBlockState(blockpos1);
                iblockstate.getBlock().dropBlockAsItem(worldIn, blockpos1, iblockstate, 0);
                worldIn.setBlockToAir(blockpos1);
                --k;
                aiblockstate[k] = iblockstate;
            }

            for (int l = list.size() - 1; l >= 0; --l)
            {
                BlockPos blockpos3 = (BlockPos)list.get(l);
                IBlockState iblockstate2 = worldIn.getBlockState(blockpos3);
                worldIn.setBlockState(blockpos3, Blocks.air.getDefaultState(), 4);
                blockpos3 = blockpos3.offset(enumfacing);
                worldIn.setBlockState(blockpos3, Blocks.piston_extension.getDefaultState().withProperty(FACING, direction), 4);
                worldIn.setTileEntity(blockpos3, BlockPistonMoving.func_185588_a((IBlockState)list1.get(l), direction, extending, false));
                --k;
                aiblockstate[k] = iblockstate2;
            }

            BlockPos blockpos2 = pos.offset(direction);

            if (extending)
            {
                BlockPistonExtension.EnumPistonType blockpistonextension$enumpistontype = this.isSticky ? BlockPistonExtension.EnumPistonType.STICKY : BlockPistonExtension.EnumPistonType.DEFAULT;
                IBlockState iblockstate3 = Blocks.piston_head.getDefaultState().withProperty(BlockPistonExtension.FACING, direction).withProperty(BlockPistonExtension.TYPE, blockpistonextension$enumpistontype);
                IBlockState iblockstate1 = Blocks.piston_extension.getDefaultState().withProperty(BlockPistonMoving.FACING, direction).withProperty(BlockPistonMoving.TYPE, this.isSticky ? BlockPistonExtension.EnumPistonType.STICKY : BlockPistonExtension.EnumPistonType.DEFAULT);
                worldIn.setBlockState(blockpos2, iblockstate1, 4);
                worldIn.setTileEntity(blockpos2, BlockPistonMoving.func_185588_a(iblockstate3, direction, true, false));
            }

            for (int i1 = list2.size() - 1; i1 >= 0; --i1)
            {
                worldIn.notifyNeighborsOfStateChange((BlockPos)list2.get(i1), aiblockstate[k++].getBlock());
            }

            for (int j1 = list.size() - 1; j1 >= 0; --j1)
            {
                worldIn.notifyNeighborsOfStateChange((BlockPos)list.get(j1), aiblockstate[k++].getBlock());
            }

            if (extending)
            {
                worldIn.notifyNeighborsOfStateChange(blockpos2, Blocks.piston_head);
                worldIn.notifyNeighborsOfStateChange(pos, this);
            }

            return true;
        }
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(FACING, getFacing(meta)).withProperty(EXTENDED, Boolean.valueOf((meta & 8) > 0));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i = 0;
        i = i | ((EnumFacing)state.getValue(FACING)).getIndex();

        if (((Boolean)state.getValue(EXTENDED)).booleanValue())
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
        return new BlockStateContainer(this, new IProperty[] {FACING, EXTENDED});
    }
}
