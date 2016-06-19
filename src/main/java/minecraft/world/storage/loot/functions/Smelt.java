package net.minecraft.world.storage.loot.functions;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import java.util.Random;
import net.minecraft.item.ItemStack;
import net.minecraft.item.crafting.FurnaceRecipes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.conditions.LootCondition;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class Smelt extends LootFunction
{
    private static final Logger field_186574_a = LogManager.getLogger();

    public Smelt(LootCondition[] p_i46619_1_)
    {
        super(p_i46619_1_);
    }

    public ItemStack apply(ItemStack p_186553_1_, Random p_186553_2_, LootContext p_186553_3_)
    {
        ItemStack itemstack = FurnaceRecipes.instance().getSmeltingResult(p_186553_1_);

        if (itemstack == null)
        {
            field_186574_a.warn("Couldn\'t smelt " + p_186553_1_ + " because there is no smelting recipe");
            return p_186553_1_;
        }
        else
        {
            ItemStack itemstack1 = itemstack.copy();
            itemstack1.stackSize = p_186553_1_.stackSize;
            return itemstack1;
        }
    }

    public static class Serializer extends LootFunction.Serializer<Smelt>
    {
        protected Serializer()
        {
            super(new ResourceLocation("furnace_smelt"), Smelt.class);
        }

        public void func_186532_a(JsonObject p_186532_1_, Smelt p_186532_2_, JsonSerializationContext p_186532_3_)
        {
        }

        public Smelt func_186530_b(JsonObject p_186530_1_, JsonDeserializationContext p_186530_2_, LootCondition[] p_186530_3_)
        {
            return new Smelt(p_186530_3_);
        }
    }
}
