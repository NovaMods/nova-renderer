package com.continuum.nova.mixin.gui;

import com.continuum.nova.NovaConstants;
import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.FontRenderer;
import net.minecraft.client.gui.Gui;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.util.ResourceLocation;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;

import java.awt.geom.Rectangle2D;

@Mixin(GuiButton.class)
public abstract class MixinGuiButton extends Gui {

    @Shadow public boolean visible;
    @Shadow protected boolean hovered;
    @Shadow public int x;
    @Shadow public int y;
    @Shadow public int width;
    @Shadow public int height;

    @Shadow protected abstract int getHoverState(boolean mouseOver);

    @Shadow @Final protected static ResourceLocation BUTTON_TEXTURES;

    @Shadow protected abstract void mouseDragged(Minecraft mc, int mouseX, int mouseY);

    @Shadow public boolean enabled;
    @Shadow public String displayString;


    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawButton(Minecraft mc, int mouseX, int mouseY, float partialTick) {
        if (this.visible) {
            float offsetY;
            this.hovered = mouseX >= this.x && mouseY >= this.y && mouseX < this.x + this.width && mouseY < this.y + this.height;
            int drawState = getHoverState(hovered);
            offsetY = (float) drawState * 0.078125f;
            NovaDraw.incrementZ();

            NovaDraw.drawRectangle(
                    BUTTON_TEXTURES,
                    new Rectangle2D.Float(x, y, width / 2, height),
                    new Rectangle2D.Float(
                            NovaConstants.STANDARD_BUTTON_LOCATIONS[0], NovaConstants.STANDARD_BUTTON_LOCATIONS[1] + offsetY,
                            width / 2 * NovaConstants.WIDGETS_TEXTURE_FACTOR, height * NovaConstants.WIDGETS_TEXTURE_FACTOR)
            );
            NovaDraw.drawRectangle(
                    BUTTON_TEXTURES,
                    new Rectangle2D.Float(x + width / 2, y, width / 2, height),
                    new Rectangle2D.Float(
                            NovaConstants.STANDARD_BUTTON_LOCATIONS[2] - ((width / 2) * NovaConstants.WIDGETS_TEXTURE_FACTOR),
                            NovaConstants.STANDARD_BUTTON_LOCATIONS[1] + offsetY,
                            (width / 2) * NovaConstants.WIDGETS_TEXTURE_FACTOR, height * NovaConstants.WIDGETS_TEXTURE_FACTOR)
            );
            this.mouseDragged(mc, NovaDraw.getMouseX(), NovaDraw.getMouseY());
            int color = 0x00E0E0E0;
            if (!this.enabled) {
                color = 0x00A0A0A0;
            } else if (this.hovered) {
                color = 0x00FFFFA0;
            }

            FontRenderer fontrenderer = mc.fontRenderer;
            this.drawCenteredString(fontrenderer, this.displayString, this.x + this.width / 2, this.y + (this.height - 8) / 2, color);
        }
    }
}
