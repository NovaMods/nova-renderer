package com.continuum.nova;

import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.util.ResourceLocation;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.lang.management.ManagementFactory;

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
    private final ResourceLocation[] TEXTURES_LOCATION = {
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

        for(ResourceLocation textureLocation : TEXTURES_LOCATION) {
            try {
                IResource texture = resourceManager.getResource(textureLocation);

                BufferedInputStream in = new BufferedInputStream(texture.getInputStream());
                BufferedImage image = ImageIO.read(in);
                int width = image.getWidth();
                int height = image.getHeight();
                byte[] pixels = ((DataBufferByte) image.getRaster().getDataBuffer()).getData();

                NovaNative.INSTANCE.add_texture(new NovaNative.mc_texture(width, height, 4, pixels, texture.getResourceLocation().getResourcePath()));
            } catch(IOException e) {
                LOG.warn("IOException when loading texture " + textureLocation.toString() + ": " +e.getMessage());
            }
        }

    }

    public void preInit() {
        // TODO: Remove this and use the win32-x86 thing to package the DLL into the jar
        System.getProperties().setProperty("jna.library.path", "C:/Users/David/Documents/Nova Renderer/run");
        LOG.info("Pre-initing");
        LOG.info("PID: " + ManagementFactory.getRuntimeMXBean().getName());
        LOG.info("Attempting to initialize native code...");
        NovaNative.INSTANCE.init_nova();
        LOG.info("Native code initialized!");
    }
}
