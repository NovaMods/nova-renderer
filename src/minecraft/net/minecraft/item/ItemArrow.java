package net.minecraft.item;

import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.entity.projectile.EntityTippedArrow;
import net.minecraft.world.World;

public class ItemArrow extends Item
{
    public ItemArrow()
    {
        this.setCreativeTab(CreativeTabs.tabCombat);
    }

    public EntityArrow func_185052_a(World worldIn, ItemStack p_185052_2_, EntityLivingBase shooter)
    {
        EntityTippedArrow entitytippedarrow = new EntityTippedArrow(worldIn, shooter);
        entitytippedarrow.func_184555_a(p_185052_2_);
        return entitytippedarrow;
    }
}
