package net.minecraft.item;

import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityPotion;
import net.minecraft.init.SoundEvents;
import net.minecraft.potion.PotionUtils;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.World;

public class ItemSplashPotion extends ItemPotion
{
    public String getItemStackDisplayName(ItemStack stack)
    {
        return I18n.translateToLocal(PotionUtils.func_185191_c(stack).func_185174_b("splash_potion.effect."));
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        if (!playerIn.capabilities.isCreativeMode)
        {
            --itemStackIn.stackSize;
        }

        worldIn.func_184148_a((EntityPlayer)null, playerIn.posX, playerIn.posY, playerIn.posZ, SoundEvents.entity_splash_potion_throw, SoundCategory.NEUTRAL, 0.5F, 0.4F / (itemRand.nextFloat() * 0.4F + 0.8F));

        if (!worldIn.isRemote)
        {
            EntityPotion entitypotion = new EntityPotion(worldIn, playerIn, itemStackIn);
            entitypotion.func_184538_a(playerIn, playerIn.rotationPitch, playerIn.rotationYaw, -20.0F, 0.5F, 1.0F);
            worldIn.spawnEntityInWorld(entitypotion);
        }

        playerIn.triggerAchievement(StatList.func_188057_b(this));
        return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
    }
}
