package net.minecraft.item;

import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.world.World;

public class ItemBucketMilk extends Item
{
    public ItemBucketMilk()
    {
        this.setMaxStackSize(1);
        this.setCreativeTab(CreativeTabs.tabMisc);
    }

    /**
     * Called when the player finishes using this Item (E.g. finishes eating.). Not called when the player stops using
     * the Item before the action is complete.
     */
    public ItemStack onItemUseFinish(ItemStack stack, World worldIn, EntityLivingBase playerIn)
    {
        if (playerIn instanceof EntityPlayer && !((EntityPlayer)playerIn).capabilities.isCreativeMode)
        {
            --stack.stackSize;
        }

        if (!worldIn.isRemote)
        {
            playerIn.clearActivePotions();
        }

        if (playerIn instanceof EntityPlayer)
        {
            ((EntityPlayer)playerIn).triggerAchievement(StatList.func_188057_b(this));
        }

        return stack.stackSize <= 0 ? new ItemStack(Items.bucket) : stack;
    }

    /**
     * How long it takes to use or consume an item
     */
    public int getMaxItemUseDuration(ItemStack stack)
    {
        return 32;
    }

    /**
     * returns the action that specifies what animation to play when the items is being used
     */
    public EnumAction getItemUseAction(ItemStack stack)
    {
        return EnumAction.DRINK;
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        playerIn.func_184598_c(hand);
        return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
    }
}
