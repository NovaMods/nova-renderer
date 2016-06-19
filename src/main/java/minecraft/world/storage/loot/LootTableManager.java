package net.minecraft.world.storage.loot;

import com.google.common.base.Charsets;
import com.google.common.cache.CacheBuilder;
import com.google.common.cache.CacheLoader;
import com.google.common.cache.LoadingCache;
import com.google.common.io.Files;
import com.google.common.io.Resources;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonParseException;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.storage.loot.conditions.LootCondition;
import net.minecraft.world.storage.loot.conditions.LootConditionManager;
import net.minecraft.world.storage.loot.functions.LootFunction;
import net.minecraft.world.storage.loot.functions.LootFunctionManager;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class LootTableManager
{
    private static final Logger LOGGER = LogManager.getLogger();
    private static final Gson field_186526_b = (new GsonBuilder()).registerTypeAdapter(RandomValueRange.class, new RandomValueRange.Serializer()).registerTypeAdapter(LootPool.class, new LootPool.Serializer()).registerTypeAdapter(LootTable.class, new LootTable.Serializer()).registerTypeHierarchyAdapter(LootEntry.class, new LootEntry.Serializer()).registerTypeHierarchyAdapter(LootFunction.class, new LootFunctionManager.Serializer()).registerTypeHierarchyAdapter(LootCondition.class, new LootConditionManager.Serializer()).registerTypeHierarchyAdapter(LootContext.EntityTarget.class, new LootContext.EntityTarget.Serializer()).create();
    private final LoadingCache<ResourceLocation, LootTable> field_186527_c = CacheBuilder.newBuilder().<ResourceLocation, LootTable>build(new LootTableManager.Loader());
    private final File field_186528_d;

    public LootTableManager(File p_i46632_1_)
    {
        this.field_186528_d = p_i46632_1_;
        this.func_186522_a();
    }

    public LootTable func_186521_a(ResourceLocation p_186521_1_)
    {
        return (LootTable)this.field_186527_c.getUnchecked(p_186521_1_);
    }

    public void func_186522_a()
    {
        this.field_186527_c.invalidateAll();

        for (ResourceLocation resourcelocation : LootTableList.func_186374_a())
        {
            this.func_186521_a(resourcelocation);
        }
    }

    class Loader extends CacheLoader<ResourceLocation, LootTable>
    {
        private Loader()
        {
        }

        public LootTable load(ResourceLocation p_load_1_) throws Exception
        {
            if (p_load_1_.getResourcePath().contains("."))
            {
                LootTableManager.LOGGER.debug("Invalid loot table name \'" + p_load_1_ + "\' (can\'t contain periods)");
                return LootTable.EMPTY_LOOT_TABLE;
            }
            else
            {
                LootTable loottable = this.loadLootTable(p_load_1_);

                if (loottable == null)
                {
                    loottable = this.loadBuiltinLootTable(p_load_1_);
                }

                if (loottable == null)
                {
                    loottable = LootTable.EMPTY_LOOT_TABLE;
                    LootTableManager.LOGGER.warn("Couldn\'t find resource table {}", new Object[] {p_load_1_});
                }

                return loottable;
            }
        }

        private LootTable loadLootTable(ResourceLocation p_186517_1_)
        {
            File file1 = new File(new File(LootTableManager.this.field_186528_d, p_186517_1_.getResourceDomain()), p_186517_1_.getResourcePath() + ".json");

            if (file1.exists())
            {
                if (file1.isFile())
                {
                    String s;

                    try
                    {
                        s = Files.toString(file1, Charsets.UTF_8);
                    }
                    catch (IOException ioexception)
                    {
                        LootTableManager.LOGGER.warn((String)("Couldn\'t load loot table " + p_186517_1_ + " from " + file1), (Throwable)ioexception);
                        return LootTable.EMPTY_LOOT_TABLE;
                    }

                    try
                    {
                        return (LootTable)LootTableManager.field_186526_b.fromJson(s, LootTable.class);
                    }
                    catch (JsonParseException jsonparseexception)
                    {
                        LootTableManager.LOGGER.error((String)("Couldn\'t load loot table " + p_186517_1_ + " from " + file1), (Throwable)jsonparseexception);
                        return LootTable.EMPTY_LOOT_TABLE;
                    }
                }
                else
                {
                    LootTableManager.LOGGER.warn("Expected to find loot table " + p_186517_1_ + " at " + file1 + " but it was a folder.");
                    return LootTable.EMPTY_LOOT_TABLE;
                }
            }
            else
            {
                return null;
            }
        }

        private LootTable loadBuiltinLootTable(ResourceLocation p_186518_1_)
        {
            URL url = LootTableManager.class.getResource("/assets/" + p_186518_1_.getResourceDomain() + "/loot_tables/" + p_186518_1_.getResourcePath() + ".json");

            if (url != null)
            {
                String s;

                try
                {
                    s = Resources.toString(url, Charsets.UTF_8);
                }
                catch (IOException ioexception)
                {
                    LootTableManager.LOGGER.warn((String)("Couldn\'t load loot table " + p_186518_1_ + " from " + url), (Throwable)ioexception);
                    return LootTable.EMPTY_LOOT_TABLE;
                }

                try
                {
                    return (LootTable)LootTableManager.field_186526_b.fromJson(s, LootTable.class);
                }
                catch (JsonParseException jsonparseexception)
                {
                    LootTableManager.LOGGER.error((String)("Couldn\'t load loot table " + p_186518_1_ + " from " + url), (Throwable)jsonparseexception);
                    return LootTable.EMPTY_LOOT_TABLE;
                }
            }
            else
            {
                return null;
            }
        }
    }
}
