package com.continuum.nova.mixin.gui;

import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.Gui;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.client.gui.GuiLabel;
import net.minecraft.client.gui.GuiScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.util.List;

@Mixin(GuiScreen.class)
public class MixinGuiScreen extends Gui {
    @Shadow public Minecraft mc;

    @Shadow protected List<GuiButton> buttonList;

    @Shadow protected List<GuiLabel> labelList;

    @Shadow public int width;

    @Shadow public int height;

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawScreen(int mouseX, int mouseY, float partialTicks) {
        NovaDraw.incrementZ();

        this.buttonList.forEach(guiButton -> guiButton.drawButton(this.mc, mouseX, mouseY, partialTicks));

        this.labelList.forEach(guiLabel -> guiLabel.drawLabel(this.mc, mouseX, mouseY));
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawBackground(int tint) {
        NovaDraw.drawRectangle(
                OPTIONS_BACKGROUND,
                new Rectangle2D.Float(0, 0, width, height),
                new Rectangle2D.Float(0, 0,width / 32, height /32),
                new Color(64, 64, 64, 255)
        );
    }
}
