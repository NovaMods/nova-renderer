package net.minecraft.world.storage.loot;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Collection;
import java.util.Random;
import net.minecraft.item.ItemStack;
import net.minecraft.world.storage.loot.conditions.LootCondition;

public class LootEntryEmpty extends LootEntry
{
    public LootEntryEmpty(int weightIn, int qualityIn, LootCondition[] conditionsIn)
    {
        super(weightIn, qualityIn, conditionsIn);
    }

    public void addLoot(Collection<ItemStack> p_186363_1_, Random rand, LootContext context)
    {
    }

    protected void serialize(JsonObject json, JsonSerializationContext context)
    {
    }

    public static LootEntryEmpty func_186372_a(JsonObject p_186372_0_, JsonDeserializationContext p_186372_1_, int p_186372_2_, int p_186372_3_, LootCondition[] p_186372_4_)
    {
        return new LootEntryEmpty(p_186372_2_, p_186372_3_, p_186372_4_);
    }
}
