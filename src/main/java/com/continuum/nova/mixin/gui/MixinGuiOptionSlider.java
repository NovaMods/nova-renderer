package com.continuum.nova.mixin.gui;

import com.continuum.nova.NovaConstants;
import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.client.gui.GuiOptionSlider;
import net.minecraft.client.settings.GameSettings;
import net.minecraft.util.math.MathHelper;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;

import java.awt.*;
import java.awt.geom.Rectangle2D;

@Mixin(GuiOptionSlider.class)
public class MixinGuiOptionSlider extends GuiButton {
    @Shadow public boolean dragging;

    @Shadow private float sliderValue;

    @Shadow @Final private GameSettings.Options options;

    public MixinGuiOptionSlider(int buttonId, int x, int y, String buttonText) {
        super(buttonId, x, y, buttonText);
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    protected void mouseDragged(Minecraft mc, int mouseX, int mouseY) {
        if (this.visible)
        {
            if (this.dragging)
            {
                this.sliderValue = (float)(mouseX - (this.xPosition + 4)) / (float)(this.width - 8);
                this.sliderValue = MathHelper.clamp(this.sliderValue, 0.0F, 1.0F);
                float f = this.options.denormalizeValue(this.sliderValue);
                mc.gameSettings.setOptionFloatValue(this.options, f);
                this.sliderValue = this.options.normalizeValue(f);
                this.displayString = mc.gameSettings.getKeyBinding(this.options);
            }



            float offsetY = 0.078125f;
            NovaDraw.incrementZ();

            NovaDraw.drawRectangle(
                    BUTTON_TEXTURES,
                    new Rectangle2D.Float(this.xPosition + (int)(this.sliderValue * (float)(this.width - 8)), yPosition,4, height),
                    new Rectangle2D.Float(
                            NovaConstants.STANDARD_BUTTON_LOCATIONS[0],
                            NovaConstants.STANDARD_BUTTON_LOCATIONS[1] + offsetY,
                            4* NovaConstants.WIDGETS_TEXTURE_FACTOR,
                            height * NovaConstants.WIDGETS_TEXTURE_FACTOR),
                    Color.white
            );
            NovaDraw.drawRectangle(
                    BUTTON_TEXTURES,
                    new Rectangle2D.Float(this.xPosition + (int)(this.sliderValue * (float)(this.width - 8)) + 4, yPosition,4, height),
                    new Rectangle2D.Float(
                            NovaConstants.STANDARD_BUTTON_LOCATIONS[2] - (4 * NovaConstants.WIDGETS_TEXTURE_FACTOR),
                            NovaConstants.STANDARD_BUTTON_LOCATIONS[1] + offsetY,
                            4 * NovaConstants.WIDGETS_TEXTURE_FACTOR,
                            height * NovaConstants.WIDGETS_TEXTURE_FACTOR),
                    Color.white
            );
        }
    }
}
