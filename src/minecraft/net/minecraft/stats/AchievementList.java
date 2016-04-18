package net.minecraft.stats;

import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.ItemStack;
import net.minecraft.util.JsonSerializableSet;

public class AchievementList
{
    /** Is the smallest column used to display a achievement on the GUI. */
    public static int minDisplayColumn;

    /** Is the smallest row used to display a achievement on the GUI. */
    public static int minDisplayRow;

    /** Is the biggest column used to display a achievement on the GUI. */
    public static int maxDisplayColumn;

    /** Is the biggest row used to display a achievement on the GUI. */
    public static int maxDisplayRow;
    public static final List<Achievement> field_187981_e = Lists.<Achievement>newArrayList();
    public static final Achievement field_187982_f = (new Achievement("achievement.openInventory", "openInventory", 0, 0, Items.book, (Achievement)null)).initIndependentStat().registerStat();
    public static final Achievement field_187983_g = (new Achievement("achievement.mineWood", "mineWood", 2, 1, Blocks.log, field_187982_f)).registerStat();
    public static final Achievement field_187984_h = (new Achievement("achievement.buildWorkBench", "buildWorkBench", 4, -1, Blocks.crafting_table, field_187983_g)).registerStat();
    public static final Achievement field_187985_i = (new Achievement("achievement.buildPickaxe", "buildPickaxe", 4, 2, Items.wooden_pickaxe, field_187984_h)).registerStat();
    public static final Achievement field_187986_j = (new Achievement("achievement.buildFurnace", "buildFurnace", 3, 4, Blocks.furnace, field_187985_i)).registerStat();
    public static final Achievement field_187987_k = (new Achievement("achievement.acquireIron", "acquireIron", 1, 4, Items.iron_ingot, field_187986_j)).registerStat();

    /** Is the 'time to farm' achievement. */
    public static final Achievement buildHoe = (new Achievement("achievement.buildHoe", "buildHoe", 2, -3, Items.wooden_hoe, field_187984_h)).registerStat();
    public static final Achievement field_187988_m = (new Achievement("achievement.makeBread", "makeBread", -1, -3, Items.bread, buildHoe)).registerStat();

    /** Is the 'the lie' achievement. */
    public static final Achievement bakeCake = (new Achievement("achievement.bakeCake", "bakeCake", 0, -5, Items.cake, buildHoe)).registerStat();
    public static final Achievement field_187989_o = (new Achievement("achievement.buildBetterPickaxe", "buildBetterPickaxe", 6, 2, Items.stone_pickaxe, field_187985_i)).registerStat();

    /** Is the 'delicious fish' achievement. */
    public static final Achievement cookFish = (new Achievement("achievement.cookFish", "cookFish", 2, 6, Items.cooked_fish, field_187986_j)).registerStat();
    public static final Achievement field_187990_q = (new Achievement("achievement.onARail", "onARail", 2, 3, Blocks.rail, field_187987_k)).setSpecial().registerStat();
    public static final Achievement field_187991_r = (new Achievement("achievement.buildSword", "buildSword", 6, -1, Items.wooden_sword, field_187984_h)).registerStat();
    public static final Achievement field_187992_s = (new Achievement("achievement.killEnemy", "killEnemy", 8, -1, Items.bone, field_187991_r)).registerStat();

    /** is the 'cow tipper' achievement. */
    public static final Achievement killCow = (new Achievement("achievement.killCow", "killCow", 7, -3, Items.leather, field_187991_r)).registerStat();
    public static final Achievement field_187993_u = (new Achievement("achievement.flyPig", "flyPig", 9, -3, Items.saddle, killCow)).setSpecial().registerStat();
    public static final Achievement field_187994_v = (new Achievement("achievement.snipeSkeleton", "snipeSkeleton", 7, 0, Items.bow, field_187992_s)).setSpecial().registerStat();
    public static final Achievement field_187995_w = (new Achievement("achievement.diamonds", "diamonds", -1, 5, Blocks.diamond_ore, field_187987_k)).registerStat();
    public static final Achievement field_187996_x = (new Achievement("achievement.diamondsToYou", "diamondsToYou", -1, 2, Items.diamond, field_187995_w)).registerStat();
    public static final Achievement field_187997_y = (new Achievement("achievement.portal", "portal", -1, 7, Blocks.obsidian, field_187995_w)).registerStat();

    /** Is the 'Return to Sender' achievement */
    public static final Achievement ghast = (new Achievement("achievement.ghast", "ghast", -4, 8, Items.ghast_tear, field_187997_y)).setSpecial().registerStat();
    public static final Achievement field_187969_A = (new Achievement("achievement.blazeRod", "blazeRod", 0, 9, Items.blaze_rod, field_187997_y)).registerStat();
    public static final Achievement field_187970_B = (new Achievement("achievement.potion", "potion", 2, 8, Items.potionitem, field_187969_A)).registerStat();

    /** Is the 'The End?' achievement */
    public static final Achievement theEnd = (new Achievement("achievement.theEnd", "theEnd", 3, 10, Items.ender_eye, field_187969_A)).setSpecial().registerStat();
    public static final Achievement field_187971_D = (new Achievement("achievement.theEnd2", "theEnd2", 4, 13, Blocks.dragon_egg, theEnd)).setSpecial().registerStat();
    public static final Achievement field_187972_E = (new Achievement("achievement.enchantments", "enchantments", -4, 4, Blocks.enchanting_table, field_187995_w)).registerStat();
    public static final Achievement field_187973_F = (new Achievement("achievement.overkill", "overkill", -4, 1, Items.diamond_sword, field_187972_E)).setSpecial().registerStat();
    public static final Achievement field_187974_G = (new Achievement("achievement.bookcase", "bookcase", -3, 6, Blocks.bookshelf, field_187972_E)).registerStat();
    public static final Achievement field_187975_H = (new Achievement("achievement.breedCow", "breedCow", 7, -5, Items.wheat, killCow)).registerStat();
    public static final Achievement field_187976_I = (new Achievement("achievement.spawnWither", "spawnWither", 7, 12, new ItemStack(Items.skull, 1, 1), field_187971_D)).registerStat();
    public static final Achievement field_187977_J = (new Achievement("achievement.killWither", "killWither", 7, 10, Items.nether_star, field_187976_I)).registerStat();
    public static final Achievement field_187978_K = (new Achievement("achievement.fullBeacon", "fullBeacon", 7, 8, Blocks.beacon, field_187977_J)).setSpecial().registerStat();
    public static final Achievement field_187979_L = (new Achievement("achievement.exploreAllBiomes", "exploreAllBiomes", 4, 8, Items.diamond_boots, theEnd)).func_150953_b(JsonSerializableSet.class).setSpecial().registerStat();
    public static final Achievement field_187980_M = (new Achievement("achievement.overpowered", "overpowered", 6, 4, new ItemStack(Items.golden_apple, 1, 1), field_187989_o)).setSpecial().registerStat();

    /**
     * A stub functions called to make the static initializer for this class run.
     */
    public static void init()
    {
    }
}
