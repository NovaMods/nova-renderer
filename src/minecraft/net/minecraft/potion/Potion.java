package net.minecraft.potion;

import com.google.common.collect.Maps;
import java.util.Map;
import java.util.UUID;
import java.util.Map.Entry;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.attributes.AbstractAttributeMap;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttribute;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.MobEffects;
import net.minecraft.util.DamageSource;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.StringUtils;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.registry.RegistryNamespaced;

public class Potion
{
    public static final RegistryNamespaced<ResourceLocation, Potion> potionRegistry = new RegistryNamespaced();
    private final Map<IAttribute, AttributeModifier> attributeModifierMap = Maps.<IAttribute, AttributeModifier>newHashMap();

    /**
     * This field indicated if the effect is 'bad' - negative - for the entity.
     */
    private final boolean isBadEffect;

    /** Is the color of the liquid for this potion. */
    private final int liquidColor;

    /** The name of the Potion. */
    private String name = "";

    /** The index for the icon displayed when the potion effect is active. */
    private int statusIconIndex = -1;
    private double effectiveness;
    private boolean field_188415_h;

    /**
     * Gets a Potion from the potion registry using a numeric Id.
     *  
     * @param potionID The ID of the potion to search for.
     */
    public static Potion getPotionById(int potionID)
    {
        return (Potion)potionRegistry.getObjectById(potionID);
    }

    /**
     * Gets the numeric Id associated with a potion.
     */
    public static int getIdFromPotion(Potion p_188409_0_)
    {
        return potionRegistry.getIDForObject(p_188409_0_);
    }

    public static Potion getPotionFromResourceLocation(String location)
    {
        return (Potion)potionRegistry.getObject(new ResourceLocation(location));
    }

    protected Potion(boolean p_i46815_1_, int p_i46815_2_)
    {
        this.isBadEffect = p_i46815_1_;

        if (p_i46815_1_)
        {
            this.effectiveness = 0.5D;
        }
        else
        {
            this.effectiveness = 1.0D;
        }

        this.liquidColor = p_i46815_2_;
    }

    /**
     * Sets the index for the icon displayed in the player's inventory when the status is active.
     */
    protected Potion setIconIndex(int p_76399_1_, int p_76399_2_)
    {
        this.statusIconIndex = p_76399_1_ + p_76399_2_ * 8;
        return this;
    }

    public void performEffect(EntityLivingBase entityLivingBaseIn, int p_76394_2_)
    {
        if (this == MobEffects.regeneration)
        {
            if (entityLivingBaseIn.getHealth() < entityLivingBaseIn.getMaxHealth())
            {
                entityLivingBaseIn.heal(1.0F);
            }
        }
        else if (this == MobEffects.poison)
        {
            if (entityLivingBaseIn.getHealth() > 1.0F)
            {
                entityLivingBaseIn.attackEntityFrom(DamageSource.magic, 1.0F);
            }
        }
        else if (this == MobEffects.wither)
        {
            entityLivingBaseIn.attackEntityFrom(DamageSource.wither, 1.0F);
        }
        else if (this == MobEffects.hunger && entityLivingBaseIn instanceof EntityPlayer)
        {
            ((EntityPlayer)entityLivingBaseIn).addExhaustion(0.025F * (float)(p_76394_2_ + 1));
        }
        else if (this == MobEffects.saturation && entityLivingBaseIn instanceof EntityPlayer)
        {
            if (!entityLivingBaseIn.worldObj.isRemote)
            {
                ((EntityPlayer)entityLivingBaseIn).getFoodStats().addStats(p_76394_2_ + 1, 1.0F);
            }
        }
        else if ((this != MobEffects.heal || entityLivingBaseIn.isEntityUndead()) && (this != MobEffects.harm || !entityLivingBaseIn.isEntityUndead()))
        {
            if (this == MobEffects.harm && !entityLivingBaseIn.isEntityUndead() || this == MobEffects.heal && entityLivingBaseIn.isEntityUndead())
            {
                entityLivingBaseIn.attackEntityFrom(DamageSource.magic, (float)(6 << p_76394_2_));
            }
        }
        else
        {
            entityLivingBaseIn.heal((float)Math.max(4 << p_76394_2_, 0));
        }
    }

    public void affectEntity(Entity p_180793_1_, Entity p_180793_2_, EntityLivingBase entityLivingBaseIn, int p_180793_4_, double p_180793_5_)
    {
        if ((this != MobEffects.heal || entityLivingBaseIn.isEntityUndead()) && (this != MobEffects.harm || !entityLivingBaseIn.isEntityUndead()))
        {
            if (this == MobEffects.harm && !entityLivingBaseIn.isEntityUndead() || this == MobEffects.heal && entityLivingBaseIn.isEntityUndead())
            {
                int j = (int)(p_180793_5_ * (double)(6 << p_180793_4_) + 0.5D);

                if (p_180793_1_ == null)
                {
                    entityLivingBaseIn.attackEntityFrom(DamageSource.magic, (float)j);
                }
                else
                {
                    entityLivingBaseIn.attackEntityFrom(DamageSource.causeIndirectMagicDamage(p_180793_1_, p_180793_2_), (float)j);
                }
            }
        }
        else
        {
            int i = (int)(p_180793_5_ * (double)(4 << p_180793_4_) + 0.5D);
            entityLivingBaseIn.heal((float)i);
        }
    }

    /**
     * checks if Potion effect is ready to be applied this tick.
     */
    public boolean isReady(int p_76397_1_, int p_76397_2_)
    {
        if (this == MobEffects.regeneration)
        {
            int k = 50 >> p_76397_2_;
            return k > 0 ? p_76397_1_ % k == 0 : true;
        }
        else if (this == MobEffects.poison)
        {
            int j = 25 >> p_76397_2_;
            return j > 0 ? p_76397_1_ % j == 0 : true;
        }
        else if (this == MobEffects.wither)
        {
            int i = 40 >> p_76397_2_;
            return i > 0 ? p_76397_1_ % i == 0 : true;
        }
        else
        {
            return this == MobEffects.hunger;
        }
    }

    /**
     * Returns true if the potion has an instant effect instead of a continuous one (eg Harming)
     */
    public boolean isInstant()
    {
        return false;
    }

    /**
     * Set the potion name.
     */
    public Potion setPotionName(String nameIn)
    {
        this.name = nameIn;
        return this;
    }

    /**
     * returns the name of the potion
     */
    public String getName()
    {
        return this.name;
    }

    /**
     * Returns true if the potion has a associated status icon to display in then inventory when active.
     */
    public boolean hasStatusIcon()
    {
        return this.statusIconIndex >= 0;
    }

    /**
     * Returns the index for the icon to display when the potion is active.
     */
    public int getStatusIconIndex()
    {
        return this.statusIconIndex;
    }

    /**
     * This method returns true if the potion effect is bad - negative - for the entity.
     */
    public boolean isBadEffect()
    {
        return this.isBadEffect;
    }

    public static String func_188410_a(PotionEffect p_188410_0_, float p_188410_1_)
    {
        if (p_188410_0_.getIsPotionDurationMax())
        {
            return "**:**";
        }
        else
        {
            int i = MathHelper.floor_float((float)p_188410_0_.getDuration() * p_188410_1_);
            return StringUtils.ticksToElapsedTime(i);
        }
    }

    protected Potion setEffectiveness(double effectivenessIn)
    {
        this.effectiveness = effectivenessIn;
        return this;
    }

    /**
     * Returns the color of the potion liquid.
     */
    public int getLiquidColor()
    {
        return this.liquidColor;
    }

    /**
     * Used by potions to register the attribute they modify.
     */
    public Potion registerPotionAttributeModifier(IAttribute p_111184_1_, String p_111184_2_, double p_111184_3_, int p_111184_5_)
    {
        AttributeModifier attributemodifier = new AttributeModifier(UUID.fromString(p_111184_2_), this.getName(), p_111184_3_, p_111184_5_);
        this.attributeModifierMap.put(p_111184_1_, attributemodifier);
        return this;
    }

    public Map<IAttribute, AttributeModifier> getAttributeModifierMap()
    {
        return this.attributeModifierMap;
    }

    public void removeAttributesModifiersFromEntity(EntityLivingBase entityLivingBaseIn, AbstractAttributeMap p_111187_2_, int amplifier)
    {
        for (Entry<IAttribute, AttributeModifier> entry : this.attributeModifierMap.entrySet())
        {
            IAttributeInstance iattributeinstance = p_111187_2_.getAttributeInstance((IAttribute)entry.getKey());

            if (iattributeinstance != null)
            {
                iattributeinstance.removeModifier((AttributeModifier)entry.getValue());
            }
        }
    }

    public void applyAttributesModifiersToEntity(EntityLivingBase entityLivingBaseIn, AbstractAttributeMap p_111185_2_, int amplifier)
    {
        for (Entry<IAttribute, AttributeModifier> entry : this.attributeModifierMap.entrySet())
        {
            IAttributeInstance iattributeinstance = p_111185_2_.getAttributeInstance((IAttribute)entry.getKey());

            if (iattributeinstance != null)
            {
                AttributeModifier attributemodifier = (AttributeModifier)entry.getValue();
                iattributeinstance.removeModifier(attributemodifier);
                iattributeinstance.applyModifier(new AttributeModifier(attributemodifier.getID(), this.getName() + " " + amplifier, this.getAttributeModifierAmount(amplifier, attributemodifier), attributemodifier.getOperation()));
            }
        }
    }

    public double getAttributeModifierAmount(int p_111183_1_, AttributeModifier modifier)
    {
        return modifier.getAmount() * (double)(p_111183_1_ + 1);
    }

    public boolean func_188408_i()
    {
        return this.field_188415_h;
    }

    public Potion func_188413_j()
    {
        this.field_188415_h = true;
        return this;
    }

    public static void registerPotions()
    {
        potionRegistry.register(1, new ResourceLocation("speed"), (new Potion(false, 8171462)).setPotionName("effect.moveSpeed").setIconIndex(0, 0).registerPotionAttributeModifier(SharedMonsterAttributes.MOVEMENT_SPEED, "91AEAA56-376B-4498-935B-2F7F68070635", 0.20000000298023224D, 2).func_188413_j());
        potionRegistry.register(2, new ResourceLocation("slowness"), (new Potion(true, 5926017)).setPotionName("effect.moveSlowdown").setIconIndex(1, 0).registerPotionAttributeModifier(SharedMonsterAttributes.MOVEMENT_SPEED, "7107DE5E-7CE8-4030-940E-514C1F160890", -0.15000000596046448D, 2));
        potionRegistry.register(3, new ResourceLocation("haste"), (new Potion(false, 14270531)).setPotionName("effect.digSpeed").setIconIndex(2, 0).setEffectiveness(1.5D).func_188413_j().registerPotionAttributeModifier(SharedMonsterAttributes.ATTACK_SPEED, "AF8B6E3F-3328-4C0A-AA36-5BA2BB9DBEF3", 0.10000000149011612D, 2));
        potionRegistry.register(4, new ResourceLocation("mining_fatigue"), (new Potion(true, 4866583)).setPotionName("effect.digSlowDown").setIconIndex(3, 0).registerPotionAttributeModifier(SharedMonsterAttributes.ATTACK_SPEED, "55FCED67-E92A-486E-9800-B47F202C4386", -0.10000000149011612D, 2));
        potionRegistry.register(5, new ResourceLocation("strength"), (new PotionAttackDamage(false, 9643043, 3.0D)).setPotionName("effect.damageBoost").setIconIndex(4, 0).registerPotionAttributeModifier(SharedMonsterAttributes.ATTACK_DAMAGE, "648D7064-6A60-4F59-8ABE-C2C23A6DD7A9", 0.0D, 0).func_188413_j());
        potionRegistry.register(6, new ResourceLocation("instant_health"), (new PotionHealth(false, 16262179)).setPotionName("effect.heal").func_188413_j());
        potionRegistry.register(7, new ResourceLocation("instant_damage"), (new PotionHealth(true, 4393481)).setPotionName("effect.harm").func_188413_j());
        potionRegistry.register(8, new ResourceLocation("jump_boost"), (new Potion(false, 2293580)).setPotionName("effect.jump").setIconIndex(2, 1).func_188413_j());
        potionRegistry.register(9, new ResourceLocation("nausea"), (new Potion(true, 5578058)).setPotionName("effect.confusion").setIconIndex(3, 1).setEffectiveness(0.25D));
        potionRegistry.register(10, new ResourceLocation("regeneration"), (new Potion(false, 13458603)).setPotionName("effect.regeneration").setIconIndex(7, 0).setEffectiveness(0.25D).func_188413_j());
        potionRegistry.register(11, new ResourceLocation("resistance"), (new Potion(false, 10044730)).setPotionName("effect.resistance").setIconIndex(6, 1).func_188413_j());
        potionRegistry.register(12, new ResourceLocation("fire_resistance"), (new Potion(false, 14981690)).setPotionName("effect.fireResistance").setIconIndex(7, 1).func_188413_j());
        potionRegistry.register(13, new ResourceLocation("water_breathing"), (new Potion(false, 3035801)).setPotionName("effect.waterBreathing").setIconIndex(0, 2).func_188413_j());
        potionRegistry.register(14, new ResourceLocation("invisibility"), (new Potion(false, 8356754)).setPotionName("effect.invisibility").setIconIndex(0, 1).func_188413_j());
        potionRegistry.register(15, new ResourceLocation("blindness"), (new Potion(true, 2039587)).setPotionName("effect.blindness").setIconIndex(5, 1).setEffectiveness(0.25D));
        potionRegistry.register(16, new ResourceLocation("night_vision"), (new Potion(false, 2039713)).setPotionName("effect.nightVision").setIconIndex(4, 1).func_188413_j());
        potionRegistry.register(17, new ResourceLocation("hunger"), (new Potion(true, 5797459)).setPotionName("effect.hunger").setIconIndex(1, 1));
        potionRegistry.register(18, new ResourceLocation("weakness"), (new PotionAttackDamage(true, 4738376, -4.0D)).setPotionName("effect.weakness").setIconIndex(5, 0).registerPotionAttributeModifier(SharedMonsterAttributes.ATTACK_DAMAGE, "22653B89-116E-49DC-9B6B-9971489B5BE5", 0.0D, 0));
        potionRegistry.register(19, new ResourceLocation("poison"), (new Potion(true, 5149489)).setPotionName("effect.poison").setIconIndex(6, 0).setEffectiveness(0.25D));
        potionRegistry.register(20, new ResourceLocation("wither"), (new Potion(true, 3484199)).setPotionName("effect.wither").setIconIndex(1, 2).setEffectiveness(0.25D));
        potionRegistry.register(21, new ResourceLocation("health_boost"), (new PotionHealthBoost(false, 16284963)).setPotionName("effect.healthBoost").setIconIndex(7, 2).registerPotionAttributeModifier(SharedMonsterAttributes.MAX_HEALTH, "5D6F0BA2-1186-46AC-B896-C61C5CEE99CC", 4.0D, 0).func_188413_j());
        potionRegistry.register(22, new ResourceLocation("absorption"), (new PotionAbsorption(false, 2445989)).setPotionName("effect.absorption").setIconIndex(2, 2).func_188413_j());
        potionRegistry.register(23, new ResourceLocation("saturation"), (new PotionHealth(false, 16262179)).setPotionName("effect.saturation").func_188413_j());
        potionRegistry.register(24, new ResourceLocation("glowing"), (new Potion(false, 9740385)).setPotionName("effect.glowing").setIconIndex(4, 2));
        potionRegistry.register(25, new ResourceLocation("levitation"), (new Potion(true, 13565951)).setPotionName("effect.levitation").setIconIndex(3, 2));
        potionRegistry.register(26, new ResourceLocation("luck"), (new Potion(false, 3381504)).setPotionName("effect.luck").setIconIndex(5, 2).func_188413_j().registerPotionAttributeModifier(SharedMonsterAttributes.LUCK, "03C3C89D-7037-4B42-869F-B146BCB64D2E", 1.0D, 0));
        potionRegistry.register(27, new ResourceLocation("unluck"), (new Potion(true, 12624973)).setPotionName("effect.unluck").setIconIndex(6, 2).registerPotionAttributeModifier(SharedMonsterAttributes.LUCK, "CC5AF142-2BD2-4215-B636-2605AED11727", -1.0D, 0));
    }
}
