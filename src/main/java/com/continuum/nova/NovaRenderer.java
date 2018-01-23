package com.continuum.nova;

import com.continuum.nova.NovaNative.window_size;
import com.continuum.nova.chunks.ChunkBuilder;
import com.continuum.nova.chunks.ChunkUpdateListener;
import com.continuum.nova.chunks.IGeometryFilter;
import com.continuum.nova.gui.NovaDraw;
import com.continuum.nova.utils.Profiler;
import com.continuum.nova.utils.Utils;
import glm.Glm;
import glm.vec._2.Vec2;
import glm.vec._3.i.Vec3i;
import net.minecraft.client.Minecraft;
import net.minecraft.client.entity.EntityPlayerSP;
import net.minecraft.client.gui.ScaledResolution;
import net.minecraft.client.renderer.BlockModelShapes;
import net.minecraft.client.renderer.EntityRenderer;
import net.minecraft.client.renderer.color.BlockColors;
import net.minecraft.client.renderer.texture.DynamicTexture;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.entity.Entity;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import javax.annotation.Nonnull;
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.*;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import static com.continuum.nova.NovaConstants.*;
import static com.continuum.nova.utils.Utils.getImageData;

public class NovaRenderer implements IResourceManagerReloadListener {

    private static final Logger LOG = LogManager.getLogger(NovaRenderer.class);
    private BlockModelShapes shapes;

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

    private PriorityQueue<ChunkUpdateListener.BlockUpdateRange> chunksToUpdate;
    private Set<ChunkUpdateListener.BlockUpdateRange> updatedChunks = new HashSet<>();
    private World world;

    final private Executor chunkUpdateThreadPool = Executors.newFixedThreadPool(10);

    private ChunkBuilder chunkBuilder;
    private HashMap<String, IGeometryFilter> filterMap;

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
        addAtlas(resourceManager, guiAtlas, GUI_COLOR_TEXTURES_LOCATIONS, guiSpriteLocations, GUI_ATLAS_NAME);
        LOG.debug("Created GUI atlas");
    }

    private void addFontAtlas(@Nonnull IResourceManager resourceManager) {
        addAtlas(resourceManager, fontAtlas, FONT_COLOR_TEXTURES_LOCATIONS, fontSpriteLocations, FONT_ATLAS_NAME);
        LOG.debug("Created font atlas");
    }

    public void addTerrainAtlas(@Nonnull TextureMap blockColorMap) {
        // Copy over the atlas
        NovaNative.mc_atlas_texture blockColorTexture = getFullImage(blockColorMap.getWidth(), blockColorMap.getHeight(), blockColorMap.getMapUploadedSprites().values());
        blockColorTexture.name = BLOCK_COLOR_ATLAS_NAME;
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

        LOG.info("Adding atlas texture {}", atlasTexture);
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

    public void preInit() {
        System.getProperties().setProperty("jna.library.path", System.getProperty("java.library.path"));
        System.getProperties().setProperty("jna.dump_memory", "false");
        String pid = ManagementFactory.getRuntimeMXBean().getName();
        LOG.info("PID: " + pid + " TID: " + Thread.currentThread().getId());
        NovaNative.INSTANCE.initialize();
        LOG.info("Native code initialized");
        updateWindowSize();

        // Moved here so that it's initialized after the native code is loaded
        chunksToUpdate = new PriorityQueue<>((range1, range2) -> {
            Vec3i range1Center = new Vec3i();
            Vec3i range2Center = new Vec3i();

            Glm.add(range1Center, range1.min, new Vec3i(8, 128, 8));
            Glm.add(range2Center, range2.min, new Vec3i(8, 128, 8));

            Entity player = Minecraft.getMinecraft().thePlayer;
            Vec2 playerPos = new Vec2(player.posX, player.posZ);
            float range1DistToPlayer = new Vec2().distance(new Vec2(range1Center.x, range1Center.z), playerPos);
            float range2DistToPlayer = new Vec2().distance(new Vec2(range2Center.x, range2Center.z), playerPos);

            return Float.compare(range1DistToPlayer, range2DistToPlayer);
        });
        chunkUpdateListener  = new ChunkUpdateListener(chunksToUpdate);
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

        EntityRenderer entityRenderer = Minecraft.getMinecraft().entityRenderer;

        boolean shouldUpdateLightmap = entityRenderer.isLightmapUpdateNeeded();
        entityRenderer.updateLightmap(renderPartialTicks);
        if(shouldUpdateLightmap) {
            sendLightmapTexture(entityRenderer.getLightmapTexture());
        }


        Profiler.start("render_gui");
        if (mc.currentScreen != null) {

            NovaDraw.novaDrawScreen(mc.currentScreen, renderPartialTicks);

        }
        Profiler.end("render_gui");

        Profiler.start("update_chunks");
        int numChunksUpdated = 0;
        while(!chunksToUpdate.isEmpty()) {
            ChunkUpdateListener.BlockUpdateRange range = chunksToUpdate.remove();
            // chunkBuilder.createMeshesForChunk(range);
            chunkUpdateThreadPool.execute(() -> chunkBuilder.createMeshesForChunk(range));
            updatedChunks.add(range);
            numChunksUpdated++;
            if(numChunksUpdated > 10) {
                break;
            }
        }
        Profiler.end("update_chunks");

        Profiler.start("update_player");
        EntityPlayerSP viewEntity = mc.thePlayer;
        if(viewEntity != null) {
            float pitch = viewEntity.rotationPitch;
            float yaw = viewEntity.rotationYaw;
            double x = viewEntity.posX;
            double y = viewEntity.posY + viewEntity.getEyeHeight();
            double z = viewEntity.posZ;
            NovaNative.INSTANCE.set_player_camera_transform(x, y, z, yaw, pitch);
        }
        Profiler.end("update_player");

        Profiler.start("execute_frame");
        NovaNative.INSTANCE.execute_frame();
        Profiler.end("execute_frame");

        Profiler.start("update_window");
        updateWindowSize();
        Profiler.end("update_window");
        int scalefactor = new ScaledResolution(mc).getScaleFactor() * 2;
        if (scalefactor != this.scalefactor) {
            NovaNative.INSTANCE.set_float_setting("scalefactor", scalefactor);
            this.scalefactor = scalefactor;
        }

        printProfilerData();
    }

    private void sendLightmapTexture(DynamicTexture lightmapTexture) {
        int[] data = lightmapTexture.getTextureData();
        NovaNative.INSTANCE.send_lightmap_texture(data, data.length, lightmapTexture.getWidth(), lightmapTexture.getHeight());
    }

    private void printProfilerData() {
        Profiler.logData();
    }

    public void setWorld(World world) {
        if(world != null) {
            world.addEventListener(chunkUpdateListener);
            this.world = world;
            chunksToUpdate.clear();

            if(chunkBuilder != null) {
                chunkBuilder.setWorld(world);
            }
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

    public static String atlasTextureOfSprite(ResourceLocation texture) {
        ResourceLocation strippedLocation = new ResourceLocation(texture.getResourceDomain(), texture.getResourcePath().replace(".png", "").replace("textures/", ""));

        if (BLOCK_COLOR_TEXTURES_LOCATIONS.contains(strippedLocation)) {
            return BLOCK_COLOR_ATLAS_NAME;
        } else if (GUI_COLOR_TEXTURES_LOCATIONS.contains(strippedLocation) || texture == WHITE_TEXTURE_GUI_LOCATION) {
            return GUI_ATLAS_NAME;
        } else if (FONT_COLOR_TEXTURES_LOCATIONS.contains(strippedLocation)) {
            return FONT_ATLAS_NAME;
        }

        return texture.toString();
    }

    public void loadShaderpack(String shaderpackName, BlockColors blockColors) {
        Profiler.start("load_shaderpack");
        NovaNative.INSTANCE.set_string_setting("loadedShaderpack", shaderpackName);

        String filters = NovaNative.INSTANCE.get_shaders_and_filters();
        Profiler.end("load_shaderpack");

        if(filters.length() < 2 || filters.length() % 2 != 0) {
            throw new IllegalStateException("Must have a POT number of filters and shader names");
        }

        Profiler.start("build_filters");
        String[] filtersSplit = filters.split("\n");

        LOG.debug("Filters: '{}'", String.join(", ", filtersSplit));
        LOG.debug("Received {} shaders with filters", filtersSplit.length);

        filterMap = new HashMap<>();
        for(int i = 0; i < filtersSplit.length; i += 2) {
            String filterName = filtersSplit[i];
            IGeometryFilter filter = IGeometryFilter.parseFilterString(filtersSplit[i + 1]);
            filterMap.put(filterName, filter);
        }
        Profiler.end("build_filters");

        Profiler.start("new_chunk_builder");
        chunkBuilder = new ChunkBuilder(filterMap, world, blockColors);

        chunksToUpdate.addAll(updatedChunks);
        updatedChunks.clear();
        Profiler.end("new_chunk_builder");
    }
}


