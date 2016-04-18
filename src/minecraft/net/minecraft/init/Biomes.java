package net.minecraft.init;

import net.minecraft.util.ResourceLocation;
import net.minecraft.world.biome.BiomeGenBase;

public abstract class Biomes
{
    public static final BiomeGenBase ocean;
    public static final BiomeGenBase DEFAULT;
    public static final BiomeGenBase plains;
    public static final BiomeGenBase desert;
    public static final BiomeGenBase extremeHills;
    public static final BiomeGenBase forest;
    public static final BiomeGenBase taiga;
    public static final BiomeGenBase swampland;
    public static final BiomeGenBase river;
    public static final BiomeGenBase hell;

    /** Is the biome used for sky world. */
    public static final BiomeGenBase sky;
    public static final BiomeGenBase frozenOcean;
    public static final BiomeGenBase frozenRiver;
    public static final BiomeGenBase icePlains;
    public static final BiomeGenBase iceMountains;
    public static final BiomeGenBase mushroomIsland;
    public static final BiomeGenBase mushroomIslandShore;

    /** Beach biome. */
    public static final BiomeGenBase beach;

    /** Desert Hills biome. */
    public static final BiomeGenBase desertHills;

    /** Forest Hills biome. */
    public static final BiomeGenBase forestHills;

    /** Taiga Hills biome. */
    public static final BiomeGenBase taigaHills;

    /** Extreme Hills Edge biome. */
    public static final BiomeGenBase extremeHillsEdge;

    /** Jungle biome identifier */
    public static final BiomeGenBase jungle;
    public static final BiomeGenBase jungleHills;
    public static final BiomeGenBase jungleEdge;
    public static final BiomeGenBase deepOcean;
    public static final BiomeGenBase stoneBeach;
    public static final BiomeGenBase coldBeach;
    public static final BiomeGenBase birchForest;
    public static final BiomeGenBase birchForestHills;
    public static final BiomeGenBase roofedForest;
    public static final BiomeGenBase coldTaiga;
    public static final BiomeGenBase coldTaigaHills;
    public static final BiomeGenBase megaTaiga;
    public static final BiomeGenBase megaTaigaHills;
    public static final BiomeGenBase extremeHillsPlus;
    public static final BiomeGenBase savanna;
    public static final BiomeGenBase savannaPlateau;
    public static final BiomeGenBase mesa;
    public static final BiomeGenBase mesaPlateau_F;
    public static final BiomeGenBase mesaPlateau;
    public static final BiomeGenBase voidBiome;
    public static final BiomeGenBase mutated_plains;
    public static final BiomeGenBase mutated_desert;
    public static final BiomeGenBase mutated_extreme_hills;
    public static final BiomeGenBase mutated_forest;
    public static final BiomeGenBase mutated_taiga;
    public static final BiomeGenBase mutated_swampland;
    public static final BiomeGenBase mutated_ice_flats;
    public static final BiomeGenBase mutated_jungle;
    public static final BiomeGenBase mutated_jungle_edge;
    public static final BiomeGenBase mutated_birch_forest;
    public static final BiomeGenBase mutated_birch_forest_hills;
    public static final BiomeGenBase mutated_roofed_forest;
    public static final BiomeGenBase mutated_taiga_cold;
    public static final BiomeGenBase mutated_redwood_taiga;
    public static final BiomeGenBase mutated_redwood_taiga_hills;
    public static final BiomeGenBase mutated_extreme_hills_with_trees;
    public static final BiomeGenBase mutated_savanna;
    public static final BiomeGenBase mutated_savanna_rock;
    public static final BiomeGenBase mutated_mesa;
    public static final BiomeGenBase mutated_mesa_rock;
    public static final BiomeGenBase mutated_mesa_clear_rock;

    private static BiomeGenBase getRegisteredBiome(String id)
    {
        BiomeGenBase biomegenbase = (BiomeGenBase)BiomeGenBase.biomeRegistry.getObject(new ResourceLocation(id));

        if (biomegenbase == null)
        {
            throw new IllegalStateException("Invalid Biome requested: " + id);
        }
        else
        {
            return biomegenbase;
        }
    }

    static
    {
        if (!Bootstrap.isRegistered())
        {
            throw new RuntimeException("Accessed Biomes before Bootstrap!");
        }
        else
        {
            ocean = getRegisteredBiome("ocean");
            DEFAULT = ocean;
            plains = getRegisteredBiome("plains");
            desert = getRegisteredBiome("desert");
            extremeHills = getRegisteredBiome("extreme_hills");
            forest = getRegisteredBiome("forest");
            taiga = getRegisteredBiome("taiga");
            swampland = getRegisteredBiome("swampland");
            river = getRegisteredBiome("river");
            hell = getRegisteredBiome("hell");
            sky = getRegisteredBiome("sky");
            frozenOcean = getRegisteredBiome("frozen_ocean");
            frozenRiver = getRegisteredBiome("frozen_river");
            icePlains = getRegisteredBiome("ice_flats");
            iceMountains = getRegisteredBiome("ice_mountains");
            mushroomIsland = getRegisteredBiome("mushroom_island");
            mushroomIslandShore = getRegisteredBiome("mushroom_island_shore");
            beach = getRegisteredBiome("beaches");
            desertHills = getRegisteredBiome("desert_hills");
            forestHills = getRegisteredBiome("forest_hills");
            taigaHills = getRegisteredBiome("taiga_hills");
            extremeHillsEdge = getRegisteredBiome("smaller_extreme_hills");
            jungle = getRegisteredBiome("jungle");
            jungleHills = getRegisteredBiome("jungle_hills");
            jungleEdge = getRegisteredBiome("jungle_edge");
            deepOcean = getRegisteredBiome("deep_ocean");
            stoneBeach = getRegisteredBiome("stone_beach");
            coldBeach = getRegisteredBiome("cold_beach");
            birchForest = getRegisteredBiome("birch_forest");
            birchForestHills = getRegisteredBiome("birch_forest_hills");
            roofedForest = getRegisteredBiome("roofed_forest");
            coldTaiga = getRegisteredBiome("taiga_cold");
            coldTaigaHills = getRegisteredBiome("taiga_cold_hills");
            megaTaiga = getRegisteredBiome("redwood_taiga");
            megaTaigaHills = getRegisteredBiome("redwood_taiga_hills");
            extremeHillsPlus = getRegisteredBiome("extreme_hills_with_trees");
            savanna = getRegisteredBiome("savanna");
            savannaPlateau = getRegisteredBiome("savanna_rock");
            mesa = getRegisteredBiome("mesa");
            mesaPlateau_F = getRegisteredBiome("mesa_rock");
            mesaPlateau = getRegisteredBiome("mesa_clear_rock");
            voidBiome = getRegisteredBiome("void");
            mutated_plains = getRegisteredBiome("mutated_plains");
            mutated_desert = getRegisteredBiome("mutated_desert");
            mutated_extreme_hills = getRegisteredBiome("mutated_extreme_hills");
            mutated_forest = getRegisteredBiome("mutated_forest");
            mutated_taiga = getRegisteredBiome("mutated_taiga");
            mutated_swampland = getRegisteredBiome("mutated_swampland");
            mutated_ice_flats = getRegisteredBiome("mutated_ice_flats");
            mutated_jungle = getRegisteredBiome("mutated_jungle");
            mutated_jungle_edge = getRegisteredBiome("mutated_jungle_edge");
            mutated_birch_forest = getRegisteredBiome("mutated_birch_forest");
            mutated_birch_forest_hills = getRegisteredBiome("mutated_birch_forest_hills");
            mutated_roofed_forest = getRegisteredBiome("mutated_roofed_forest");
            mutated_taiga_cold = getRegisteredBiome("mutated_taiga_cold");
            mutated_redwood_taiga = getRegisteredBiome("mutated_redwood_taiga");
            mutated_redwood_taiga_hills = getRegisteredBiome("mutated_redwood_taiga_hills");
            mutated_extreme_hills_with_trees = getRegisteredBiome("mutated_extreme_hills_with_trees");
            mutated_savanna = getRegisteredBiome("mutated_savanna");
            mutated_savanna_rock = getRegisteredBiome("mutated_savanna_rock");
            mutated_mesa = getRegisteredBiome("mutated_mesa");
            mutated_mesa_rock = getRegisteredBiome("mutated_mesa_rock");
            mutated_mesa_clear_rock = getRegisteredBiome("mutated_mesa_clear_rock");
        }
    }
}
