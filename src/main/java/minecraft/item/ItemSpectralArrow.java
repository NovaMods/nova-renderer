package net.minecraft.item;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.entity.projectile.EntitySpectralArrow;
import net.minecraft.world.World;

public class ItemSpectralArrow extends ItemArrow
{
    public EntityArrow func_185052_a(World worldIn, ItemStack p_185052_2_, EntityLivingBase shooter)
    {
        return new EntitySpectralArrow(worldIn, shooter);
    }
}
