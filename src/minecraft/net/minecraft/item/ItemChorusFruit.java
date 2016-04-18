package net.minecraft.item;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class ItemChorusFruit extends ItemFood
{
    public ItemChorusFruit(int p_i46747_1_, float p_i46747_2_)
    {
        super(p_i46747_1_, p_i46747_2_, false);
    }

    /**
     * Called when the player finishes using this Item (E.g. finishes eating.). Not called when the player stops using
     * the Item before the action is complete.
     */
    public ItemStack onItemUseFinish(ItemStack stack, World worldIn, EntityLivingBase playerIn)
    {
        ItemStack itemstack = super.onItemUseFinish(stack, worldIn, playerIn);

        if (!worldIn.isRemote)
        {
            double d0 = playerIn.posX;
            double d1 = playerIn.posY;
            double d2 = playerIn.posZ;

            for (int i = 0; i < 16; ++i)
            {
                double d3 = playerIn.posX + (playerIn.getRNG().nextDouble() - 0.5D) * 16.0D;
                double d4 = MathHelper.clamp_double(playerIn.posY + (double)(playerIn.getRNG().nextInt(16) - 8), 0.0D, (double)(worldIn.getActualHeight() - 1));
                double d5 = playerIn.posZ + (playerIn.getRNG().nextDouble() - 0.5D) * 16.0D;

                if (playerIn.func_184595_k(d3, d4, d5))
                {
                    worldIn.func_184148_a((EntityPlayer)null, d0, d1, d2, SoundEvents.item_chorus_fruit_teleport, SoundCategory.PLAYERS, 1.0F, 1.0F);
                    playerIn.playSound(SoundEvents.item_chorus_fruit_teleport, 1.0F, 1.0F);
                    break;
                }
            }

            if (playerIn instanceof EntityPlayer)
            {
                ((EntityPlayer)playerIn).func_184811_cZ().setCooldown(this, 20);
            }
        }

        return itemstack;
    }
}
