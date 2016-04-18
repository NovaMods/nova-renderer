package net.minecraft.potion;

import net.minecraft.entity.ai.attributes.AttributeModifier;

public class PotionAttackDamage extends Potion
{
    protected final double field_188416_a;

    protected PotionAttackDamage(boolean p_i46819_1_, int p_i46819_2_, double p_i46819_3_)
    {
        super(p_i46819_1_, p_i46819_2_);
        this.field_188416_a = p_i46819_3_;
    }

    public double getAttributeModifierAmount(int p_111183_1_, AttributeModifier modifier)
    {
        return this.field_188416_a * (double)(p_111183_1_ + 1);
    }
}
