package net.minecraft.world.storage.loot.conditions;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Random;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.LootContext;

public interface LootCondition
{
    boolean testCondition(Random rand, LootContext context);

    public abstract static class Serializer<T extends LootCondition>
    {
        private final ResourceLocation field_186606_a;
        private final Class<T> field_186607_b;

        protected Serializer(ResourceLocation p_i47021_1_, Class<T> p_i47021_2_)
        {
            this.field_186606_a = p_i47021_1_;
            this.field_186607_b = p_i47021_2_;
        }

        public ResourceLocation func_186602_a()
        {
            return this.field_186606_a;
        }

        public Class<T> func_186604_b()
        {
            return this.field_186607_b;
        }

        public abstract void serialize(JsonObject json, T value, JsonSerializationContext context);

        public abstract T deserialize(JsonObject json, JsonDeserializationContext context);
    }
}
