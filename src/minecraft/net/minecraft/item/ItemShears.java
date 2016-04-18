package net.minecraft.item;

import net.minecraft.block.Block;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemShears extends Item
{
    public ItemShears()
    {
        this.setMaxStackSize(1);
        this.setMaxDamage(238);
        this.setCreativeTab(CreativeTabs.tabTools);
    }

    /**
     * Called when a Block is destroyed using this Item. Return true to trigger the "Use Item" statistic.
     */
    public boolean onBlockDestroyed(ItemStack stack, World worldIn, IBlockState blockIn, BlockPos pos, EntityLivingBase playerIn)
    {
        stack.damageItem(1, playerIn);
        Block block = blockIn.getBlock();
        return blockIn.getMaterial() != Material.leaves && block != Blocks.web && block != Blocks.tallgrass && block != Blocks.vine && block != Blocks.tripwire && block != Blocks.wool ? super.onBlockDestroyed(stack, worldIn, blockIn, pos, playerIn) : true;
    }

    /**
     * Check whether this Item can harvest the given Block
     */
    public boolean canHarvestBlock(IBlockState blockIn)
    {
        Block block = blockIn.getBlock();
        return block == Blocks.web || block == Blocks.redstone_wire || block == Blocks.tripwire;
    }

    public float getStrVsBlock(ItemStack stack, IBlockState state)
    {
        Block block = state.getBlock();
        return block != Blocks.web && state.getMaterial() != Material.leaves ? (block == Blocks.wool ? 5.0F : super.getStrVsBlock(stack, state)) : 15.0F;
    }
}
