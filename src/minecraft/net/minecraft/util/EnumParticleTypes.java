package net.minecraft.util;

import com.google.common.collect.Maps;
import java.util.Map;
import java.util.Set;

public enum EnumParticleTypes
{
    EXPLOSION_NORMAL("explode", 0, true),
    EXPLOSION_LARGE("largeexplode", 1, true),
    EXPLOSION_HUGE("hugeexplosion", 2, true),
    FIREWORKS_SPARK("fireworksSpark", 3, false),
    WATER_BUBBLE("bubble", 4, false),
    WATER_SPLASH("splash", 5, false),
    WATER_WAKE("wake", 6, false),
    SUSPENDED("suspended", 7, false),
    SUSPENDED_DEPTH("depthsuspend", 8, false),
    CRIT("crit", 9, false),
    CRIT_MAGIC("magicCrit", 10, false),
    SMOKE_NORMAL("smoke", 11, false),
    SMOKE_LARGE("largesmoke", 12, false),
    SPELL("spell", 13, false),
    SPELL_INSTANT("instantSpell", 14, false),
    SPELL_MOB("mobSpell", 15, false),
    SPELL_MOB_AMBIENT("mobSpellAmbient", 16, false),
    SPELL_WITCH("witchMagic", 17, false),
    DRIP_WATER("dripWater", 18, false),
    DRIP_LAVA("dripLava", 19, false),
    VILLAGER_ANGRY("angryVillager", 20, false),
    VILLAGER_HAPPY("happyVillager", 21, false),
    TOWN_AURA("townaura", 22, false),
    NOTE("note", 23, false),
    PORTAL("portal", 24, false),
    ENCHANTMENT_TABLE("enchantmenttable", 25, false),
    FLAME("flame", 26, false),
    LAVA("lava", 27, false),
    FOOTSTEP("footstep", 28, false),
    CLOUD("cloud", 29, false),
    REDSTONE("reddust", 30, false),
    SNOWBALL("snowballpoof", 31, false),
    SNOW_SHOVEL("snowshovel", 32, false),
    SLIME("slime", 33, false),
    HEART("heart", 34, false),
    BARRIER("barrier", 35, false),
    ITEM_CRACK("iconcrack", 36, false, 2),
    BLOCK_CRACK("blockcrack", 37, false, 1),
    BLOCK_DUST("blockdust", 38, false, 1),
    WATER_DROP("droplet", 39, false),
    ITEM_TAKE("take", 40, false),
    MOB_APPEARANCE("mobappearance", 41, true),
    DRAGON_BREATH("dragonbreath", 42, false),
    END_ROD("endRod", 43, false),
    DAMAGE_INDICATOR("damageIndicator", 44, true),
    SWEEP_ATTACK("sweepAttack", 45, true);

    private final String particleName;
    private final int particleID;
    private final boolean shouldIgnoreRange;
    private final int argumentCount;
    private static final Map<Integer, EnumParticleTypes> PARTICLES = Maps.<Integer, EnumParticleTypes>newHashMap();
    private static final Map<String, EnumParticleTypes> field_186837_Z = Maps.<String, EnumParticleTypes>newHashMap();

    private EnumParticleTypes(String particleNameIn, int particleIDIn, boolean p_i46011_5_, int argumentCountIn)
    {
        this.particleName = particleNameIn;
        this.particleID = particleIDIn;
        this.shouldIgnoreRange = p_i46011_5_;
        this.argumentCount = argumentCountIn;
    }

    private EnumParticleTypes(String particleNameIn, int particleIDIn, boolean p_i46012_5_)
    {
        this(particleNameIn, particleIDIn, p_i46012_5_, 0);
    }

    public static Set<String> func_186832_a()
    {
        return field_186837_Z.keySet();
    }

    public String getParticleName()
    {
        return this.particleName;
    }

    public int getParticleID()
    {
        return this.particleID;
    }

    public int getArgumentCount()
    {
        return this.argumentCount;
    }

    public boolean getShouldIgnoreRange()
    {
        return this.shouldIgnoreRange;
    }

    /**
     * Gets the relative EnumParticleTypes by id.
     */
    public static EnumParticleTypes getParticleFromId(int particleId)
    {
        return (EnumParticleTypes)PARTICLES.get(Integer.valueOf(particleId));
    }

    public static EnumParticleTypes func_186831_a(String p_186831_0_)
    {
        return (EnumParticleTypes)field_186837_Z.get(p_186831_0_);
    }

    static {
        for (EnumParticleTypes enumparticletypes : values())
        {
            PARTICLES.put(Integer.valueOf(enumparticletypes.getParticleID()), enumparticletypes);
            field_186837_Z.put(enumparticletypes.getParticleName(), enumparticletypes);
        }
    }
}
