package com.continuum.nova.mixin;

import org.lwjgl.input.Mouse;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

@Mixin(Mouse.class)
public class MixinLWJGLMouse {

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isClipMouseCoordinatesToWindow() {
        return com.continuum.nova.input.Mouse.isClipMouseCoordinatesToWindow();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void setClipMouseCoordinatesToWindow(boolean clip) {
        com.continuum.nova.input.Mouse.setClipMouseCoordinatesToWindow(clip);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void setCursorPosition(int x, int y) {
        com.continuum.nova.input.Mouse.setCursorPosition(x, y);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void create() {
        com.continuum.nova.input.Mouse.create();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isCreated() {
        return com.continuum.nova.input.Mouse.isCreated();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void destroy() {
        com.continuum.nova.input.Mouse.destroy();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isButtonDown(int button) {
        return com.continuum.nova.input.Mouse.isButtonDown(button);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static String getButtonName(int button) {
        return com.continuum.nova.input.Mouse.getButtonName(button);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getButtonIndex(String buttonName) {
        return com.continuum.nova.input.Mouse.getButtonIndex(buttonName);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean next() {
        return com.continuum.nova.input.Mouse.next();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getEventButton() {
        return com.continuum.nova.input.Mouse.getEventButton();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean getEventButtonState() {
        return com.continuum.nova.input.Mouse.getEventButtonState();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getEventX() {
        return com.continuum.nova.input.Mouse.getEventX();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getEventY() {
        return com.continuum.nova.input.Mouse.getEventY();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getEventDWheel() {
        return com.continuum.nova.input.Mouse.getEventDWheel();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static long getEventNanoseconds() {
        return com.continuum.nova.input.Mouse.getEventNanoseconds();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getX() {
        return com.continuum.nova.input.Mouse.getX();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getY() {
        return com.continuum.nova.input.Mouse.getY();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getDX() {
        return com.continuum.nova.input.Mouse.getDX();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getDY() {
        return com.continuum.nova.input.Mouse.getDY();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getDWheel() {
        return com.continuum.nova.input.Mouse.getDWheel();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static int getButtonCount() {
        return com.continuum.nova.input.Mouse.getButtonCount();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean hasWheel() {
        return com.continuum.nova.input.Mouse.hasWheel();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isGrabbed() {
        return com.continuum.nova.input.Mouse.isGrabbed();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void setGrabbed(boolean grab) {
        com.continuum.nova.input.Mouse.setGrabbed(grab);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static void updateCursor() {
        com.continuum.nova.input.Mouse.updateCursor();
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public static boolean isInsideWindow() {
        return com.continuum.nova.input.Mouse.isInsideWindow();
    }
}
