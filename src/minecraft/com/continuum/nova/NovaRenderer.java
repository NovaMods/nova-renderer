package com.continuum.nova;

import com.continuum.nova.nativethread.NativeInterface;
import com.continuum.nova.utils.NativeThreadException;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.lang.management.ManagementFactory;

/**
 * Created by David on 24-Dec-15.
 *
 * Entry point for the mod. Will use JVM trickery to replace certain MC classes amd methods with some JNI classes and
 * methods, then will re-initialize MC with all the new things. Should be super gross.
 */
public class NovaRenderer {
    public static final String MODID = "Nova Renderer";
    public static final String VERSION = "0.0.1";

    private static final Logger LOG = LogManager.getLogger(NovaRenderer.class);

    static {
        System.loadLibrary("/libvulkan-mod-native");
    }

    public void preInit() {
        LOG.info("Pre-initing");
        LOG.info("PID: " + ManagementFactory.getRuntimeMXBean().getName());
        LOG.info("Attempting to initialize native code...");
        NativeInterface.startup();
        LOG.info("Native code initialized!");
    }
}
