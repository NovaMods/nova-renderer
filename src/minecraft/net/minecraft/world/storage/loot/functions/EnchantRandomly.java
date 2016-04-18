package net.minecraft.world.storage.loot.functions;

import com.google.common.collect.Lists;
import com.google.gson.JsonArray;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSyntaxException;
import java.util.List;
import java.util.Random;
import net.minecraft.enchantment.Enchantment;
import net.minecraft.enchantment.EnchantmentData;
import net.minecraft.init.Items;
import net.minecraft.item.ItemStack;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.conditions.LootCondition;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class EnchantRandomly extends LootFunction
{
    private static final Logger field_186557_a = LogManager.getLogger();
    private final List<Enchantment> field_186558_b;

    public EnchantRandomly(LootCondition[] p_i46628_1_, List<Enchantment> p_i46628_2_)
    {
        super(p_i46628_1_);
        this.field_186558_b = p_i46628_2_;
    }

    public ItemStack apply(ItemStack p_186553_1_, Random p_186553_2_, LootContext p_186553_3_)
    {
        Enchantment enchantment;

        if (this.field_186558_b != null && !this.field_186558_b.isEmpty())
        {
            enchantment = (Enchantment)this.field_186558_b.get(p_186553_2_.nextInt(this.field_186558_b.size()));
        }
        else
        {
            List<Enchantment> list = Lists.<Enchantment>newArrayList();

            for (Enchantment enchantment1 : Enchantment.enchantmentRegistry)
            {
                if (p_186553_1_.getItem() == Items.book || enchantment1.canApply(p_186553_1_))
                {
                    list.add(enchantment1);
                }
            }

            if (list.isEmpty())
            {
                field_186557_a.warn("Couldn\'t find a compatible enchantment for " + p_186553_1_);
                return p_186553_1_;
            }

            enchantment = (Enchantment)list.get(p_186553_2_.nextInt(list.size()));
        }

        int i = MathHelper.getRandomIntegerInRange(p_186553_2_, enchantment.getMinLevel(), enchantment.getMaxLevel());

        if (p_186553_1_.getItem() == Items.book)
        {
            p_186553_1_.setItem(Items.enchanted_book);
            Items.enchanted_book.addEnchantment(p_186553_1_, new EnchantmentData(enchantment, i));
        }
        else
        {
            p_186553_1_.addEnchantment(enchantment, i);
        }

        return p_186553_1_;
    }

    public static class Serializer extends LootFunction.Serializer<EnchantRandomly>
    {
        public Serializer()
        {
            super(new ResourceLocation("enchant_randomly"), EnchantRandomly.class);
        }

        public void func_186532_a(JsonObject p_186532_1_, EnchantRandomly p_186532_2_, JsonSerializationContext p_186532_3_)
        {
            if (p_186532_2_.field_186558_b != null && !p_186532_2_.field_186558_b.isEmpty())
            {
                JsonArray jsonarray = new JsonArray();

                for (Enchantment enchantment : p_186532_2_.field_186558_b)
                {
                    ResourceLocation resourcelocation = (ResourceLocation)Enchantment.enchantmentRegistry.getNameForObject(enchantment);

                    if (resourcelocation == null)
                    {
                        throw new IllegalArgumentException("Don\'t know how to serialize enchantment " + enchantment);
                    }

                    jsonarray.add(new JsonPrimitive(resourcelocation.toString()));
                }

                p_186532_1_.add("enchantments", jsonarray);
            }
        }

        public EnchantRandomly func_186530_b(JsonObject p_186530_1_, JsonDeserializationContext p_186530_2_, LootCondition[] p_186530_3_)
        {
            List<Enchantment> list = null;

            if (p_186530_1_.has("enchantments"))
            {
                list = Lists.<Enchantment>newArrayList();

                for (JsonElement jsonelement : JsonUtils.getJsonArray(p_186530_1_, "enchantments"))
                {
                    String s = JsonUtils.getString(jsonelement, "enchantment");
                    Enchantment enchantment = (Enchantment)Enchantment.enchantmentRegistry.getObject(new ResourceLocation(s));

                    if (enchantment == null)
                    {
                        throw new JsonSyntaxException("Unknown enchantment \'" + s + "\'");
                    }

                    list.add(enchantment);
                }
            }

            return new EnchantRandomly(p_186530_3_, list);
        }
    }
}
