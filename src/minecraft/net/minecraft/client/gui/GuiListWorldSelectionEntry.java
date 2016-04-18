package net.minecraft.client.gui;

import java.awt.image.BufferedImage;
import java.io.File;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import javax.imageio.ImageIO;
import net.minecraft.client.Minecraft;
import net.minecraft.client.audio.PositionedSoundRecord;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.texture.DynamicTexture;
import net.minecraft.client.resources.I18n;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.world.WorldSettings;
import net.minecraft.world.storage.ISaveFormat;
import net.minecraft.world.storage.ISaveHandler;
import net.minecraft.world.storage.SaveFormatComparator;
import net.minecraft.world.storage.WorldInfo;
import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GuiListWorldSelectionEntry implements GuiListExtended.IGuiListEntry
{
    private static final Logger field_186780_a = LogManager.getLogger();
    private static final DateFormat field_186781_b = new SimpleDateFormat();
    private static final ResourceLocation field_186782_c = new ResourceLocation("textures/misc/unknown_server.png");
    private static final ResourceLocation field_186783_d = new ResourceLocation("textures/gui/world_selection.png");
    private final Minecraft field_186784_e;
    private final GuiWorldSelection field_186785_f;
    private final SaveFormatComparator field_186786_g;
    private final ResourceLocation field_186787_h;
    private final GuiListWorldSelection field_186788_i;
    private File field_186789_j;
    private DynamicTexture field_186790_k;
    private long field_186791_l;

    public GuiListWorldSelectionEntry(GuiListWorldSelection p_i46591_1_, SaveFormatComparator p_i46591_2_, ISaveFormat p_i46591_3_)
    {
        this.field_186788_i = p_i46591_1_;
        this.field_186785_f = p_i46591_1_.func_186796_g();
        this.field_186786_g = p_i46591_2_;
        this.field_186784_e = Minecraft.getMinecraft();
        this.field_186787_h = new ResourceLocation("worlds/" + p_i46591_2_.getFileName() + "/icon");
        this.field_186789_j = p_i46591_3_.func_186352_b(p_i46591_2_.getFileName(), "icon.png");

        if (!this.field_186789_j.isFile())
        {
            this.field_186789_j = null;
        }

        this.func_186769_f();
    }

    public void drawEntry(int slotIndex, int x, int y, int listWidth, int slotHeight, int mouseX, int mouseY, boolean isSelected)
    {
        String s = this.field_186786_g.getDisplayName();
        String s1 = this.field_186786_g.getFileName() + " (" + field_186781_b.format(new Date(this.field_186786_g.getLastTimePlayed())) + ")";
        String s2 = "";

        if (StringUtils.isEmpty(s))
        {
            s = I18n.format("selectWorld.world", new Object[0]) + " " + (slotIndex + 1);
        }

        if (this.field_186786_g.requiresConversion())
        {
            s2 = I18n.format("selectWorld.conversion", new Object[0]) + " " + s2;
        }
        else
        {
            s2 = I18n.format("gameMode." + this.field_186786_g.getEnumGameType().getName(), new Object[0]);

            if (this.field_186786_g.isHardcoreModeEnabled())
            {
                s2 = TextFormatting.DARK_RED + I18n.format("gameMode.hardcore", new Object[0]) + TextFormatting.RESET;
            }

            if (this.field_186786_g.getCheatsEnabled())
            {
                s2 = s2 + ", " + I18n.format("selectWorld.cheats", new Object[0]);
            }

            String s3 = this.field_186786_g.func_186357_i();

            if (this.field_186786_g.func_186355_l())
            {
                if (this.field_186786_g.func_186356_m())
                {
                    s2 = s2 + ", " + I18n.format("selectWorld.version", new Object[0]) + " " + TextFormatting.RED + s3 + TextFormatting.RESET;
                }
                else
                {
                    s2 = s2 + ", " + I18n.format("selectWorld.version", new Object[0]) + " " + TextFormatting.ITALIC + s3 + TextFormatting.RESET;
                }
            }
            else
            {
                s2 = s2 + ", " + I18n.format("selectWorld.version", new Object[0]) + " " + s3;
            }
        }

        this.field_186784_e.fontRendererObj.drawString(s, x + 32 + 3, y + 1, 16777215);
        this.field_186784_e.fontRendererObj.drawString(s1, x + 32 + 3, y + this.field_186784_e.fontRendererObj.FONT_HEIGHT + 3, 8421504);
        this.field_186784_e.fontRendererObj.drawString(s2, x + 32 + 3, y + this.field_186784_e.fontRendererObj.FONT_HEIGHT + this.field_186784_e.fontRendererObj.FONT_HEIGHT + 3, 8421504);
        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
        this.field_186784_e.getTextureManager().bindTexture(this.field_186790_k != null ? this.field_186787_h : field_186782_c);
        GlStateManager.enableBlend();
        Gui.drawModalRectWithCustomSizedTexture(x, y, 0.0F, 0.0F, 32, 32, 32.0F, 32.0F);
        GlStateManager.disableBlend();

        if (this.field_186784_e.gameSettings.touchscreen || isSelected)
        {
            this.field_186784_e.getTextureManager().bindTexture(field_186783_d);
            Gui.drawRect(x, y, x + 32, y + 32, -1601138544);
            GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
            int j = mouseX - x;
            int i = j < 32 ? 32 : 0;

            if (this.field_186786_g.func_186355_l())
            {
                Gui.drawModalRectWithCustomSizedTexture(x, y, 32.0F, (float)i, 32, 32, 256.0F, 256.0F);

                if (this.field_186786_g.func_186356_m())
                {
                    Gui.drawModalRectWithCustomSizedTexture(x, y, 96.0F, (float)i, 32, 32, 256.0F, 256.0F);

                    if (j < 32)
                    {
                        this.field_186785_f.func_184861_a(TextFormatting.RED + I18n.format("selectWorld.tooltip.fromNewerVersion1", new Object[0]) + "\n" + TextFormatting.RED + I18n.format("selectWorld.tooltip.fromNewerVersion2", new Object[0]));
                    }
                }
                else
                {
                    Gui.drawModalRectWithCustomSizedTexture(x, y, 64.0F, (float)i, 32, 32, 256.0F, 256.0F);

                    if (j < 32)
                    {
                        this.field_186785_f.func_184861_a(TextFormatting.GOLD + I18n.format("selectWorld.tooltip.snapshot1", new Object[0]) + "\n" + TextFormatting.GOLD + I18n.format("selectWorld.tooltip.snapshot2", new Object[0]));
                    }
                }
            }
            else
            {
                Gui.drawModalRectWithCustomSizedTexture(x, y, 0.0F, (float)i, 32, 32, 256.0F, 256.0F);
            }
        }
    }

    /**
     * Returns true if the mouse has been pressed on this control.
     */
    public boolean mousePressed(int slotIndex, int p_148278_2_, int p_148278_3_, int p_148278_4_, int p_148278_5_, int p_148278_6_)
    {
        this.field_186788_i.func_186792_d(slotIndex);

        if (p_148278_5_ <= 32 && p_148278_5_ < 32)
        {
            this.func_186774_a();
            return true;
        }
        else if (Minecraft.getSystemTime() - this.field_186791_l < 250L)
        {
            this.func_186774_a();
            return true;
        }
        else
        {
            this.field_186791_l = Minecraft.getSystemTime();
            return false;
        }
    }

    public void func_186774_a()
    {
        if (this.field_186786_g.func_186356_m())
        {
            this.field_186784_e.displayGuiScreen(new GuiYesNo(new GuiYesNoCallback()
            {
                public void confirmClicked(boolean result, int id)
                {
                    if (result)
                    {
                        GuiListWorldSelectionEntry.this.func_186777_e();
                    }
                    else
                    {
                        GuiListWorldSelectionEntry.this.field_186784_e.displayGuiScreen(GuiListWorldSelectionEntry.this.field_186785_f);
                    }
                }
            }, I18n.format("selectWorld.versionQuestion", new Object[0]), I18n.format("selectWorld.versionWarning", new Object[] {this.field_186786_g.func_186357_i()}), I18n.format("selectWorld.versionJoinButton", new Object[0]), I18n.format("gui.cancel", new Object[0]), 0));
        }
        else
        {
            this.func_186777_e();
        }
    }

    public void func_186776_b()
    {
        this.field_186784_e.displayGuiScreen(new GuiYesNo(new GuiYesNoCallback()
        {
            public void confirmClicked(boolean result, int id)
            {
                if (result)
                {
                    GuiListWorldSelectionEntry.this.field_186784_e.displayGuiScreen(new GuiScreenWorking());
                    ISaveFormat isaveformat = GuiListWorldSelectionEntry.this.field_186784_e.getSaveLoader();
                    isaveformat.flushCache();
                    isaveformat.deleteWorldDirectory(GuiListWorldSelectionEntry.this.field_186786_g.getFileName());
                    GuiListWorldSelectionEntry.this.field_186788_i.func_186795_e();
                }

                GuiListWorldSelectionEntry.this.field_186784_e.displayGuiScreen(GuiListWorldSelectionEntry.this.field_186785_f);
            }
        }, I18n.format("selectWorld.deleteQuestion", new Object[0]), "\'" + this.field_186786_g.getDisplayName() + "\' " + I18n.format("selectWorld.deleteWarning", new Object[0]), I18n.format("selectWorld.deleteButton", new Object[0]), I18n.format("gui.cancel", new Object[0]), 0));
    }

    public void func_186778_c()
    {
        this.field_186784_e.displayGuiScreen(new GuiWorldEdit(this.field_186785_f, this.field_186786_g.getFileName()));
    }

    public void func_186779_d()
    {
        this.field_186784_e.displayGuiScreen(new GuiScreenWorking());
        GuiCreateWorld guicreateworld = new GuiCreateWorld(this.field_186785_f);
        ISaveHandler isavehandler = this.field_186784_e.getSaveLoader().getSaveLoader(this.field_186786_g.getFileName(), false);
        WorldInfo worldinfo = isavehandler.loadWorldInfo();
        isavehandler.flush();
        guicreateworld.recreateFromExistingWorld(worldinfo);
        this.field_186784_e.displayGuiScreen(guicreateworld);
    }

    private void func_186777_e()
    {
        this.field_186784_e.getSoundHandler().playSound(PositionedSoundRecord.func_184371_a(SoundEvents.ui_button_click, 1.0F));

        if (this.field_186784_e.getSaveLoader().canLoadWorld(this.field_186786_g.getFileName()))
        {
            this.field_186784_e.launchIntegratedServer(this.field_186786_g.getFileName(), this.field_186786_g.getDisplayName(), (WorldSettings)null);
        }
    }

    private void func_186769_f()
    {
        boolean flag = this.field_186789_j != null && this.field_186789_j.isFile();

        if (flag)
        {
            BufferedImage bufferedimage;

            try
            {
                bufferedimage = ImageIO.read(this.field_186789_j);
                Validate.validState(bufferedimage.getWidth() == 64, "Must be 64 pixels wide", new Object[0]);
                Validate.validState(bufferedimage.getHeight() == 64, "Must be 64 pixels high", new Object[0]);
            }
            catch (Throwable throwable)
            {
                field_186780_a.error("Invalid icon for world " + this.field_186786_g.getFileName(), throwable);
                this.field_186789_j = null;
                return;
            }

            if (this.field_186790_k == null)
            {
                this.field_186790_k = new DynamicTexture(bufferedimage.getWidth(), bufferedimage.getHeight());
                this.field_186784_e.getTextureManager().loadTexture(this.field_186787_h, this.field_186790_k);
            }

            bufferedimage.getRGB(0, 0, bufferedimage.getWidth(), bufferedimage.getHeight(), this.field_186790_k.getTextureData(), 0, bufferedimage.getWidth());
            this.field_186790_k.updateDynamicTexture();
        }
        else if (!flag)
        {
            this.field_186784_e.getTextureManager().deleteTexture(this.field_186787_h);
            this.field_186790_k = null;
        }
    }

    /**
     * Fired when the mouse button is released. Arguments: index, x, y, mouseEvent, relativeX, relativeY
     */
    public void mouseReleased(int slotIndex, int x, int y, int mouseEvent, int relativeX, int relativeY)
    {
    }

    public void setSelected(int p_178011_1_, int p_178011_2_, int p_178011_3_)
    {
    }
}
