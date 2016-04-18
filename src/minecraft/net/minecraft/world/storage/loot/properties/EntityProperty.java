package net.minecraft.world.storage.loot.properties;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonElement;
import com.google.gson.JsonSerializationContext;
import java.util.Random;
import net.minecraft.entity.Entity;
import net.minecraft.util.ResourceLocation;

public interface EntityProperty
{
    boolean testProperty(Random p_186657_1_, Entity p_186657_2_);

    public abstract static class Serializer<T extends EntityProperty>
    {
        private final ResourceLocation field_186653_a;
        private final Class<T> field_186654_b;

        protected Serializer(ResourceLocation p_i46831_1_, Class<T> p_i46831_2_)
        {
            this.field_186653_a = p_i46831_1_;
            this.field_186654_b = p_i46831_2_;
        }

        public ResourceLocation func_186649_a()
        {
            return this.field_186653_a;
        }

        public Class<T> func_186651_b()
        {
            return this.field_186654_b;
        }

        public abstract JsonElement serialize(T p_186650_1_, JsonSerializationContext p_186650_2_);

        public abstract T deserialize(JsonElement p_186652_1_, JsonDeserializationContext p_186652_2_);
    }
}
