package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockGrassPath extends Block
{
    protected static final AxisAlignedBB field_185673_a = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.9375D, 1.0D);

    protected BlockGrassPath()
    {
        super(Material.ground);
        this.setLightOpacity(255);
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        switch (facing)
        {
            case UP:
                return true;

            case NORTH:
            case SOUTH:
            case WEST:
            case EAST:
                IBlockState iblockstate = pos.getBlockState(side.offset(facing));
                Block block = iblockstate.getBlock();
                return !iblockstate.isOpaqueCube() && block != Blocks.farmland && block != Blocks.grass_path;

            default:
                return super.shouldSideBeRendered(worldIn, pos, side, facing);
        }
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185673_a;
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
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Blocks.dirt.getItemDropped(Blocks.dirt.getDefaultState().withProperty(BlockDirt.VARIANT, BlockDirt.DirtType.DIRT), rand, fortune);
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return new ItemStack(this);
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        super.onNeighborBlockChange(worldIn, pos, state, neighborBlock);

        if (worldIn.getBlockState(pos.up()).getMaterial().isSolid())
        {
            worldIn.setBlockState(pos, Blocks.dirt.getDefaultState());
        }
    }
}
