package com.continuum.nova.mixin.gui;

import com.continuum.nova.NovaConstants;
import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.gui.GuiButton;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

import java.awt.*;
import java.awt.geom.Rectangle2D;

@Mixin(targets = {"net.minecraft.client.gui.GuiScreenOptionsSounds$Button"})
public class MixinGuiScreenOptionsSounds$Button extends GuiButton {
    @Shadow public float volume;

    public MixinGuiScreenOptionsSounds$Button(int buttonId, int x, int y, String buttonText) {
        super(buttonId, x, y, buttonText);
    }

    @Inject(
            method = "mouseDragged(Lnet/minecraft/client/Minecraft;II)V",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/GlStateManager;color(FFFF)V", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private void hookAndReturnMouseDragged(CallbackInfo callbackInfo) {
        float offsetY = 0.078125f;
        NovaDraw.incrementZ();

        NovaDraw.drawRectangle(
                BUTTON_TEXTURES,
                new Rectangle2D.Float(this.xPosition + (int)(this.volume * (float)(this.width - 8)), yPosition,4, height),
                new Rectangle2D.Float(
                        NovaConstants.STANDARD_BUTTON_LOCATIONS[0],
                        NovaConstants.STANDARD_BUTTON_LOCATIONS[1] + offsetY,
                        4* NovaConstants.WIDGETS_TEXTURE_FACTOR,
                        height * NovaConstants.WIDGETS_TEXTURE_FACTOR),
                Color.white
        );
        NovaDraw.drawRectangle(
                BUTTON_TEXTURES,
                new Rectangle2D.Float(this.xPosition + (int)(this.volume * (float)(this.width - 8)) + 4, yPosition,4, height),
                new Rectangle2D.Float(
                        NovaConstants.STANDARD_BUTTON_LOCATIONS[2] - (4 * NovaConstants.WIDGETS_TEXTURE_FACTOR),
                        NovaConstants.STANDARD_BUTTON_LOCATIONS[1] + offsetY,
                        4 * NovaConstants.WIDGETS_TEXTURE_FACTOR,
                        height * NovaConstants.WIDGETS_TEXTURE_FACTOR),
                Color.white
        );

        callbackInfo.cancel();
    }
}
