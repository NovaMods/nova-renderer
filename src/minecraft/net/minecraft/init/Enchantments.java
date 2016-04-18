package net.minecraft.init;

import net.minecraft.enchantment.Enchantment;
import net.minecraft.util.ResourceLocation;

public class Enchantments
{
    public static final Enchantment protection = getRegisteredEnchantment("protection");

    /** Protection against fire */
    public static final Enchantment fireProtection = getRegisteredEnchantment("fire_protection");
    public static final Enchantment featherFalling = getRegisteredEnchantment("feather_falling");
    public static final Enchantment blastProtection = getRegisteredEnchantment("blast_protection");
    public static final Enchantment projectileProtection = getRegisteredEnchantment("projectile_protection");
    public static final Enchantment respiration = getRegisteredEnchantment("respiration");
    public static final Enchantment aquaAffinity = getRegisteredEnchantment("aqua_affinity");
    public static final Enchantment thorns = getRegisteredEnchantment("thorns");
    public static final Enchantment depthStrider = getRegisteredEnchantment("depth_strider");
    public static final Enchantment frostWalker = getRegisteredEnchantment("frost_walker");
    public static final Enchantment sharpness = getRegisteredEnchantment("sharpness");
    public static final Enchantment smite = getRegisteredEnchantment("smite");
    public static final Enchantment baneOfArthropods = getRegisteredEnchantment("bane_of_arthropods");
    public static final Enchantment knockback = getRegisteredEnchantment("knockback");

    /** Lights mobs on fire */
    public static final Enchantment fireAspect = getRegisteredEnchantment("fire_aspect");
    public static final Enchantment looting = getRegisteredEnchantment("looting");
    public static final Enchantment efficiency = getRegisteredEnchantment("efficiency");
    public static final Enchantment silkTouch = getRegisteredEnchantment("silk_touch");
    public static final Enchantment unbreaking = getRegisteredEnchantment("unbreaking");
    public static final Enchantment fortune = getRegisteredEnchantment("fortune");
    public static final Enchantment power = getRegisteredEnchantment("power");
    public static final Enchantment punch = getRegisteredEnchantment("punch");
    public static final Enchantment flame = getRegisteredEnchantment("flame");
    public static final Enchantment infinity = getRegisteredEnchantment("infinity");
    public static final Enchantment luckOfTheSea = getRegisteredEnchantment("luck_of_the_sea");
    public static final Enchantment lure = getRegisteredEnchantment("lure");
    public static final Enchantment mending = getRegisteredEnchantment("mending");

    private static Enchantment getRegisteredEnchantment(String id)
    {
        Enchantment enchantment = (Enchantment)Enchantment.enchantmentRegistry.getObject(new ResourceLocation(id));

        if (enchantment == null)
        {
            throw new IllegalStateException("Invalid Enchantment requested: " + id);
        }
        else
        {
            return enchantment;
        }
    }

    static
    {
        if (!Bootstrap.isRegistered())
        {
            throw new RuntimeException("Accessed MobEffects before Bootstrap!");
        }
    }
}
