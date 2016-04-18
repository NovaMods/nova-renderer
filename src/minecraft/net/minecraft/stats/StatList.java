package net.minecraft.stats;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;
import java.util.List;
import java.util.Map;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.entity.EntityList;
import net.minecraft.init.Blocks;
import net.minecraft.item.Item;
import net.minecraft.item.ItemBlock;
import net.minecraft.item.ItemStack;
import net.minecraft.item.crafting.CraftingManager;
import net.minecraft.item.crafting.FurnaceRecipes;
import net.minecraft.item.crafting.IRecipe;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.text.TextComponentTranslation;

public class StatList
{
    protected static final Map<String, StatBase> field_188093_a = Maps.<String, StatBase>newHashMap();
    public static final List<StatBase> allStats = Lists.<StatBase>newArrayList();
    public static final List<StatBase> field_188094_c = Lists.<StatBase>newArrayList();
    public static final List<StatCrafting> field_188095_d = Lists.<StatCrafting>newArrayList();
    public static final List<StatCrafting> field_188096_e = Lists.<StatCrafting>newArrayList();

    /** number of times you've left a game */
    public static final StatBase leaveGameStat = (new StatBasic("stat.leaveGame", new TextComponentTranslation("stat.leaveGame", new Object[0]))).initIndependentStat().registerStat();
    public static final StatBase field_188097_g = (new StatBasic("stat.playOneMinute", new TextComponentTranslation("stat.playOneMinute", new Object[0]), StatBase.timeStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188098_h = (new StatBasic("stat.timeSinceDeath", new TextComponentTranslation("stat.timeSinceDeath", new Object[0]), StatBase.timeStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188099_i = (new StatBasic("stat.sneakTime", new TextComponentTranslation("stat.sneakTime", new Object[0]), StatBase.timeStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188100_j = (new StatBasic("stat.walkOneCm", new TextComponentTranslation("stat.walkOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188101_k = (new StatBasic("stat.crouchOneCm", new TextComponentTranslation("stat.crouchOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188102_l = (new StatBasic("stat.sprintOneCm", new TextComponentTranslation("stat.sprintOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();

    /** distance you have swam */
    public static final StatBase distanceSwumStat = (new StatBasic("stat.swimOneCm", new TextComponentTranslation("stat.swimOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();

    /** the distance you have fallen */
    public static final StatBase distanceFallenStat = (new StatBasic("stat.fallOneCm", new TextComponentTranslation("stat.fallOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188103_o = (new StatBasic("stat.climbOneCm", new TextComponentTranslation("stat.climbOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188104_p = (new StatBasic("stat.flyOneCm", new TextComponentTranslation("stat.flyOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188105_q = (new StatBasic("stat.diveOneCm", new TextComponentTranslation("stat.diveOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188106_r = (new StatBasic("stat.minecartOneCm", new TextComponentTranslation("stat.minecartOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188107_s = (new StatBasic("stat.boatOneCm", new TextComponentTranslation("stat.boatOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188108_t = (new StatBasic("stat.pigOneCm", new TextComponentTranslation("stat.pigOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188109_u = (new StatBasic("stat.horseOneCm", new TextComponentTranslation("stat.horseOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();
    public static final StatBase field_188110_v = (new StatBasic("stat.aviateOneCm", new TextComponentTranslation("stat.aviateOneCm", new Object[0]), StatBase.distanceStatType)).initIndependentStat().registerStat();

    /** the times you've jumped */
    public static final StatBase jumpStat = (new StatBasic("stat.jump", new TextComponentTranslation("stat.jump", new Object[0]))).initIndependentStat().registerStat();

    /** the distance you've dropped (or times you've fallen?) */
    public static final StatBase dropStat = (new StatBasic("stat.drop", new TextComponentTranslation("stat.drop", new Object[0]))).initIndependentStat().registerStat();
    public static final StatBase field_188111_y = (new StatBasic("stat.damageDealt", new TextComponentTranslation("stat.damageDealt", new Object[0]), StatBase.field_111202_k)).registerStat();
    public static final StatBase field_188112_z = (new StatBasic("stat.damageTaken", new TextComponentTranslation("stat.damageTaken", new Object[0]), StatBase.field_111202_k)).registerStat();
    public static final StatBase field_188069_A = (new StatBasic("stat.deaths", new TextComponentTranslation("stat.deaths", new Object[0]))).registerStat();
    public static final StatBase field_188070_B = (new StatBasic("stat.mobKills", new TextComponentTranslation("stat.mobKills", new Object[0]))).registerStat();

    /** the number of animals you have bred */
    public static final StatBase animalsBredStat = (new StatBasic("stat.animalsBred", new TextComponentTranslation("stat.animalsBred", new Object[0]))).registerStat();

    /** counts the number of times you've killed a player */
    public static final StatBase playerKillsStat = (new StatBasic("stat.playerKills", new TextComponentTranslation("stat.playerKills", new Object[0]))).registerStat();
    public static final StatBase field_188071_E = (new StatBasic("stat.fishCaught", new TextComponentTranslation("stat.fishCaught", new Object[0]))).registerStat();
    public static final StatBase field_188072_F = (new StatBasic("stat.junkFished", new TextComponentTranslation("stat.junkFished", new Object[0]))).registerStat();
    public static final StatBase field_188073_G = (new StatBasic("stat.treasureFished", new TextComponentTranslation("stat.treasureFished", new Object[0]))).registerStat();
    public static final StatBase field_188074_H = (new StatBasic("stat.talkedToVillager", new TextComponentTranslation("stat.talkedToVillager", new Object[0]))).registerStat();
    public static final StatBase field_188075_I = (new StatBasic("stat.tradedWithVillager", new TextComponentTranslation("stat.tradedWithVillager", new Object[0]))).registerStat();
    public static final StatBase field_188076_J = (new StatBasic("stat.cakeSlicesEaten", new TextComponentTranslation("stat.cakeSlicesEaten", new Object[0]))).registerStat();
    public static final StatBase field_188077_K = (new StatBasic("stat.cauldronFilled", new TextComponentTranslation("stat.cauldronFilled", new Object[0]))).registerStat();
    public static final StatBase field_188078_L = (new StatBasic("stat.cauldronUsed", new TextComponentTranslation("stat.cauldronUsed", new Object[0]))).registerStat();
    public static final StatBase field_188079_M = (new StatBasic("stat.armorCleaned", new TextComponentTranslation("stat.armorCleaned", new Object[0]))).registerStat();
    public static final StatBase field_188080_N = (new StatBasic("stat.bannerCleaned", new TextComponentTranslation("stat.bannerCleaned", new Object[0]))).registerStat();
    public static final StatBase field_188081_O = (new StatBasic("stat.brewingstandInteraction", new TextComponentTranslation("stat.brewingstandInteraction", new Object[0]))).registerStat();
    public static final StatBase field_188082_P = (new StatBasic("stat.beaconInteraction", new TextComponentTranslation("stat.beaconInteraction", new Object[0]))).registerStat();
    public static final StatBase field_188083_Q = (new StatBasic("stat.dropperInspected", new TextComponentTranslation("stat.dropperInspected", new Object[0]))).registerStat();
    public static final StatBase field_188084_R = (new StatBasic("stat.hopperInspected", new TextComponentTranslation("stat.hopperInspected", new Object[0]))).registerStat();
    public static final StatBase field_188085_S = (new StatBasic("stat.dispenserInspected", new TextComponentTranslation("stat.dispenserInspected", new Object[0]))).registerStat();
    public static final StatBase field_188086_T = (new StatBasic("stat.noteblockPlayed", new TextComponentTranslation("stat.noteblockPlayed", new Object[0]))).registerStat();
    public static final StatBase field_188087_U = (new StatBasic("stat.noteblockTuned", new TextComponentTranslation("stat.noteblockTuned", new Object[0]))).registerStat();
    public static final StatBase field_188088_V = (new StatBasic("stat.flowerPotted", new TextComponentTranslation("stat.flowerPotted", new Object[0]))).registerStat();
    public static final StatBase field_188089_W = (new StatBasic("stat.trappedChestTriggered", new TextComponentTranslation("stat.trappedChestTriggered", new Object[0]))).registerStat();
    public static final StatBase field_188090_X = (new StatBasic("stat.enderchestOpened", new TextComponentTranslation("stat.enderchestOpened", new Object[0]))).registerStat();
    public static final StatBase field_188091_Y = (new StatBasic("stat.itemEnchanted", new TextComponentTranslation("stat.itemEnchanted", new Object[0]))).registerStat();
    public static final StatBase field_188092_Z = (new StatBasic("stat.recordPlayed", new TextComponentTranslation("stat.recordPlayed", new Object[0]))).registerStat();
    public static final StatBase field_188061_aa = (new StatBasic("stat.furnaceInteraction", new TextComponentTranslation("stat.furnaceInteraction", new Object[0]))).registerStat();
    public static final StatBase field_188062_ab = (new StatBasic("stat.craftingTableInteraction", new TextComponentTranslation("stat.workbenchInteraction", new Object[0]))).registerStat();
    public static final StatBase field_188063_ac = (new StatBasic("stat.chestOpened", new TextComponentTranslation("stat.chestOpened", new Object[0]))).registerStat();
    public static final StatBase field_188064_ad = (new StatBasic("stat.sleepInBed", new TextComponentTranslation("stat.sleepInBed", new Object[0]))).registerStat();
    private static final StatBase[] field_188065_ae = new StatBase[4096];
    private static final StatBase[] field_188066_af = new StatBase[32000];

    /** Tracks the number of times a given block or item has been used. */
    private static final StatBase[] objectUseStats = new StatBase[32000];

    /** Tracks the number of times a given block or item has been broken. */
    private static final StatBase[] objectBreakStats = new StatBase[32000];
    private static final StatBase[] field_188067_ai = new StatBase[32000];
    private static final StatBase[] field_188068_aj = new StatBase[32000];

    public static StatBase func_188055_a(Block p_188055_0_)
    {
        return field_188065_ae[Block.getIdFromBlock(p_188055_0_)];
    }

    public static StatBase func_188060_a(Item p_188060_0_)
    {
        return field_188066_af[Item.getIdFromItem(p_188060_0_)];
    }

    public static StatBase func_188057_b(Item p_188057_0_)
    {
        return objectUseStats[Item.getIdFromItem(p_188057_0_)];
    }

    public static StatBase func_188059_c(Item p_188059_0_)
    {
        return objectBreakStats[Item.getIdFromItem(p_188059_0_)];
    }

    public static StatBase func_188056_d(Item p_188056_0_)
    {
        return field_188067_ai[Item.getIdFromItem(p_188056_0_)];
    }

    public static StatBase func_188058_e(Item p_188058_0_)
    {
        return field_188068_aj[Item.getIdFromItem(p_188058_0_)];
    }

    public static void init()
    {
        initMiningStats();
        initStats();
        initItemDepleteStats();
        initCraftableStats();
        func_188054_f();
        AchievementList.init();
        EntityList.func_151514_a();
    }

    /**
     * Initializes statistics related to craftable items. Is only called after both block and item stats have been
     * initialized.
     */
    private static void initCraftableStats()
    {
        Set<Item> set = Sets.<Item>newHashSet();

        for (IRecipe irecipe : CraftingManager.getInstance().getRecipeList())
        {
            if (irecipe.getRecipeOutput() != null)
            {
                set.add(irecipe.getRecipeOutput().getItem());
            }
        }

        for (ItemStack itemstack : FurnaceRecipes.instance().getSmeltingList().values())
        {
            set.add(itemstack.getItem());
        }

        for (Item item : set)
        {
            if (item != null)
            {
                int i = Item.getIdFromItem(item);
                String s = func_180204_a(item);

                if (s != null)
                {
                    field_188066_af[i] = (new StatCrafting("stat.craftItem.", s, new TextComponentTranslation("stat.craftItem", new Object[] {(new ItemStack(item)).getChatComponent()}), item)).registerStat();
                }
            }
        }

        replaceAllSimilarBlocks(field_188066_af);
    }

    private static void initMiningStats()
    {
        for (Block block : Block.blockRegistry)
        {
            Item item = Item.getItemFromBlock(block);

            if (item != null)
            {
                int i = Block.getIdFromBlock(block);
                String s = func_180204_a(item);

                if (s != null && block.getEnableStats())
                {
                    field_188065_ae[i] = (new StatCrafting("stat.mineBlock.", s, new TextComponentTranslation("stat.mineBlock", new Object[] {(new ItemStack(block)).getChatComponent()}), item)).registerStat();
                    field_188096_e.add((StatCrafting)field_188065_ae[i]);
                }
            }
        }

        replaceAllSimilarBlocks(field_188065_ae);
    }

    private static void initStats()
    {
        for (Item item : Item.itemRegistry)
        {
            if (item != null)
            {
                int i = Item.getIdFromItem(item);
                String s = func_180204_a(item);

                if (s != null)
                {
                    objectUseStats[i] = (new StatCrafting("stat.useItem.", s, new TextComponentTranslation("stat.useItem", new Object[] {(new ItemStack(item)).getChatComponent()}), item)).registerStat();

                    if (!(item instanceof ItemBlock))
                    {
                        field_188095_d.add((StatCrafting)objectUseStats[i]);
                    }
                }
            }
        }

        replaceAllSimilarBlocks(objectUseStats);
    }

    private static void initItemDepleteStats()
    {
        for (Item item : Item.itemRegistry)
        {
            if (item != null)
            {
                int i = Item.getIdFromItem(item);
                String s = func_180204_a(item);

                if (s != null && item.isDamageable())
                {
                    objectBreakStats[i] = (new StatCrafting("stat.breakItem.", s, new TextComponentTranslation("stat.breakItem", new Object[] {(new ItemStack(item)).getChatComponent()}), item)).registerStat();
                }
            }
        }

        replaceAllSimilarBlocks(objectBreakStats);
    }

    private static void func_188054_f()
    {
        for (Item item : Item.itemRegistry)
        {
            if (item != null)
            {
                int i = Item.getIdFromItem(item);
                String s = func_180204_a(item);

                if (s != null)
                {
                    field_188067_ai[i] = (new StatCrafting("stat.pickup.", s, new TextComponentTranslation("stat.pickup", new Object[] {(new ItemStack(item)).getChatComponent()}), item)).registerStat();
                    field_188068_aj[i] = (new StatCrafting("stat.drop.", s, new TextComponentTranslation("stat.drop", new Object[] {(new ItemStack(item)).getChatComponent()}), item)).registerStat();
                }
            }
        }

        replaceAllSimilarBlocks(objectBreakStats);
    }

    private static String func_180204_a(Item p_180204_0_)
    {
        ResourceLocation resourcelocation = (ResourceLocation)Item.itemRegistry.getNameForObject(p_180204_0_);
        return resourcelocation != null ? resourcelocation.toString().replace(':', '.') : null;
    }

    /**
     * Forces all dual blocks to count for each other on the stats list
     */
    private static void replaceAllSimilarBlocks(StatBase[] p_75924_0_)
    {
        mergeStatBases(p_75924_0_, Blocks.water, Blocks.flowing_water);
        mergeStatBases(p_75924_0_, Blocks.lava, Blocks.flowing_lava);
        mergeStatBases(p_75924_0_, Blocks.lit_pumpkin, Blocks.pumpkin);
        mergeStatBases(p_75924_0_, Blocks.lit_furnace, Blocks.furnace);
        mergeStatBases(p_75924_0_, Blocks.lit_redstone_ore, Blocks.redstone_ore);
        mergeStatBases(p_75924_0_, Blocks.powered_repeater, Blocks.unpowered_repeater);
        mergeStatBases(p_75924_0_, Blocks.powered_comparator, Blocks.unpowered_comparator);
        mergeStatBases(p_75924_0_, Blocks.redstone_torch, Blocks.unlit_redstone_torch);
        mergeStatBases(p_75924_0_, Blocks.lit_redstone_lamp, Blocks.redstone_lamp);
        mergeStatBases(p_75924_0_, Blocks.double_stone_slab, Blocks.stone_slab);
        mergeStatBases(p_75924_0_, Blocks.double_wooden_slab, Blocks.wooden_slab);
        mergeStatBases(p_75924_0_, Blocks.double_stone_slab2, Blocks.stone_slab2);
        mergeStatBases(p_75924_0_, Blocks.grass, Blocks.dirt);
        mergeStatBases(p_75924_0_, Blocks.farmland, Blocks.dirt);
    }

    /**
     * Merge {@link StatBase} object references for similar blocks
     */
    private static void mergeStatBases(StatBase[] statBaseIn, Block p_151180_1_, Block p_151180_2_)
    {
        int i = Block.getIdFromBlock(p_151180_1_);
        int j = Block.getIdFromBlock(p_151180_2_);

        if (statBaseIn[i] != null && statBaseIn[j] == null)
        {
            statBaseIn[j] = statBaseIn[i];
        }
        else
        {
            allStats.remove(statBaseIn[i]);
            field_188096_e.remove(statBaseIn[i]);
            field_188094_c.remove(statBaseIn[i]);
            statBaseIn[i] = statBaseIn[j];
        }
    }

    public static StatBase getStatKillEntity(EntityList.EntityEggInfo eggInfo)
    {
        return eggInfo.spawnedID == null ? null : (new StatBase("stat.killEntity." + eggInfo.spawnedID, new TextComponentTranslation("stat.entityKill", new Object[] {new TextComponentTranslation("entity." + eggInfo.spawnedID + ".name", new Object[0])}))).registerStat();
    }

    public static StatBase getStatEntityKilledBy(EntityList.EntityEggInfo eggInfo)
    {
        return eggInfo.spawnedID == null ? null : (new StatBase("stat.entityKilledBy." + eggInfo.spawnedID, new TextComponentTranslation("stat.entityKilledBy", new Object[] {new TextComponentTranslation("entity." + eggInfo.spawnedID + ".name", new Object[0])}))).registerStat();
    }

    public static StatBase getOneShotStat(String p_151177_0_)
    {
        return (StatBase)field_188093_a.get(p_151177_0_);
    }
}
