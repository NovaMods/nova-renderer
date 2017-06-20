package com.continuum.nova;

import com.continuum.nova.NovaNative.window_size;
import com.continuum.nova.chunks.ChunkUpdateListener;
import com.continuum.nova.gui.NovaDraw;
import com.continuum.nova.utils.Utils;
import net.minecraft.client.Minecraft;
import net.minecraft.client.entity.EntityPlayerSP;
import net.minecraft.client.gui.ScaledResolution;
import net.minecraft.client.renderer.block.model.IBakedModel;
import net.minecraft.client.renderer.block.model.SimpleBakedModel;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import javax.annotation.Nonnull;
import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
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

    private static final List<ResourceLocation> GUI_COLOR_TEXTURES_LOCATIONS = new ArrayList<>();
    private TextureMap guiAtlas = new TextureMap("textures");
    private Map<ResourceLocation, TextureAtlasSprite> guiSpriteLocations = new HashMap<>();

    private static final List<ResourceLocation> BLOCK_COLOR_TEXTURES_LOCATIONS = new ArrayList<>();
    private TextureMap blockAtlas = new TextureMap("textures");
    private Map<ResourceLocation, TextureAtlasSprite> blockSpriteLocations = new HashMap<>();

    private static final List<ResourceLocation> FONT_COLOR_TEXTURES_LOCATIONS = new ArrayList<>();
    private TextureMap fontAtlas = new TextureMap("textures");
    private Map<ResourceLocation, TextureAtlasSprite> fontSpriteLocations = new HashMap<>();

    private static final List<ResourceLocation> FREE_TEXTURES = new ArrayList<>();

    private int height;
    private int width;

    private boolean resized;
    private int scalefactor;

    private IResourceManager resourceManager;

    private ChunkUpdateListener chunkUpdateListener;

    public NovaRenderer() {
        // I put these in Utils to make this class smaller
        Utils.initBlockTextureLocations(BLOCK_COLOR_TEXTURES_LOCATIONS);
        Utils.initGuiTextureLocations(GUI_COLOR_TEXTURES_LOCATIONS);
        Utils.initFontTextureLocations(FONT_COLOR_TEXTURES_LOCATIONS);
        Utils.initFreeTextures(FREE_TEXTURES);
    }

    @Override
    public void onResourceManagerReload(@Nonnull IResourceManager resourceManager) {
        this.resourceManager = resourceManager;

        if (firstLoad) {
            firstLoad = false;
        }

        NovaNative.INSTANCE.reset_texture_manager();

        addGuiAtlas(resourceManager);
        addFontAtlas(resourceManager);
        addFreeTextures(resourceManager);
    }

    /**
     * Adds the textures that just hang out without a texture atlas
     *
     * @param resourceManager The IResoruceManager to get the textures from
     */
    private void addFreeTextures(IResourceManager resourceManager) {
        for (ResourceLocation loc : FREE_TEXTURES) {
            try {
                IResource texture = resourceManager.getResource(loc);
                BufferedInputStream in = new BufferedInputStream(texture.getInputStream());
                BufferedImage image = ImageIO.read(in);
                if (image != null) {
                    loadTexture(loc, image);
                } else {
                    LOG.error("Free texture " + loc + " has no data!");
                }
            } catch (IOException e) {
                LOG.error("Could not load free texture " + loc, e);
            }
        }
    }

    private void addGuiAtlas(@Nonnull IResourceManager resourceManager) {
        guiAtlas.createWhiteTexture(WHITE_TEXTURE_GUI_LOCATION);
        addAtlas(resourceManager, guiAtlas, GUI_COLOR_TEXTURES_LOCATIONS, guiSpriteLocations, NovaNative.GUI_ATLAS_NAME);
        LOG.debug("Created GUI atlas");
    }

    private void addFontAtlas(@Nonnull IResourceManager resourceManager) {
        addAtlas(resourceManager, fontAtlas, FONT_COLOR_TEXTURES_LOCATIONS, fontSpriteLocations, NovaNative.FONT_ATLAS_NAME);
        LOG.debug("Created font atlas");
    }

    public void addTerrainAtlas(TextureMap blockColorMap) {
        // Copy over the atlas
        NovaNative.mc_atlas_texture blockColorTexture = getFullImage(blockColorMap.getWidth(), blockColorMap.getHeight(), blockColorMap.getMapUploadedSprites().values());
        blockColorTexture.name = NovaNative.BLOCK_COLOR_ATLAS_NAME;
        NovaNative.INSTANCE.add_texture(blockColorTexture);

        // Copy over all the icon locations
        for(String spriteName : blockColorMap.getMapUploadedSprites().keySet()) {
            TextureAtlasSprite sprite = blockColorMap.getAtlasSprite(spriteName);
            NovaNative.mc_texture_atlas_location location = new NovaNative.mc_texture_atlas_location(
                    sprite.getIconName(),
                    sprite.getMinU(),
                    sprite.getMinV(),
                    sprite.getMaxU(),
                    sprite.getMaxV()
            );

            NovaNative.INSTANCE.add_texture_location(location);
        }
    }

    private void addAtlas(@Nonnull IResourceManager resourceManager, TextureMap atlas, List<ResourceLocation> resources,
                          Map<ResourceLocation, TextureAtlasSprite> spriteLocations, String textureName) {
        atlas.loadSprites(resourceManager, textureMapIn -> resources.forEach(location -> {
            TextureAtlasSprite textureAtlasSprite = textureMapIn.registerSprite(location);
            spriteLocations.put(location, textureAtlasSprite);
        }));

        Optional<TextureAtlasSprite> whiteImage = atlas.getWhiteImage();
        whiteImage.ifPresent(image -> spriteLocations.put(image.getLocation(), image));

        NovaNative.mc_atlas_texture atlasTexture = getFullImage(atlas.getWidth(), atlas.getHeight(), spriteLocations.values());
        atlasTexture.setName(textureName);

        NovaNative.INSTANCE.add_texture(atlasTexture);

        for (TextureAtlasSprite sprite : spriteLocations.values()) {
            NovaNative.mc_texture_atlas_location location = new NovaNative.mc_texture_atlas_location(
                    sprite.getIconName(),
                    sprite.getMinU(),
                    sprite.getMinV(),
                    sprite.getMaxU(),
                    sprite.getMaxV()
            );

            NovaNative.INSTANCE.add_texture_location(location);
        }
    }

    private NovaNative.mc_atlas_texture getFullImage(int atlasWidth, int atlasHeight, Collection<TextureAtlasSprite> sprites) {
        byte[] imageData = new byte[atlasWidth * atlasHeight * 4];

        for (TextureAtlasSprite sprite : sprites) {
            int startY = sprite.getOriginY() * atlasWidth * 4;
            int startPos = sprite.getOriginX() * 4 + startY;

            if(sprite.getFrameCount() > 0) {
                int[] data = sprite.getFrameTextureData(0)[0];
                for(int y = 0; y < sprite.getIconHeight(); y++) {
                    for(int x = 0; x < sprite.getIconWidth(); x++) {
                        // Reverse the order of the color channels
                        int pixel = data[y * sprite.getIconWidth() + x];

                        byte red = (byte) (pixel & 0xFF);
                        byte green = (byte) ((pixel >> 8) & 0xFF);
                        byte blue = (byte) ((pixel >> 16) & 0xFF);
                        byte alpha = (byte) ((pixel >> 24) & 0xFF);

                        int imageDataBasePos = startPos + x * 4 + y * atlasWidth * 4;
                        imageData[imageDataBasePos] = blue;
                        imageData[imageDataBasePos + 1] = green;
                        imageData[imageDataBasePos + 2] = red;
                        imageData[imageDataBasePos + 3] = alpha;
                    }
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

    private byte[] getImageData(BufferedImage image) {

        byte[] convertedImageData = new byte[image.getWidth()*image.getHeight()*4];
            int counter = 0;
            for (int y = 0; y < image.getHeight(); y ++) {
                    for (int x = 0;x<image.getWidth();x++) {

                        Color c = new Color(image.getRGB(x,y),image.getColorModel().hasAlpha());

                        convertedImageData[counter] =(byte) (c.getRed());
                        convertedImageData[counter + 1] = (byte)(c.getGreen());
                        convertedImageData[counter + 2] = (byte)(c.getBlue());
                        convertedImageData[counter + 3] = (byte) (image.getColorModel().getNumComponents() == 3 ? 255 : c.getAlpha());
                        counter+=4;
                    }
            }
            return convertedImageData;



    }

    public void preInit() {
        System.getProperties().setProperty("jna.library.path", System.getProperty("java.library.path"));
        System.getProperties().setProperty("jna.dump_memory", "false");
        String pid = ManagementFactory.getRuntimeMXBean().getName();
        LOG.info("PID: " + pid + " TID: " + Thread.currentThread().getId());
        NovaNative.INSTANCE.initialize();
        LOG.info("Native code initialized");
        updateWindowSize();

        // Moved here so that it's initialized after the native code is loaded
        chunkUpdateListener  = new ChunkUpdateListener();
    }

    private void updateWindowSize() {
        window_size size = NovaNative.INSTANCE.get_window_size();
        int oldHeight = height;
        int oldWidth = width;
        if (oldHeight != size.height || oldWidth != size.width) {
            resized = true;
        } else {
            resized = false;
        }
        height = size.height;
        width = size.width;

    }

    public int getHeight() {
        return height;
    }

    public int getWidth() {
        return width;
    }

    public boolean wasResized() {
        return resized;
    }

    public void updateCameraAndRender(float renderPartialTicks, long systemNanoTime, Minecraft mc) {
        if (NovaNative.INSTANCE.should_close()) {
            mc.shutdown();
        }

        if (mc.currentScreen != null) {

            NovaDraw.novaDrawScreen(mc.currentScreen, renderPartialTicks);

        }

        EntityPlayerSP viewEntity = mc.thePlayer;
        if(viewEntity != null) {
            float pitch = viewEntity.rotationPitch;
            float yaw = viewEntity.rotationYaw;
            double x = viewEntity.posX;
            double y = viewEntity.posY + viewEntity.getEyeHeight();
            double z = viewEntity.posZ;
            // LOG.trace("Setting player position to ({}, {}, {}), yaw to {}, and pitch to {}", x, y, z, yaw, pitch);
            NovaNative.INSTANCE.set_player_camera_transform(x, y, z, yaw, pitch);
        }

        NovaNative.INSTANCE.execute_frame();
        updateWindowSize();
        int scalefactor = new ScaledResolution(mc).getScaleFactor() * 2;
        if (scalefactor != this.scalefactor) {
            NovaNative.INSTANCE.set_float_setting("scalefactor", scalefactor);
            this.scalefactor = scalefactor;
        }
    }

    public static String atlasTextureOfSprite(ResourceLocation texture) {
        ResourceLocation strippedLocation = new ResourceLocation(texture.getResourceDomain(), texture.getResourcePath().replace(".png", "").replace("textures/", ""));

        if (BLOCK_COLOR_TEXTURES_LOCATIONS.contains(strippedLocation)) {
            return NovaNative.BLOCK_COLOR_ATLAS_NAME;
        } else if (GUI_COLOR_TEXTURES_LOCATIONS.contains(strippedLocation) || texture == WHITE_TEXTURE_GUI_LOCATION) {
            return NovaNative.GUI_ATLAS_NAME;
        } else if (FONT_COLOR_TEXTURES_LOCATIONS.contains(strippedLocation)) {
            return NovaNative.FONT_ATLAS_NAME;
        }

        return texture.toString();
    }

    public void setWorld(World world) {
        if(world != null) {
            chunkUpdateListener.setWorld(world);
            world.addEventListener(chunkUpdateListener);
        }
    }

    /**
     * Loads the specified texture, adding it to Minecraft as a texture outside of an atlas
     *
     * @param location The location of the texture
     * @param image    The texture itself
     */
    public void loadTexture(ResourceLocation location, BufferedImage image) {
        if (resourceManager == null) {
            LOG.error("Trying to load texture " + location + " but there's no resource manager");
            return;
        }

        byte[] imageData = getImageData(image);

        NovaNative.mc_atlas_texture tex = new NovaNative.mc_atlas_texture(image.getWidth(), image.getHeight(), 4, imageData);
        tex.setName(location.toString());
        NovaNative.INSTANCE.add_texture(tex);

        NovaNative.mc_texture_atlas_location loc = new NovaNative.mc_texture_atlas_location(location.toString(), 0, 0, 1, 1);
        NovaNative.INSTANCE.add_texture_location(loc);
    }

    /**
     * Registers a static model with Nova, so that Nova can use it for drawing
     *
     * @param key The ResourceLocation which identifies this model
     * @param model The model to register
     */
    public void registerStaticModel(ResourceLocation key, IBakedModel model) {
        if(model instanceof SimpleBakedModel) {
            NovaNative.mc_simple_model mc_model = new NovaNative.mc_simple_model((SimpleBakedModel) model);
            NovaNative.INSTANCE.register_simple_model(key.toString(), mc_model);
        }
    }
}
