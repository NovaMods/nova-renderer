package net.minecraft.enchantment;

import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.EnumCreatureAttribute;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemStack;
import net.minecraft.util.DamageSource;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.registry.RegistryNamespaced;
import net.minecraft.util.text.translation.I18n;

public abstract class Enchantment
{
    public static final RegistryNamespaced<ResourceLocation, Enchantment> enchantmentRegistry = new RegistryNamespaced();

    /** Where this enchantment has an effect, e.g. offhand, pants */
    private final EntityEquipmentSlot[] applicableEquipmentTypes;
    private final Enchantment.Rarity weight;

    /** The EnumEnchantmentType given to this Enchantment. */
    public EnumEnchantmentType type;

    /** Used in localisation and stats. */
    protected String name;

    /**
     * Gets an Enchantment from the registry, based on a numeric ID.
     */
    public static Enchantment getEnchantmentByID(int id)
    {
        return (Enchantment)enchantmentRegistry.getObjectById(id);
    }

    /**
     * Gets the numeric ID for the passed enchantment.
     */
    public static int getEnchantmentID(Enchantment enchantmentIn)
    {
        return enchantmentRegistry.getIDForObject(enchantmentIn);
    }

    /**
     * Retrieves an enchantment by using its location name.
     */
    public static Enchantment getEnchantmentByLocation(String location)
    {
        return (Enchantment)enchantmentRegistry.getObject(new ResourceLocation(location));
    }

    protected Enchantment(Enchantment.Rarity rarityIn, EnumEnchantmentType typeIn, EntityEquipmentSlot[] slots)
    {
        this.weight = rarityIn;
        this.type = typeIn;
        this.applicableEquipmentTypes = slots;
    }

    public Iterable<ItemStack> getEntityEquipment(EntityLivingBase entityIn)
    {
        List<ItemStack> list = Lists.<ItemStack>newArrayList();

        for (EntityEquipmentSlot entityequipmentslot : this.applicableEquipmentTypes)
        {
            ItemStack itemstack = entityIn.getItemStackFromSlot(entityequipmentslot);

            if (itemstack != null)
            {
                list.add(itemstack);
            }
        }

        return list.size() > 0 ? list : null;
    }

    /**
     * Retrieves the weight value of an Enchantment. This weight value is used within vanilla to determine how rare an
     * enchantment is.
     */
    public Enchantment.Rarity getWeight()
    {
        return this.weight;
    }

    /**
     * Returns the minimum level that the enchantment can have.
     */
    public int getMinLevel()
    {
        return 1;
    }

    /**
     * Returns the maximum level that the enchantment can have.
     */
    public int getMaxLevel()
    {
        return 1;
    }

    /**
     * Returns the minimal value of enchantability needed on the enchantment level passed.
     */
    public int getMinEnchantability(int enchantmentLevel)
    {
        return 1 + enchantmentLevel * 10;
    }

    /**
     * Returns the maximum value of enchantability nedded on the enchantment level passed.
     */
    public int getMaxEnchantability(int enchantmentLevel)
    {
        return this.getMinEnchantability(enchantmentLevel) + 5;
    }

    /**
     * Calculates the damage protection of the enchantment based on level and damage source passed.
     */
    public int calcModifierDamage(int level, DamageSource source)
    {
        return 0;
    }

    /**
     * Calculates the additional damage that will be dealt by an item with this enchantment. This alternative to
     * calcModifierDamage is sensitive to the targets EnumCreatureAttribute.
     */
    public float calcDamageByCreature(int level, EnumCreatureAttribute creatureType)
    {
        return 0.0F;
    }

    /**
     * Determines if the enchantment passed can be applyied together with this enchantment.
     */
    public boolean canApplyTogether(Enchantment ench)
    {
        return this != ench;
    }

    /**
     * Sets the enchantment name
     */
    public Enchantment setName(String enchName)
    {
        this.name = enchName;
        return this;
    }

    /**
     * Return the name of key in translation table of this enchantment.
     */
    public String getName()
    {
        return "enchantment." + this.name;
    }

    /**
     * Returns the correct traslated name of the enchantment and the level in roman numbers.
     */
    public String getTranslatedName(int level)
    {
        String s = I18n.translateToLocal(this.getName());
        return level == 1 && this.getMaxLevel() == 1 ? s : s + " " + I18n.translateToLocal("enchantment.level." + level);
    }

    /**
     * Determines if this enchantment can be applied to a specific ItemStack.
     */
    public boolean canApply(ItemStack stack)
    {
        return this.type.canEnchantItem(stack.getItem());
    }

    /**
     * Called whenever a mob is damaged with an item that has this enchantment on it.
     */
    public void onEntityDamaged(EntityLivingBase user, Entity target, int level)
    {
    }

    /**
     * Whenever an entity that has this enchantment on one of its associated items is damaged this method will be
     * called.
     */
    public void onUserHurt(EntityLivingBase user, Entity attacker, int level)
    {
    }

    public boolean isTreasureEnchantment()
    {
        return false;
    }

    /**
     * Registers all of the vanilla enchantments.
     */
    public static void registerEnchantments()
    {
        EntityEquipmentSlot[] aentityequipmentslot = new EntityEquipmentSlot[] {EntityEquipmentSlot.HEAD, EntityEquipmentSlot.CHEST, EntityEquipmentSlot.LEGS, EntityEquipmentSlot.FEET};
        enchantmentRegistry.register(0, new ResourceLocation("protection"), new EnchantmentProtection(Enchantment.Rarity.COMMON, EnchantmentProtection.Type.ALL, aentityequipmentslot));
        enchantmentRegistry.register(1, new ResourceLocation("fire_protection"), new EnchantmentProtection(Enchantment.Rarity.UNCOMMON, EnchantmentProtection.Type.FIRE, aentityequipmentslot));
        enchantmentRegistry.register(2, new ResourceLocation("feather_falling"), new EnchantmentProtection(Enchantment.Rarity.UNCOMMON, EnchantmentProtection.Type.FALL, aentityequipmentslot));
        enchantmentRegistry.register(3, new ResourceLocation("blast_protection"), new EnchantmentProtection(Enchantment.Rarity.RARE, EnchantmentProtection.Type.EXPLOSION, aentityequipmentslot));
        enchantmentRegistry.register(4, new ResourceLocation("projectile_protection"), new EnchantmentProtection(Enchantment.Rarity.UNCOMMON, EnchantmentProtection.Type.PROJECTILE, aentityequipmentslot));
        enchantmentRegistry.register(5, new ResourceLocation("respiration"), new EnchantmentOxygen(Enchantment.Rarity.RARE, aentityequipmentslot));
        enchantmentRegistry.register(6, new ResourceLocation("aqua_affinity"), new EnchantmentWaterWorker(Enchantment.Rarity.RARE, aentityequipmentslot));
        enchantmentRegistry.register(7, new ResourceLocation("thorns"), new EnchantmentThorns(Enchantment.Rarity.VERY_RARE, aentityequipmentslot));
        enchantmentRegistry.register(8, new ResourceLocation("depth_strider"), new EnchantmentWaterWalker(Enchantment.Rarity.RARE, aentityequipmentslot));
        enchantmentRegistry.register(9, new ResourceLocation("frost_walker"), new EnchantmentFrostWalker(Enchantment.Rarity.RARE, new EntityEquipmentSlot[] {EntityEquipmentSlot.FEET}));
        enchantmentRegistry.register(16, new ResourceLocation("sharpness"), new EnchantmentDamage(Enchantment.Rarity.COMMON, 0, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(17, new ResourceLocation("smite"), new EnchantmentDamage(Enchantment.Rarity.UNCOMMON, 1, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(18, new ResourceLocation("bane_of_arthropods"), new EnchantmentDamage(Enchantment.Rarity.UNCOMMON, 2, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(19, new ResourceLocation("knockback"), new EnchantmentKnockback(Enchantment.Rarity.UNCOMMON, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(20, new ResourceLocation("fire_aspect"), new EnchantmentFireAspect(Enchantment.Rarity.RARE, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(21, new ResourceLocation("looting"), new EnchantmentLootBonus(Enchantment.Rarity.RARE, EnumEnchantmentType.WEAPON, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(32, new ResourceLocation("efficiency"), new EnchantmentDigging(Enchantment.Rarity.COMMON, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(33, new ResourceLocation("silk_touch"), new EnchantmentUntouching(Enchantment.Rarity.VERY_RARE, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(34, new ResourceLocation("unbreaking"), new EnchantmentDurability(Enchantment.Rarity.UNCOMMON, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(35, new ResourceLocation("fortune"), new EnchantmentLootBonus(Enchantment.Rarity.RARE, EnumEnchantmentType.DIGGER, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(48, new ResourceLocation("power"), new EnchantmentArrowDamage(Enchantment.Rarity.COMMON, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(49, new ResourceLocation("punch"), new EnchantmentArrowKnockback(Enchantment.Rarity.RARE, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(50, new ResourceLocation("flame"), new EnchantmentArrowFire(Enchantment.Rarity.RARE, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(51, new ResourceLocation("infinity"), new EnchantmentArrowInfinite(Enchantment.Rarity.VERY_RARE, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(61, new ResourceLocation("luck_of_the_sea"), new EnchantmentLootBonus(Enchantment.Rarity.RARE, EnumEnchantmentType.FISHING_ROD, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(62, new ResourceLocation("lure"), new EnchantmentFishingSpeed(Enchantment.Rarity.RARE, EnumEnchantmentType.FISHING_ROD, new EntityEquipmentSlot[] {EntityEquipmentSlot.MAINHAND}));
        enchantmentRegistry.register(70, new ResourceLocation("mending"), new EnchantmentMending(Enchantment.Rarity.RARE, EntityEquipmentSlot.values()));
    }

    public static enum Rarity
    {
        COMMON(10),
        UNCOMMON(5),
        RARE(2),
        VERY_RARE(1);

        private final int weight;

        private Rarity(int rarityWeight)
        {
            this.weight = rarityWeight;
        }

        public int getWeight()
        {
            return this.weight;
        }
    }
}
