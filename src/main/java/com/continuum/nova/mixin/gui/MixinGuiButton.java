package com.continuum.nova.mixin.gui;

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
    @Shadow public int xPosition;
    @Shadow public int yPosition;
    @Shadow public int width;
    @Shadow public int height;

    @Shadow protected abstract int getHoverState(boolean mouseOver);

    @Shadow @Final protected static ResourceLocation BUTTON_TEXTURES;

    @Shadow protected abstract void mouseDragged(Minecraft mc, int mouseX, int mouseY);

    @Shadow public boolean enabled;
    @Shadow public String displayString;
    // TODO: What is this?!
    private float[] stdButtonLocation = {
            0.0f, 0.1796862f,
            0.78125f, 0.3359375f
    };
    // TODO: What is this?!
    private float widgetsTextureFactor = 0.00390625F;

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawButton(Minecraft mc, int mouseX, int mouseY) {
        if (this.visible) {
            float offsetY;
            this.hovered = mouseX >= this.xPosition && mouseY >= this.yPosition && mouseX < this.xPosition + this.width && mouseY < this.yPosition + this.height;
            int drawState = getHoverState(hovered);
            offsetY = (float) drawState * 0.078125f;
            NovaDraw.incrementZ();

            NovaDraw.drawRectangle(
                    BUTTON_TEXTURES,
                    new Rectangle2D.Float(xPosition, yPosition, width / 2, height),
                    new Rectangle2D.Float(
                            stdButtonLocation[0], stdButtonLocation[1] + offsetY,
                            width / 2 * widgetsTextureFactor, height * widgetsTextureFactor)
            );
            NovaDraw.drawRectangle(
                    BUTTON_TEXTURES,
                    new Rectangle2D.Float(xPosition + width / 2, yPosition, width / 2, height),
                    new Rectangle2D.Float(
                            stdButtonLocation[2] - ((width / 2) * widgetsTextureFactor), stdButtonLocation[1] + offsetY,
                            (width / 2) * widgetsTextureFactor, height * widgetsTextureFactor)
            );
            this.mouseDragged(mc, NovaDraw.getMouseX(), NovaDraw.getMouseY());
            int color = 0x00E0E0E0;
            if (!this.enabled) {
                color = 0x00A0A0A0;
            } else if (this.hovered) {
                color = 0x00FFFFA0;
            }

            FontRenderer fontrenderer = mc.fontRendererObj;
            this.drawCenteredString(fontrenderer, this.displayString, this.xPosition + this.width / 2, this.yPosition + (this.height - 8) / 2, color);
        }
    }
}
