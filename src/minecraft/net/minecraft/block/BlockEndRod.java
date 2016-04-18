package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Blocks;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockEndRod extends BlockDirectional
{
    protected static final AxisAlignedBB field_185630_a = new AxisAlignedBB(0.375D, 0.0D, 0.375D, 0.625D, 1.0D, 0.625D);
    protected static final AxisAlignedBB field_185631_b = new AxisAlignedBB(0.375D, 0.375D, 0.0D, 0.625D, 0.625D, 1.0D);
    protected static final AxisAlignedBB field_185632_c = new AxisAlignedBB(0.0D, 0.375D, 0.375D, 1.0D, 0.625D, 0.625D);

    protected BlockEndRod()
    {
        super(Material.circuits);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.UP));
        this.setCreativeTab(CreativeTabs.tabDecorations);
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
        return state.withProperty(FACING, mirrorIn.func_185803_b((EnumFacing)state.getValue(FACING)));
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        switch (((EnumFacing)state.getValue(FACING)).getAxis())
        {
            case X:
            default:
                return field_185632_c;

            case Z:
                return field_185631_b;

            case Y:
                return field_185630_a;
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

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return true;
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        IBlockState iblockstate = worldIn.getBlockState(pos.offset(facing.getOpposite()));

        if (iblockstate.getBlock() == Blocks.end_rod)
        {
            EnumFacing enumfacing = (EnumFacing)iblockstate.getValue(FACING);

            if (enumfacing == facing)
            {
                return this.getDefaultState().withProperty(FACING, facing.getOpposite());
            }
        }

        return this.getDefaultState().withProperty(FACING, facing);
    }

    public void onBlockAdded(World worldIn, BlockPos pos, IBlockState state)
    {
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        EnumFacing enumfacing = (EnumFacing)worldIn.getValue(FACING);
        double d0 = (double)state.getX() + 0.55D - (double)(rand.nextFloat() * 0.1F);
        double d1 = (double)state.getY() + 0.55D - (double)(rand.nextFloat() * 0.1F);
        double d2 = (double)state.getZ() + 0.55D - (double)(rand.nextFloat() * 0.1F);
        double d3 = (double)(0.4F - (rand.nextFloat() + rand.nextFloat()) * 0.4F);

        if (rand.nextInt(5) == 0)
        {
            pos.spawnParticle(EnumParticleTypes.END_ROD, d0 + (double)enumfacing.getFrontOffsetX() * d3, d1 + (double)enumfacing.getFrontOffsetY() * d3, d2 + (double)enumfacing.getFrontOffsetZ() * d3, rand.nextGaussian() * 0.005D, rand.nextGaussian() * 0.005D, rand.nextGaussian() * 0.005D, new int[0]);
        }
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        IBlockState iblockstate = this.getDefaultState();
        iblockstate = iblockstate.withProperty(FACING, EnumFacing.getFront(meta));
        return iblockstate;
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((EnumFacing)state.getValue(FACING)).getIndex();
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {FACING});
    }

    public EnumPushReaction getMobilityFlag(IBlockState state)
    {
        return EnumPushReaction.NORMAL;
    }
}
