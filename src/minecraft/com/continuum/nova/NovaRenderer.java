package com.continuum.nova;

import com.continuum.nova.nativethread.NativeInterface;
import com.continuum.nova.utils.NativeThreadException;
import net.minecraftforge.fml.common.Mod;
import net.minecraftforge.fml.common.event.FMLInitializationEvent;
import net.minecraftforge.fml.common.event.FMLPreInitializationEvent;
import net.minecraftforge.fml.relauncher.IFMLLoadingPlugin;

import java.lang.management.ManagementFactory;
import java.util.Map;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

// TODO: Find a better name

/**
 * Created by David on 24-Dec-15.
 *
 * Entry point for the mod. Will use JVM trickery to replace certain MC classes amd methods with some JNI classes and
 * methods, then will re-initialize MC with all the new things. Should be super gross.
 */
@IFMLLoadingPlugin.Name(NovaRenderer.MODID)
@IFMLLoadingPlugin.MCVersion("1.8.8")
@IFMLLoadingPlugin.TransformerExclusions("com.continuum.nova")
@IFMLLoadingPlugin.SortingIndex(1001)
@Mod(modid = NovaRenderer.MODID, version = NovaRenderer.VERSION)
public class NovaRenderer implements IFMLLoadingPlugin {
    public static final String MODID = "Nova Renderer";
    public static final String VERSION = "0.0.1";

    private static final Logger LOG = LogManager.getLogger(NovaRenderer.class);

    static {
        System.loadLibrary("/libvulkan-mod-native");
    }

    @Mod.EventHandler
    public void preInit(FMLPreInitializationEvent event) {
        LOG.info("Pre-initing");
        LOG.info("PID: " + ManagementFactory.getRuntimeMXBean().getName());
        try {
            LOG.info("Attempting to initialize native code...");
            NativeInterface.startup();
            LOG.info("Native thread running!");
        } catch(NativeThreadException e) {
            LOG.trace(e);
            LOG.fatal("Could not start rendering thread");
        }
    }

    @Mod.EventHandler
    public void init(FMLInitializationEvent event) {
    }

    @Override
    public String[] getASMTransformerClass() {
        // Do nothing for now
        return new String[]{"com.continuum.nova.transformers.VulkanModTransformer"};
    }

    @Override
    public String getModContainerClass() {
        return null;
    }

    @Override
    public String getSetupClass() {
        return null;
    }

    @Override
    public void injectData(Map<String, Object> data) {
        System.out.println("Injected data: " + data);
        // TODO: Figure out if I can look at the other coremods for compatibility
    }

    @Override
    public String getAccessTransformerClass() {
        return "";
    }
}
