package com.continuum.nova.mixin.renderer;

import com.continuum.nova.NovaRenderer;
import com.continuum.nova.interfaces.INovaEntityRenderer;
import com.continuum.nova.system.NovaNative;
import net.minecraft.client.renderer.EntityRenderer;
import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.client.renderer.texture.DynamicTexture;
import net.minecraft.client.settings.GameSettings;
import org.spongepowered.asm.mixin.*;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Redirect;

@Mixin(EntityRenderer.class)
public abstract class MixinEntityRenderer implements INovaEntityRenderer {

    @Shadow
    private boolean lightmapUpdateNeeded;

    @Shadow
    @Final
    private DynamicTexture lightmapTexture;

    @Shadow protected abstract void updateLightmap(float partialTicks);

    @Override public void updateLightmapNOVA(float partialTicks) {
        updateLightmap(partialTicks);
    }

    @Override public boolean isLightmapUpdateNeeded() {
        return lightmapUpdateNeeded;
    }

    @Override public DynamicTexture getLightmapTexture() {
        return lightmapTexture;
    }

    @Redirect(method = "renderWorld",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/EntityRenderer;updateLightmap(F)V"))
    private void noopUpdateLightmapInRender(EntityRenderer _this, float partialTicks) {
    }

    @Redirect(method = "updateCameraAndRender", at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/Display;isActive()Z"))
    private boolean isWindowActiveNova() {
        return NovaNative.displayIsActive();
    }

    @Redirect(method = "renderWorldPass",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/EntityRenderer;setupFog(IF)V"))
    private void novaSetupFog(EntityRenderer _this, int startCoords, float partialTicks) {
        // currently do nothing, replace it because it crashes
    }

    @Redirect(method = "renderWorldPass",
            at = @At(
                    value = "INVOKE",
                    target = "Lnet/minecraft/client/renderer/EntityRenderer;renderCloudsCheck(Lnet/minecraft/client/renderer/RenderGlobal;FIDDD)V"
            ))
    private void renderCloudsNova(EntityRenderer _this, RenderGlobal renderGlobalIn, float partialTicks, int pass, double x, double y, double z) {
        // currently do nothing, replace it because it crashes
    }

    @Redirect(method = "renderWorldPass",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/EntityRenderer;renderHand(FI)V"))
    private void novaRenderHand(EntityRenderer _this, float partialTicks, int pass) {
        // currently do nothing, replace it because it crashes
    }

    @Redirect(method = "renderWorldPass",
            at = @At(
                    value = "INVOKE",
                    target = "Lnet/minecraft/client/renderer/EntityRenderer;setupCameraTransform(FI)V"
            ))
    private void novaSetupCameraTransform(EntityRenderer _this, float partialTicks, int pass) {
        // done in native code
    }

    @Redirect(method = "renderWorldPass",
            at = @At(
                    value = "FIELD",
                    target = "Lnet/minecraft/client/settings/GameSettings;renderDistanceChunks:I"
            ))
    private int novaForceNoSky(GameSettings _this) {
        // do custom sky rendering later once we get to it
        // the only access to renderDistance here is to check if it's >= 4 to do sky rendering
        return 0;
    }
}
