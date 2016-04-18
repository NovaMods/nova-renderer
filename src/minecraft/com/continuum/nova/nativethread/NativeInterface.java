package com.continuum.nova.nativethread;

import com.continuum.nova.utils.NativeThreadException;
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
    private static NativeInterface instance;
    private long instancePointer;

    private boolean renderReady = false;

    public static void startup() throws NativeThreadException {
        instance = new NativeInterface();
    }

    public static NativeInterface getInstance() {
        return instance;
    }

    private NativeInterface() {
        instancePointer = runVulkanMod();
        LOG.info("Native object at memory location " + instancePointer);
    }

    private native long runVulkanMod();
}
