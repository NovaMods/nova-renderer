package com.continuum.nova.mixin.gui;

import com.continuum.nova.NovaConstants;
import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.client.gui.GuiButtonLanguage;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

import java.awt.geom.Rectangle2D;

@Mixin(GuiButtonLanguage.class)
public class MixinGuiButtonLanguage extends GuiButton {
    public MixinGuiButtonLanguage(int buttonId, int x, int y, String buttonText) {
        super(buttonId, x, y, buttonText);
    }

    /**
     * @author Janrupf
     * @reason Redirect to nova needed
     * @inheritDoc
     */
    @Overwrite
    public void drawButton(Minecraft mc, int mouseX, int mouseY) {
        if (this.visible) {
            NovaDraw.incrementZ();
            NovaDraw.drawRectangle(
                    BUTTON_TEXTURES,
                    new Rectangle2D.Float(xPosition, yPosition, width, height),
                    new Rectangle2D.Float(
                            0, 0.4140625f + (hovered ? 0.078125f : 0),
                            width * NovaConstants.WIDGETS_TEXTURE_FACTOR, height * NovaConstants.WIDGETS_TEXTURE_FACTOR)
            );
        }
    }
}
