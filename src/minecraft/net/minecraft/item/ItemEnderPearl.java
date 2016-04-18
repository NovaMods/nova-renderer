package net.minecraft.item;

import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.item.EntityEnderPearl;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.SoundEvents;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.world.World;

public class ItemEnderPearl extends Item
{
    public ItemEnderPearl()
    {
        this.maxStackSize = 16;
        this.setCreativeTab(CreativeTabs.tabMisc);
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        if (!playerIn.capabilities.isCreativeMode)
        {
            --itemStackIn.stackSize;
        }

        worldIn.func_184148_a((EntityPlayer)null, playerIn.posX, playerIn.posY, playerIn.posZ, SoundEvents.entity_enderpearl_throw, SoundCategory.NEUTRAL, 0.5F, 0.4F / (itemRand.nextFloat() * 0.4F + 0.8F));
        playerIn.func_184811_cZ().setCooldown(this, 20);

        if (!worldIn.isRemote)
        {
            EntityEnderPearl entityenderpearl = new EntityEnderPearl(worldIn, playerIn);
            entityenderpearl.func_184538_a(playerIn, playerIn.rotationPitch, playerIn.rotationYaw, 0.0F, 1.5F, 1.0F);
            worldIn.spawnEntityInWorld(entityenderpearl);
        }

        playerIn.triggerAchievement(StatList.func_188057_b(this));
        return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
    }
}
