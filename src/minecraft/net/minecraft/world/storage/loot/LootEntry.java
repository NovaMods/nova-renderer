package net.minecraft.world.storage.loot;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParseException;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.google.gson.JsonSyntaxException;
import java.lang.reflect.Type;
import java.util.Collection;
import java.util.Random;
import net.minecraft.item.ItemStack;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.storage.loot.conditions.LootCondition;

public abstract class LootEntry
{
    protected final int weight;
    protected final int quality;
    protected final LootCondition[] conditions;

    protected LootEntry(int weightIn, int qualityIn, LootCondition[] conditionsIn)
    {
        this.weight = weightIn;
        this.quality = qualityIn;
        this.conditions = conditionsIn;
    }

    public int func_186361_a(float p_186361_1_)
    {
        return Math.max(MathHelper.floor_float((float)this.weight + (float)this.quality * p_186361_1_), 0);
    }

    public abstract void addLoot(Collection<ItemStack> p_186363_1_, Random rand, LootContext context);

    protected abstract void serialize(JsonObject json, JsonSerializationContext context);

    public static class Serializer implements JsonDeserializer<LootEntry>, JsonSerializer<LootEntry>
    {
        public LootEntry deserialize(JsonElement p_deserialize_1_, Type p_deserialize_2_, JsonDeserializationContext p_deserialize_3_) throws JsonParseException
        {
            JsonObject jsonobject = JsonUtils.getJsonObject(p_deserialize_1_, "loot item");
            String s = JsonUtils.getString(jsonobject, "type");
            int i = JsonUtils.getInt(jsonobject, "weight", 1);
            int j = JsonUtils.getInt(jsonobject, "quality", 0);
            LootCondition[] alootcondition;

            if (jsonobject.has("conditions"))
            {
                alootcondition = (LootCondition[])JsonUtils.func_188174_a(jsonobject, "conditions", p_deserialize_3_, LootCondition[].class);
            }
            else
            {
                alootcondition = new LootCondition[0];
            }

            if (s.equals("item"))
            {
                return LootEntryItem.func_186367_a(jsonobject, p_deserialize_3_, i, j, alootcondition);
            }
            else if (s.equals("loot_table"))
            {
                return LootEntryTable.func_186370_a(jsonobject, p_deserialize_3_, i, j, alootcondition);
            }
            else if (s.equals("empty"))
            {
                return LootEntryEmpty.func_186372_a(jsonobject, p_deserialize_3_, i, j, alootcondition);
            }
            else
            {
                throw new JsonSyntaxException("Unknown loot entry type \'" + s + "\'");
            }
        }

        public JsonElement serialize(LootEntry p_serialize_1_, Type p_serialize_2_, JsonSerializationContext p_serialize_3_)
        {
            JsonObject jsonobject = new JsonObject();
            jsonobject.addProperty("weight", (Number)Integer.valueOf(p_serialize_1_.weight));
            jsonobject.addProperty("quality", (Number)Integer.valueOf(p_serialize_1_.quality));

            if (p_serialize_1_.conditions.length > 0)
            {
                jsonobject.add("conditions", p_serialize_3_.serialize(p_serialize_1_.conditions));
            }

            if (p_serialize_1_ instanceof LootEntryItem)
            {
                jsonobject.addProperty("type", "item");
            }
            else if (p_serialize_1_ instanceof LootEntryTable)
            {
                jsonobject.addProperty("type", "item");
            }
            else
            {
                if (!(p_serialize_1_ instanceof LootEntryEmpty))
                {
                    throw new IllegalArgumentException("Don\'t know how to serialize " + p_serialize_1_);
                }

                jsonobject.addProperty("type", "empty");
            }

            p_serialize_1_.serialize(jsonobject, p_serialize_3_);
            return jsonobject;
        }
    }
}
