package net.minecraft.item;

import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.passive.EntityPig;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.world.World;

public class ItemCarrotOnAStick extends Item
{
    public ItemCarrotOnAStick()
    {
        this.setCreativeTab(CreativeTabs.tabTransport);
        this.setMaxStackSize(1);
        this.setMaxDamage(25);
    }

    /**
     * Returns True is the item is renderer in full 3D when hold.
     */
    public boolean isFull3D()
    {
        return true;
    }

    /**
     * Returns true if this item should be rotated by 180 degrees around the Y axis when being held in an entities
     * hands.
     */
    public boolean shouldRotateAroundWhenRendering()
    {
        return true;
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        if (playerIn.isRiding() && playerIn.getRidingEntity() instanceof EntityPig)
        {
            EntityPig entitypig = (EntityPig)playerIn.getRidingEntity();

            if (itemStackIn.getMaxDamage() - itemStackIn.getMetadata() >= 7 && entitypig.func_184762_da())
            {
                itemStackIn.damageItem(7, playerIn);

                if (itemStackIn.stackSize == 0)
                {
                    ItemStack itemstack = new ItemStack(Items.fishing_rod);
                    itemstack.setTagCompound(itemStackIn.getTagCompound());
                    return new ActionResult(EnumActionResult.SUCCESS, itemstack);
                }

                return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
            }
        }

        playerIn.triggerAchievement(StatList.func_188057_b(this));
        return new ActionResult(EnumActionResult.PASS, itemStackIn);
    }
}
