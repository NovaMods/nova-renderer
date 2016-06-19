package net.minecraft.item;

import java.util.List;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.potion.PotionEffect;
import net.minecraft.potion.PotionType;
import net.minecraft.potion.PotionUtils;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.World;

public class ItemPotion extends Item
{
    public ItemPotion()
    {
        this.setMaxStackSize(1);
        this.setCreativeTab(CreativeTabs.tabBrewing);
    }

    /**
     * Called when the player finishes using this Item (E.g. finishes eating.). Not called when the player stops using
     * the Item before the action is complete.
     */
    public ItemStack onItemUseFinish(ItemStack stack, World worldIn, EntityLivingBase playerIn)
    {
        EntityPlayer entityplayer = playerIn instanceof EntityPlayer ? (EntityPlayer)playerIn : null;

        if (entityplayer == null || !entityplayer.capabilities.isCreativeMode)
        {
            --stack.stackSize;
        }

        if (!worldIn.isRemote)
        {
            for (PotionEffect potioneffect : PotionUtils.getEffectsFromStack(stack))
            {
                playerIn.addPotionEffect(new PotionEffect(potioneffect));
            }
        }

        if (entityplayer != null)
        {
            entityplayer.triggerAchievement(StatList.func_188057_b(this));
        }

        if (entityplayer == null || !entityplayer.capabilities.isCreativeMode)
        {
            if (stack.stackSize <= 0)
            {
                return new ItemStack(Items.glass_bottle);
            }

            if (entityplayer != null)
            {
                entityplayer.inventory.addItemStackToInventory(new ItemStack(Items.glass_bottle));
            }
        }

        return stack;
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

    public String getItemStackDisplayName(ItemStack stack)
    {
        return I18n.translateToLocal(PotionUtils.func_185191_c(stack).func_185174_b("potion.effect."));
    }

    /**
     * allows items to add custom lines of information to the mouseover description
     */
    public void addInformation(ItemStack stack, EntityPlayer playerIn, List<String> tooltip, boolean advanced)
    {
        PotionUtils.addPotionTooltip(stack, tooltip, 1.0F);
    }

    public boolean hasEffect(ItemStack stack)
    {
        return !PotionUtils.getEffectsFromStack(stack).isEmpty();
    }

    /**
     * returns a list of items with the same ID, but different meta (eg: dye returns 16 items)
     */
    public void getSubItems(Item itemIn, CreativeTabs tab, List<ItemStack> subItems)
    {
        for (PotionType potiontype : PotionType.REGISTRY)
        {
            subItems.add(PotionUtils.func_185188_a(new ItemStack(itemIn), potiontype));
        }
    }
}
