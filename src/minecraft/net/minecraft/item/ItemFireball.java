package net.minecraft.item;

import net.minecraft.block.material.Material;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemFireball extends Item
{
    public ItemFireball()
    {
        this.setCreativeTab(CreativeTabs.tabMisc);
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        if (worldIn.isRemote)
        {
            return EnumActionResult.SUCCESS;
        }
        else
        {
            pos = pos.offset(hitX);

            if (!playerIn.canPlayerEdit(pos, hitX, stack))
            {
                return EnumActionResult.FAIL;
            }
            else
            {
                if (worldIn.getBlockState(pos).getMaterial() == Material.air)
                {
                    worldIn.func_184133_a((EntityPlayer)null, pos, SoundEvents.item_firecharge_use, SoundCategory.BLOCKS, 1.0F, (itemRand.nextFloat() - itemRand.nextFloat()) * 0.2F + 1.0F);
                    worldIn.setBlockState(pos, Blocks.fire.getDefaultState());
                }

                if (!playerIn.capabilities.isCreativeMode)
                {
                    --stack.stackSize;
                }

                return EnumActionResult.SUCCESS;
            }
        }
    }
}
