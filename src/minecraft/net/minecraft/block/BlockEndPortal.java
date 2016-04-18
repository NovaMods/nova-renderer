package net.minecraft.block;

import java.util.List;
import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.item.ItemStack;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityEndPortal;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockEndPortal extends BlockContainer
{
    protected static final AxisAlignedBB field_185568_a = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.75D, 1.0D);

    protected BlockEndPortal(Material materialIn)
    {
        super(materialIn);
        this.setLightLevel(1.0F);
    }

    /**
     * Returns a new instance of a block's tile entity class. Called on placing the block.
     */
    public TileEntity createNewTileEntity(World worldIn, int meta)
    {
        return new TileEntityEndPortal();
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185568_a;
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        return facing == EnumFacing.DOWN ? super.shouldSideBeRendered(worldIn, pos, side, facing) : false;
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
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
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return 0;
    }

    /**
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
        if (!entityIn.isRiding() && !entityIn.isBeingRidden() && entityIn.func_184222_aU() && !worldIn.isRemote && entityIn.getEntityBoundingBox().intersectsWith(state.func_185900_c(worldIn, pos).offset(pos)))
        {
            entityIn.changeDimension(1);
        }
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        double d0 = (double)((float)state.getX() + rand.nextFloat());
        double d1 = (double)((float)state.getY() + 0.8F);
        double d2 = (double)((float)state.getZ() + rand.nextFloat());
        double d3 = 0.0D;
        double d4 = 0.0D;
        double d5 = 0.0D;
        pos.spawnParticle(EnumParticleTypes.SMOKE_NORMAL, d0, d1, d2, d3, d4, d5, new int[0]);
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return null;
    }

    /**
     * Get the MapColor for this Block and the given BlockState
     */
    public MapColor getMapColor(IBlockState state)
    {
        return MapColor.blackColor;
    }
}
