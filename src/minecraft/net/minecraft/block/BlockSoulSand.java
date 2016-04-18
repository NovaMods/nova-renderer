package net.minecraft.block;

import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class BlockSoulSand extends Block
{
    protected static final AxisAlignedBB field_185703_a = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.875D, 1.0D);

    public BlockSoulSand()
    {
        super(Material.sand, MapColor.brownColor);
        this.setCreativeTab(CreativeTabs.tabBlock);
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return field_185703_a;
    }

    /**
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
        entityIn.motionX *= 0.4D;
        entityIn.motionZ *= 0.4D;
    }
}
