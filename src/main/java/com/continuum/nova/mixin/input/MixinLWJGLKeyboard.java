package com.continuum.nova.mixin.input;

import org.lwjgl.input.Keyboard;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

@Mixin(value = Keyboard.class, remap = false)
public class MixinLWJGLKeyboard {
    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void create() {
        com.continuum.nova.input.Keyboard.create();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isCreated() {
        return com.continuum.nova.input.Keyboard.isCreated();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void poll() {
        com.continuum.nova.input.Keyboard.poll();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isKeyDown(int key)  {
        return com.continuum.nova.input.Keyboard.isKeyDown(key);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static synchronized String getKeyName(int key) {
        return com.continuum.nova.input.Keyboard.getKeyName(key);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static synchronized int getKeyIndex(String keyName) {
        return com.continuum.nova.input.Keyboard.getKeyIndex(keyName);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean next() {
        return com.continuum.nova.input.Keyboard.next();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void enableRepeatEvents(boolean enable) {
        com.continuum.nova.input.Keyboard.enableRepeatEvents(enable);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static char getEventCharacter() {
        return com.continuum.nova.input.Keyboard.getEventCharacter();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getEventKey() {
        return com.continuum.nova.input.Keyboard.getEventKey();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean getEventKeyState() {
        return com.continuum.nova.input.Keyboard.getEventKeyState();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isRepeatEvent() {
        return com.continuum.nova.input.Keyboard.isRepeatEvent();
    }
}
