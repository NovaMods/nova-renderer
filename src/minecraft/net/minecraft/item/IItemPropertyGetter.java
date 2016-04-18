package net.minecraft.item;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.world.World;

public interface IItemPropertyGetter
{
    float apply(ItemStack stack, World worldIn, EntityLivingBase entityIn);
}
