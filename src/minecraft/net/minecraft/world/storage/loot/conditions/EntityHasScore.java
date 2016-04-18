package net.minecraft.world.storage.loot.conditions;

import com.google.common.collect.Maps;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.Map.Entry;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.scoreboard.ScoreObjective;
import net.minecraft.scoreboard.Scoreboard;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.RandomValueRange;

public class EntityHasScore implements LootCondition
{
    private final Map<String, RandomValueRange> field_186634_a;
    private final LootContext.EntityTarget field_186635_b;

    public EntityHasScore(Map<String, RandomValueRange> p_i46618_1_, LootContext.EntityTarget p_i46618_2_)
    {
        this.field_186634_a = p_i46618_1_;
        this.field_186635_b = p_i46618_2_;
    }

    public boolean testCondition(Random rand, LootContext context)
    {
        Entity entity = context.getEntity(this.field_186635_b);

        if (entity == null)
        {
            return false;
        }
        else
        {
            Scoreboard scoreboard = entity.worldObj.getScoreboard();

            for (Entry<String, RandomValueRange> entry : this.field_186634_a.entrySet())
            {
                if (!this.func_186631_a(entity, scoreboard, (String)entry.getKey(), (RandomValueRange)entry.getValue()))
                {
                    return false;
                }
            }

            return true;
        }
    }

    protected boolean func_186631_a(Entity p_186631_1_, Scoreboard p_186631_2_, String p_186631_3_, RandomValueRange p_186631_4_)
    {
        ScoreObjective scoreobjective = p_186631_2_.getObjective(p_186631_3_);

        if (scoreobjective == null)
        {
            return false;
        }
        else
        {
            String s = p_186631_1_ instanceof EntityPlayerMP ? p_186631_1_.getName() : p_186631_1_.getUniqueID().toString();
            return !p_186631_2_.entityHasObjective(s, scoreobjective) ? false : p_186631_4_.isInRange(p_186631_2_.getValueFromObjective(s, scoreobjective).getScorePoints());
        }
    }

    public static class Serializer extends LootCondition.Serializer<EntityHasScore>
    {
        protected Serializer()
        {
            super(new ResourceLocation("entity_scores"), EntityHasScore.class);
        }

        public void serialize(JsonObject json, EntityHasScore value, JsonSerializationContext context)
        {
            JsonObject jsonobject = new JsonObject();

            for (Entry<String, RandomValueRange> entry : value.field_186634_a.entrySet())
            {
                jsonobject.add((String)entry.getKey(), context.serialize(entry.getValue()));
            }

            json.add("scores", jsonobject);
            json.add("entity", context.serialize(value.field_186635_b));
        }

        public EntityHasScore deserialize(JsonObject json, JsonDeserializationContext context)
        {
            Set<Entry<String, JsonElement>> set = JsonUtils.getJsonObject(json, "scores").entrySet();
            Map<String, RandomValueRange> map = Maps.<String, RandomValueRange>newLinkedHashMap();

            for (Entry<String, JsonElement> entry : set)
            {
                map.put(entry.getKey(), JsonUtils.func_188179_a((JsonElement)entry.getValue(), "score", context, RandomValueRange.class));
            }

            return new EntityHasScore(map, (LootContext.EntityTarget)JsonUtils.func_188174_a(json, "entity", context, LootContext.EntityTarget.class));
        }
    }
}
