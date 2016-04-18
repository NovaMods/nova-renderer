package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.init.Blocks;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockBush extends Block
{
    protected static final AxisAlignedBB field_185515_b = new AxisAlignedBB(0.30000001192092896D, 0.0D, 0.30000001192092896D, 0.699999988079071D, 0.6000000238418579D, 0.699999988079071D);

    protected BlockBush()
    {
        this(Material.plants);
    }

    protected BlockBush(Material materialIn)
    {
        this(materialIn, materialIn.getMaterialMapColor());
    }

    protected BlockBush(Material p_i46452_1_, MapColor p_i46452_2_)
    {
        super(p_i46452_1_, p_i46452_2_);
        this.setTickRandomly(true);
        this.setCreativeTab(CreativeTabs.tabDecorations);
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return super.canPlaceBlockAt(worldIn, pos) && this.func_185514_i(worldIn.getBlockState(pos.down()));
    }

    protected boolean func_185514_i(IBlockState p_185514_1_)
    {
        return p_185514_1_.getBlock() == Blocks.grass || p_185514_1_.getBlock() == Blocks.dirt || p_185514_1_.getBlock() == Blocks.farmland;
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        super.onNeighborBlockChange(worldIn, pos, state, neighborBlock);
        this.checkAndDropBlock(worldIn, pos, state);
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        this.checkAndDropBlock(worldIn, pos, state);
    }

    protected void checkAndDropBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        if (!this.canBlockStay(worldIn, pos, state))
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockState(pos, Blocks.air.getDefaultState(), 3);
        }
    }

    public boolean canBlockStay(World worldIn, BlockPos pos, IBlockState state)
    {
        return this.func_185514_i(worldIn.getBlockState(pos.down()));
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185515_b;
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

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }
}
