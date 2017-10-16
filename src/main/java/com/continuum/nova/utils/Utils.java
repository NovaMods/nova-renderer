package com.continuum.nova.utils;

import net.minecraft.util.ResourceLocation;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.util.List;

public class Utils {
    private Utils() { }

    static boolean exists(Object objectToCheck) {
        return objectToCheck != null;
    }

    public static void initGuiTextureLocations(List<ResourceLocation> locations) {
        addResources(locations,
                "gui/bars", "gui/book", "gui/demo_background",
                "gui/icons", "gui/resource_packs", "gui/server_selection",
                "gui/spectator_widgets", "gui/widgets", "gui/world_selection",

                // "gui/stream_indicator" // non-square texture, breaks

                "gui/achievement/achievement_background", "gui/achievement/achievement_icons",

                "gui/container/anvil", "gui/container/beacon", "gui/container/brewing_stand",
                "gui/container/crafting_table", "gui/container/dispenser", "gui/container/enchanting_table",
                "gui/container/furnace", "gui/container/generic_54", "gui/container/hopper", "gui/container/horse",
                "gui/container/inventory", "gui/container/stats_icons", "gui/container/villager",
                "gui/container/creative_inventory/tabs", "gui/container/creative_inventory/tab_inventory",
                "gui/container/creative_inventory/tab_items", "gui/container/creative_inventory/tab_item_search",

                "gui/presets/chaos", "gui/presets/delight",
                "gui/presets/drought", "gui/presets/isles",
                "gui/presets/luck", "gui/presets/madness",
                "gui/presets/water",

                "gui/title/minecraft", "gui/title/mojang",

                "gui/title/background/panorama_0", "gui/title/background/panorama_1",
                "gui/title/background/panorama_2", "gui/title/background/panorama_3",
                "gui/title/background/panorama_4", "gui/title/background/panorama_5"
        );
    }

    public static void initBlockTextureLocations(List<ResourceLocation> locations) {
        addResources(locations,
                "blocks/anvil_base", "blocks/anvil_top_damaged_0",
                "blocks/anvil_top_damaged_1", "blocks/anvil_top_damaged_2",

                "blocks/beacon", "blocks/bedrock",

                "blocks/bed_feet_end", "blocks/bed_feet_side",
                "blocks/bed_feet_top", "blocks/bed_head_end",
                "blocks/bed_head_side", "blocks/bed_head_top",

                "blocks/bookshelf",
                "blocks/brewing_stand", "blocks/brewing_stand_base",

                "blocks/brick", "blocks/brickALT", "blocks/brick_ALT",
                "blocks/cactus_side", "blocks/cactus_top",

                "blocks/cake_bottom", "blocks/cake_inner",
                "blocks/cake_side", "blocks/cake_top",

                "blocks/carrots_stage_0", "blocks/carrots_stage_1",
                "blocks/carrots_stage_2", "blocks/carrots_stage_3",

                "blocks/cauldron_bottom", "blocks/cauldron_inner",
                "blocks/cauldron_side", "blocks/cauldron_top",

                "blocks/clay",
                "blocks/coal_block", "blocks/coal_ore",
                "blocks/coarse_dirt", "blocks/coarse_dirtt",
                "blocks/cobblestone-A", "blocks/cobblestone", "blocks/cobblestone_mossy",

                "blocks/cocoa_stage_0", "blocks/cocoa_stage_1", "blocks/cocoa_stage_2",
                "blocks/command_block", "blocks/command_block.mcmeta", "blocks/command_blockk",
                "blocks/comparator_off", "blocks/comparator_on",
                "blocks/crafting_table_front", "blocks/crafting_table_side", "blocks/crafting_table_top",
                "blocks/daylight_detector_side", "blocks/daylight_detector_top", "blocks/daylight_detector_top2",
                "blocks/deadbush",

                "blocks/destroy_stage_0", "blocks/destroy_stage_1",
                "blocks/destroy_stage_2", "blocks/destroy_stage_3",
                "blocks/destroy_stage_4", "blocks/destroy_stage_5",
                "blocks/destroy_stage_6", "blocks/destroy_stage_7",
                "blocks/destroy_stage_8", "blocks/destroy_stage_9",

                "blocks/diamond_block", "blocks/diamond_ore",
                "blocks/dirt", "blocks/dirt_podzol_side", "blocks/dirt_podzol_top",
                "blocks/dispenser_front_horizontal", "blocks/dispenser_front_vertical",
                "blocks/door_iron_lower", "blocks/door_iron_upper",
                "blocks/door_wood_lower", "blocks/door_wood_upper",
                "blocks/doubleflower_paeonia",

                "blocks/double_plant_fern_bottom", "blocks/double_plant_fern_top",
                "blocks/double_plant_grass_bottom", "blocks/double_plant_grass_top",
                "blocks/double_plant_paeonia_bottom", "blocks/double_plant_paeonia_top",
                "blocks/double_plant_rose_bottom", "blocks/double_plant_rose_top",
                "blocks/double_plant_sunflower_back", "blocks/double_plant_sunflower_bottom",
                "blocks/double_plant_sunflower_front", "blocks/double_plant_sunflower_top",
                "blocks/double_plant_syringa_bottom", "blocks/double_plant_syringa_top",

                "blocks/dragon_egg",

                "blocks/dropper_front_horizontal", "blocks/dropper_front_vertical",
                "blocks/emerald_block", "blocks/emerald_ore", "blocks/emerald_ore.mcmeta",
                "blocks/enchanting_table_bottom", "blocks/enchanting_table_side", "blocks/enchanting_table_top",
                "blocks/endframe_eye", "blocks/endframe_side", "blocks/endframe_top",
                "blocks/end_stone",
                "blocks/farmland_dry", "blocks/farmland_wet",

                "blocks/fern",

                "blocks/fire_layer_0", "blocks/fire_layer_0.mcmeta",
                "blocks/fire_layer_00", "blocks/fire_layer_00.mcmeta",
                "blocks/fire_layer_1", "blocks/fire_layer_1.mcmeta",
                "blocks/fire_layer_11.mcmeta", "blocks/fire_layer_11",

                "blocks/flower_allium", "blocks/flower_blue_orchid", "blocks/flower_dandelion",
                "blocks/flower_houstonia", "blocks/flower_oxeye_daisy", "blocks/flower_pot",
                "blocks/flower_rose", "blocks/flower_tulip_orange", "blocks/flower_tulip_pink",
                "blocks/flower_tulip_red", "blocks/flower_tulip_white",

                "blocks/furnace_front_off", "blocks/furnace_front_on",
                "blocks/furnace_side", "blocks/furnace_top",
                "blocks/gggrass_side_snowed", "blocks/ggold_block",
                "blocks/ggrass_side", "blocks/ggrass_side_overlay", "blocks/ggrass_top",

                "blocks/glass", "blocks/glass_black", "blocks/glass_blue",
                "blocks/glass_brown", "blocks/glass_cyan", "blocks/glass_gray",
                "blocks/glass_green", "blocks/glass_light_blue", "blocks/glass_lime",
                "blocks/glass_magenta", "blocks/glass_orange", "blocks/glass_pane_top",
                "blocks/glass_pink", "blocks/glass_purple", "blocks/glass_red",
                "blocks/glass_silver", "blocks/glass_white", "blocks/glass_yellow",

                "blocks/glowstone", "blocks/glowstone.mcmeta",
                "blocks/gold_block", "blocks/gold_ore",
                "blocks/grass_side", "blocks/grass_side_overlay", "blocks/grass_side_overlayALT",
                "blocks/grass_side_snowed", "blocks/grass_top", "blocks/grass_topALT",
                "blocks/gravel",

                "blocks/hardened_clay", "blocks/hardened_clay_stained_black",
                "blocks/hardened_clay_stained_blue", "blocks/hardened_clay_stained_brown",
                "blocks/hardened_clay_stained_cyan", "blocks/hardened_clay_stained_gray",
                "blocks/hardened_clay_stained_green", "blocks/hardened_clay_stained_light_blue",
                "blocks/hardened_clay_stained_lime", "blocks/hardened_clay_stained_magenta",
                "blocks/hardened_clay_stained_orange", "blocks/hardened_clay_stained_pink",
                "blocks/hardened_clay_stained_purple", "blocks/hardened_clay_stained_red",
                "blocks/hardened_clay_stained_silver", "blocks/hardened_clay_stained_white",
                "blocks/hardened_clay_stained_yellow",

                "blocks/hay_block_side", "blocks/hay_block_top",
                "blocks/hopper_inside", "blocks/hopper_outside", "blocks/hopper_top",
                "blocks/ice", "blocks/ice_packed",
                "blocks/iiron_block", "blocks/iron_bars", "blocks/iron_block",
                "blocks/iron_ore", "blocks/iron_trapdoor", "blocks/itemframe_background",
                "blocks/jukebox_side", "blocks/jukebox_top", "blocks/jukebox_top.mcmeta",
                "blocks/ladder", "blocks/lapis_block", "blocks/lapis_ore",
                "blocks/lava_flow", "blocks/lava_flow.mcmeta",
                "blocks/lava_still", "blocks/lava_still.mcmeta",

                "blocks/leaves_acacia", "blocks/leaves_big_oak",
                "blocks/leaves_birch", "blocks/leaves_jungle",
                "blocks/leaves_oak", "blocks/leaves_spruce",

                "blocks/lever",

                "blocks/log_acacia", "blocks/log_acacia_top",
                "blocks/log_big_oak", "blocks/log_big_oak_top",
                "blocks/log_birch", "blocks/log_birch_top",
                "blocks/log_jungle", "blocks/log_junglee", "blocks/log_jungle_top",
                "blocks/log_oak", "blocks/log_oak_top",
                "blocks/log_spruce", "blocks/log_spruce_top",

                "blocks/melon_side", "blocks/melon_stem_connected",
                "blocks/melon_stem_disconnected", "blocks/melon_top",
                "blocks/mob_spawner",

                "blocks/mushroom_block_inside", "blocks/mushroom_block_skin_brown",
                "blocks/mushroom_block_skin_red", "blocks/mushroom_block_skin_stem",
                "blocks/mushroom_brown", "blocks/mushroom_red",

                "blocks/mycelium_side", "blocks/mycelium_top",
                "blocks/netherrack", "blocks/nether_brick",
                "blocks/nether_wart_stage_0", "blocks/nether_wart_stage_1", "blocks/nether_wart_stage_2",
                "blocks/noteblock", "blocks/obsidian",

                "blocks/piston_bottom", "blocks/piston_inner",
                "blocks/piston_side", "blocks/piston_top_normal",
                "blocks/piston_top_sticky",

                "blocks/planks_acacia", "blocks/planks_big_oak",
                "blocks/planks_birch", "blocks/planks_jungle",
                "blocks/planks_oak", "blocks/planks_spruce",

                "blocks/portal", "blocks/portal.mcmeta",
                "blocks/potatoes_stage_0", "blocks/potatoes_stage_1",
                "blocks/potatoes_stage_2", "blocks/potatoes_stage_3",

                "blocks/prismarine_bricks", "blocks/prismarine_dark",
                "blocks/prismarine_rough", "blocks/prismarine_rough.mcmeta",
                "blocks/prismarine_roughh",

                "blocks/pumpkin_face_off", "blocks/pumpkin_face_offALT",
                "blocks/pumpkin_face_on", "blocks/pumpkin_side",
                "blocks/pumpkin_stem_connected", "blocks/pumpkin_stem_disconnected",
                "blocks/pumpkin_top",

                "blocks/qquartz_block_bottom", "blocks/quartz_block_bottom",
                "blocks/quartz_block_chiseled", "blocks/quartz_block_chiseled_top",
                "blocks/quartz_block_lines", "blocks/quartz_block_lines_top",
                "blocks/quartz_block_side", "blocks/quartz_block_top",
                "blocks/quartz_ore",

                "blocks/rail_activator", "blocks/rail_activator_powered",
                "blocks/rail_detector", "blocks/rail_detector_powered",
                "blocks/rail_golden", "blocks/rail_golden_powered",
                "blocks/rail_normal", "blocks/rail_normal_turned",

                "blocks/redstone_block",
                "blocks/redstone_dust_cross", "blocks/redstone_dust_cross_overlay",
                "blocks/redstone_dust_line", "blocks/redstone_dust_line_overlay",
                "blocks/redstone_lamp_off", "blocks/redstone_lamp_on",
                "blocks/redstone_ore", "blocks/redstone_ore.mcmeta",
                "blocks/redstone_torch_off", "blocks/redstone_torch_on",

                "blocks/red_sand", "blocks/red_sandstone_bottom",
                "blocks/red_sandstone_carved", "blocks/red_sandstone_normal",
                "blocks/red_sandstone_smooth", "blocks/red_sandstone_top",

                "blocks/reeds",
                "blocks/repeater_off", "blocks/repeater_on",

                "blocks/sand", "blocks/sandstone_bottom",
                "blocks/sandstone_carved", "blocks/sandstone_normal",
                "blocks/sandstone_smooth", "blocks/sandstone_top",

                "blocks/sapling_acacia", "blocks/sapling_birch",
                "blocks/sapling_jungle", "blocks/sapling_oak",
                "blocks/sapling_roofed_oak", "blocks/sapling_spruce",

                "blocks/sea_lantern", "blocks/slime", "blocks/snow",
                "blocks/soul_sand", "blocks/soul_sand.mcmeta",
                "blocks/sponge", "blocks/spongeALT1", "blocks/sponge_wet",
                "blocks/sstone", "blocks/stone", "blocks/stone1",
                "blocks/stonebrick", "blocks/stonebrickALT", "blocks/stonebrick_carved",
                "blocks/stonebrick_cracked", "blocks/stonebrick_crackedALT",
                "blocks/stonebrick_mossy", "blocks/stonebrick_mossyALT",

                "blocks/stone_andesite", "blocks/stone_andesite_smooth",
                "blocks/stone_diorite", "blocks/stone_diorite_smooth",
                "blocks/stone_granite", "blocks/stone_granite_smooth",

                "blocks/stone_slab_side", "blocks/stone_slab_top",
                "blocks/tallgrass",
                "blocks/tnt_bottom", "blocks/tnt_side", "blocks/tnt_top",
                "blocks/torch_on", "blocks/torch_on.mcmeta",
                "blocks/trapdoor",
                "blocks/trip_wire", "blocks/trip_wire_source",
                "blocks/vine", "blocks/waterlily",
                "blocks/water_flow", "blocks/water_flow.mcmeta",
                "blocks/water_still", "blocks/water_still.mcmeta",
                "blocks/web",

                "blocks/wheat_stage_0", "blocks/wheat_stage_1",
                "blocks/wheat_stage_2", "blocks/wheat_stage_3",
                "blocks/wheat_stage_4", "blocks/wheat_stage_5",
                "blocks/wheat_stage_6", "blocks/wheat_stage_7",

                "blocks/wool_colored_black", "blocks/wool_colored_blue",
                "blocks/wool_colored_brown", "blocks/wool_colored_cyan",
                "blocks/wool_colored_gray", "blocks/wool_colored_green",
                "blocks/wool_colored_light_blue", "blocks/wool_colored_lime",
                "blocks/wool_colored_magenta", "blocks/wool_colored_orange",
                "blocks/wool_colored_pink", "blocks/wool_colored_purple",
                "blocks/wool_colored_red", "blocks/wool_colored_silver",
                "blocks/wool_colored_white", "blocks/wool_colored_yellow",
                "blocks/wwool_colored_pink"
        );
    }

    public static void initFontTextureLocations(List<ResourceLocation> locations) {
        addResources(locations,
                "font/ascii", "font/ascii_sga",

                "font/unicode_page_00", "font/unicode_page_01", "font/unicode_page_02", "font/unicode_page_03",
                "font/unicode_page_04", "font/unicode_page_05", "font/unicode_page_06", "font/unicode_page_07",
                "font/unicode_page_09", "font/unicode_page_0a", "font/unicode_page_0b", "font/unicode_page_0c",
                "font/unicode_page_0d", "font/unicode_page_0e", "font/unicode_page_0f", "font/unicode_page_10",
                "font/unicode_page_11", "font/unicode_page_12", "font/unicode_page_13", "font/unicode_page_14",
                "font/unicode_page_15", "font/unicode_page_16", "font/unicode_page_17", "font/unicode_page_18",
                "font/unicode_page_19", "font/unicode_page_1a", "font/unicode_page_1b", "font/unicode_page_1c",
                "font/unicode_page_1d", "font/unicode_page_1e", "font/unicode_page_1f", "font/unicode_page_20",
                "font/unicode_page_21", "font/unicode_page_22", "font/unicode_page_23", "font/unicode_page_24",
                "font/unicode_page_25", "font/unicode_page_26", "font/unicode_page_27", "font/unicode_page_28",
                "font/unicode_page_29", "font/unicode_page_2a", "font/unicode_page_2b", "font/unicode_page_2c",
                "font/unicode_page_2d", "font/unicode_page_2e", "font/unicode_page_2f", "font/unicode_page_30",
                "font/unicode_page_31", "font/unicode_page_32", "font/unicode_page_33", "font/unicode_page_34",
                "font/unicode_page_35", "font/unicode_page_36", "font/unicode_page_37", "font/unicode_page_38",
                "font/unicode_page_39", "font/unicode_page_3a", "font/unicode_page_3b", "font/unicode_page_3c",
                "font/unicode_page_3d", "font/unicode_page_3e", "font/unicode_page_3f", "font/unicode_page_40",
                "font/unicode_page_41", "font/unicode_page_42", "font/unicode_page_43", "font/unicode_page_44",
                "font/unicode_page_45", "font/unicode_page_46", "font/unicode_page_47", "font/unicode_page_48",
                "font/unicode_page_49", "font/unicode_page_4a", "font/unicode_page_4b", "font/unicode_page_4c",
                "font/unicode_page_4d", "font/unicode_page_4e", "font/unicode_page_4f", "font/unicode_page_50",
                "font/unicode_page_51", "font/unicode_page_52", "font/unicode_page_53", "font/unicode_page_54",
                "font/unicode_page_55", "font/unicode_page_56", "font/unicode_page_57", "font/unicode_page_58",
                "font/unicode_page_59", "font/unicode_page_5a", "font/unicode_page_5b", "font/unicode_page_5c",
                "font/unicode_page_5d", "font/unicode_page_5e", "font/unicode_page_5f", "font/unicode_page_60",
                "font/unicode_page_61", "font/unicode_page_62", "font/unicode_page_63", "font/unicode_page_64",
                "font/unicode_page_65", "font/unicode_page_66", "font/unicode_page_67", "font/unicode_page_68",
                "font/unicode_page_69", "font/unicode_page_6a", "font/unicode_page_6b", "font/unicode_page_6c",
                "font/unicode_page_6d", "font/unicode_page_6e", "font/unicode_page_6f", "font/unicode_page_70",
                "font/unicode_page_71", "font/unicode_page_72", "font/unicode_page_73", "font/unicode_page_74",
                "font/unicode_page_75", "font/unicode_page_76", "font/unicode_page_77", "font/unicode_page_78",
                "font/unicode_page_79", "font/unicode_page_7a", "font/unicode_page_7b", "font/unicode_page_7c",
                "font/unicode_page_7d", "font/unicode_page_7e", "font/unicode_page_7f", "font/unicode_page_80",
                "font/unicode_page_81", "font/unicode_page_82", "font/unicode_page_83", "font/unicode_page_84",
                "font/unicode_page_85", "font/unicode_page_86", "font/unicode_page_87", "font/unicode_page_88",
                "font/unicode_page_89", "font/unicode_page_8a", "font/unicode_page_8b", "font/unicode_page_8c",
                "font/unicode_page_8d", "font/unicode_page_8e", "font/unicode_page_8f", "font/unicode_page_90",
                "font/unicode_page_91", "font/unicode_page_92", "font/unicode_page_93", "font/unicode_page_94",
                "font/unicode_page_95", "font/unicode_page_96", "font/unicode_page_97", "font/unicode_page_98",
                "font/unicode_page_99", "font/unicode_page_9a", "font/unicode_page_9b", "font/unicode_page_9c",
                "font/unicode_page_9d", "font/unicode_page_9e", "font/unicode_page_9f", "font/unicode_page_a0",
                "font/unicode_page_a1", "font/unicode_page_a2", "font/unicode_page_a3", "font/unicode_page_a4",
                "font/unicode_page_a5", "font/unicode_page_a6", "font/unicode_page_a7", "font/unicode_page_a8",
                "font/unicode_page_a9", "font/unicode_page_aa", "font/unicode_page_ab", "font/unicode_page_ac",
                "font/unicode_page_ad", "font/unicode_page_ae", "font/unicode_page_af", "font/unicode_page_b0",
                "font/unicode_page_b1", "font/unicode_page_b2", "font/unicode_page_b3", "font/unicode_page_b4",
                "font/unicode_page_b5", "font/unicode_page_b6", "font/unicode_page_b7", "font/unicode_page_b8",
                "font/unicode_page_b9", "font/unicode_page_ba", "font/unicode_page_bb", "font/unicode_page_bc",
                "font/unicode_page_bd", "font/unicode_page_be", "font/unicode_page_bf", "font/unicode_page_c0",
                "font/unicode_page_c1", "font/unicode_page_c2", "font/unicode_page_c3", "font/unicode_page_c4",
                "font/unicode_page_c5", "font/unicode_page_c6", "font/unicode_page_c7", "font/unicode_page_c8",
                "font/unicode_page_c9", "font/unicode_page_ca", "font/unicode_page_cb", "font/unicode_page_cc",
                "font/unicode_page_cd", "font/unicode_page_ce", "font/unicode_page_cf", "font/unicode_page_d0",
                "font/unicode_page_d1", "font/unicode_page_d2", "font/unicode_page_d3", "font/unicode_page_d4",
                "font/unicode_page_d5", "font/unicode_page_d6", "font/unicode_page_d7", "font/unicode_page_f9",
                "font/unicode_page_fa", "font/unicode_page_fb", "font/unicode_page_fc", "font/unicode_page_fd",
                "font/unicode_page_fe", "font/unicode_page_ff"
        );
    }

    public static void initFreeTextures(List<ResourceLocation> locations) {
        addResources(locations, "textures/misc/unknown_server.png", "textures/gui/options_background.png");
    }

    public static byte[] getImageData(BufferedImage image) {
        byte[] convertedImageData = new byte[image.getWidth() * image.getHeight() * 4];
        int counter = 0;
        for (int y = 0; y < image.getHeight(); y++) {
            for (int x = 0; x < image.getWidth(); x++) {

                Color c = new Color(image.getRGB(x, y), image.getColorModel().hasAlpha());

                convertedImageData[counter] = (byte) (c.getRed());
                convertedImageData[counter + 1] = (byte) (c.getGreen());
                convertedImageData[counter + 2] = (byte) (c.getBlue());
                convertedImageData[counter + 3] = (byte) (image.getColorModel().getNumComponents() == 3 ? 255 : c.getAlpha());
                counter += 4;
            }
        }
        return convertedImageData;
    }


    private static void addResources(List<ResourceLocation> locations, String... resourceNames) {
        for (String resourceName : resourceNames)
            locations.add(new ResourceLocation(resourceName));
    }
}
