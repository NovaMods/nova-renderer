package net.minecraft.potion;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.ai.attributes.AbstractAttributeMap;

public class PotionAbsorption extends Potion
{
    protected PotionAbsorption(boolean p_i46820_1_, int p_i46820_2_)
    {
        super(p_i46820_1_, p_i46820_2_);
    }

    public void removeAttributesModifiersFromEntity(EntityLivingBase entityLivingBaseIn, AbstractAttributeMap p_111187_2_, int amplifier)
    {
        entityLivingBaseIn.setAbsorptionAmount(entityLivingBaseIn.getAbsorptionAmount() - (float)(4 * (amplifier + 1)));
        super.removeAttributesModifiersFromEntity(entityLivingBaseIn, p_111187_2_, amplifier);
    }

    public void applyAttributesModifiersToEntity(EntityLivingBase entityLivingBaseIn, AbstractAttributeMap p_111185_2_, int amplifier)
    {
        entityLivingBaseIn.setAbsorptionAmount(entityLivingBaseIn.getAbsorptionAmount() + (float)(4 * (amplifier + 1)));
        super.applyAttributesModifiersToEntity(entityLivingBaseIn, p_111185_2_, amplifier);
    }
}
