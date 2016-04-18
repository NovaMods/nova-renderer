package net.minecraft.potion;

public class PotionHealth extends Potion
{
    public PotionHealth(boolean p_i46816_1_, int p_i46816_2_)
    {
        super(p_i46816_1_, p_i46816_2_);
    }

    /**
     * Returns true if the potion has an instant effect instead of a continuous one (eg Harming)
     */
    public boolean isInstant()
    {
        return true;
    }

    /**
     * checks if Potion effect is ready to be applied this tick.
     */
    public boolean isReady(int p_76397_1_, int p_76397_2_)
    {
        return p_76397_1_ >= 1;
    }
}
