package net.minecraft.init;

import com.google.common.collect.Sets;
import java.util.Set;
import net.minecraft.potion.PotionType;
import net.minecraft.util.ResourceLocation;

public class PotionTypes
{
    private static final Set<PotionType> CACHE;
    public static final PotionType empty;
    public static final PotionType water;
    public static final PotionType mundane;
    public static final PotionType thick;
    public static final PotionType awkward;
    public static final PotionType night_vision;
    public static final PotionType long_night_vision;
    public static final PotionType invisibility;
    public static final PotionType long_invisibility;
    public static final PotionType leaping;
    public static final PotionType long_leaping;
    public static final PotionType strong_leaping;
    public static final PotionType fire_resistance;
    public static final PotionType long_fire_resistance;
    public static final PotionType swiftness;
    public static final PotionType long_swiftness;
    public static final PotionType strong_swiftness;
    public static final PotionType slowness;
    public static final PotionType long_slowness;
    public static final PotionType water_breathing;
    public static final PotionType long_water_breathing;
    public static final PotionType healing;
    public static final PotionType strong_healing;
    public static final PotionType harming;
    public static final PotionType strong_harming;
    public static final PotionType poison;
    public static final PotionType long_poison;
    public static final PotionType strong_poison;
    public static final PotionType regeneration;
    public static final PotionType long_regeneration;
    public static final PotionType strong_regeneration;
    public static final PotionType strength;
    public static final PotionType long_strength;
    public static final PotionType strong_strength;
    public static final PotionType weakness;
    public static final PotionType long_weakness;

    private static PotionType func_185217_a(String id)
    {
        PotionType potiontype = (PotionType)PotionType.REGISTRY.getObject(new ResourceLocation(id));

        if (!CACHE.add(potiontype))
        {
            throw new IllegalStateException("Invalid Potion requested: " + id);
        }
        else
        {
            return potiontype;
        }
    }

    static
    {
        if (!Bootstrap.isRegistered())
        {
            throw new RuntimeException("Accessed Potions before Bootstrap!");
        }
        else
        {
            CACHE = Sets.<PotionType>newHashSet();
            empty = func_185217_a("empty");
            water = func_185217_a("water");
            mundane = func_185217_a("mundane");
            thick = func_185217_a("thick");
            awkward = func_185217_a("awkward");
            night_vision = func_185217_a("night_vision");
            long_night_vision = func_185217_a("long_night_vision");
            invisibility = func_185217_a("invisibility");
            long_invisibility = func_185217_a("long_invisibility");
            leaping = func_185217_a("leaping");
            long_leaping = func_185217_a("long_leaping");
            strong_leaping = func_185217_a("strong_leaping");
            fire_resistance = func_185217_a("fire_resistance");
            long_fire_resistance = func_185217_a("long_fire_resistance");
            swiftness = func_185217_a("swiftness");
            long_swiftness = func_185217_a("long_swiftness");
            strong_swiftness = func_185217_a("strong_swiftness");
            slowness = func_185217_a("slowness");
            long_slowness = func_185217_a("long_slowness");
            water_breathing = func_185217_a("water_breathing");
            long_water_breathing = func_185217_a("long_water_breathing");
            healing = func_185217_a("healing");
            strong_healing = func_185217_a("strong_healing");
            harming = func_185217_a("harming");
            strong_harming = func_185217_a("strong_harming");
            poison = func_185217_a("poison");
            long_poison = func_185217_a("long_poison");
            strong_poison = func_185217_a("strong_poison");
            regeneration = func_185217_a("regeneration");
            long_regeneration = func_185217_a("long_regeneration");
            strong_regeneration = func_185217_a("strong_regeneration");
            strength = func_185217_a("strength");
            long_strength = func_185217_a("long_strength");
            strong_strength = func_185217_a("strong_strength");
            weakness = func_185217_a("weakness");
            long_weakness = func_185217_a("long_weakness");
            CACHE.clear();
        }
    }
}
