package net.minecraft.potion;

import com.google.common.collect.ImmutableList;
import java.util.List;
import net.minecraft.init.MobEffects;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.registry.RegistryNamespacedDefaultedByKey;

public class PotionType
{
    private static final ResourceLocation WATER = new ResourceLocation("water");
    public static final RegistryNamespacedDefaultedByKey<ResourceLocation, PotionType> REGISTRY = new RegistryNamespacedDefaultedByKey(WATER);
    private static int field_185178_c;
    private final String name;
    private final ImmutableList<PotionEffect> effects;

    public static PotionType getPotionTypeForID(int p_185169_0_)
    {
        return (PotionType)REGISTRY.getObjectById(p_185169_0_);
    }

    public static int getID(PotionType p_185171_0_)
    {
        return REGISTRY.getIDForObject(p_185171_0_);
    }

    public static PotionType getPotionTypeForName(String p_185168_0_)
    {
        return (PotionType)REGISTRY.getObject(new ResourceLocation(p_185168_0_));
    }

    public PotionType(PotionEffect... p_i46739_1_)
    {
        this((String)null, p_i46739_1_);
    }

    public PotionType(String p_i46740_1_, PotionEffect... p_i46740_2_)
    {
        this.name = p_i46740_1_;
        this.effects = ImmutableList.copyOf(p_i46740_2_);
    }

    public String func_185174_b(String p_185174_1_)
    {
        return this.name == null ? p_185174_1_ + ((ResourceLocation)REGISTRY.getNameForObject(this)).getResourcePath() : p_185174_1_ + this.name;
    }

    public List<PotionEffect> getEffects()
    {
        return this.effects;
    }

    public static void registerPotionTypes()
    {
        registerPotionType("empty", new PotionType(new PotionEffect[0]));
        registerPotionType("water", new PotionType(new PotionEffect[0]));
        registerPotionType("mundane", new PotionType(new PotionEffect[0]));
        registerPotionType("thick", new PotionType(new PotionEffect[0]));
        registerPotionType("awkward", new PotionType(new PotionEffect[0]));
        registerPotionType("night_vision", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.nightVision, 3600)}));
        registerPotionType("long_night_vision", new PotionType("night_vision", new PotionEffect[] {new PotionEffect(MobEffects.nightVision, 9600)}));
        registerPotionType("invisibility", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.invisibility, 3600)}));
        registerPotionType("long_invisibility", new PotionType("invisibility", new PotionEffect[] {new PotionEffect(MobEffects.invisibility, 9600)}));
        registerPotionType("leaping", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.jump, 3600)}));
        registerPotionType("long_leaping", new PotionType("leaping", new PotionEffect[] {new PotionEffect(MobEffects.jump, 9600)}));
        registerPotionType("strong_leaping", new PotionType("leaping", new PotionEffect[] {new PotionEffect(MobEffects.jump, 1800, 1)}));
        registerPotionType("fire_resistance", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.fireResistance, 3600)}));
        registerPotionType("long_fire_resistance", new PotionType("fire_resistance", new PotionEffect[] {new PotionEffect(MobEffects.fireResistance, 9600)}));
        registerPotionType("swiftness", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.moveSpeed, 3600)}));
        registerPotionType("long_swiftness", new PotionType("swiftness", new PotionEffect[] {new PotionEffect(MobEffects.moveSpeed, 9600)}));
        registerPotionType("strong_swiftness", new PotionType("swiftness", new PotionEffect[] {new PotionEffect(MobEffects.moveSpeed, 1800, 1)}));
        registerPotionType("slowness", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.moveSlowdown, 1800)}));
        registerPotionType("long_slowness", new PotionType("slowness", new PotionEffect[] {new PotionEffect(MobEffects.moveSlowdown, 4800)}));
        registerPotionType("water_breathing", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.waterBreathing, 3600)}));
        registerPotionType("long_water_breathing", new PotionType("water_breathing", new PotionEffect[] {new PotionEffect(MobEffects.waterBreathing, 9600)}));
        registerPotionType("healing", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.heal, 1)}));
        registerPotionType("strong_healing", new PotionType("healing", new PotionEffect[] {new PotionEffect(MobEffects.heal, 1, 1)}));
        registerPotionType("harming", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.harm, 1)}));
        registerPotionType("strong_harming", new PotionType("harming", new PotionEffect[] {new PotionEffect(MobEffects.harm, 1, 1)}));
        registerPotionType("poison", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.poison, 900)}));
        registerPotionType("long_poison", new PotionType("poison", new PotionEffect[] {new PotionEffect(MobEffects.poison, 1800)}));
        registerPotionType("strong_poison", new PotionType("poison", new PotionEffect[] {new PotionEffect(MobEffects.poison, 432, 1)}));
        registerPotionType("regeneration", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.regeneration, 900)}));
        registerPotionType("long_regeneration", new PotionType("regeneration", new PotionEffect[] {new PotionEffect(MobEffects.regeneration, 1800)}));
        registerPotionType("strong_regeneration", new PotionType("regeneration", new PotionEffect[] {new PotionEffect(MobEffects.regeneration, 450, 1)}));
        registerPotionType("strength", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.damageBoost, 3600)}));
        registerPotionType("long_strength", new PotionType("strength", new PotionEffect[] {new PotionEffect(MobEffects.damageBoost, 9600)}));
        registerPotionType("strong_strength", new PotionType("strength", new PotionEffect[] {new PotionEffect(MobEffects.damageBoost, 1800, 1)}));
        registerPotionType("weakness", new PotionType(new PotionEffect[] {new PotionEffect(MobEffects.weakness, 1800)}));
        registerPotionType("long_weakness", new PotionType("weakness", new PotionEffect[] {new PotionEffect(MobEffects.weakness, 4800)}));
        registerPotionType("luck", new PotionType("luck", new PotionEffect[] {new PotionEffect(MobEffects.luck, 6000)}));
        REGISTRY.validateKey();
    }

    protected static void registerPotionType(String p_185173_0_, PotionType p_185173_1_)
    {
        REGISTRY.register(field_185178_c++, new ResourceLocation(p_185173_0_), p_185173_1_);
    }

    public boolean func_185172_c()
    {
        if (!this.effects.isEmpty())
        {
            for (PotionEffect potioneffect : this.effects)
            {
                if (potioneffect.func_188419_a().isInstant())
                {
                    return true;
                }
            }
        }

        return false;
    }
}
