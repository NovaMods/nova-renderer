package com.continuum.nova;

import com.continuum.nova.utils.AtlasGenerator;
import com.continuum.nova.utils.RenderCommandBuilder;
import com.continuum.nova.NovaNative.window_size;

import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.gui.ScaledResolution;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.util.ResourceLocation;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import javax.annotation.Nonnull;
import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.util.*;
import java.util.List;

public class NovaRenderer implements IResourceManagerReloadListener {
    public static final String MODID = "Nova Renderer";
    public static final String VERSION = "0.0.3";

    private static final Logger LOG = LogManager.getLogger(NovaRenderer.class);
    public static final ResourceLocation WHITE_TEXTURE_GUI_LOCATION = new ResourceLocation("white_gui");

    private boolean firstLoad = true;

    private static final List<ResourceLocation> GUI_ALBEDO_TEXTURES_LOCATIONS = new ArrayList<>();
    private TextureMap guiAtlas = new TextureMap("textures");
    private Map<ResourceLocation, TextureAtlasSprite> guiSpriteLocations = new HashMap<>();

    private static final List<ResourceLocation> TERRAIN_ALBEDO_TEXTURES_LOCATIONS = new ArrayList<>();
    private TextureMap blockAtlas = new TextureMap("textures");
    private Map<ResourceLocation, TextureAtlasSprite> blockSpriteLocations = new HashMap<>();

    private int height;

    private int width;

    private boolean resized;
    private int scalefactor;

    private static final List<ResourceLocation> FONT_ALBEDO_TEXTURES_LOCATIONS = new ArrayList<>();
    private TextureMap fontAtlas = new TextureMap("textures");
    private Map<ResourceLocation, TextureAtlasSprite> fontSpriteLocations = new HashMap<>();

    public NovaRenderer() {
        addBlockTextureLocations();
        addGuiTextureLocations();
        addFontTextureLocations();
    }

    @Override
    public void onResourceManagerReload(@Nonnull IResourceManager resourceManager) {
        if(firstLoad) {
            firstLoad = false;
        }

        NovaNative.INSTANCE.reset_texture_manager();
        int maxAtlasSize = NovaNative.INSTANCE.get_max_texture_size();
        addTextures(TERRAIN_ALBEDO_TEXTURES_LOCATIONS, NovaNative.TextureType.TERRAIN_COLOR, resourceManager, maxAtlasSize);
        LOG.debug("Created block color atlas");

        addGuiAtlas(resourceManager);
        addFontAtlas(resourceManager);
    }

    private void addGuiAtlas(@Nonnull IResourceManager resourceManager) {
        guiAtlas.createWhiteTexture(WHITE_TEXTURE_GUI_LOCATION);
        addAtlas(resourceManager, guiAtlas, GUI_ALBEDO_TEXTURES_LOCATIONS, guiSpriteLocations, NovaNative.TextureType.GUI);
        LOG.debug("Created GUI atlas");
    }

    private void addFontAtlas(@Nonnull IResourceManager resourceManager) {
        addAtlas(resourceManager, fontAtlas, FONT_ALBEDO_TEXTURES_LOCATIONS, fontSpriteLocations, NovaNative.TextureType.FONT);
        LOG.debug("Created font atlas");
    }

    private void addAtlas(@Nonnull IResourceManager resourceManager, TextureMap atlas, List<ResourceLocation> resoruces,
                          Map<ResourceLocation, TextureAtlasSprite> spriteLocations, NovaNative.TextureType textureType) {
        atlas.loadSprites(resourceManager, textureMapIn -> resoruces.forEach(location -> {
            TextureAtlasSprite textureAtlasSprite = textureMapIn.registerSprite(location);
            spriteLocations.put(location, textureAtlasSprite);
        }));

        Optional<TextureAtlasSprite> whiteImage = atlas.getWhiteImage();
        whiteImage.ifPresent(image -> spriteLocations.put(image.getLocation(), image));

        NovaNative.mc_atlas_texture guiAtlasTexture = getFullImage(atlas.getWidth(), atlas.getHeight(), spriteLocations.values());

        NovaNative.INSTANCE.add_texture(guiAtlasTexture, textureType.ordinal());

        for(TextureAtlasSprite sprite : spriteLocations.values()) {
            NovaNative.mc_texture_atlas_location location = new NovaNative.mc_texture_atlas_location(
                    sprite.getIconName(),
                    sprite.getMinU(),
                    sprite.getMinV(),
                    sprite.getMaxU(),
                    sprite.getMaxV()
            );

            LOG.info("Adding a sprite with name " + sprite.getIconName());

            NovaNative.INSTANCE.add_texture_location(location);
        }
    }

    private NovaNative.mc_atlas_texture getFullImage(int atlasWidth, int atlasHeight, Collection<TextureAtlasSprite> sprites) {
        byte[] imageData = new byte[atlasWidth * atlasHeight * 4];

        for(TextureAtlasSprite sprite : sprites) {
            LOG.debug("Looking at sprite " + sprite.getIconName());
            int startY = sprite.getOriginY() * atlasWidth * 4;
            int startPos = sprite.getOriginX() * 4 + startY;

            int[] data = sprite.getFrameTextureData(0)[0];
            for(int y = 0; y < sprite.getIconHeight(); y++) {
                for(int x = 0; x < sprite.getIconWidth(); x++) {
                    // Reverse the order of the color channels
                    int pixel = data[y * sprite.getIconWidth() + x];

                    byte red    = (byte)( pixel        & 0xFF);
                    byte green  = (byte)((pixel >>  8) & 0xFF);
                    byte blue   = (byte)((pixel >> 16) & 0xFF);
                    byte alpha  = (byte)((pixel >> 24) & 0xFF);

                    int imageDataBasePos = startPos + x * 4 + y * atlasWidth * 4;
                    imageData[imageDataBasePos]     = blue;
                    imageData[imageDataBasePos + 1] = green;
                    imageData[imageDataBasePos + 2] = red;
                    imageData[imageDataBasePos + 3] = alpha;
                }
            }
        }

        return new NovaNative.mc_atlas_texture(
                atlasWidth,
                atlasHeight,
                4,
                imageData
        );
    }

    private void addTextures(List<ResourceLocation> locations, NovaNative.TextureType textureType,  IResourceManager resourceManager, int maxAtlasSize) {
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

        List<AtlasGenerator.Texture> atlases = gen.Run(maxAtlasSize, maxAtlasSize, 0, images);

        for(AtlasGenerator.Texture texture : atlases) {
            try {
                BufferedImage image = texture.getImage();
                byte[] imageData = ((DataBufferByte) image.getRaster().getDataBuffer()).getData();
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
                NovaNative.INSTANCE.add_texture(atlasTex, textureType.ordinal());
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
        System.getProperties().setProperty("jna.library.path", System.getProperty("java.library.path"));
        System.getProperties().setProperty("jna.dump_memory", "false");
        String pid = ManagementFactory.getRuntimeMXBean().getName();
        LOG.info("PID: " + pid);
        NovaNative.INSTANCE.initialize();
        LOG.info("Native code initialized");
        updateWindowSize();
    }

    private void updateWindowSize(){
        window_size size = NovaNative.INSTANCE.get_window_size();
        int oldHeight = height;
        int oldWidth = width;
        if (oldHeight != size.height || oldWidth != size.width){
            resized = true;
        } else {
            resized = false;
        }
        height = size.height;
        width = size.width;

    }

    public int getHeight(){
        return height;
    }

    public int getWidth(){
        return width;
    }

    public boolean wasResized(){
        return resized;
    }

    public void updateCameraAndRender(float renderPartialTicks, long systemNanoTime, Minecraft mc) {
        if(NovaNative.INSTANCE.should_close()) {
            Minecraft.getMinecraft().shutdown();
        }
        NovaNative.INSTANCE.execute_frame();
        updateWindowSize();
        int scalefactor = new ScaledResolution(Minecraft.getMinecraft()).getScaleFactor()*2;
        if (scalefactor!=this.scalefactor) {
            NovaNative.INSTANCE.set_float_setting("scalefactor", scalefactor);
            this.scalefactor = scalefactor;
        }
    }

    private void addGuiTextureLocations() {
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/bars"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/book"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/demo_background"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/icons"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/options_background"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/resource_packs"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/server_selection"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/spectator_widgets"));
        // GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/stream_indicator")); // non-square texture, breaks
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/widgets"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/world_selection"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/achievement/achievement_background"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/achievement/achievement_icons"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/anvil"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/beacon"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/brewing_stand"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/crafting_table"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/dispenser"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/enchanting_table"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/furnace"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/generic_54"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/hopper"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/horse"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/inventory"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/stats_icons"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/villager"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/creative_inventory/tabs"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/creative_inventory/tab_inventory"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/creative_inventory/tab_items"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/container/creative_inventory/tab_item_search"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/presets/chaos"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/presets/delight"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/presets/drought"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/presets/isles"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/presets/luck"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/presets/madness"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/presets/water"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/minecraft"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/mojang"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/background/panorama_0"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/background/panorama_1"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/background/panorama_2"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/background/panorama_3"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/background/panorama_4"));
        GUI_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("gui/title/background/panorama_5"));
    }

    private void addBlockTextureLocations() {
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/anvil_base.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/anvil_top_damaged_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/anvil_top_damaged_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/anvil_top_damaged_2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/beacon.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bedrock.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bed_feet_end.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bed_feet_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bed_feet_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bed_head_end.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bed_head_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bed_head_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/bookshelf.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/brewing_stand.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/brewing_stand_base.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/brick.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/brickALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/brick_ALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cactus_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cactus_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cake_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cake_inner.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cake_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cake_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/carrots_stage_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/carrots_stage_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/carrots_stage_2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/carrots_stage_3.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cauldron_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cauldron_inner.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cauldron_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cauldron_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/clay.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/coal_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/coal_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/coarse_dirt.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/coarse_dirtt.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cobblestone-A.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cobblestone.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cobblestone_mossy.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cocoa_stage_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cocoa_stage_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/cocoa_stage_2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/command_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/command_block.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/command_blockk.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/comparator_off.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/comparator_on.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/crafting_table_front.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/crafting_table_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/crafting_table_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/daylight_detector_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/daylight_detector_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/daylight_detector_top2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/deadbush.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_3.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_4.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_5.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_6.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_7.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_8.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/destroy_stage_9.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/diamond_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/diamond_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dirt.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dirt_podzol_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dirt_podzol_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dispenser_front_horizontal.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dispenser_front_vertical.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/door_iron_lower.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/door_iron_upper.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/door_wood_lower.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/door_wood_upper.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/doubleflower_paeonia.png.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_fern_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_fern_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_grass_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_grass_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_paeonia_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_paeonia_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_rose_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_rose_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_sunflower_back.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_sunflower_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_sunflower_front.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_sunflower_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_syringa_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/double_plant_syringa_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dragon_egg.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dropper_front_horizontal.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/dropper_front_vertical.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/emerald_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/emerald_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/emerald_ore.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/enchanting_table_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/enchanting_table_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/enchanting_table_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/endframe_eye.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/endframe_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/endframe_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/end_stone.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/farmland_dry.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/farmland_wet.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fern.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_0.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_00.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_00.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_1.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_11.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/fire_layer_11.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_allium.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_blue_orchid.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_dandelion.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_houstonia.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_oxeye_daisy.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_pot.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_rose.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_tulip_orange.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_tulip_pink.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_tulip_red.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/flower_tulip_white.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/furnace_front_off.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/furnace_front_on.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/furnace_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/furnace_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/gggrass_side_snowed.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/ggold_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/ggrass_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/ggrass_side_overlay.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/ggrass_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_black.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_blue.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_brown.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_cyan.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_gray.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_green.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_light_blue.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_lime.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_magenta.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_orange.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_pane_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_pink.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_purple.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_red.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_silver.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_white.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glass_yellow.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glowstone.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/glowstone.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/gold_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/gold_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/grass_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/grass_side_overlay.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/grass_side_overlayALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/grass_side_snowed.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/grass_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/grass_topALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/gravel.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_black.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_blue.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_brown.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_cyan.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_gray.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_green.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_light_blue.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_lime.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_magenta.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_orange.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_pink.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_purple.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_red.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_silver.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_white.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hardened_clay_stained_yellow.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hay_block_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hay_block_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hopper_inside.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hopper_outside.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/hopper_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/ice.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/ice_packed.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/iiron_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/iron_bars.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/iron_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/iron_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/iron_trapdoor.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/itemframe_background.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/jukebox_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/jukebox_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/jukebox_top.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/ladder.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/lapis_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/lapis_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/lava_flow.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/lava_flow.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/lava_still.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/lava_still.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/leaves_acacia.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/leaves_big_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/leaves_birch.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/leaves_jungle.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/leaves_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/leaves_spruce.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/lever.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_acacia.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_acacia_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_big_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_big_oak_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_birch.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_birch_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_jungle.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_junglee.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_jungle_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_oak_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_spruce.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/log_spruce_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/melon_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/melon_stem_connected.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/melon_stem_disconnected.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/melon_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mob_spawner.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mushroom_block_inside.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mushroom_block_skin_brown.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mushroom_block_skin_red.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mushroom_block_skin_stem.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mushroom_brown.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mushroom_red.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mycelium_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/mycelium_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/netherrack.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/nether_brick.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/nether_wart_stage_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/nether_wart_stage_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/nether_wart_stage_2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/noteblock.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/obsidian.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/piston_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/piston_inner.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/piston_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/piston_top_normal.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/piston_top_sticky.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/planks_acacia.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/planks_big_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/planks_birch.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/planks_jungle.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/planks_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/planks_spruce.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/portal.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/portal.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/potatoes_stage_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/potatoes_stage_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/potatoes_stage_2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/potatoes_stage_3.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/prismarine_bricks.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/prismarine_dark.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/prismarine_rough.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/prismarine_rough.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/prismarine_roughh.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/pumpkin_face_off.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/pumpkin_face_offALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/pumpkin_face_on.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/pumpkin_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/pumpkin_stem_connected.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/pumpkin_stem_disconnected.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/pumpkin_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/qquartz_block_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_block_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_block_chiseled.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_block_chiseled_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_block_lines.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_block_lines_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_block_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_block_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/quartz_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_activator.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_activator_powered.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_detector.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_detector_powered.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_golden.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_golden_powered.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_normal.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/rail_normal_turned.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_block.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_dust_cross.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_dust_cross_overlay.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_dust_line.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_dust_line_overlay.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_lamp_off.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_lamp_on.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_ore.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_ore.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_torch_off.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/redstone_torch_on.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/red_sand.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/red_sandstone_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/red_sandstone_carved.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/red_sandstone_normal.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/red_sandstone_smooth.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/red_sandstone_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/reeds.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/repeater_off.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/repeater_on.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sand.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sandstone_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sandstone_carved.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sandstone_normal.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sandstone_smooth.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sandstone_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sapling_acacia.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sapling_birch.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sapling_jungle.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sapling_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sapling_roofed_oak.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sapling_spruce.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sea_lantern.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/slime.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/snow.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/soul_sand.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/soul_sand.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sponge.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/spongeALT1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sponge_wet.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/sstone.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stonebrick.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stonebrickALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stonebrick_carved.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stonebrick_cracked.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stonebrick_crackedALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stonebrick_mossy.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stonebrick_mossyALT.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_andesite.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_andesite_smooth.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_diorite.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_diorite_smooth.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_granite.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_granite_smooth.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_slab_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/stone_slab_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/tallgrass.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/tnt_bottom.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/tnt_side.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/tnt_top.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/torch_on.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/torch_on.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/trapdoor.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/trip_wire.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/trip_wire_source.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/vine.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/waterlily.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/water_flow.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/water_flow.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/water_still.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/water_still.png.mcmeta"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/web.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_0.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_1.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_2.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_3.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_4.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_5.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_6.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wheat_stage_7.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_black.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_blue.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_brown.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_cyan.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_gray.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_green.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_light_blue.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_lime.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_magenta.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_orange.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_pink.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_purple.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_red.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_silver.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_white.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wool_colored_yellow.png"));
        TERRAIN_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("textures/blocks/wwool_colored_pink.png"));
    }

    private void addFontTextureLocations() {
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/ascii"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/ascii_sga"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_00"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_01"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_02"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_03"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_04"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_05"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_06"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_07"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_09"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_0a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_0b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_0c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_0d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_0e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_0f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_10"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_11"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_12"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_13"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_14"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_15"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_16"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_17"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_18"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_19"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_1a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_1b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_1c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_1d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_1e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_1f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_20"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_21"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_22"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_23"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_24"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_25"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_26"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_27"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_28"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_29"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_2a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_2b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_2c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_2d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_2e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_2f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_30"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_31"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_32"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_33"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_34"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_35"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_36"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_37"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_38"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_39"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_3a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_3b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_3c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_3d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_3e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_3f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_40"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_41"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_42"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_43"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_44"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_45"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_46"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_47"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_48"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_49"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_4a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_4b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_4c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_4d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_4e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_4f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_50"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_51"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_52"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_53"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_54"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_55"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_56"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_57"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_58"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_59"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_5a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_5b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_5c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_5d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_5e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_5f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_60"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_61"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_62"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_63"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_64"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_65"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_66"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_67"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_68"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_69"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_6a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_6b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_6c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_6d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_6e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_6f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_70"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_71"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_72"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_73"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_74"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_75"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_76"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_77"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_78"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_79"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_7a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_7b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_7c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_7d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_7e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_7f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_80"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_81"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_82"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_83"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_84"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_85"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_86"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_87"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_88"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_89"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_8a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_8b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_8c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_8d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_8e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_8f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_90"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_91"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_92"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_93"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_94"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_95"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_96"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_97"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_98"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_99"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_9a"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_9b"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_9c"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_9d"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_9e"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_9f"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a0"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a1"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a2"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a3"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a4"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a5"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a6"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a7"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a8"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_a9"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_aa"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ab"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ac"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ad"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ae"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_af"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b0"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b1"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b2"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b3"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b4"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b5"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b6"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b7"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b8"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_b9"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ba"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_bb"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_bc"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_bd"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_be"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_bf"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c0"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c1"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c2"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c3"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c4"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c5"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c6"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c7"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c8"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_c9"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ca"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_cb"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_cc"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_cd"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ce"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_cf"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d0"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d1"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d2"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d3"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d4"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d5"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d6"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_d7"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_f9"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_fa"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_fb"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_fc"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_fd"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_fe"));
        FONT_ALBEDO_TEXTURES_LOCATIONS.add(new ResourceLocation("font/unicode_page_ff"));
    }

    public static NovaNative.TextureType atlasTextureOfSprite(ResourceLocation texture) {
        ResourceLocation strippedLocation = new ResourceLocation(texture.getResourceDomain(), texture.getResourcePath().replace(".png","").replace("textures/",""));

        LOG.info("Need to get atlas that " + strippedLocation + " is in");
        if(TERRAIN_ALBEDO_TEXTURES_LOCATIONS.contains(strippedLocation)) {
            LOG.info("It's in the terrain");
            return NovaNative.TextureType.TERRAIN_COLOR;
        } else if(GUI_ALBEDO_TEXTURES_LOCATIONS.contains(strippedLocation)) {
            LOG.info("It's in the gui");
            return NovaNative.TextureType.GUI;
        } else if(FONT_ALBEDO_TEXTURES_LOCATIONS.contains(strippedLocation)) {
            LOG.info("It's in the font");
            return NovaNative.TextureType.FONT;
        }

        LOG.error("Texture " + strippedLocation + " requested, but it's not in at atlas that Nova knows about");
        return NovaNative.TextureType.NO_TEXTURE;
    }
}
