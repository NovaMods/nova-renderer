package com.continuum.nova.mixin.gui;

import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.Gui;
import net.minecraft.client.gui.GuiListWorldSelectionEntry;
import net.minecraft.client.gui.GuiWorldSelection;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.texture.DynamicTexture;
import net.minecraft.client.resources.I18n;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.world.storage.WorldSummary;
import org.apache.commons.lang3.StringUtils;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;

import java.awt.geom.Rectangle2D;
import java.text.DateFormat;
import java.util.Date;

@Mixin(GuiListWorldSelectionEntry.class)
public class MixinGuiListWorldSelectionEntry {
    @Shadow @Final private WorldSummary worldSummary;

    @Shadow @Final private static DateFormat DATE_FORMAT;

    @Shadow @Final private Minecraft client;

    @Shadow private DynamicTexture icon;

    @Shadow @Final private ResourceLocation iconLocation;

    @Shadow @Final private static ResourceLocation ICON_MISSING;

    @Shadow @Final private static ResourceLocation ICON_OVERLAY_LOCATION;

    @Shadow @Final private GuiWorldSelection worldSelScreen;

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawEntry(int slotIndex, int x, int y, int listWidth, int slotHeight, int mouseX, int mouseY, boolean isSelected) {
        String s = this.worldSummary.getDisplayName();
        String s1 = this.worldSummary.getFileName() + " (" + DATE_FORMAT.format(new Date(this.worldSummary.getLastTimePlayed())) + ")";
        String s2 = "";

        if (StringUtils.isEmpty(s)) {
            s = I18n.format("selectWorld.world") + " " + (slotIndex + 1);
        }

        if (this.worldSummary.requiresConversion()) {
            s2 = I18n.format("selectWorld.conversion") + " " + s2;
        } else {
            s2 = I18n.format("gameMode." + this.worldSummary.getEnumGameType().getName());

            if (this.worldSummary.isHardcoreModeEnabled()) {
                s2 = TextFormatting.DARK_RED + I18n.format("gameMode.hardcore") + TextFormatting.RESET;
            }

            if (this.worldSummary.getCheatsEnabled()) {
                s2 = s2 + ", " + I18n.format("selectWorld.cheats");
            }

            String s3 = this.worldSummary.getVersionName();

            if (this.worldSummary.markVersionInList()) {
                if (this.worldSummary.askToOpenWorld()) {
                    s2 = s2 + ", " + I18n.format("selectWorld.version") + " " + TextFormatting.RED + s3 + TextFormatting.RESET;
                } else {
                    s2 = s2 + ", " + I18n.format("selectWorld.version") + " " + TextFormatting.ITALIC + s3 + TextFormatting.RESET;
                }
            }
            else {
                s2 = s2 + ", " + I18n.format("selectWorld.version") + " " + s3;
            }
        }

        this.client.fontRendererObj.drawString(s, x + 32 + 3, y + 1, 16777215);
        this.client.fontRendererObj.drawString(s1, x + 32 + 3, y + this.client.fontRendererObj.FONT_HEIGHT + 3, 8421504);
        this.client.fontRendererObj.drawString(s2, x + 32 + 3, y + this.client.fontRendererObj.FONT_HEIGHT + this.client.fontRendererObj.FONT_HEIGHT + 3, 8421504);
        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
        this.client.getTextureManager().bindTexture(this.icon != null ? this.iconLocation : ICON_MISSING);

        ResourceLocation rectIcon = this.icon != null ? iconLocation : ICON_MISSING;
        NovaDraw.drawRectangle(rectIcon, new Rectangle2D.Float(x, y, 32, 32), new Rectangle2D.Float(0, 0, 1, 1));

        if(this.client.gameSettings.touchscreen || isSelected) {
            this.client.getTextureManager().bindTexture(ICON_OVERLAY_LOCATION);
            Gui.drawRect(x, y, x + 32, y + 32, -1601138544);
            GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
            int j = mouseX - x;
            int i = j < 32 ? 32 : 0;

            if (this.worldSummary.markVersionInList()) {
                Gui.drawModalRectWithCustomSizedTexture(x, y, 32.0F, (float)i, 32, 32, 256.0F, 256.0F);

                if (this.worldSummary.askToOpenWorld()) {
                    Gui.drawModalRectWithCustomSizedTexture(x, y, 96.0F, (float)i, 32, 32, 256.0F, 256.0F);

                    if (j < 32) {
                        this.worldSelScreen.setVersionTooltip(TextFormatting.RED + I18n.format("selectWorld.tooltip.fromNewerVersion1") + "\n" + TextFormatting.RED + I18n.format("selectWorld.tooltip.fromNewerVersion2", new Object[0]));
                    }
                } else {
                    Gui.drawModalRectWithCustomSizedTexture(x, y, 64.0F, (float)i, 32, 32, 256.0F, 256.0F);

                    if (j < 32) {
                        this.worldSelScreen.setVersionTooltip(TextFormatting.GOLD + I18n.format("selectWorld.tooltip.snapshot1") + "\n" + TextFormatting.GOLD + I18n.format("selectWorld.tooltip.snapshot2", new Object[0]));
                    }
                }
            } else {
                Gui.drawModalRectWithCustomSizedTexture(x, y, 0.0F, (float)i, 32, 32, 256.0F, 256.0F);
            }
        }
    }

    // TODO: Overwrite more methods
}
