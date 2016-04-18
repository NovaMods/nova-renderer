package net.minecraft.world.storage.loot;

import com.google.common.collect.Sets;
import java.util.Collections;
import java.util.Set;
import net.minecraft.util.ResourceLocation;

public class LootTableList
{
    private static final Set<ResourceLocation> field_186391_ap = Sets.<ResourceLocation>newHashSet();
    private static final Set<ResourceLocation> field_186392_aq = Collections.<ResourceLocation>unmodifiableSet(field_186391_ap);
    public static final ResourceLocation EMPTY = func_186373_a("empty");
    public static final ResourceLocation CHESTS_SPAWN_BONUS_CHEST = func_186373_a("chests/spawn_bonus_chest");
    public static final ResourceLocation CHESTS_END_CITY_TREASURE = func_186373_a("chests/end_city_treasure");
    public static final ResourceLocation CHESTS_SIMPLE_DUNGEON = func_186373_a("chests/simple_dungeon");
    public static final ResourceLocation CHESTS_VILLAGE_BLACKSMITH = func_186373_a("chests/village_blacksmith");
    public static final ResourceLocation CHESTS_ABANDONED_MINESHAFT = func_186373_a("chests/abandoned_mineshaft");
    public static final ResourceLocation CHESTS_NETHER_BRIDGE = func_186373_a("chests/nether_bridge");
    public static final ResourceLocation CHESTS_STRONGHOLD_LIBRARY = func_186373_a("chests/stronghold_library");
    public static final ResourceLocation CHESTS_STRONGHOLD_CROSSING = func_186373_a("chests/stronghold_crossing");
    public static final ResourceLocation CHESTS_STRONGHOLD_CORRIDOR = func_186373_a("chests/stronghold_corridor");
    public static final ResourceLocation CHESTS_DESERT_PYRAMID = func_186373_a("chests/desert_pyramid");
    public static final ResourceLocation CHESTS_JUNGLE_TEMPLE = func_186373_a("chests/jungle_temple");
    public static final ResourceLocation CHESTS_IGLOO_CHEST = func_186373_a("chests/igloo_chest");
    public static final ResourceLocation ENTITIES_WITCH = func_186373_a("entities/witch");
    public static final ResourceLocation ENTITIES_BLAZE = func_186373_a("entities/blaze");
    public static final ResourceLocation ENTITIES_CREEPER = func_186373_a("entities/creeper");
    public static final ResourceLocation ENTITIES_SPIDER = func_186373_a("entities/spider");
    public static final ResourceLocation ENTITIES_CAVE_SPIDER = func_186373_a("entities/cave_spider");
    public static final ResourceLocation ENTITIES_GIANT = func_186373_a("entities/giant");
    public static final ResourceLocation ENTITIES_SILVERFISH = func_186373_a("entities/silverfish");
    public static final ResourceLocation ENTITIES_ENDERMAN = func_186373_a("entities/enderman");
    public static final ResourceLocation ENTITIES_GUARDIAN = func_186373_a("entities/guardian");
    public static final ResourceLocation ENTITIES_ELDER_GUARDIAN = func_186373_a("entities/elder_guardian");
    public static final ResourceLocation ENTITIES_SHULKER = func_186373_a("entities/shulker");
    public static final ResourceLocation ENTITIES_IRON_GOLEM = func_186373_a("entities/iron_golem");
    public static final ResourceLocation ENTITIES_SNOWMAN = func_186373_a("entities/snowman");
    public static final ResourceLocation ENTITIES_RABBIT = func_186373_a("entities/rabbit");
    public static final ResourceLocation ENTITIES_CHICKEN = func_186373_a("entities/chicken");
    public static final ResourceLocation ENTITIES_PIG = func_186373_a("entities/pig");
    public static final ResourceLocation ENTITIES_HORSE = func_186373_a("entities/horse");
    public static final ResourceLocation ENTITIES_ZOMBIE_HORSE = func_186373_a("entities/zombie_horse");
    public static final ResourceLocation ENTITIES_SKELETON_HORSE = func_186373_a("entities/skeleton_horse");
    public static final ResourceLocation ENTITIES_COW = func_186373_a("entities/cow");
    public static final ResourceLocation ENTITIES_MUSHROOM_COW = func_186373_a("entities/mushroom_cow");
    public static final ResourceLocation ENTITIES_WOLF = func_186373_a("entities/wolf");
    public static final ResourceLocation ENTITIES_OCELOT = func_186373_a("entities/ocelot");
    public static final ResourceLocation ENTITIES_SHEEP = func_186373_a("entities/sheep");
    public static final ResourceLocation ENTITIES_SHEEP_WHITE = func_186373_a("entities/sheep/white");
    public static final ResourceLocation ENTITIES_SHEEP_ORANGE = func_186373_a("entities/sheep/orange");
    public static final ResourceLocation ENTITIES_SHEEP_MAGENTA = func_186373_a("entities/sheep/magenta");
    public static final ResourceLocation ENTITIES_SHEEP_LIGHT_BLUE = func_186373_a("entities/sheep/light_blue");
    public static final ResourceLocation ENTITIES_SHEEP_YELLOW = func_186373_a("entities/sheep/yellow");
    public static final ResourceLocation ENTITIES_SHEEP_LIME = func_186373_a("entities/sheep/lime");
    public static final ResourceLocation ENTITIES_SHEEP_PINK = func_186373_a("entities/sheep/pink");
    public static final ResourceLocation ENTITIES_SHEEP_GRAY = func_186373_a("entities/sheep/gray");
    public static final ResourceLocation ENTITIES_SHEEP_SILVER = func_186373_a("entities/sheep/silver");
    public static final ResourceLocation ENTITIES_SHEEP_CYAN = func_186373_a("entities/sheep/cyan");
    public static final ResourceLocation ENTITIES_SHEEP_PURPLE = func_186373_a("entities/sheep/purple");
    public static final ResourceLocation ENTITIES_SHEEP_BLUE = func_186373_a("entities/sheep/blue");
    public static final ResourceLocation ENTITIES_SHEEP_BROWN = func_186373_a("entities/sheep/brown");
    public static final ResourceLocation ENTITIES_SHEEP_GREEN = func_186373_a("entities/sheep/green");
    public static final ResourceLocation ENTITIES_SHEEP_RED = func_186373_a("entities/sheep/red");
    public static final ResourceLocation ENTITIES_SHEEP_BLACK = func_186373_a("entities/sheep/black");
    public static final ResourceLocation ENTITIES_BAT = func_186373_a("entities/bat");
    public static final ResourceLocation ENTITIES_SLIME = func_186373_a("entities/slime");
    public static final ResourceLocation ENTITIES_MAGMA_CUBE = func_186373_a("entities/magma_cube");
    public static final ResourceLocation ENTITIES_GHAST = func_186373_a("entities/ghast");
    public static final ResourceLocation ENTITIES_SQUID = func_186373_a("entities/squid");
    public static final ResourceLocation ENTITIES_ENDERMITE = func_186373_a("entities/endermite");
    public static final ResourceLocation ENTITIES_ZOMBIE = func_186373_a("entities/zombie");
    public static final ResourceLocation ENTITIES_ZOMBIE_PIGMAN = func_186373_a("entities/zombie_pigman");
    public static final ResourceLocation ENTITIES_SKELETON = func_186373_a("entities/skeleton");
    public static final ResourceLocation ENTITIES_WITHER_SKELETON = func_186373_a("entities/wither_skeleton");
    public static final ResourceLocation GAMEPLAY_FISHING = func_186373_a("gameplay/fishing");
    public static final ResourceLocation GAMEPLAY_FISHING_JUNK = func_186373_a("gameplay/fishing/junk");
    public static final ResourceLocation GAMEPLAY_FISHING_TREASURE = func_186373_a("gameplay/fishing/treasure");
    public static final ResourceLocation GAMEPLAY_FISHING_FISH = func_186373_a("gameplay/fishing/fish");

    private static ResourceLocation func_186373_a(String p_186373_0_)
    {
        return func_186375_a(new ResourceLocation("minecraft", p_186373_0_));
    }

    public static ResourceLocation func_186375_a(ResourceLocation p_186375_0_)
    {
        if (field_186391_ap.add(p_186375_0_))
        {
            return p_186375_0_;
        }
        else
        {
            throw new IllegalArgumentException(p_186375_0_ + " is already a registered built-in loot table");
        }
    }

    public static Set<ResourceLocation> func_186374_a()
    {
        return field_186392_aq;
    }
}
