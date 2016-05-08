package com.continuum.nova;

import com.continuum.nova.utils.AtlasGenerator;
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
import java.awt.image.DataBufferInt;
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
    private final ResourceLocation[] ALBEDO_TEXTURES_LOCATION = {
            new ResourceLocation("textures/blocks/anvil_base.png"),
            new ResourceLocation("textures/blocks/anvil_top_damaged_0.png"),
            new ResourceLocation("textures/blocks/anvil_top_damaged_1.png"),
            new ResourceLocation("textures/blocks/anvil_top_damaged_2.png"),
            new ResourceLocation("textures/blocks/beacon.png"),
            new ResourceLocation("textures/blocks/bedrock.png"),
            new ResourceLocation("textures/blocks/bed_feet_end.png"),
            new ResourceLocation("textures/blocks/bed_feet_side.png"),
            new ResourceLocation("textures/blocks/bed_feet_top.png"),
            new ResourceLocation("textures/blocks/bed_head_end.png"),
            new ResourceLocation("textures/blocks/bed_head_side.png"),
            new ResourceLocation("textures/blocks/bed_head_top.png"),
            new ResourceLocation("textures/blocks/beetroots_stage_0.png"),
            new ResourceLocation("textures/blocks/beetroots_stage_1.png"),
            new ResourceLocation("textures/blocks/beetroots_stage_2.png"),
            new ResourceLocation("textures/blocks/beetroots_stage_3.png"),
            new ResourceLocation("textures/blocks/grass_top.png"),
            new ResourceLocation("textures/blocks/grass_side.png"),
    };

    private boolean firstLoad = true;

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
        maxAtlasSize = Math.min(maxAtlasSize, 8192);

        AtlasGenerator gen = new AtlasGenerator();

        List<AtlasGenerator.ImageName> images = new ArrayList<>();

        for(ResourceLocation textureLocation : ALBEDO_TEXTURES_LOCATION) {
            try {
                IResource texture = resourceManager.getResource(textureLocation);

                BufferedInputStream in = new BufferedInputStream(texture.getInputStream());
                BufferedImage image = ImageIO.read(in);

                images.add(new AtlasGenerator.ImageName(image, textureLocation.toString()));

            } catch(IOException e) {
                LOG.warn("IOException when loading texture " + textureLocation.toString() + ": " +e.getMessage());
            }
        }

        List<AtlasGenerator.Texture> albedoAtlases = gen.Run("albedo", maxAtlasSize, maxAtlasSize, 1, true, images);

        for(AtlasGenerator.Texture texture : albedoAtlases) {
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
                NovaNative.INSTANCE.add_texture(atlasTex, NovaNative.AtlasType.TERRAIN.ordinal(), NovaNative.TextureType.ALBEDO.ordinal());

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
}
