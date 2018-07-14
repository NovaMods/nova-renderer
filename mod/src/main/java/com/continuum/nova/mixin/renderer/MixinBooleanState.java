package com.continuum.nova.mixin.renderer;

import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(targets = "net.minecraft.client.renderer.GlStateManager$BooleanState")
public class MixinBooleanState {

    @Inject(
            method = "setState(Z)V",
            at = {
                    @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glEnable(I)V", shift = At.Shift.BEFORE, remap = false),
                    @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/GL11;glDisable(I)V", shift = At.Shift.BEFORE, remap = false)
            },
            cancellable = true
    )
    private void cancelDepthFunc(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }
}
