package net.minecraft.potion;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.ai.attributes.AbstractAttributeMap;

public class PotionHealthBoost extends Potion
{
    public PotionHealthBoost(boolean p_i46817_1_, int p_i46817_2_)
    {
        super(p_i46817_1_, p_i46817_2_);
    }

    public void removeAttributesModifiersFromEntity(EntityLivingBase entityLivingBaseIn, AbstractAttributeMap p_111187_2_, int amplifier)
    {
        super.removeAttributesModifiersFromEntity(entityLivingBaseIn, p_111187_2_, amplifier);

        if (entityLivingBaseIn.getHealth() > entityLivingBaseIn.getMaxHealth())
        {
            entityLivingBaseIn.setHealth(entityLivingBaseIn.getMaxHealth());
        }
    }
}
