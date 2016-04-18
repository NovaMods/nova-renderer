package com.continuum.nova.nativethread;

import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;


/**
 * The interface into the native code that flops around all the time
 *
 * <p>The main rendering method sets a specific </p>
 *
 * <p>Utilizes the singleton pattern to allow for global access. I don't want to throw an instance around to everything
 * and its brother</p>
 *
 * @author David
 */
public class NativeInterface {
    private static final Logger LOG = LogManager.getLogger(NativeInterface.class);
    private static NativeInterface instance = null;
    private long nativeHandle;

    private boolean renderReady = false;

    public static void startup() {
        if(instance == null) {
            instance = new NativeInterface();
        }
    }

    public static NativeInterface getInstance() {
        return instance;
    }

    private NativeInterface() {
        runVulkanMod();
        LOG.info("Native object at memory location " + nativeHandle);
    }

    private native void runVulkanMod();

    public native void setWorld(World world);
}
