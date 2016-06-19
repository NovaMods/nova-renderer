package net.minecraft.world.storage.loot.conditions;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Random;
import java.util.Set;
import java.util.Map.Entry;
import net.minecraft.entity.Entity;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.properties.EntityProperty;
import net.minecraft.world.storage.loot.properties.EntityPropertyManager;

public class EntityHasProperty implements LootCondition
{
    private final EntityProperty[] field_186623_a;
    private final LootContext.EntityTarget field_186624_b;

    public EntityHasProperty(EntityProperty[] p_i46617_1_, LootContext.EntityTarget p_i46617_2_)
    {
        this.field_186623_a = p_i46617_1_;
        this.field_186624_b = p_i46617_2_;
    }

    public boolean testCondition(Random rand, LootContext context)
    {
        Entity entity = context.getEntity(this.field_186624_b);

        if (entity == null)
        {
            return false;
        }
        else
        {
            int i = 0;

            for (int j = this.field_186623_a.length; i < j; ++i)
            {
                if (!this.field_186623_a[i].testProperty(rand, entity))
                {
                    return false;
                }
            }

            return true;
        }
    }

    public static class Serializer extends LootCondition.Serializer<EntityHasProperty>
    {
        protected Serializer()
        {
            super(new ResourceLocation("entity_properties"), EntityHasProperty.class);
        }

        public void serialize(JsonObject json, EntityHasProperty value, JsonSerializationContext context)
        {
            JsonObject jsonobject = new JsonObject();

            for (EntityProperty entityproperty : value.field_186623_a)
            {
                EntityProperty.Serializer<EntityProperty> serializer = EntityPropertyManager.<EntityProperty>getSerializerFor(entityproperty);
                jsonobject.add(serializer.func_186649_a().toString(), serializer.serialize(entityproperty, context));
            }

            json.add("properties", jsonobject);
            json.add("entity", context.serialize(value.field_186624_b));
        }

        public EntityHasProperty deserialize(JsonObject json, JsonDeserializationContext context)
        {
            Set<Entry<String, JsonElement>> set = JsonUtils.getJsonObject(json, "properties").entrySet();
            EntityProperty[] aentityproperty = new EntityProperty[set.size()];
            int i = 0;

            for (Entry<String, JsonElement> entry : set)
            {
                aentityproperty[i++] = EntityPropertyManager.getSerializerForName(new ResourceLocation((String)entry.getKey())).deserialize((JsonElement)entry.getValue(), context);
            }

            return new EntityHasProperty(aentityproperty, (LootContext.EntityTarget)JsonUtils.func_188174_a(json, "entity", context, LootContext.EntityTarget.class));
        }
    }
}
