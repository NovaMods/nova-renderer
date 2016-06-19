package net.minecraft.world.storage.loot.conditions;

import com.google.common.collect.Maps;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParseException;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.google.gson.JsonSyntaxException;
import java.lang.reflect.Type;
import java.util.Map;
import java.util.Random;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.LootContext;

public class LootConditionManager
{
    private static final Map < ResourceLocation, LootCondition.Serializer<? >> NAME_TO_SERIALIZER_MAP = Maps. < ResourceLocation, LootCondition.Serializer<? >> newHashMap();
    private static final Map < Class <? extends LootCondition > , LootCondition.Serializer<? >> CLASS_TO_SERIALIZER_MAP = Maps. < Class <? extends LootCondition > , LootCondition.Serializer<? >> newHashMap();

    public static <T extends LootCondition> void registerCondition(LootCondition.Serializer <? extends T > p_186639_0_)
    {
        ResourceLocation resourcelocation = p_186639_0_.func_186602_a();
        Class<T> oclass = (Class<T>)p_186639_0_.func_186604_b();

        if (NAME_TO_SERIALIZER_MAP.containsKey(resourcelocation))
        {
            throw new IllegalArgumentException("Can\'t re-register item condition name " + resourcelocation);
        }
        else if (CLASS_TO_SERIALIZER_MAP.containsKey(oclass))
        {
            throw new IllegalArgumentException("Can\'t re-register item condition class " + oclass.getName());
        }
        else
        {
            NAME_TO_SERIALIZER_MAP.put(resourcelocation, p_186639_0_);
            CLASS_TO_SERIALIZER_MAP.put(oclass, p_186639_0_);
        }
    }

    public static boolean testAllConditions(LootCondition[] p_186638_0_, Random p_186638_1_, LootContext p_186638_2_)
    {
        if (p_186638_0_ == null)
        {
            return true;
        }
        else
        {
            int i = 0;

            for (int j = p_186638_0_.length; i < j; ++i)
            {
                LootCondition lootcondition = p_186638_0_[i];

                if (!lootcondition.testCondition(p_186638_1_, p_186638_2_))
                {
                    return false;
                }
            }

            return true;
        }
    }

    public static LootCondition.Serializer<?> getSerializerForName(ResourceLocation p_186641_0_)
    {
        LootCondition.Serializer<?> serializer = (LootCondition.Serializer)NAME_TO_SERIALIZER_MAP.get(p_186641_0_);

        if (serializer == null)
        {
            throw new IllegalArgumentException("Unknown loot item condition \'" + p_186641_0_ + "\'");
        }
        else
        {
            return serializer;
        }
    }

    public static <T extends LootCondition> LootCondition.Serializer<T> getSerializerFor(T p_186640_0_)
    {
        LootCondition.Serializer<T> serializer = (LootCondition.Serializer)CLASS_TO_SERIALIZER_MAP.get(p_186640_0_.getClass());

        if (serializer == null)
        {
            throw new IllegalArgumentException("Unknown loot item condition " + p_186640_0_);
        }
        else
        {
            return serializer;
        }
    }

    static
    {
        registerCondition(new RandomChance.Serializer());
        registerCondition(new RandomChanceWithLooting.Serializer());
        registerCondition(new EntityHasProperty.Serializer());
        registerCondition(new KilledByPlayer.Serializer());
        registerCondition(new EntityHasScore.Serializer());
    }

    public static class Serializer implements JsonDeserializer<LootCondition>, JsonSerializer<LootCondition>
    {
        public LootCondition deserialize(JsonElement p_deserialize_1_, Type p_deserialize_2_, JsonDeserializationContext p_deserialize_3_) throws JsonParseException
        {
            JsonObject jsonobject = JsonUtils.getJsonObject(p_deserialize_1_, "condition");
            ResourceLocation resourcelocation = new ResourceLocation(JsonUtils.getString(jsonobject, "condition"));
            LootCondition.Serializer<?> serializer;

            try
            {
                serializer = LootConditionManager.getSerializerForName(resourcelocation);
            }
            catch (IllegalArgumentException var8)
            {
                throw new JsonSyntaxException("Unknown condition \'" + resourcelocation + "\'");
            }

            return serializer.deserialize(jsonobject, p_deserialize_3_);
        }

        public JsonElement serialize(LootCondition p_serialize_1_, Type p_serialize_2_, JsonSerializationContext p_serialize_3_)
        {
            LootCondition.Serializer<LootCondition> serializer = LootConditionManager.<LootCondition>getSerializerFor(p_serialize_1_);
            JsonObject jsonobject = new JsonObject();
            serializer.serialize(jsonobject, p_serialize_1_, p_serialize_3_);
            jsonobject.addProperty("condition", serializer.func_186602_a().toString());
            return jsonobject;
        }
    }
}
