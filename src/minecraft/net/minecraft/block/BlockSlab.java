package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyEnum;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Blocks;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.IStringSerializable;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public abstract class BlockSlab extends Block
{
    public static final PropertyEnum<BlockSlab.EnumBlockHalf> HALF = PropertyEnum.<BlockSlab.EnumBlockHalf>create("half", BlockSlab.EnumBlockHalf.class);
    protected static final AxisAlignedBB field_185676_b = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.5D, 1.0D);
    protected static final AxisAlignedBB field_185677_c = new AxisAlignedBB(0.0D, 0.5D, 0.0D, 1.0D, 1.0D, 1.0D);

    public BlockSlab(Material materialIn)
    {
        super(materialIn);
        this.fullBlock = this.isDouble();
        this.setLightOpacity(255);
    }

    protected boolean canSilkHarvest()
    {
        return false;
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return this.isDouble() ? FULL_BLOCK_AABB : (state.getValue(HALF) == BlockSlab.EnumBlockHalf.TOP ? field_185677_c : field_185676_b);
    }

    /**
     * Checks if an IBlockState represents a block that is opaque and a full cube.
     *  
     * @param state The block state to check.
     */
    public boolean isFullyOpaque(IBlockState state)
    {
        return ((BlockSlab)state.getBlock()).isDouble() || state.getValue(HALF) == BlockSlab.EnumBlockHalf.TOP;
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return this.isDouble();
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        IBlockState iblockstate = super.onBlockPlaced(worldIn, pos, facing, hitX, hitY, hitZ, meta, placer).withProperty(HALF, BlockSlab.EnumBlockHalf.BOTTOM);
        return this.isDouble() ? iblockstate : (facing != EnumFacing.DOWN && (facing == EnumFacing.UP || (double)hitY <= 0.5D) ? iblockstate : iblockstate.withProperty(HALF, BlockSlab.EnumBlockHalf.TOP));
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return this.isDouble() ? 2 : 1;
    }

    public boolean isFullCube(IBlockState state)
    {
        return this.isDouble();
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        if (this.isDouble())
        {
            return super.shouldSideBeRendered(worldIn, pos, side, facing);
        }
        else if (facing != EnumFacing.UP && facing != EnumFacing.DOWN && !super.shouldSideBeRendered(worldIn, pos, side, facing))
        {
            return false;
        }
        else
        {
            IBlockState iblockstate = pos.getBlockState(side.offset(facing));
            boolean flag = func_185675_i(iblockstate) && iblockstate.getValue(HALF) == BlockSlab.EnumBlockHalf.TOP;
            boolean flag1 = func_185675_i(worldIn) && worldIn.getValue(HALF) == BlockSlab.EnumBlockHalf.TOP;
            return flag1 ? (facing == EnumFacing.DOWN ? true : (facing == EnumFacing.UP && super.shouldSideBeRendered(worldIn, pos, side, facing) ? true : !func_185675_i(iblockstate) || !flag)) : (facing == EnumFacing.UP ? true : (facing == EnumFacing.DOWN && super.shouldSideBeRendered(worldIn, pos, side, facing) ? true : !func_185675_i(iblockstate) || flag));
        }
    }

    protected static boolean func_185675_i(IBlockState p_185675_0_)
    {
        Block block = p_185675_0_.getBlock();
        return block == Blocks.stone_slab || block == Blocks.wooden_slab || block == Blocks.stone_slab2 || block == Blocks.purpur_slab;
    }

    /**
     * Returns the slab block name with the type associated with it
     */
    public abstract String getUnlocalizedName(int meta);

    public abstract boolean isDouble();

    public abstract IProperty<?> getVariantProperty();

    public abstract Comparable<?> func_185674_a(ItemStack p_185674_1_);

    public static enum EnumBlockHalf implements IStringSerializable
    {
        TOP("top"),
        BOTTOM("bottom");

        private final String name;

        private EnumBlockHalf(String name)
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
