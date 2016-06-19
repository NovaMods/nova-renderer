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
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockChorusPlant extends Block
{
    public static final PropertyBool NORTH = PropertyBool.create("north");
    public static final PropertyBool EAST = PropertyBool.create("east");
    public static final PropertyBool SOUTH = PropertyBool.create("south");
    public static final PropertyBool WEST = PropertyBool.create("west");
    public static final PropertyBool UP = PropertyBool.create("up");
    public static final PropertyBool DOWN = PropertyBool.create("down");

    protected BlockChorusPlant()
    {
        super(Material.plants);
        this.setCreativeTab(CreativeTabs.tabDecorations);
        this.setDefaultState(this.blockState.getBaseState().withProperty(NORTH, Boolean.valueOf(false)).withProperty(EAST, Boolean.valueOf(false)).withProperty(SOUTH, Boolean.valueOf(false)).withProperty(WEST, Boolean.valueOf(false)).withProperty(UP, Boolean.valueOf(false)).withProperty(DOWN, Boolean.valueOf(false)));
    }

    /**
     * Get the actual Block state of this Block at the given position. This applies properties not visible in the
     * metadata, such as fence connections.
     */
    public IBlockState getActualState(IBlockState state, IBlockAccess worldIn, BlockPos pos)
    {
        Block block = worldIn.getBlockState(pos.down()).getBlock();
        Block block1 = worldIn.getBlockState(pos.up()).getBlock();
        Block block2 = worldIn.getBlockState(pos.north()).getBlock();
        Block block3 = worldIn.getBlockState(pos.east()).getBlock();
        Block block4 = worldIn.getBlockState(pos.south()).getBlock();
        Block block5 = worldIn.getBlockState(pos.west()).getBlock();
        return state.withProperty(DOWN, Boolean.valueOf(block == this || block == Blocks.chorus_flower || block == Blocks.end_stone)).withProperty(UP, Boolean.valueOf(block1 == this || block1 == Blocks.chorus_flower)).withProperty(NORTH, Boolean.valueOf(block2 == this || block2 == Blocks.chorus_flower)).withProperty(EAST, Boolean.valueOf(block3 == this || block3 == Blocks.chorus_flower)).withProperty(SOUTH, Boolean.valueOf(block4 == this || block4 == Blocks.chorus_flower)).withProperty(WEST, Boolean.valueOf(block5 == this || block5 == Blocks.chorus_flower));
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        state = state.getActualState(source, pos);
        float f = 0.1875F;
        float f1 = ((Boolean)state.getValue(WEST)).booleanValue() ? 0.0F : 0.1875F;
        float f2 = ((Boolean)state.getValue(DOWN)).booleanValue() ? 0.0F : 0.1875F;
        float f3 = ((Boolean)state.getValue(NORTH)).booleanValue() ? 0.0F : 0.1875F;
        float f4 = ((Boolean)state.getValue(EAST)).booleanValue() ? 1.0F : 0.8125F;
        float f5 = ((Boolean)state.getValue(UP)).booleanValue() ? 1.0F : 0.8125F;
        float f6 = ((Boolean)state.getValue(SOUTH)).booleanValue() ? 1.0F : 0.8125F;
        return new AxisAlignedBB((double)f1, (double)f2, (double)f3, (double)f4, (double)f5, (double)f6);
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
        state = state.getActualState(worldIn, pos);
        float f = 0.1875F;
        float f1 = 0.8125F;
        func_185492_a(pos, p_185477_4_, p_185477_5_, new AxisAlignedBB(0.1875D, 0.1875D, 0.1875D, 0.8125D, 0.8125D, 0.8125D));

        if (((Boolean)state.getValue(WEST)).booleanValue())
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, new AxisAlignedBB(0.0D, 0.1875D, 0.1875D, 0.1875D, 0.8125D, 0.8125D));
        }

        if (((Boolean)state.getValue(EAST)).booleanValue())
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, new AxisAlignedBB(0.8125D, 0.1875D, 0.1875D, 1.0D, 0.8125D, 0.8125D));
        }

        if (((Boolean)state.getValue(UP)).booleanValue())
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, new AxisAlignedBB(0.1875D, 0.8125D, 0.1875D, 0.8125D, 1.0D, 0.8125D));
        }

        if (((Boolean)state.getValue(DOWN)).booleanValue())
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, new AxisAlignedBB(0.1875D, 0.0D, 0.1875D, 0.8125D, 0.1875D, 0.8125D));
        }

        if (((Boolean)state.getValue(NORTH)).booleanValue())
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, new AxisAlignedBB(0.1875D, 0.1875D, 0.0D, 0.8125D, 0.8125D, 0.1875D));
        }

        if (((Boolean)state.getValue(SOUTH)).booleanValue())
        {
            func_185492_a(pos, p_185477_4_, p_185477_5_, new AxisAlignedBB(0.1875D, 0.1875D, 0.8125D, 0.8125D, 0.8125D, 1.0D));
        }
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return 0;
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (!this.func_185608_b(worldIn, pos))
        {
            worldIn.destroyBlock(pos, true);
        }
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.chorus_fruit;
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return random.nextInt(2);
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return super.canPlaceBlockAt(worldIn, pos) ? this.func_185608_b(worldIn, pos) : false;
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (!this.func_185608_b(worldIn, pos))
        {
            worldIn.scheduleUpdate(pos, this, 1);
        }
    }

    public boolean func_185608_b(World p_185608_1_, BlockPos p_185608_2_)
    {
        boolean flag = p_185608_1_.isAirBlock(p_185608_2_.up());
        boolean flag1 = p_185608_1_.isAirBlock(p_185608_2_.down());

        for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
        {
            BlockPos blockpos = p_185608_2_.offset(enumfacing);
            Block block = p_185608_1_.getBlockState(blockpos).getBlock();

            if (block == this)
            {
                if (!flag && !flag1)
                {
                    return false;
                }

                Block block1 = p_185608_1_.getBlockState(blockpos.down()).getBlock();

                if (block1 == this || block1 == Blocks.end_stone)
                {
                    return true;
                }
            }
        }

        Block block2 = p_185608_1_.getBlockState(p_185608_2_.down()).getBlock();
        return block2 == this || block2 == Blocks.end_stone;
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        Block block = pos.getBlockState(side.offset(facing)).getBlock();
        return block != this && block != Blocks.chorus_flower && (facing != EnumFacing.DOWN || block != Blocks.end_stone);
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {NORTH, EAST, SOUTH, WEST, UP, DOWN});
    }

    public boolean isPassable(IBlockAccess worldIn, BlockPos pos)
    {
        return false;
    }
}
