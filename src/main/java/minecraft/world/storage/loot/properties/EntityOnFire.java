package net.minecraft.world.storage.loot.properties;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonElement;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSerializationContext;
import java.util.Random;
import net.minecraft.entity.Entity;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;

public class EntityOnFire implements EntityProperty
{
    private final boolean onFire;

    public EntityOnFire(boolean p_i46613_1_)
    {
        this.onFire = p_i46613_1_;
    }

    public boolean testProperty(Random p_186657_1_, Entity p_186657_2_)
    {
        return p_186657_2_.isBurning() == this.onFire;
    }

    public static class Serializer extends EntityProperty.Serializer<EntityOnFire>
    {
        protected Serializer()
        {
            super(new ResourceLocation("on_fire"), EntityOnFire.class);
        }

        public JsonElement serialize(EntityOnFire p_186650_1_, JsonSerializationContext p_186650_2_)
        {
            return new JsonPrimitive(Boolean.valueOf(p_186650_1_.onFire));
        }

        public EntityOnFire deserialize(JsonElement p_186652_1_, JsonDeserializationContext p_186652_2_)
        {
            return new EntityOnFire(JsonUtils.getBoolean(p_186652_1_, "on_fire"));
        }
    }
}
