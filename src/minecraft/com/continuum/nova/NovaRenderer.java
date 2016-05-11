package com.continuum.nova;

import com.continuum.nova.utils.AtlasGenerator;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.RegionRenderCache;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.MobEffects;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;
import net.minecraft.world.biome.BiomeCache;
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

/**
 * Created by David on 24-Dec-15.
 *
 * Entry point for the mod. Will use JVM trickery to replace certain MC classes amd methods with some JNI classes and
 * methods, then will re-initialize MC with all the new things. Should be super gross.
 */
public class NovaRenderer implements IResourceManagerReloadListener {
    public static final String MODID = "Nova Renderer";
    public static final String VERSION = "0.0.1";

    private static final Logger LOG = LogManager.getLogger(NovaRenderer.class);

    // Looks like I have to enumerate every texture I want. Bah.
    // Optifine/Shaders mod seems to have a better solution for this, but I can't figure it out just yet
    private final List<ResourceLocation> TERRAIN_ALBEDO_TEXTURES_LOCATION = new ArrayList<>();
    {{
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/anvil_base.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/anvil_top_damaged_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/anvil_top_damaged_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/anvil_top_damaged_2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/beacon.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bedrock.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bed_feet_end.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bed_feet_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bed_feet_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bed_head_end.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bed_head_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bed_head_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/bookshelf.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/brewing_stand.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/brewing_stand_base.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/brick.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/brickALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/brick_ALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cactus_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cactus_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cake_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cake_inner.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cake_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cake_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/carrots_stage_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/carrots_stage_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/carrots_stage_2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/carrots_stage_3.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cauldron_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cauldron_inner.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cauldron_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cauldron_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/clay.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/coal_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/coal_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/coarse_dirt.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/coarse_dirtt.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cobblestone-A.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cobblestone.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cobblestone_mossy.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cocoa_stage_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cocoa_stage_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/cocoa_stage_2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/command_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/command_block.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/command_blockk.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/comparator_off.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/comparator_on.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/crafting_table_front.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/crafting_table_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/crafting_table_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/daylight_detector_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/daylight_detector_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/daylight_detector_top2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/deadbush.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_3.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_4.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_5.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_6.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_7.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_8.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/destroy_stage_9.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/diamond_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/diamond_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dirt.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dirt_podzol_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dirt_podzol_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dispenser_front_horizontal.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dispenser_front_vertical.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/door_iron_lower.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/door_iron_upper.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/door_wood_lower.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/door_wood_upper.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/doubleflower_paeonia.png.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_fern_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_fern_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_grass_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_grass_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_paeonia_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_paeonia_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_rose_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_rose_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_sunflower_back.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_sunflower_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_sunflower_front.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_sunflower_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_syringa_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/double_plant_syringa_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dragon_egg.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dropper_front_horizontal.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/dropper_front_vertical.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/emerald_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/emerald_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/emerald_ore.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/enchanting_table_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/enchanting_table_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/enchanting_table_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/endframe_eye.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/endframe_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/endframe_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/end_stone.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/farmland_dry.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/farmland_wet.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fern.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_0.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_00.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_00.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_1.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_11.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/fire_layer_11.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_allium.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_blue_orchid.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_dandelion.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_houstonia.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_oxeye_daisy.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_pot.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_rose.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_tulip_orange.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_tulip_pink.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_tulip_red.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/flower_tulip_white.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/furnace_front_off.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/furnace_front_on.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/furnace_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/furnace_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/gggrass_side_snowed.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/ggold_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/ggrass_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/ggrass_side_overlay.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/ggrass_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_black.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_blue.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_brown.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_cyan.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_gray.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_green.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_light_blue.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_lime.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_magenta.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_orange.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_pane_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_pink.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_purple.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_red.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_silver.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_white.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glass_yellow.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glowstone.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/glowstone.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/gold_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/gold_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/grass_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/grass_side_overlay.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/grass_side_overlayALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/grass_side_snowed.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/grass_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/grass_topALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/gravel.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_black.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_blue.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_brown.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_cyan.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_gray.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_green.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_light_blue.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_lime.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_magenta.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_orange.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_pink.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_purple.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_red.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_silver.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_white.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hardened_clay_stained_yellow.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hay_block_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hay_block_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hopper_inside.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hopper_outside.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/hopper_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/ice.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/ice_packed.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/iiron_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/iron_bars.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/iron_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/iron_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/iron_trapdoor.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/itemframe_background.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/jukebox_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/jukebox_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/jukebox_top.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/ladder.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/lapis_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/lapis_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/lava_flow.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/lava_flow.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/lava_still.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/lava_still.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/leaves_acacia.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/leaves_big_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/leaves_birch.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/leaves_jungle.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/leaves_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/leaves_spruce.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/lever.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_acacia.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_acacia_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_big_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_big_oak_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_birch.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_birch_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_jungle.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_junglee.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_jungle_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_oak_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_spruce.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/log_spruce_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/melon_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/melon_stem_connected.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/melon_stem_disconnected.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/melon_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mob_spawner.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mushroom_block_inside.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mushroom_block_skin_brown.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mushroom_block_skin_red.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mushroom_block_skin_stem.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mushroom_brown.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mushroom_red.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mycelium_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/mycelium_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/netherrack.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/nether_brick.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/nether_wart_stage_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/nether_wart_stage_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/nether_wart_stage_2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/noteblock.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/obsidian.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/piston_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/piston_inner.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/piston_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/piston_top_normal.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/piston_top_sticky.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/planks_acacia.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/planks_big_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/planks_birch.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/planks_jungle.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/planks_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/planks_spruce.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/portal.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/portal.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/potatoes_stage_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/potatoes_stage_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/potatoes_stage_2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/potatoes_stage_3.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/prismarine_bricks.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/prismarine_dark.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/prismarine_rough.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/prismarine_rough.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/prismarine_roughh.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/pumpkin_face_off.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/pumpkin_face_offALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/pumpkin_face_on.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/pumpkin_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/pumpkin_stem_connected.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/pumpkin_stem_disconnected.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/pumpkin_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/qquartz_block_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_block_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_block_chiseled.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_block_chiseled_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_block_lines.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_block_lines_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_block_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_block_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/quartz_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_activator.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_activator_powered.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_detector.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_detector_powered.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_golden.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_golden_powered.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_normal.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/rail_normal_turned.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_block.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_dust_cross.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_dust_cross_overlay.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_dust_line.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_dust_line_overlay.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_lamp_off.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_lamp_on.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_ore.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_ore.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_torch_off.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/redstone_torch_on.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/red_sand.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/red_sandstone_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/red_sandstone_carved.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/red_sandstone_normal.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/red_sandstone_smooth.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/red_sandstone_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/reeds.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/repeater_off.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/repeater_on.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sand.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sandstone_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sandstone_carved.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sandstone_normal.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sandstone_smooth.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sandstone_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sapling_acacia.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sapling_birch.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sapling_jungle.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sapling_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sapling_roofed_oak.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sapling_spruce.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sea_lantern.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/slime.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/snow.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/soul_sand.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/soul_sand.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sponge.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/spongeALT1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sponge_wet.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/sstone.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stonebrick.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stonebrickALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stonebrick_carved.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stonebrick_cracked.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stonebrick_crackedALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stonebrick_mossy.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stonebrick_mossyALT.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_andesite.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_andesite_smooth.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_diorite.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_diorite_smooth.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_granite.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_granite_smooth.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_slab_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/stone_slab_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/tallgrass.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/tnt_bottom.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/tnt_side.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/tnt_top.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/torch_on.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/torch_on.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/trapdoor.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/trip_wire.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/trip_wire_source.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/vine.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/waterlily.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/water_flow.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/water_flow.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/water_still.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/water_still.png.mcmeta"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/web.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_0.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_1.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_2.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_3.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_4.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_5.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_6.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wheat_stage_7.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_black.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_blue.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_brown.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_cyan.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_gray.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_green.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_light_blue.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_lime.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_magenta.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_orange.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_pink.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_purple.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_red.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_silver.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_white.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wool_colored_yellow.png"));
            TERRAIN_ALBEDO_TEXTURES_LOCATION.add(new ResourceLocation("textures/blocks/wwool_colored_pink.png"));
    }}

    private boolean firstLoad = true;
    private int renderChunkDistance = 4;

    /**
     * Sends all the resources to the native thread
     *
     * <p>This method is going to be pretty long and gross as it grabs all the graphics resources</p>
     *
     * @param resourceManager The resource manager to get things from
     */
    @Override
    public void onResourceManagerReload(IResourceManager resourceManager) {
        // Find all the texture resources somehow. We really only need their pixels, width, and height
        // I can actually just call #getAllResoruces with a location for textures...
        if(firstLoad) {
            // For some reason, this method is called before any resources are actually loaded. Then it's called again
            // after all the resources are loaded. So, by ignoring the first time this method is called, nothing bad
            // should happen
            firstLoad = false;
            return;
        }

        NovaNative.INSTANCE.reset_texture_manager();
        int maxAtlasSize = NovaNative.INSTANCE.get_max_texture_size();

        // Make sure that the atlas isn't super enormously huge, because while that would be good to avoid texture,
        // switching, it runs out of memory
        //maxAtlasSize = Math.min(maxAtlasSize, 8192);

        AtlasGenerator gen = new AtlasGenerator();

        addTextures(TERRAIN_ALBEDO_TEXTURES_LOCATION, NovaNative.AtlasType.TERRAIN, NovaNative.TextureType.ALBEDO, resourceManager, maxAtlasSize, gen);
    }

    private void addTextures(
            List<ResourceLocation> locations,
            NovaNative.AtlasType atlasType,
            NovaNative.TextureType textureType,
            IResourceManager resourceManager,
            int maxAtlasSize, AtlasGenerator gen
    ) {
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
                LOG.warn("IOException when loading texture " + textureLocation.toString() + ": " +e.getMessage());
            }
        }

        List<AtlasGenerator.Texture> atlases = gen.Run("albedo", maxAtlasSize, maxAtlasSize, 1, true, images);

        for(AtlasGenerator.Texture texture : atlases) {
            try {
                BufferedImage image = texture.getImage();
                byte[] imageData = ((DataBufferByte)image.getRaster().getDataBuffer()).getData();
                LOG.info("The image has " + imageData.length + " separate pixels");

                for(int i = 0; i < imageData.length; i += 4) {
                    byte a = imageData[i];
                    byte b = imageData[i + 1];
                    byte g = imageData[i + 2];
                    byte r = imageData[i + 3];

                    imageData[i] = r;
                    imageData[i + 1] = g;
                    imageData[i + 2] = b;
                    imageData[i + 3] = a;
                }

                NovaNative.mc_atlas_texture atlasTex = new NovaNative.mc_atlas_texture(
                        image.getWidth(),
                        image.getHeight(),
                        image.getColorModel().getNumComponents(),
                        imageData
                );

                // TODO: This is bad. This will only use one texture per atlas type, and I definitely might want more than
                // that. I'll have to change a lot of code to support that though
                NovaNative.INSTANCE.add_texture(atlasTex, atlasType.ordinal(), textureType.ordinal());

                Map<String, Rectangle> rectangleMap = texture.getRectangleMap();
                for(String texName : rectangleMap.keySet()) {
                    Rectangle rect = rectangleMap.get(texName);
                    NovaNative.mc_texture_atlas_location atlasLoc = new NovaNative.mc_texture_atlas_location(
                            texName,
                            rect.x / (float)image.getWidth(),
                            rect.y / (float)image.getHeight(),
                            rect.width / (float)image.getWidth(),
                            rect.height / (float)image.getHeight()
                    );

                    NovaNative.INSTANCE.add_texture_location(atlasLoc);
                }

            } catch(AtlasGenerator.Texture.WrongNumComponentsException e) {
                e.printStackTrace();
            }
        }
    }


    public void preInit() {
        // TODO: Remove this and use the win32-x86 thing to package the DLL into the jar
        System.getProperties().setProperty("jna.library.path", "C:/Users/David/Documents/Nova Renderer/run");
        System.getProperties().setProperty("jna.dump_memory", "false");
        LOG.info("PID: " + ManagementFactory.getRuntimeMXBean().getName());
        NovaNative.INSTANCE.init_nova();
        LOG.info("Native code initialized");
    }

    /**
     * Updates the renderer's tick-specific state
     */
    public void updateRenderer() {
    }

    /**
     * Renders a single in-game frame
     *
     * <p>This method needs to accomplish two things: prepare the current game state into a format that the native code
     * can effectively use, and send that data to the native code. First I'll prepare a structure of all the needed
     * parameters</p>
     *
     * @param renderPartialTicks How much time has elapsed since the last tick, in ticks
     * @param systemNanoTime The current time
     * @param mc The Minecraft instance we're currently rendering
     */
    public void updateCameraAndRender(float renderPartialTicks, long systemNanoTime, Minecraft mc) {
        NovaNative.mc_render_command cmd = makeRenderCommand(mc, renderPartialTicks);

        NovaNative.INSTANCE.send_render_command(cmd);
    }

    private NovaNative.mc_chunk makeChunk(World world, BlockPos chunkCoordinates) {
        NovaNative.mc_chunk chunk = new NovaNative.mc_chunk();
        NovaNative.mc_block[] blocks = new NovaNative.mc_block[16 * 16 * 16];

        // We really only want an array of block IDs
        IBlockAccess blockAccess = new RegionRenderCache(world, chunkCoordinates.add(-1, -1, -1), chunkCoordinates.add(16, 16, 16), 1);
        for(BlockPos.MutableBlockPos curPos : BlockPos.getAllInBoxMutable(chunkCoordinates, chunkCoordinates.add(15, 15, 15))) {
            IBlockState blockState = blockAccess.getBlockState(curPos);
            Block block = blockState.getBlock();
            int motherFuckingIdFinally = Block.getIdFromBlock(block);

            NovaNative.mc_block cur_block = new NovaNative.mc_block();
            cur_block.block_id = motherFuckingIdFinally;
            cur_block.is_on_fire = false;

            blocks[curPos.getX() + curPos.getY() * 16 + curPos.getZ() * 256] = cur_block;
        }

        chunk.blocks = new Memory(16 * 16 * 16 * Native.getNativeSize(NovaNative.mc_block.class));


        return chunk;
    }

    private List<NovaNative.mc_chunk> getAllChunks(World world) {
        List<NovaNative.mc_chunk> renderableChunks = new ArrayList<>();

        int chunkCountX = renderChunkDistance;
        int chunkCountY = 16;
        int chunkCountZ = renderChunkDistance;

        for(int x = 0; x < chunkCountX; x++) {
            for(int y = 0; y < chunkCountY; y++) {
                for(int z = 0; z < chunkCountZ; z++) {
                    BlockPos pos = new BlockPos(x * 16, y * 16, z * 16);
                    renderableChunks.add(makeChunk(world, pos));
                }
            }
        }

        return renderableChunks;
    }

    private NovaNative.mc_render_command makeRenderCommand(Minecraft mc, float partialTicks) {
        NovaNative.mc_render_command command = new NovaNative.mc_render_command();

        command.anaglyph = mc.gameSettings.anaglyph;

        command.display_height = mc.displayHeight;
        command.display_width = mc.displayWidth;

        Entity viewEntity = mc.getRenderViewEntity();
        command.world_params.camera_x = viewEntity.lastTickPosX + (viewEntity.posX - viewEntity.lastTickPosX) * (double)partialTicks;
        command.world_params.camera_y = viewEntity.lastTickPosY + (viewEntity.posY - viewEntity.lastTickPosY) * (double)partialTicks;
        command.world_params.camera_z = viewEntity.lastTickPosZ + (viewEntity.posZ - viewEntity.lastTickPosZ) * (double)partialTicks;

        command.world_params.view_bobbing = mc.gameSettings.viewBobbing;

        command.world_params.render_distance = mc.gameSettings.renderDistanceChunks;
        command.world_params.has_blindness = ((EntityLivingBase)viewEntity).isPotionActive(MobEffects.blindness);

        command.world_params.fog_color_red = mc.entityRenderer.getFogColorRed();
        command.world_params.fog_color_green = mc.entityRenderer.getFogColorGreen();
        command.world_params.fog_color_blue = mc.entityRenderer.getFogColorBlue();

        // If we're on the surface world, we can render clouds no problem. If we're not, we shouldn't even be thinking
        // about rendering clouds
        command.world_params.should_render_clouds = mc.theWorld.provider.isSurfaceWorld() ? mc.gameSettings.shouldRenderClouds() : 0;

        // TODO: Portal effects

        return command;
    }
}
