package com.continuum.nova.mixin.gui;

import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.gui.GuiLabel;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(GuiLabel.class)
public class MixinGuiLabel {
    @Inject(method = "drawLabel", at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/GlStateManager;enableBlend()V", shift = At.Shift.BEFORE))
    private void hookDrawLabel(CallbackInfo callbackInfo) {
        NovaDraw.incrementZ();
    }
}
