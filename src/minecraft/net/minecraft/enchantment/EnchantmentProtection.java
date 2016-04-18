package net.minecraft.enchantment;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Enchantments;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.util.DamageSource;
import net.minecraft.util.math.MathHelper;

public class EnchantmentProtection extends Enchantment
{
    /**
     * Defines the type of protection of the enchantment, 0 = all, 1 = fire, 2 = fall (feather fall), 3 = explosion and
     * 4 = projectile.
     */
    public final EnchantmentProtection.Type protectionType;

    public EnchantmentProtection(Enchantment.Rarity rarityIn, EnchantmentProtection.Type protectionTypeIn, EntityEquipmentSlot... slots)
    {
        super(rarityIn, EnumEnchantmentType.ARMOR, slots);
        this.protectionType = protectionTypeIn;

        if (protectionTypeIn == EnchantmentProtection.Type.FALL)
        {
            this.type = EnumEnchantmentType.ARMOR_FEET;
        }
    }

    /**
     * Returns the minimal value of enchantability needed on the enchantment level passed.
     */
    public int getMinEnchantability(int enchantmentLevel)
    {
        return this.protectionType.func_185316_b() + (enchantmentLevel - 1) * this.protectionType.func_185315_c();
    }

    /**
     * Returns the maximum value of enchantability nedded on the enchantment level passed.
     */
    public int getMaxEnchantability(int enchantmentLevel)
    {
        return this.getMinEnchantability(enchantmentLevel) + this.protectionType.func_185315_c();
    }

    /**
     * Returns the maximum level that the enchantment can have.
     */
    public int getMaxLevel()
    {
        return 4;
    }

    /**
     * Calculates the damage protection of the enchantment based on level and damage source passed.
     */
    public int calcModifierDamage(int level, DamageSource source)
    {
        return source.canHarmInCreative() ? 0 : (this.protectionType == EnchantmentProtection.Type.ALL ? level : (this.protectionType == EnchantmentProtection.Type.FIRE && source.isFireDamage() ? level * 2 : (this.protectionType == EnchantmentProtection.Type.FALL && source == DamageSource.fall ? level * 3 : (this.protectionType == EnchantmentProtection.Type.EXPLOSION && source.isExplosion() ? level * 2 : (this.protectionType == EnchantmentProtection.Type.PROJECTILE && source.isProjectile() ? level * 2 : 0)))));
    }

    /**
     * Return the name of key in translation table of this enchantment.
     */
    public String getName()
    {
        return "enchantment.protect." + this.protectionType.func_185314_a();
    }

    /**
     * Determines if the enchantment passed can be applyied together with this enchantment.
     */
    public boolean canApplyTogether(Enchantment ench)
    {
        if (ench instanceof EnchantmentProtection)
        {
            EnchantmentProtection enchantmentprotection = (EnchantmentProtection)ench;
            return this.protectionType == enchantmentprotection.protectionType ? false : this.protectionType == EnchantmentProtection.Type.FALL || enchantmentprotection.protectionType == EnchantmentProtection.Type.FALL;
        }
        else
        {
            return super.canApplyTogether(ench);
        }
    }

    /**
     * Gets the amount of ticks an entity should be set fire, adjusted for fire protection.
     */
    public static int getFireTimeForEntity(EntityLivingBase p_92093_0_, int p_92093_1_)
    {
        int i = EnchantmentHelper.func_185284_a(Enchantments.fireProtection, p_92093_0_);

        if (i > 0)
        {
            p_92093_1_ -= MathHelper.floor_float((float)p_92093_1_ * (float)i * 0.15F);
        }

        return p_92093_1_;
    }

    public static double func_92092_a(EntityLivingBase p_92092_0_, double p_92092_1_)
    {
        int i = EnchantmentHelper.func_185284_a(Enchantments.blastProtection, p_92092_0_);

        if (i > 0)
        {
            p_92092_1_ -= (double)MathHelper.floor_double(p_92092_1_ * (double)((float)i * 0.15F));
        }

        return p_92092_1_;
    }

    public static enum Type
    {
        ALL("all", 1, 11, 20),
        FIRE("fire", 10, 8, 12),
        FALL("fall", 5, 6, 10),
        EXPLOSION("explosion", 5, 8, 12),
        PROJECTILE("projectile", 3, 6, 15);

        private final String field_185322_f;
        private final int field_185323_g;
        private final int field_185324_h;
        private final int field_185325_i;

        private Type(String p_i47051_3_, int p_i47051_4_, int p_i47051_5_, int p_i47051_6_)
        {
            this.field_185322_f = p_i47051_3_;
            this.field_185323_g = p_i47051_4_;
            this.field_185324_h = p_i47051_5_;
            this.field_185325_i = p_i47051_6_;
        }

        public String func_185314_a()
        {
            return this.field_185322_f;
        }

        public int func_185316_b()
        {
            return this.field_185323_g;
        }

        public int func_185315_c()
        {
            return this.field_185324_h;
        }
    }
}
