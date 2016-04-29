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
import java.util.List;

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

    // So I only have to allocate it once. Small optimization, but it shouldn't hurt readability at all
    private final ResourceLocation texturesLocation = new ResourceLocation("assets/minecraft/textures");

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

        try {
            List<IResource> textures = resourceManager.getAllResources(texturesLocation);

            for(IResource texture : textures) {
                BufferedInputStream in = new BufferedInputStream(texture.getInputStream());
                BufferedImage image = ImageIO.read(in);
                int width = image.getWidth();
                int height = image.getHeight();
                byte[] pixels = ((DataBufferByte) image.getRaster().getDataBuffer()).getData();

                NovaNative.INSTANCE.add_texture(new NovaNative.mc_texture(width, height, 4, pixels, texture.getResourceLocation().getResourcePath()));
            }

        } catch(IOException e) {
            LOG.trace(e);
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
