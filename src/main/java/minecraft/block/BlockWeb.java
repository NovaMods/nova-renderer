package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class BlockWeb extends Block
{
    public BlockWeb()
    {
        super(Material.web);
        this.setCreativeTab(CreativeTabs.tabDecorations);
    }

    /**
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
        entityIn.setInWeb();
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return NULL_AABB;
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
        return Items.string;
    }

    protected boolean canSilkHarvest()
    {
        return true;
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }

    public void harvestBlock(World worldIn, EntityPlayer player, BlockPos pos, IBlockState state, TileEntity te, ItemStack p_180657_6_)
    {
        if (!worldIn.isRemote && p_180657_6_ != null && p_180657_6_.getItem() == Items.shears)
        {
            player.triggerAchievement(StatList.func_188055_a(this));
            spawnAsEntity(worldIn, pos, new ItemStack(Item.getItemFromBlock(this), 1));
        }
        else
        {
            super.harvestBlock(worldIn, player, pos, state, te, p_180657_6_);
        }
    }
}
