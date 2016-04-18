package net.minecraft.world.storage.loot.functions;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Random;
import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.item.ItemStack;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.RandomValueRange;
import net.minecraft.world.storage.loot.conditions.LootCondition;

public class LootingEnchantBonus extends LootFunction
{
    private final RandomValueRange field_186563_a;

    public LootingEnchantBonus(LootCondition[] p_i46625_1_, RandomValueRange p_i46625_2_)
    {
        super(p_i46625_1_);
        this.field_186563_a = p_i46625_2_;
    }

    public ItemStack apply(ItemStack p_186553_1_, Random p_186553_2_, LootContext p_186553_3_)
    {
        Entity entity = p_186553_3_.getKiller();

        if (entity instanceof EntityLivingBase)
        {
            int i = EnchantmentHelper.func_185283_h((EntityLivingBase)entity);

            if (i == 0)
            {
                return p_186553_1_;
            }

            float f = (float)i * this.field_186563_a.generateFloat(p_186553_2_);
            p_186553_1_.stackSize += Math.round(f);
        }

        return p_186553_1_;
    }

    public static class Serializer extends LootFunction.Serializer<LootingEnchantBonus>
    {
        protected Serializer()
        {
            super(new ResourceLocation("looting_enchant"), LootingEnchantBonus.class);
        }

        public void func_186532_a(JsonObject p_186532_1_, LootingEnchantBonus p_186532_2_, JsonSerializationContext p_186532_3_)
        {
            p_186532_1_.add("count", p_186532_3_.serialize(p_186532_2_.field_186563_a));
        }

        public LootingEnchantBonus func_186530_b(JsonObject p_186530_1_, JsonDeserializationContext p_186530_2_, LootCondition[] p_186530_3_)
        {
            return new LootingEnchantBonus(p_186530_3_, (RandomValueRange)JsonUtils.func_188174_a(p_186530_1_, "count", p_186530_2_, RandomValueRange.class));
        }
    }
}
