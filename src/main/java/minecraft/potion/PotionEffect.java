package net.minecraft.potion;

import com.google.common.collect.ComparisonChain;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.nbt.NBTTagCompound;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class PotionEffect implements Comparable<PotionEffect>
{
    private static final Logger LOGGER = LogManager.getLogger();
    private final Potion field_188420_b;

    /** The duration of the potion effect */
    private int duration;

    /** The amplifier of the potion effect */
    private int amplifier;

    /** Whether the potion is a splash potion */
    private boolean isSplashPotion;

    /** Whether the potion effect came from a beacon */
    private boolean isAmbient;

    /** True if potion effect duration is at maximum, false otherwise. */
    private boolean isPotionDurationMax;
    private boolean field_188421_h;

    public PotionEffect(Potion p_i46811_1_)
    {
        this(p_i46811_1_, 0, 0);
    }

    public PotionEffect(Potion p_i46812_1_, int p_i46812_2_)
    {
        this(p_i46812_1_, p_i46812_2_, 0);
    }

    public PotionEffect(Potion p_i46813_1_, int p_i46813_2_, int p_i46813_3_)
    {
        this(p_i46813_1_, p_i46813_2_, p_i46813_3_, false, true);
    }

    public PotionEffect(Potion p_i46814_1_, int p_i46814_2_, int p_i46814_3_, boolean p_i46814_4_, boolean p_i46814_5_)
    {
        this.field_188420_b = p_i46814_1_;
        this.duration = p_i46814_2_;
        this.amplifier = p_i46814_3_;
        this.isAmbient = p_i46814_4_;
        this.field_188421_h = p_i46814_5_;
    }

    public PotionEffect(PotionEffect other)
    {
        this.field_188420_b = other.field_188420_b;
        this.duration = other.duration;
        this.amplifier = other.amplifier;
        this.isAmbient = other.isAmbient;
        this.field_188421_h = other.field_188421_h;
    }

    /**
     * merges the input PotionEffect into this one if this.amplifier <= tomerge.amplifier. The duration in the supplied
     * potion effect is assumed to be greater.
     */
    public void combine(PotionEffect other)
    {
        if (this.field_188420_b != other.field_188420_b)
        {
            LOGGER.warn("This method should only be called for matching effects!");
        }

        if (other.amplifier > this.amplifier)
        {
            this.amplifier = other.amplifier;
            this.duration = other.duration;
        }
        else if (other.amplifier == this.amplifier && this.duration < other.duration)
        {
            this.duration = other.duration;
        }
        else if (!other.isAmbient && this.isAmbient)
        {
            this.isAmbient = other.isAmbient;
        }

        this.field_188421_h = other.field_188421_h;
    }

    public Potion func_188419_a()
    {
        return this.field_188420_b;
    }

    public int getDuration()
    {
        return this.duration;
    }

    public int getAmplifier()
    {
        return this.amplifier;
    }

    /**
     * Gets whether this potion effect originated from a beacon
     */
    public boolean getIsAmbient()
    {
        return this.isAmbient;
    }

    public boolean func_188418_e()
    {
        return this.field_188421_h;
    }

    public boolean onUpdate(EntityLivingBase entityIn)
    {
        if (this.duration > 0)
        {
            if (this.field_188420_b.isReady(this.duration, this.amplifier))
            {
                this.performEffect(entityIn);
            }

            this.deincrementDuration();
        }

        return this.duration > 0;
    }

    private int deincrementDuration()
    {
        return --this.duration;
    }

    public void performEffect(EntityLivingBase entityIn)
    {
        if (this.duration > 0)
        {
            this.field_188420_b.performEffect(entityIn, this.amplifier);
        }
    }

    public String getEffectName()
    {
        return this.field_188420_b.getName();
    }

    public String toString()
    {
        String s = "";

        if (this.amplifier > 0)
        {
            s = this.getEffectName() + " x " + (this.amplifier + 1) + ", Duration: " + this.duration;
        }
        else
        {
            s = this.getEffectName() + ", Duration: " + this.duration;
        }

        if (this.isSplashPotion)
        {
            s = s + ", Splash: true";
        }

        if (!this.field_188421_h)
        {
            s = s + ", Particles: false";
        }

        return s;
    }

    public boolean equals(Object p_equals_1_)
    {
        if (this == p_equals_1_)
        {
            return true;
        }
        else if (!(p_equals_1_ instanceof PotionEffect))
        {
            return false;
        }
        else
        {
            PotionEffect potioneffect = (PotionEffect)p_equals_1_;
            return this.duration == potioneffect.duration && this.amplifier == potioneffect.amplifier && this.isSplashPotion == potioneffect.isSplashPotion && this.isAmbient == potioneffect.isAmbient && this.field_188420_b.equals(potioneffect.field_188420_b);
        }
    }

    public int hashCode()
    {
        int i = this.field_188420_b.hashCode();
        i = 31 * i + this.duration;
        i = 31 * i + this.amplifier;
        i = 31 * i + (this.isSplashPotion ? 1 : 0);
        i = 31 * i + (this.isAmbient ? 1 : 0);
        return i;
    }

    /**
     * Write a custom potion effect to a potion item's NBT data.
     */
    public NBTTagCompound writeCustomPotionEffectToNBT(NBTTagCompound nbt)
    {
        nbt.setByte("Id", (byte)Potion.getIdFromPotion(this.func_188419_a()));
        nbt.setByte("Amplifier", (byte)this.getAmplifier());
        nbt.setInteger("Duration", this.getDuration());
        nbt.setBoolean("Ambient", this.getIsAmbient());
        nbt.setBoolean("ShowParticles", this.func_188418_e());
        return nbt;
    }

    /**
     * Read a custom potion effect from a potion item's NBT data.
     */
    public static PotionEffect readCustomPotionEffectFromNBT(NBTTagCompound nbt)
    {
        int i = nbt.getByte("Id");
        Potion potion = Potion.getPotionById(i);

        if (potion == null)
        {
            return null;
        }
        else
        {
            int j = nbt.getByte("Amplifier");
            int k = nbt.getInteger("Duration");
            boolean flag = nbt.getBoolean("Ambient");
            boolean flag1 = true;

            if (nbt.hasKey("ShowParticles", 1))
            {
                flag1 = nbt.getBoolean("ShowParticles");
            }

            return new PotionEffect(potion, k, j, flag, flag1);
        }
    }

    /**
     * Toggle the isPotionDurationMax field.
     */
    public void setPotionDurationMax(boolean maxDuration)
    {
        this.isPotionDurationMax = maxDuration;
    }

    public boolean getIsPotionDurationMax()
    {
        return this.isPotionDurationMax;
    }

    public int compareTo(PotionEffect p_compareTo_1_)
    {
        int i = 32147;
        return (this.getDuration() <= 32147 || p_compareTo_1_.getDuration() <= 32147) && (!this.getIsAmbient() || !p_compareTo_1_.getIsAmbient()) ? ComparisonChain.start().compare(Boolean.valueOf(this.getIsAmbient()), Boolean.valueOf(p_compareTo_1_.getIsAmbient())).compare(this.getDuration(), p_compareTo_1_.getDuration()).compare(this.func_188419_a().getLiquidColor(), p_compareTo_1_.func_188419_a().getLiquidColor()).result() : ComparisonChain.start().compare(Boolean.valueOf(this.getIsAmbient()), Boolean.valueOf(p_compareTo_1_.getIsAmbient())).compare(this.func_188419_a().getLiquidColor(), p_compareTo_1_.func_188419_a().getLiquidColor()).result();
    }
}
