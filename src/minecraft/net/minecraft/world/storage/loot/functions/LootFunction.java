package net.minecraft.world.storage.loot.functions;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Random;
import net.minecraft.item.ItemStack;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.conditions.LootCondition;

public abstract class LootFunction
{
    private final LootCondition[] conditions;

    protected LootFunction(LootCondition[] p_i46626_1_)
    {
        this.conditions = p_i46626_1_;
    }

    public abstract ItemStack apply(ItemStack p_186553_1_, Random p_186553_2_, LootContext p_186553_3_);

    public LootCondition[] getConditions()
    {
        return this.conditions;
    }

    public abstract static class Serializer<T extends LootFunction>
    {
        private final ResourceLocation field_186533_a;
        private final Class<T> field_186534_b;

        protected Serializer(ResourceLocation p_i47002_1_, Class<T> p_i47002_2_)
        {
            this.field_186533_a = p_i47002_1_;
            this.field_186534_b = p_i47002_2_;
        }

        public ResourceLocation func_186529_a()
        {
            return this.field_186533_a;
        }

        public Class<T> func_186531_b()
        {
            return this.field_186534_b;
        }

        public abstract void func_186532_a(JsonObject p_186532_1_, T p_186532_2_, JsonSerializationContext p_186532_3_);

        public abstract T func_186530_b(JsonObject p_186530_1_, JsonDeserializationContext p_186530_2_, LootCondition[] p_186530_3_);
    }
}
