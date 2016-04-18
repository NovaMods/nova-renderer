package net.minecraft.item;

import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemRedstone extends Item
{
    public ItemRedstone()
    {
        this.setCreativeTab(CreativeTabs.tabRedstone);
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        boolean flag = worldIn.getBlockState(pos).getBlock().isReplaceable(worldIn, pos);
        BlockPos blockpos = flag ? pos : pos.offset(hitX);

        if (playerIn.canPlayerEdit(blockpos, hitX, stack) && worldIn.canBlockBePlaced(worldIn.getBlockState(blockpos).getBlock(), blockpos, false, hitX, (Entity)null, stack) && Blocks.redstone_wire.canPlaceBlockAt(worldIn, blockpos))
        {
            --stack.stackSize;
            worldIn.setBlockState(blockpos, Blocks.redstone_wire.getDefaultState());
            return EnumActionResult.SUCCESS;
        }
        else
        {
            return EnumActionResult.FAIL;
        }
    }
}
