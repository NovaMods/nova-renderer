package net.minecraft.init;

import net.minecraft.potion.Potion;
import net.minecraft.util.ResourceLocation;

public class MobEffects
{
    public static final Potion moveSpeed;
    public static final Potion moveSlowdown;
    public static final Potion digSpeed;
    public static final Potion digSlowdown;
    public static final Potion damageBoost;
    public static final Potion heal;
    public static final Potion harm;
    public static final Potion jump;
    public static final Potion confusion;

    /** The regeneration Potion object. */
    public static final Potion regeneration;
    public static final Potion resistance;

    /** The fire resistance Potion object. */
    public static final Potion fireResistance;

    /** The water breathing Potion object. */
    public static final Potion waterBreathing;

    /** The invisibility Potion object. */
    public static final Potion invisibility;

    /** The has_blindness Potion object. */
    public static final Potion blindness;

    /** The night vision Potion object. */
    public static final Potion nightVision;

    /** The hunger Potion object. */
    public static final Potion hunger;

    /** The weakness Potion object. */
    public static final Potion weakness;

    /** The poison Potion object. */
    public static final Potion poison;

    /** The wither Potion object. */
    public static final Potion wither;

    /** The health boost Potion object. */
    public static final Potion healthBoost;

    /** The absorption Potion object. */
    public static final Potion absorption;

    /** The saturation Potion object. */
    public static final Potion saturation;
    public static final Potion glowing;
    public static final Potion levitation;
    public static final Potion luck;
    public static final Potion unluck;

    private static Potion getRegisteredMobEffect(String id)
    {
        Potion potion = (Potion)Potion.potionRegistry.getObject(new ResourceLocation(id));

        if (potion == null)
        {
            throw new IllegalStateException("Invalid MobEffect requested: " + id);
        }
        else
        {
            return potion;
        }
    }

    static
    {
        if (!Bootstrap.isRegistered())
        {
            throw new RuntimeException("Accessed MobEffects before Bootstrap!");
        }
        else
        {
            moveSpeed = getRegisteredMobEffect("speed");
            moveSlowdown = getRegisteredMobEffect("slowness");
            digSpeed = getRegisteredMobEffect("haste");
            digSlowdown = getRegisteredMobEffect("mining_fatigue");
            damageBoost = getRegisteredMobEffect("strength");
            heal = getRegisteredMobEffect("instant_health");
            harm = getRegisteredMobEffect("instant_damage");
            jump = getRegisteredMobEffect("jump_boost");
            confusion = getRegisteredMobEffect("nausea");
            regeneration = getRegisteredMobEffect("regeneration");
            resistance = getRegisteredMobEffect("resistance");
            fireResistance = getRegisteredMobEffect("fire_resistance");
            waterBreathing = getRegisteredMobEffect("water_breathing");
            invisibility = getRegisteredMobEffect("invisibility");
            blindness = getRegisteredMobEffect("has_blindness");
            nightVision = getRegisteredMobEffect("night_vision");
            hunger = getRegisteredMobEffect("hunger");
            weakness = getRegisteredMobEffect("weakness");
            poison = getRegisteredMobEffect("poison");
            wither = getRegisteredMobEffect("wither");
            healthBoost = getRegisteredMobEffect("health_boost");
            absorption = getRegisteredMobEffect("absorption");
            saturation = getRegisteredMobEffect("saturation");
            glowing = getRegisteredMobEffect("glowing");
            levitation = getRegisteredMobEffect("levitation");
            luck = getRegisteredMobEffect("luck");
            unluck = getRegisteredMobEffect("unluck");
        }
    }
}
