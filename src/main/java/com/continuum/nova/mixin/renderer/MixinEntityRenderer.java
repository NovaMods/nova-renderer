package com.continuum.nova.mixin.renderer;

import com.continuum.nova.NovaRenderer;
import com.continuum.nova.input.Mouse;
import com.continuum.nova.interfaces.INovaEntityRenderer;
import com.continuum.nova.system.NovaNative;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.EntityRenderer;
import net.minecraft.client.renderer.texture.DynamicTexture;
import org.lwjgl.opengl.Display;
import org.spongepowered.asm.mixin.*;

@Mixin(EntityRenderer.class)
@Implements(@Interface(iface = INovaEntityRenderer.class, prefix = "nova$"))
public abstract class MixinEntityRenderer {
    @Shadow
    protected abstract void updateLightmap(float partialTicks);

    @Shadow
    @Final
    private Minecraft mc;

    @Shadow
    private float smoothCamYaw;

    @Shadow
    private float smoothCamPitch;

    @Shadow
    private float smoothCamPartialTicks;

    @Shadow
    private float smoothCamFilterX;

    @Shadow
    private float smoothCamFilterY;

    @Shadow
    private boolean lightmapUpdateNeeded;

    @Shadow
    @Final
    private DynamicTexture lightmapTexture;

    public void nova$updateLightmap(float partialTicks) {
        updateLightmap(partialTicks);
    }

    /**
     * @author Janrupf
     * @reason Overwritten because a lot of changes are required that cannot be done at one specific point in the method
     * @inheritDoc
     */
    @Overwrite
    public void updateCameraAndRender(float partialTicks, long nanoTime) {
        boolean flag = NovaRenderer.getInstance().getNative().display_is_active();
        this.mc.mcProfiler.startSection("mouse");

        if (flag && Minecraft.IS_RUNNING_ON_MAC && this.mc.inGameHasFocus && !Mouse.isInsideWindow()) {
            Mouse.setGrabbed(false);
            Mouse.setCursorPosition(Display.getWidth() / 2, Display.getHeight() / 2 - 20);
            Mouse.setGrabbed(true);
        }

        if (this.mc.inGameHasFocus && flag) {
            this.mc.mouseHelper.mouseXYChange();
            float f = this.mc.gameSettings.mouseSensitivity * 0.6F + 0.2F;
            float f1 = f * f * f * 8.0F;
            float f2 = (float) this.mc.mouseHelper.deltaX * f1;
            float f3 = (float) this.mc.mouseHelper.deltaY * f1;
            int i = 1;

            if (this.mc.gameSettings.invertMouse) {
                i = -1;
            }

            if (this.mc.gameSettings.smoothCamera) {
                this.smoothCamYaw += f2;
                this.smoothCamPitch += f3;
                float f4 = partialTicks - this.smoothCamPartialTicks;
                this.smoothCamPartialTicks = partialTicks;
                f2 = this.smoothCamFilterX * f4;
                f3 = this.smoothCamFilterY * f4;
                this.mc.player.turn(f2, f3 * (float) i);
            } else {
                this.smoothCamYaw = 0.0F;
                this.smoothCamPitch = 0.0F;
                this.mc.player.turn(f2, f3 * (float) i);
            }
        }

        this.mc.mcProfiler.endSection();
    }

    public boolean nova$isLightmapUpdateNeeded() {
        return lightmapUpdateNeeded;
    }

    public DynamicTexture nova$getLightmapTexture() {
        return lightmapTexture;
    }
}
