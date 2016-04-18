package net.minecraft.item;

import java.util.List;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.entity.projectile.EntityTippedArrow;
import net.minecraft.potion.PotionType;
import net.minecraft.potion.PotionUtils;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.World;

public class ItemTippedArrow extends ItemArrow
{
    public EntityArrow func_185052_a(World worldIn, ItemStack p_185052_2_, EntityLivingBase shooter)
    {
        EntityTippedArrow entitytippedarrow = new EntityTippedArrow(worldIn, shooter);
        entitytippedarrow.func_184555_a(p_185052_2_);
        return entitytippedarrow;
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

    /**
     * allows items to add custom lines of information to the mouseover description
     */
    public void addInformation(ItemStack stack, EntityPlayer playerIn, List<String> tooltip, boolean advanced)
    {
        PotionUtils.addPotionTooltip(stack, tooltip, 0.125F);
    }

    public String getItemStackDisplayName(ItemStack stack)
    {
        return I18n.translateToLocal(PotionUtils.func_185191_c(stack).func_185174_b("tipped_arrow.effect."));
    }
}
