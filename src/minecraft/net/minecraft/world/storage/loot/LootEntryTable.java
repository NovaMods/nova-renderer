package net.minecraft.world.storage.loot;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Collection;
import java.util.Random;
import net.minecraft.item.ItemStack;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.conditions.LootCondition;

public class LootEntryTable extends LootEntry
{
    protected final ResourceLocation field_186371_a;

    public LootEntryTable(ResourceLocation p_i46639_1_, int weightIn, int qualityIn, LootCondition[] conditionsIn)
    {
        super(weightIn, qualityIn, conditionsIn);
        this.field_186371_a = p_i46639_1_;
    }

    public void addLoot(Collection<ItemStack> p_186363_1_, Random rand, LootContext context)
    {
        LootTable loottable = context.getLootTableManager().func_186521_a(this.field_186371_a);
        Collection<ItemStack> collection = loottable.func_186462_a(rand, context);
        p_186363_1_.addAll(collection);
    }

    protected void serialize(JsonObject json, JsonSerializationContext context)
    {
        json.addProperty("name", this.field_186371_a.toString());
    }

    public static LootEntryTable func_186370_a(JsonObject p_186370_0_, JsonDeserializationContext p_186370_1_, int p_186370_2_, int p_186370_3_, LootCondition[] p_186370_4_)
    {
        ResourceLocation resourcelocation = new ResourceLocation(JsonUtils.getString(p_186370_0_, "name"));
        return new LootEntryTable(resourcelocation, p_186370_2_, p_186370_3_, p_186370_4_);
    }
}
