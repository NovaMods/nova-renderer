package net.minecraft.block;

import com.google.common.base.Predicates;
import java.util.List;
import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.properties.PropertyDirection;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.BlockWorldState;
import net.minecraft.block.state.IBlockState;
import net.minecraft.block.state.pattern.BlockPattern;
import net.minecraft.block.state.pattern.BlockStateMatcher;
import net.minecraft.block.state.pattern.FactoryBlockPattern;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Blocks;
import net.minecraft.item.Item;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockEndPortalFrame extends Block
{
    public static final PropertyDirection FACING = BlockHorizontal.FACING;
    public static final PropertyBool EYE = PropertyBool.create("eye");
    protected static final AxisAlignedBB field_185662_c = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.8125D, 1.0D);
    protected static final AxisAlignedBB field_185663_d = new AxisAlignedBB(0.3125D, 0.8125D, 0.3125D, 0.6875D, 1.0D, 0.6875D);
    private static BlockPattern field_185664_e;

    public BlockEndPortalFrame()
    {
        super(Material.rock, MapColor.greenColor);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH).withProperty(EYE, Boolean.valueOf(false)));
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185662_c;
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185662_c);

        if (((Boolean)worldIn.getBlockState(pos).getValue(EYE)).booleanValue())
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, field_185663_d);
        }
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return null;
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        return this.getDefaultState().withProperty(FACING, placer.getHorizontalFacing().getOpposite()).withProperty(EYE, Boolean.valueOf(false));
    }

    public boolean hasComparatorInputOverride(IBlockState state)
    {
        return true;
    }

    public int getComparatorInputOverride(IBlockState worldIn, World pos, BlockPos p_180641_3_)
    {
        return ((Boolean)worldIn.getValue(EYE)).booleanValue() ? 15 : 0;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(EYE, Boolean.valueOf((meta & 4) != 0)).withProperty(FACING, EnumFacing.getHorizontal(meta & 3));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i = 0;
        i = i | ((EnumFacing)state.getValue(FACING)).getHorizontalIndex();

        if (((Boolean)state.getValue(EYE)).booleanValue())
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
        return new BlockStateContainer(this, new IProperty[] {FACING, EYE});
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public static BlockPattern func_185661_e()
    {
        if (field_185664_e == null)
        {
            field_185664_e = FactoryBlockPattern.start().aisle(new String[] {"?vvv?", ">   <", ">   <", ">   <", "?^^^?"}).where('?', BlockWorldState.hasState(BlockStateMatcher.field_185928_a)).where('^', BlockWorldState.hasState(BlockStateMatcher.forBlock(Blocks.end_portal_frame).where(EYE, Predicates.equalTo(Boolean.valueOf(true))).where(FACING, Predicates.equalTo(EnumFacing.SOUTH)))).where('>', BlockWorldState.hasState(BlockStateMatcher.forBlock(Blocks.end_portal_frame).where(EYE, Predicates.equalTo(Boolean.valueOf(true))).where(FACING, Predicates.equalTo(EnumFacing.WEST)))).where('v', BlockWorldState.hasState(BlockStateMatcher.forBlock(Blocks.end_portal_frame).where(EYE, Predicates.equalTo(Boolean.valueOf(true))).where(FACING, Predicates.equalTo(EnumFacing.NORTH)))).where('<', BlockWorldState.hasState(BlockStateMatcher.forBlock(Blocks.end_portal_frame).where(EYE, Predicates.equalTo(Boolean.valueOf(true))).where(FACING, Predicates.equalTo(EnumFacing.EAST)))).where(' ', BlockWorldState.func_185925_a(Blocks.air.getDefaultState())).build();
        }

        return field_185664_e;
    }
}
