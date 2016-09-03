package com.continuum.nova;

import com.continuum.nova.utils.AtlasGenerator;
import com.continuum.nova.utils.RenderCommandBuilder;
import com.sun.istack.internal.NotNull;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.renderer.entity.Render;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.util.ResourceLocation;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class NovaRenderer implements IResourceManagerReloadListener {
    public static final String MODID = "Nova Renderer";
    public static final String VERSION = "0.0.3";

    private static final Logger LOG = LogManager.getLogger(NovaRenderer.class);

    private final List<ResourceLocation> GUI_ALBEDO_TEXTURES_LOCATIONS = new ArrayList<ResourceLocation>() {{
            add(new ResourceLocation("textures/gui/widgets.png"));
    }};

    private final List<ResourceLocation> TERRAIN_ALBEDO_TEXTURES_LOCATION = new ArrayList<ResourceLocation>() {{
            //add(new ResourceLocation("textures/blocks/anvil_base.png"));
            //add(new ResourceLocation("textures/blocks/anvil_top_damaged_0.png"));
            //add(new ResourceLocation("textures/blocks/anvil_top_damaged_1.png"));
            //add(new ResourceLocation("textures/blocks/anvil_top_damaged_2.png"));
            add(new ResourceLocation("textures/blocks/beacon.png"));
            add(new ResourceLocation("textures/blocks/bedrock.png"));
            add(new ResourceLocation("textures/blocks/bed_feet_end.png"));
            add(new ResourceLocation("textures/blocks/bed_feet_side.png"));
            add(new ResourceLocation("textures/blocks/bed_feet_top.png"));
            add(new ResourceLocation("textures/blocks/bed_head_end.png"));
            add(new ResourceLocation("textures/blocks/bed_head_side.png"));
            add(new ResourceLocation("textures/blocks/bed_head_top.png"));
            add(new ResourceLocation("textures/blocks/bookshelf.png"));
            add(new ResourceLocation("textures/blocks/brewing_stand.png"));
            add(new ResourceLocation("textures/blocks/brewing_stand_base.png"));
            add(new ResourceLocation("textures/blocks/brick.png"));
            add(new ResourceLocation("textures/blocks/brickALT.png"));
            add(new ResourceLocation("textures/blocks/brick_ALT.png"));
            add(new ResourceLocation("textures/blocks/cactus_side.png"));
            add(new ResourceLocation("textures/blocks/cactus_top.png"));
            add(new ResourceLocation("textures/blocks/cake_bottom.png"));
            add(new ResourceLocation("textures/blocks/cake_inner.png"));
            add(new ResourceLocation("textures/blocks/cake_side.png"));
            add(new ResourceLocation("textures/blocks/cake_top.png"));
            add(new ResourceLocation("textures/blocks/carrots_stage_0.png"));
            add(new ResourceLocation("textures/blocks/carrots_stage_1.png"));
            add(new ResourceLocation("textures/blocks/carrots_stage_2.png"));
            add(new ResourceLocation("textures/blocks/carrots_stage_3.png"));
            add(new ResourceLocation("textures/blocks/cauldron_bottom.png"));
            add(new ResourceLocation("textures/blocks/cauldron_inner.png"));
            add(new ResourceLocation("textures/blocks/cauldron_side.png"));
            add(new ResourceLocation("textures/blocks/cauldron_top.png"));
            add(new ResourceLocation("textures/blocks/clay.png"));
            add(new ResourceLocation("textures/blocks/coal_block.png"));
            add(new ResourceLocation("textures/blocks/coal_ore.png"));
            add(new ResourceLocation("textures/blocks/coarse_dirt.png"));
            add(new ResourceLocation("textures/blocks/coarse_dirtt.png"));
            add(new ResourceLocation("textures/blocks/cobblestone-A.png"));
            add(new ResourceLocation("textures/blocks/cobblestone.png"));
            add(new ResourceLocation("textures/blocks/cobblestone_mossy.png"));
            add(new ResourceLocation("textures/blocks/cocoa_stage_0.png"));
            add(new ResourceLocation("textures/blocks/cocoa_stage_1.png"));
            add(new ResourceLocation("textures/blocks/cocoa_stage_2.png"));
            add(new ResourceLocation("textures/blocks/command_block.png"));
            add(new ResourceLocation("textures/blocks/command_block.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/command_blockk.png"));
            add(new ResourceLocation("textures/blocks/comparator_off.png"));
            add(new ResourceLocation("textures/blocks/comparator_on.png"));
            add(new ResourceLocation("textures/blocks/crafting_table_front.png"));
            add(new ResourceLocation("textures/blocks/crafting_table_side.png"));
            add(new ResourceLocation("textures/blocks/crafting_table_top.png"));
            add(new ResourceLocation("textures/blocks/daylight_detector_side.png"));
            add(new ResourceLocation("textures/blocks/daylight_detector_top.png"));
            add(new ResourceLocation("textures/blocks/daylight_detector_top2.png"));
            add(new ResourceLocation("textures/blocks/deadbush.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_0.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_1.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_2.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_3.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_4.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_5.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_6.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_7.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_8.png"));
            add(new ResourceLocation("textures/blocks/destroy_stage_9.png"));
            add(new ResourceLocation("textures/blocks/diamond_block.png"));
            add(new ResourceLocation("textures/blocks/diamond_ore.png"));
            add(new ResourceLocation("textures/blocks/dirt.png"));
            add(new ResourceLocation("textures/blocks/dirt_podzol_side.png"));
            add(new ResourceLocation("textures/blocks/dirt_podzol_top.png"));
            add(new ResourceLocation("textures/blocks/dispenser_front_horizontal.png"));
            add(new ResourceLocation("textures/blocks/dispenser_front_vertical.png"));
            add(new ResourceLocation("textures/blocks/door_iron_lower.png"));
            add(new ResourceLocation("textures/blocks/door_iron_upper.png"));
            add(new ResourceLocation("textures/blocks/door_wood_lower.png"));
            add(new ResourceLocation("textures/blocks/door_wood_upper.png"));
            add(new ResourceLocation("textures/blocks/doubleflower_paeonia.png.png"));
            add(new ResourceLocation("textures/blocks/double_plant_fern_bottom.png"));
            add(new ResourceLocation("textures/blocks/double_plant_fern_top.png"));
            add(new ResourceLocation("textures/blocks/double_plant_grass_bottom.png"));
            add(new ResourceLocation("textures/blocks/double_plant_grass_top.png"));
            add(new ResourceLocation("textures/blocks/double_plant_paeonia_bottom.png"));
            add(new ResourceLocation("textures/blocks/double_plant_paeonia_top.png"));
            add(new ResourceLocation("textures/blocks/double_plant_rose_bottom.png"));
            add(new ResourceLocation("textures/blocks/double_plant_rose_top.png"));
            add(new ResourceLocation("textures/blocks/double_plant_sunflower_back.png"));
            add(new ResourceLocation("textures/blocks/double_plant_sunflower_bottom.png"));
            add(new ResourceLocation("textures/blocks/double_plant_sunflower_front.png"));
            add(new ResourceLocation("textures/blocks/double_plant_sunflower_top.png"));
            add(new ResourceLocation("textures/blocks/double_plant_syringa_bottom.png"));
            add(new ResourceLocation("textures/blocks/double_plant_syringa_top.png"));
            add(new ResourceLocation("textures/blocks/dragon_egg.png"));
            add(new ResourceLocation("textures/blocks/dropper_front_horizontal.png"));
            add(new ResourceLocation("textures/blocks/dropper_front_vertical.png"));
            add(new ResourceLocation("textures/blocks/emerald_block.png"));
            add(new ResourceLocation("textures/blocks/emerald_ore.png"));
            add(new ResourceLocation("textures/blocks/emerald_ore.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/enchanting_table_bottom.png"));
            add(new ResourceLocation("textures/blocks/enchanting_table_side.png"));
            add(new ResourceLocation("textures/blocks/enchanting_table_top.png"));
            add(new ResourceLocation("textures/blocks/endframe_eye.png"));
            add(new ResourceLocation("textures/blocks/endframe_side.png"));
            add(new ResourceLocation("textures/blocks/endframe_top.png"));
            add(new ResourceLocation("textures/blocks/end_stone.png"));
            add(new ResourceLocation("textures/blocks/farmland_dry.png"));
            add(new ResourceLocation("textures/blocks/farmland_wet.png"));
            add(new ResourceLocation("textures/blocks/fern.png"));
            add(new ResourceLocation("textures/blocks/fire_layer_0.png"));
            add(new ResourceLocation("textures/blocks/fire_layer_0.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/fire_layer_00.png"));
            add(new ResourceLocation("textures/blocks/fire_layer_00.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/fire_layer_1.png"));
            add(new ResourceLocation("textures/blocks/fire_layer_1.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/fire_layer_11.mcmeta"));
            add(new ResourceLocation("textures/blocks/fire_layer_11.png"));
            add(new ResourceLocation("textures/blocks/flower_allium.png"));
            add(new ResourceLocation("textures/blocks/flower_blue_orchid.png"));
            add(new ResourceLocation("textures/blocks/flower_dandelion.png"));
            add(new ResourceLocation("textures/blocks/flower_houstonia.png"));
            add(new ResourceLocation("textures/blocks/flower_oxeye_daisy.png"));
            add(new ResourceLocation("textures/blocks/flower_pot.png"));
            add(new ResourceLocation("textures/blocks/flower_rose.png"));
            add(new ResourceLocation("textures/blocks/flower_tulip_orange.png"));
            add(new ResourceLocation("textures/blocks/flower_tulip_pink.png"));
            add(new ResourceLocation("textures/blocks/flower_tulip_red.png"));
            add(new ResourceLocation("textures/blocks/flower_tulip_white.png"));
            add(new ResourceLocation("textures/blocks/furnace_front_off.png"));
            add(new ResourceLocation("textures/blocks/furnace_front_on.png"));
            add(new ResourceLocation("textures/blocks/furnace_side.png"));
            add(new ResourceLocation("textures/blocks/furnace_top.png"));
            add(new ResourceLocation("textures/blocks/gggrass_side_snowed.png"));
            add(new ResourceLocation("textures/blocks/ggold_block.png"));
            add(new ResourceLocation("textures/blocks/ggrass_side.png"));
            add(new ResourceLocation("textures/blocks/ggrass_side_overlay.png"));
            add(new ResourceLocation("textures/blocks/ggrass_top.png"));
            add(new ResourceLocation("textures/blocks/glass.png"));
            add(new ResourceLocation("textures/blocks/glass_black.png"));
            add(new ResourceLocation("textures/blocks/glass_blue.png"));
            add(new ResourceLocation("textures/blocks/glass_brown.png"));
            add(new ResourceLocation("textures/blocks/glass_cyan.png"));
            add(new ResourceLocation("textures/blocks/glass_gray.png"));
            add(new ResourceLocation("textures/blocks/glass_green.png"));
            add(new ResourceLocation("textures/blocks/glass_light_blue.png"));
            add(new ResourceLocation("textures/blocks/glass_lime.png"));
            add(new ResourceLocation("textures/blocks/glass_magenta.png"));
            add(new ResourceLocation("textures/blocks/glass_orange.png"));
            add(new ResourceLocation("textures/blocks/glass_pane_top.png"));
            add(new ResourceLocation("textures/blocks/glass_pink.png"));
            add(new ResourceLocation("textures/blocks/glass_purple.png"));
            add(new ResourceLocation("textures/blocks/glass_red.png"));
            add(new ResourceLocation("textures/blocks/glass_silver.png"));
            add(new ResourceLocation("textures/blocks/glass_white.png"));
            add(new ResourceLocation("textures/blocks/glass_yellow.png"));
            add(new ResourceLocation("textures/blocks/glowstone.png"));
            add(new ResourceLocation("textures/blocks/glowstone.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/gold_block.png"));
            add(new ResourceLocation("textures/blocks/gold_ore.png"));
            add(new ResourceLocation("textures/blocks/grass_side.png"));
            add(new ResourceLocation("textures/blocks/grass_side_overlay.png"));
            add(new ResourceLocation("textures/blocks/grass_side_overlayALT.png"));
            add(new ResourceLocation("textures/blocks/grass_side_snowed.png"));
            add(new ResourceLocation("textures/blocks/grass_top.png"));
            add(new ResourceLocation("textures/blocks/grass_topALT.png"));
            add(new ResourceLocation("textures/blocks/gravel.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_black.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_blue.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_brown.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_cyan.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_gray.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_green.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_light_blue.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_lime.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_magenta.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_orange.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_pink.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_purple.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_red.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_silver.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_white.png"));
            add(new ResourceLocation("textures/blocks/hardened_clay_stained_yellow.png"));
            add(new ResourceLocation("textures/blocks/hay_block_side.png"));
            add(new ResourceLocation("textures/blocks/hay_block_top.png"));
            add(new ResourceLocation("textures/blocks/hopper_inside.png"));
            add(new ResourceLocation("textures/blocks/hopper_outside.png"));
            add(new ResourceLocation("textures/blocks/hopper_top.png"));
            add(new ResourceLocation("textures/blocks/ice.png"));
            add(new ResourceLocation("textures/blocks/ice_packed.png"));
            add(new ResourceLocation("textures/blocks/iiron_block.png"));
            add(new ResourceLocation("textures/blocks/iron_bars.png"));
            add(new ResourceLocation("textures/blocks/iron_block.png"));
            add(new ResourceLocation("textures/blocks/iron_ore.png"));
            add(new ResourceLocation("textures/blocks/iron_trapdoor.png"));
            add(new ResourceLocation("textures/blocks/itemframe_background.png"));
            add(new ResourceLocation("textures/blocks/jukebox_side.png"));
            add(new ResourceLocation("textures/blocks/jukebox_top.png"));
            add(new ResourceLocation("textures/blocks/jukebox_top.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/ladder.png"));
            add(new ResourceLocation("textures/blocks/lapis_block.png"));
            add(new ResourceLocation("textures/blocks/lapis_ore.png"));
            add(new ResourceLocation("textures/blocks/lava_flow.png"));
            add(new ResourceLocation("textures/blocks/lava_flow.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/lava_still.png"));
            add(new ResourceLocation("textures/blocks/lava_still.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/leaves_acacia.png"));
            add(new ResourceLocation("textures/blocks/leaves_big_oak.png"));
            add(new ResourceLocation("textures/blocks/leaves_birch.png"));
            add(new ResourceLocation("textures/blocks/leaves_jungle.png"));
            add(new ResourceLocation("textures/blocks/leaves_oak.png"));
            add(new ResourceLocation("textures/blocks/leaves_spruce.png"));
            add(new ResourceLocation("textures/blocks/lever.png"));
            add(new ResourceLocation("textures/blocks/log_acacia.png"));
            add(new ResourceLocation("textures/blocks/log_acacia_top.png"));
            add(new ResourceLocation("textures/blocks/log_big_oak.png"));
            add(new ResourceLocation("textures/blocks/log_big_oak_top.png"));
            add(new ResourceLocation("textures/blocks/log_birch.png"));
            add(new ResourceLocation("textures/blocks/log_birch_top.png"));
            add(new ResourceLocation("textures/blocks/log_jungle.png"));
            add(new ResourceLocation("textures/blocks/log_junglee.png"));
            add(new ResourceLocation("textures/blocks/log_jungle_top.png"));
            add(new ResourceLocation("textures/blocks/log_oak.png"));
            add(new ResourceLocation("textures/blocks/log_oak_top.png"));
            add(new ResourceLocation("textures/blocks/log_spruce.png"));
            add(new ResourceLocation("textures/blocks/log_spruce_top.png"));
            add(new ResourceLocation("textures/blocks/melon_side.png"));
            add(new ResourceLocation("textures/blocks/melon_stem_connected.png"));
            add(new ResourceLocation("textures/blocks/melon_stem_disconnected.png"));
            add(new ResourceLocation("textures/blocks/melon_top.png"));
            add(new ResourceLocation("textures/blocks/mob_spawner.png"));
            add(new ResourceLocation("textures/blocks/mushroom_block_inside.png"));
            add(new ResourceLocation("textures/blocks/mushroom_block_skin_brown.png"));
            add(new ResourceLocation("textures/blocks/mushroom_block_skin_red.png"));
            add(new ResourceLocation("textures/blocks/mushroom_block_skin_stem.png"));
            add(new ResourceLocation("textures/blocks/mushroom_brown.png"));
            add(new ResourceLocation("textures/blocks/mushroom_red.png"));
            add(new ResourceLocation("textures/blocks/mycelium_side.png"));
            add(new ResourceLocation("textures/blocks/mycelium_top.png"));
            add(new ResourceLocation("textures/blocks/netherrack.png"));
            add(new ResourceLocation("textures/blocks/nether_brick.png"));
            add(new ResourceLocation("textures/blocks/nether_wart_stage_0.png"));
            add(new ResourceLocation("textures/blocks/nether_wart_stage_1.png"));
            add(new ResourceLocation("textures/blocks/nether_wart_stage_2.png"));
            add(new ResourceLocation("textures/blocks/noteblock.png"));
            add(new ResourceLocation("textures/blocks/obsidian.png"));
            add(new ResourceLocation("textures/blocks/piston_bottom.png"));
            add(new ResourceLocation("textures/blocks/piston_inner.png"));
            add(new ResourceLocation("textures/blocks/piston_side.png"));
            add(new ResourceLocation("textures/blocks/piston_top_normal.png"));
            add(new ResourceLocation("textures/blocks/piston_top_sticky.png"));
            add(new ResourceLocation("textures/blocks/planks_acacia.png"));
            add(new ResourceLocation("textures/blocks/planks_big_oak.png"));
            add(new ResourceLocation("textures/blocks/planks_birch.png"));
            add(new ResourceLocation("textures/blocks/planks_jungle.png"));
            add(new ResourceLocation("textures/blocks/planks_oak.png"));
            add(new ResourceLocation("textures/blocks/planks_spruce.png"));
            add(new ResourceLocation("textures/blocks/portal.png"));
            add(new ResourceLocation("textures/blocks/portal.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/potatoes_stage_0.png"));
            add(new ResourceLocation("textures/blocks/potatoes_stage_1.png"));
            add(new ResourceLocation("textures/blocks/potatoes_stage_2.png"));
            add(new ResourceLocation("textures/blocks/potatoes_stage_3.png"));
            add(new ResourceLocation("textures/blocks/prismarine_bricks.png"));
            add(new ResourceLocation("textures/blocks/prismarine_dark.png"));
            add(new ResourceLocation("textures/blocks/prismarine_rough.png"));
            add(new ResourceLocation("textures/blocks/prismarine_rough.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/prismarine_roughh.png"));
            add(new ResourceLocation("textures/blocks/pumpkin_face_off.png"));
            add(new ResourceLocation("textures/blocks/pumpkin_face_offALT.png"));
            add(new ResourceLocation("textures/blocks/pumpkin_face_on.png"));
            add(new ResourceLocation("textures/blocks/pumpkin_side.png"));
            add(new ResourceLocation("textures/blocks/pumpkin_stem_connected.png"));
            add(new ResourceLocation("textures/blocks/pumpkin_stem_disconnected.png"));
            add(new ResourceLocation("textures/blocks/pumpkin_top.png"));
            add(new ResourceLocation("textures/blocks/qquartz_block_bottom.png"));
            add(new ResourceLocation("textures/blocks/quartz_block_bottom.png"));
            add(new ResourceLocation("textures/blocks/quartz_block_chiseled.png"));
            add(new ResourceLocation("textures/blocks/quartz_block_chiseled_top.png"));
            add(new ResourceLocation("textures/blocks/quartz_block_lines.png"));
            add(new ResourceLocation("textures/blocks/quartz_block_lines_top.png"));
            add(new ResourceLocation("textures/blocks/quartz_block_side.png"));
            add(new ResourceLocation("textures/blocks/quartz_block_top.png"));
            add(new ResourceLocation("textures/blocks/quartz_ore.png"));
            add(new ResourceLocation("textures/blocks/rail_activator.png"));
            add(new ResourceLocation("textures/blocks/rail_activator_powered.png"));
            add(new ResourceLocation("textures/blocks/rail_detector.png"));
            add(new ResourceLocation("textures/blocks/rail_detector_powered.png"));
            add(new ResourceLocation("textures/blocks/rail_golden.png"));
            add(new ResourceLocation("textures/blocks/rail_golden_powered.png"));
            add(new ResourceLocation("textures/blocks/rail_normal.png"));
            add(new ResourceLocation("textures/blocks/rail_normal_turned.png"));
            add(new ResourceLocation("textures/blocks/redstone_block.png"));
            add(new ResourceLocation("textures/blocks/redstone_dust_cross.png"));
            add(new ResourceLocation("textures/blocks/redstone_dust_cross_overlay.png"));
            add(new ResourceLocation("textures/blocks/redstone_dust_line.png"));
            add(new ResourceLocation("textures/blocks/redstone_dust_line_overlay.png"));
            add(new ResourceLocation("textures/blocks/redstone_lamp_off.png"));
            add(new ResourceLocation("textures/blocks/redstone_lamp_on.png"));
            add(new ResourceLocation("textures/blocks/redstone_ore.png"));
            add(new ResourceLocation("textures/blocks/redstone_ore.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/redstone_torch_off.png"));
            add(new ResourceLocation("textures/blocks/redstone_torch_on.png"));
            add(new ResourceLocation("textures/blocks/red_sand.png"));
            add(new ResourceLocation("textures/blocks/red_sandstone_bottom.png"));
            add(new ResourceLocation("textures/blocks/red_sandstone_carved.png"));
            add(new ResourceLocation("textures/blocks/red_sandstone_normal.png"));
            add(new ResourceLocation("textures/blocks/red_sandstone_smooth.png"));
            add(new ResourceLocation("textures/blocks/red_sandstone_top.png"));
            add(new ResourceLocation("textures/blocks/reeds.png"));
            add(new ResourceLocation("textures/blocks/repeater_off.png"));
            add(new ResourceLocation("textures/blocks/repeater_on.png"));
            add(new ResourceLocation("textures/blocks/sand.png"));
            add(new ResourceLocation("textures/blocks/sandstone_bottom.png"));
            add(new ResourceLocation("textures/blocks/sandstone_carved.png"));
            add(new ResourceLocation("textures/blocks/sandstone_normal.png"));
            add(new ResourceLocation("textures/blocks/sandstone_smooth.png"));
            add(new ResourceLocation("textures/blocks/sandstone_top.png"));
            add(new ResourceLocation("textures/blocks/sapling_acacia.png"));
            add(new ResourceLocation("textures/blocks/sapling_birch.png"));
            add(new ResourceLocation("textures/blocks/sapling_jungle.png"));
            add(new ResourceLocation("textures/blocks/sapling_oak.png"));
            add(new ResourceLocation("textures/blocks/sapling_roofed_oak.png"));
            add(new ResourceLocation("textures/blocks/sapling_spruce.png"));
            add(new ResourceLocation("textures/blocks/sea_lantern.png"));
            add(new ResourceLocation("textures/blocks/slime.png"));
            add(new ResourceLocation("textures/blocks/snow.png"));
            add(new ResourceLocation("textures/blocks/soul_sand.png"));
            add(new ResourceLocation("textures/blocks/soul_sand.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/sponge.png"));
            add(new ResourceLocation("textures/blocks/spongeALT1.png"));
            add(new ResourceLocation("textures/blocks/sponge_wet.png"));
            add(new ResourceLocation("textures/blocks/sstone.png"));
            add(new ResourceLocation("textures/blocks/stone.png"));
            add(new ResourceLocation("textures/blocks/stone1.png"));
            add(new ResourceLocation("textures/blocks/stonebrick.png"));
            add(new ResourceLocation("textures/blocks/stonebrickALT.png"));
            add(new ResourceLocation("textures/blocks/stonebrick_carved.png"));
            add(new ResourceLocation("textures/blocks/stonebrick_cracked.png"));
            add(new ResourceLocation("textures/blocks/stonebrick_crackedALT.png"));
            add(new ResourceLocation("textures/blocks/stonebrick_mossy.png"));
            add(new ResourceLocation("textures/blocks/stonebrick_mossyALT.png"));
            add(new ResourceLocation("textures/blocks/stone_andesite.png"));
            add(new ResourceLocation("textures/blocks/stone_andesite_smooth.png"));
            add(new ResourceLocation("textures/blocks/stone_diorite.png"));
            add(new ResourceLocation("textures/blocks/stone_diorite_smooth.png"));
            add(new ResourceLocation("textures/blocks/stone_granite.png"));
            add(new ResourceLocation("textures/blocks/stone_granite_smooth.png"));
            add(new ResourceLocation("textures/blocks/stone_slab_side.png"));
            add(new ResourceLocation("textures/blocks/stone_slab_top.png"));
            add(new ResourceLocation("textures/blocks/tallgrass.png"));
            add(new ResourceLocation("textures/blocks/tnt_bottom.png"));
            add(new ResourceLocation("textures/blocks/tnt_side.png"));
            add(new ResourceLocation("textures/blocks/tnt_top.png"));
            add(new ResourceLocation("textures/blocks/torch_on.png"));
            add(new ResourceLocation("textures/blocks/torch_on.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/trapdoor.png"));
            add(new ResourceLocation("textures/blocks/trip_wire.png"));
            add(new ResourceLocation("textures/blocks/trip_wire_source.png"));
            add(new ResourceLocation("textures/blocks/vine.png"));
            add(new ResourceLocation("textures/blocks/waterlily.png"));
            add(new ResourceLocation("textures/blocks/water_flow.png"));
            add(new ResourceLocation("textures/blocks/water_flow.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/water_still.png"));
            add(new ResourceLocation("textures/blocks/water_still.png.mcmeta"));
            add(new ResourceLocation("textures/blocks/web.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_0.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_1.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_2.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_3.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_4.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_5.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_6.png"));
            add(new ResourceLocation("textures/blocks/wheat_stage_7.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_black.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_blue.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_brown.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_cyan.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_gray.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_green.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_light_blue.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_lime.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_magenta.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_orange.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_pink.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_purple.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_red.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_silver.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_white.png"));
            add(new ResourceLocation("textures/blocks/wool_colored_yellow.png"));
            add(new ResourceLocation("textures/blocks/wwool_colored_pink.png"));
    }};

    private boolean firstLoad = true;
    private int renderChunkDistance = 4;

    @Override
    public void onResourceManagerReload(IResourceManager resourceManager) {
        if(firstLoad) {
            firstLoad = false;
        }

        NovaNative.INSTANCE.reset_texture_manager();
        int maxAtlasSize = NovaNative.INSTANCE.get_max_texture_size();
        addTextures(TERRAIN_ALBEDO_TEXTURES_LOCATION, NovaNative.AtlasType.TERRAIN, NovaNative.TextureType.ALBEDO, resourceManager, maxAtlasSize);
        //addTextures(GUI_ALBEDO_TEXTURES_LOCATIONS, NovaNative.AtlasType.GUI, NovaNative.TextureType.ALBEDO, resourceManager, maxAtlasSize);
    }

    private void addTextures(
            List<ResourceLocation> locations,
            NovaNative.AtlasType atlasType,
            NovaNative.TextureType textureType,
            IResourceManager resourceManager,
            int maxAtlasSize
    ) {
        AtlasGenerator gen = new AtlasGenerator();
        List<AtlasGenerator.ImageName> images = new ArrayList<>();

        for(ResourceLocation textureLocation : locations) {
            try {
                IResource texture = resourceManager.getResource(textureLocation);
                BufferedInputStream in = new BufferedInputStream(texture.getInputStream());
                BufferedImage image = ImageIO.read(in);

                if(image != null) {
                    images.add(new AtlasGenerator.ImageName(image, textureLocation.toString()));
                }
            } catch(IOException e) {
                LOG.warn("IOException when loading texture " + textureLocation.toString(), e);
            }
        }

        List<AtlasGenerator.Texture> atlases = gen.Run("albedo", maxAtlasSize, maxAtlasSize, 1, true, images);

        for(AtlasGenerator.Texture texture : atlases) {
            try {
                BufferedImage image = texture.getImage();
                byte[] imageData = ((DataBufferByte) image.getRaster().getDataBuffer()).getData();
                LOG.info("The image has " + imageData.length + " separate pixels");

                /*for(int i = 0; i < imageData.length; i += 4) {
                    byte a = imageData[i];
                    byte b = imageData[i + 1];
                    byte g = imageData[i + 2];
                    byte r = imageData[i + 3];
                    imageData[i] = r;
                    imageData[i + 1] = r;
                    imageData[i + 2] = r;
                    imageData[i + 3] = a;
                }*/

                NovaNative.mc_atlas_texture atlasTex = new NovaNative.mc_atlas_texture(
                        image.getWidth(),
                        image.getHeight(),
                        image.getColorModel().getNumComponents(),
                        imageData
                );
                NovaNative.INSTANCE.add_texture(atlasTex, atlasType.ordinal(), textureType.ordinal());
                Map<String, Rectangle> rectangleMap = texture.getRectangleMap();

                for(String texName : rectangleMap.keySet()) {
                    Rectangle rect = rectangleMap.get(texName);
                    NovaNative.mc_texture_atlas_location atlasLoc = new NovaNative.mc_texture_atlas_location(
                            texName,
                            rect.x / (float) image.getWidth(),
                            rect.y / (float) image.getHeight(),
                            rect.width / (float) image.getWidth(),
                            rect.height / (float) image.getHeight()
                    );
                    NovaNative.INSTANCE.add_texture_location(atlasLoc);
                }
            } catch(AtlasGenerator.Texture.WrongNumComponentsException e) {
                LOG.error("Could not process a texture", e);
            }
        }
    }

    public void preInit() {
        System.getProperties().setProperty("jna.library.path", "D:\\Documents\\Nova Renderer\\jars\\versions\\1.10\\1.10-natives");
        System.getProperties().setProperty("jna.dump_memory", "false");
        String pid = ManagementFactory.getRuntimeMXBean().getName();
        LOG.info("PID: " + pid);
        NovaNative.INSTANCE.init_nova();
        LOG.info("Native code initialized");
    }

    public void updateRenderer() {
    }

    public void updateCameraAndRender(float renderPartialTicks, long systemNanoTime, Minecraft mc) {
        NovaNative.mc_render_command cmd = RenderCommandBuilder.makeRenderCommand(mc, renderPartialTicks);
        NovaNative.INSTANCE.send_render_command(cmd);

        if(NovaNative.INSTANCE.should_close()) {
            Minecraft.getMinecraft().shutdown();
        }
    }

    public void setGuiScreen(GuiScreen guiScreenIn) {
        NovaNative.mc_set_gui_screen_command set_gui_screen = RenderCommandBuilder.createSetGuiScreenCommand(guiScreenIn);
        NovaNative.INSTANCE.send_change_gui_screen_command(set_gui_screen);
    }
}
