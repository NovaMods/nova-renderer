package net.minecraft.util;

import net.minecraft.util.math.MathHelper;

public class CombatRules
{
    public static float func_188402_a(float p_188402_0_, float p_188402_1_)
    {
        float f = MathHelper.clamp_float(p_188402_1_ - p_188402_0_ * 0.5F, p_188402_1_ * 0.2F, 20.0F);
        float f1 = p_188402_0_ * (1.0F - f / 25.0F);
        return f1;
    }

    public static float func_188401_b(float p_188401_0_, float p_188401_1_)
    {
        float f = MathHelper.clamp_float(p_188401_1_, 0.0F, 20.0F);
        return p_188401_0_ * (1.0F - f / 25.0F);
    }
}
