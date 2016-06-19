package net.minecraft.world.storage.loot;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Collection;
import java.util.Random;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.util.JsonUtils;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.conditions.LootCondition;
import net.minecraft.world.storage.loot.conditions.LootConditionManager;
import net.minecraft.world.storage.loot.functions.LootFunction;

public class LootEntryItem extends LootEntry
{
    protected final Item field_186368_a;
    protected final LootFunction[] functions;

    public LootEntryItem(Item p_i46644_1_, int weightIn, int qualityIn, LootFunction[] functionsIn, LootCondition[] conditionsIn)
    {
        super(weightIn, qualityIn, conditionsIn);
        this.field_186368_a = p_i46644_1_;
        this.functions = functionsIn;
    }

    public void addLoot(Collection<ItemStack> p_186363_1_, Random rand, LootContext context)
    {
        ItemStack itemstack = new ItemStack(this.field_186368_a);
        int i = 0;

        for (int j = this.functions.length; i < j; ++i)
        {
            LootFunction lootfunction = this.functions[i];

            if (LootConditionManager.testAllConditions(lootfunction.getConditions(), rand, context))
            {
                itemstack = lootfunction.apply(itemstack, rand, context);
            }
        }

        if (itemstack.stackSize > 0)
        {
            if (itemstack.stackSize < this.field_186368_a.getItemStackLimit())
            {
                p_186363_1_.add(itemstack);
            }
            else
            {
                i = itemstack.stackSize;

                while (i > 0)
                {
                    ItemStack itemstack1 = itemstack.copy();
                    itemstack1.stackSize = Math.min(itemstack.getMaxStackSize(), i);
                    i -= itemstack1.stackSize;
                    p_186363_1_.add(itemstack1);
                }
            }
        }
    }

    protected void serialize(JsonObject json, JsonSerializationContext context)
    {
        if (this.functions != null && this.functions.length > 0)
        {
            json.add("functions", context.serialize(this.functions));
        }

        ResourceLocation resourcelocation = (ResourceLocation)Item.itemRegistry.getNameForObject(this.field_186368_a);

        if (resourcelocation == null)
        {
            throw new IllegalArgumentException("Can\'t serialize unknown item " + this.field_186368_a);
        }
        else
        {
            json.addProperty("name", resourcelocation.toString());
        }
    }

    public static LootEntryItem func_186367_a(JsonObject p_186367_0_, JsonDeserializationContext p_186367_1_, int p_186367_2_, int p_186367_3_, LootCondition[] p_186367_4_)
    {
        Item item = JsonUtils.func_188180_i(p_186367_0_, "name");
        LootFunction[] alootfunction;

        if (p_186367_0_.has("functions"))
        {
            alootfunction = (LootFunction[])JsonUtils.func_188174_a(p_186367_0_, "functions", p_186367_1_, LootFunction[].class);
        }
        else
        {
            alootfunction = new LootFunction[0];
        }

        return new LootEntryItem(item, p_186367_2_, p_186367_3_, alootfunction, p_186367_4_);
    }
}
