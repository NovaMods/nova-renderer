package net.minecraft.potion;

import com.google.common.base.Predicate;
import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.init.Items;
import net.minecraft.init.PotionTypes;
import net.minecraft.item.Item;
import net.minecraft.item.ItemFishFood;
import net.minecraft.item.ItemPotion;
import net.minecraft.item.ItemStack;

public class PotionHelper
{
    private static final List<PotionHelper.MixPredicate<PotionType>> field_185213_a = Lists.<PotionHelper.MixPredicate<PotionType>>newArrayList();
    private static final List<PotionHelper.MixPredicate<Item>> field_185214_b = Lists.<PotionHelper.MixPredicate<Item>>newArrayList();
    private static final List<PotionHelper.ItemPredicateInstance> field_185215_c = Lists.<PotionHelper.ItemPredicateInstance>newArrayList();
    private static final Predicate<ItemStack> field_185216_d = new Predicate<ItemStack>()
    {
        public boolean apply(ItemStack p_apply_1_)
        {
            for (PotionHelper.ItemPredicateInstance potionhelper$itempredicateinstance : PotionHelper.field_185215_c)
            {
                if (potionhelper$itempredicateinstance.apply(p_apply_1_))
                {
                    return true;
                }
            }

            return false;
        }
    };

    public static boolean func_185205_a(ItemStack p_185205_0_)
    {
        return func_185203_b(p_185205_0_) || func_185211_c(p_185205_0_);
    }

    protected static boolean func_185203_b(ItemStack p_185203_0_)
    {
        int i = 0;

        for (int j = field_185214_b.size(); i < j; ++i)
        {
            if (((PotionHelper.MixPredicate)field_185214_b.get(i)).field_185199_b.apply(p_185203_0_))
            {
                return true;
            }
        }

        return false;
    }

    protected static boolean func_185211_c(ItemStack p_185211_0_)
    {
        int i = 0;

        for (int j = field_185213_a.size(); i < j; ++i)
        {
            if (((PotionHelper.MixPredicate)field_185213_a.get(i)).field_185199_b.apply(p_185211_0_))
            {
                return true;
            }
        }

        return false;
    }

    public static boolean func_185208_a(ItemStack p_185208_0_, ItemStack p_185208_1_)
    {
        return !field_185216_d.apply(p_185208_0_) ? false : func_185206_b(p_185208_0_, p_185208_1_) || func_185209_c(p_185208_0_, p_185208_1_);
    }

    protected static boolean func_185206_b(ItemStack p_185206_0_, ItemStack p_185206_1_)
    {
        Item item = p_185206_0_.getItem();
        int i = 0;

        for (int j = field_185214_b.size(); i < j; ++i)
        {
            PotionHelper.MixPredicate<Item> mixpredicate = (PotionHelper.MixPredicate)field_185214_b.get(i);

            if (mixpredicate.field_185198_a == item && mixpredicate.field_185199_b.apply(p_185206_1_))
            {
                return true;
            }
        }

        return false;
    }

    protected static boolean func_185209_c(ItemStack p_185209_0_, ItemStack p_185209_1_)
    {
        PotionType potiontype = PotionUtils.func_185191_c(p_185209_0_);
        int i = 0;

        for (int j = field_185213_a.size(); i < j; ++i)
        {
            PotionHelper.MixPredicate<PotionType> mixpredicate = (PotionHelper.MixPredicate)field_185213_a.get(i);

            if (mixpredicate.field_185198_a == potiontype && mixpredicate.field_185199_b.apply(p_185209_1_))
            {
                return true;
            }
        }

        return false;
    }

    public static ItemStack func_185212_d(ItemStack p_185212_0_, ItemStack p_185212_1_)
    {
        if (p_185212_1_ != null)
        {
            PotionType potiontype = PotionUtils.func_185191_c(p_185212_1_);
            Item item = p_185212_1_.getItem();
            int i = 0;

            for (int j = field_185214_b.size(); i < j; ++i)
            {
                PotionHelper.MixPredicate<Item> mixpredicate = (PotionHelper.MixPredicate)field_185214_b.get(i);

                if (mixpredicate.field_185198_a == item && mixpredicate.field_185199_b.apply(p_185212_0_))
                {
                    return PotionUtils.func_185188_a(new ItemStack((Item)mixpredicate.field_185200_c), potiontype);
                }
            }

            i = 0;

            for (int k = field_185213_a.size(); i < k; ++i)
            {
                PotionHelper.MixPredicate<PotionType> mixpredicate1 = (PotionHelper.MixPredicate)field_185213_a.get(i);

                if (mixpredicate1.field_185198_a == potiontype && mixpredicate1.field_185199_b.apply(p_185212_0_))
                {
                    return PotionUtils.func_185188_a(new ItemStack(item), (PotionType)mixpredicate1.field_185200_c);
                }
            }
        }

        return p_185212_1_;
    }

    public static void func_185207_a()
    {
        Predicate<ItemStack> predicate = new PotionHelper.ItemPredicateInstance(Items.nether_wart);
        Predicate<ItemStack> predicate1 = new PotionHelper.ItemPredicateInstance(Items.golden_carrot);
        Predicate<ItemStack> predicate2 = new PotionHelper.ItemPredicateInstance(Items.redstone);
        Predicate<ItemStack> predicate3 = new PotionHelper.ItemPredicateInstance(Items.fermented_spider_eye);
        Predicate<ItemStack> predicate4 = new PotionHelper.ItemPredicateInstance(Items.rabbit_foot);
        Predicate<ItemStack> predicate5 = new PotionHelper.ItemPredicateInstance(Items.glowstone_dust);
        Predicate<ItemStack> predicate6 = new PotionHelper.ItemPredicateInstance(Items.magma_cream);
        Predicate<ItemStack> predicate7 = new PotionHelper.ItemPredicateInstance(Items.sugar);
        Predicate<ItemStack> predicate8 = new PotionHelper.ItemPredicateInstance(Items.fish, ItemFishFood.FishType.PUFFERFISH.getMetadata());
        Predicate<ItemStack> predicate9 = new PotionHelper.ItemPredicateInstance(Items.speckled_melon);
        Predicate<ItemStack> predicate10 = new PotionHelper.ItemPredicateInstance(Items.spider_eye);
        Predicate<ItemStack> predicate11 = new PotionHelper.ItemPredicateInstance(Items.ghast_tear);
        Predicate<ItemStack> predicate12 = new PotionHelper.ItemPredicateInstance(Items.blaze_powder);
        func_185202_a(new PotionHelper.ItemPredicateInstance(Items.potionitem));
        func_185202_a(new PotionHelper.ItemPredicateInstance(Items.splash_potion));
        func_185202_a(new PotionHelper.ItemPredicateInstance(Items.lingering_potion));
        func_185201_a(Items.potionitem, new PotionHelper.ItemPredicateInstance(Items.gunpowder), Items.splash_potion);
        func_185201_a(Items.splash_potion, new PotionHelper.ItemPredicateInstance(Items.dragon_breath), Items.lingering_potion);
        func_185204_a(PotionTypes.water, predicate9, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate11, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate4, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate12, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate10, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate7, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate6, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate5, PotionTypes.thick);
        func_185204_a(PotionTypes.water, predicate2, PotionTypes.mundane);
        func_185204_a(PotionTypes.water, predicate, PotionTypes.awkward);
        func_185204_a(PotionTypes.awkward, predicate1, PotionTypes.night_vision);
        func_185204_a(PotionTypes.night_vision, predicate2, PotionTypes.long_night_vision);
        func_185204_a(PotionTypes.night_vision, predicate3, PotionTypes.invisibility);
        func_185204_a(PotionTypes.long_night_vision, predicate3, PotionTypes.long_invisibility);
        func_185204_a(PotionTypes.invisibility, predicate2, PotionTypes.long_invisibility);
        func_185204_a(PotionTypes.awkward, predicate6, PotionTypes.fire_resistance);
        func_185204_a(PotionTypes.fire_resistance, predicate2, PotionTypes.long_fire_resistance);
        func_185204_a(PotionTypes.awkward, predicate4, PotionTypes.leaping);
        func_185204_a(PotionTypes.leaping, predicate2, PotionTypes.long_leaping);
        func_185204_a(PotionTypes.leaping, predicate5, PotionTypes.strong_leaping);
        func_185204_a(PotionTypes.leaping, predicate3, PotionTypes.slowness);
        func_185204_a(PotionTypes.long_leaping, predicate3, PotionTypes.long_slowness);
        func_185204_a(PotionTypes.slowness, predicate2, PotionTypes.long_slowness);
        func_185204_a(PotionTypes.swiftness, predicate3, PotionTypes.slowness);
        func_185204_a(PotionTypes.long_swiftness, predicate3, PotionTypes.long_slowness);
        func_185204_a(PotionTypes.awkward, predicate7, PotionTypes.swiftness);
        func_185204_a(PotionTypes.swiftness, predicate2, PotionTypes.long_swiftness);
        func_185204_a(PotionTypes.swiftness, predicate5, PotionTypes.strong_swiftness);
        func_185204_a(PotionTypes.awkward, predicate8, PotionTypes.water_breathing);
        func_185204_a(PotionTypes.water_breathing, predicate2, PotionTypes.long_water_breathing);
        func_185204_a(PotionTypes.awkward, predicate9, PotionTypes.healing);
        func_185204_a(PotionTypes.healing, predicate5, PotionTypes.strong_healing);
        func_185204_a(PotionTypes.healing, predicate3, PotionTypes.harming);
        func_185204_a(PotionTypes.strong_healing, predicate3, PotionTypes.strong_harming);
        func_185204_a(PotionTypes.harming, predicate5, PotionTypes.strong_harming);
        func_185204_a(PotionTypes.poison, predicate3, PotionTypes.harming);
        func_185204_a(PotionTypes.long_poison, predicate3, PotionTypes.harming);
        func_185204_a(PotionTypes.strong_poison, predicate3, PotionTypes.strong_harming);
        func_185204_a(PotionTypes.awkward, predicate10, PotionTypes.poison);
        func_185204_a(PotionTypes.poison, predicate2, PotionTypes.long_poison);
        func_185204_a(PotionTypes.poison, predicate5, PotionTypes.strong_poison);
        func_185204_a(PotionTypes.awkward, predicate11, PotionTypes.regeneration);
        func_185204_a(PotionTypes.regeneration, predicate2, PotionTypes.long_regeneration);
        func_185204_a(PotionTypes.regeneration, predicate5, PotionTypes.strong_regeneration);
        func_185204_a(PotionTypes.awkward, predicate12, PotionTypes.strength);
        func_185204_a(PotionTypes.strength, predicate2, PotionTypes.long_strength);
        func_185204_a(PotionTypes.strength, predicate5, PotionTypes.strong_strength);
        func_185204_a(PotionTypes.water, predicate3, PotionTypes.weakness);
        func_185204_a(PotionTypes.weakness, predicate2, PotionTypes.long_weakness);
    }

    private static void func_185201_a(ItemPotion p_185201_0_, PotionHelper.ItemPredicateInstance p_185201_1_, ItemPotion p_185201_2_)
    {
        field_185214_b.add(new PotionHelper.MixPredicate(p_185201_0_, p_185201_1_, p_185201_2_));
    }

    private static void func_185202_a(PotionHelper.ItemPredicateInstance p_185202_0_)
    {
        field_185215_c.add(p_185202_0_);
    }

    private static void func_185204_a(PotionType p_185204_0_, Predicate<ItemStack> p_185204_1_, PotionType p_185204_2_)
    {
        field_185213_a.add(new PotionHelper.MixPredicate(p_185204_0_, p_185204_1_, p_185204_2_));
    }

    static class ItemPredicateInstance implements Predicate<ItemStack>
    {
        private final Item field_185195_a;
        private final int field_185196_b;

        public ItemPredicateInstance(Item p_i47013_1_)
        {
            this(p_i47013_1_, -1);
        }

        public ItemPredicateInstance(Item p_i47014_1_, int p_i47014_2_)
        {
            this.field_185195_a = p_i47014_1_;
            this.field_185196_b = p_i47014_2_;
        }

        public boolean apply(ItemStack p_apply_1_)
        {
            return p_apply_1_ != null && p_apply_1_.getItem() == this.field_185195_a && (this.field_185196_b == -1 || this.field_185196_b == p_apply_1_.getMetadata());
        }
    }

    static class MixPredicate<T>
    {
        final T field_185198_a;
        final Predicate<ItemStack> field_185199_b;
        final T field_185200_c;

        public MixPredicate(T p_i47012_1_, Predicate<ItemStack> p_i47012_2_, T p_i47012_3_)
        {
            this.field_185198_a = p_i47012_1_;
            this.field_185199_b = p_i47012_2_;
            this.field_185200_c = p_i47012_3_;
        }
    }
}
