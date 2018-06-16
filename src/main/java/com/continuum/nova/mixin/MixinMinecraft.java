package com.continuum.nova.mixin;

import com.continuum.nova.NovaRenderer;
import com.continuum.nova.input.Keyboard;
import com.continuum.nova.input.Mouse;
import com.continuum.nova.system.NovaNative;
import net.minecraft.client.Minecraft;
import net.minecraft.client.multiplayer.WorldClient;
import net.minecraft.client.renderer.color.BlockColors;
import net.minecraft.util.Timer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;
import org.spongepowered.asm.mixin.injection.callback.LocalCapture;

@Mixin(Minecraft.class)
public abstract class MixinMinecraft {

    @Shadow
    public boolean inGameHasFocus;

    @Shadow public int displayWidth;
    @Shadow public int displayHeight;

    @Shadow public abstract void resize(int width, int height);

    @Shadow @Final private Timer timer;
    @Shadow public WorldClient world;
    @Shadow private BlockColors blockColors;
    private Logger novaLogger;

    @Inject(method = "<init>", at = @At("RETURN"))
    private void constructorEnd(CallbackInfo info) {
        novaLogger = LogManager.getLogger("Nova");
        novaLogger.info("Loading nova...");
        NovaRenderer.create();
        NovaRenderer.getInstance().preInit();
        Mouse.create();
        Keyboard.create();
        // inGameHasFocus = NovaRenderer.getInstance().getNative().display_is_active();
    }

    /**
     * @author Janrupf
     * @reason Overwritten because this has to be handled by nova now
     * @inheritDoc
     */
    @Overwrite
    protected void checkWindowResize() {
        if (NovaRenderer.getInstance().wasResized()) {
            this.displayWidth = NovaRenderer.getInstance().getWidth();
            this.displayHeight = NovaRenderer.getInstance().getHeight();


            novaLogger.info("Window was resized to: " + displayWidth + "x" + displayHeight);
            this.resize(this.displayWidth, this.displayHeight);
        }
    }

    @Inject(
            method = "runGameLoop",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/EntityRenderer;updateCameraAndRender(FJ)V", shift = At.Shift.AFTER),
            locals = LocalCapture.CAPTURE_FAILEXCEPTION
    )
    private void hookUpdateCameraAndWindow(CallbackInfo callbackInfo) {
        NovaRenderer.getInstance().updateCameraAndRender(timer.renderPartialTicks, Minecraft.getMinecraft());
    }

    @Inject(
            method = "loadWorld(Lnet/minecraft/client/multiplayer/WorldClient;Ljava/lang/String;)V",
            at = @At(value = "FIELD", target = "Lnet/minecraft/client/Minecraft;world:Lnet/minecraft/client/multiplayer/WorldClient;", ordinal = 4, shift = At.Shift.AFTER)
    )
    private void afterWorldAssigned(CallbackInfo callbackInfo) {
        if(world == null) {
            throw new RuntimeException("WORLD IS NULL; DEBUG");
        }
        NovaRenderer.getInstance().setWorld(world);
    }

    @Inject(
            method = "init",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/color/BlockColors;init()Lnet/minecraft/client/renderer/color/BlockColors;", shift = At.Shift.AFTER)
    )
    private void afterBlockColorsInit(CallbackInfo callbackInfo) {
        NovaRenderer.getInstance().loadShaderpack("default", blockColors);
    }
}
