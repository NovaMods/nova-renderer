package net.minecraft.client.gui;

import java.io.IOException;
import net.minecraft.client.Minecraft;
import net.minecraft.client.audio.PositionedSoundRecord;
import net.minecraft.client.audio.SoundHandler;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.resources.I18n;
import net.minecraft.client.settings.GameSettings;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.MathHelper;

public class GuiScreenOptionsSounds extends GuiScreen
{
    private final GuiScreen field_146505_f;

    /** Reference to the GameSettings object. */
    private final GameSettings game_settings_4;
    protected String field_146507_a = "Options";
    private String field_146508_h;

    public GuiScreenOptionsSounds(GuiScreen p_i45025_1_, GameSettings p_i45025_2_)
    {
        this.field_146505_f = p_i45025_1_;
        this.game_settings_4 = p_i45025_2_;
    }

    /**
     * Adds the buttons (and other controls) to the screen in question. Called when the GUI is displayed and when the
     * window resizes, the buttonList is cleared beforehand.
     */
    public void initGui()
    {
        int i = 0;
        this.field_146507_a = I18n.format("options.sounds.title", new Object[0]);
        this.field_146508_h = I18n.format("options.off", new Object[0]);
        this.buttonList.add(new GuiScreenOptionsSounds.Button(SoundCategory.MASTER.ordinal(), this.width / 2 - 155 + i % 2 * 160, this.height / 6 - 12 + 24 * (i >> 1), SoundCategory.MASTER, true));
        i = i + 2;

        for (SoundCategory soundcategory : SoundCategory.values())
        {
            if (soundcategory != SoundCategory.MASTER)
            {
                this.buttonList.add(new GuiScreenOptionsSounds.Button(soundcategory.ordinal(), this.width / 2 - 155 + i % 2 * 160, this.height / 6 - 12 + 24 * (i >> 1), soundcategory, false));
                ++i;
            }
        }

        int j = this.width / 2 - 75;
        int k = this.height / 6 - 12;
        ++i;
        this.buttonList.add(new GuiOptionButton(201, j, k + 24 * (i >> 1), GameSettings.Options.SHOW_SUBTITLES, this.game_settings_4.getKeyBinding(GameSettings.Options.SHOW_SUBTITLES)));
        this.buttonList.add(new GuiButton(200, this.width / 2 - 100, this.height / 6 + 168, I18n.format("gui.done", new Object[0])));
    }

    /**
     * Called by the controls from the buttonList when activated. (Mouse pressed for buttons)
     */
    protected void actionPerformed(GuiButton button) throws IOException
    {
        if (button.enabled)
        {
            if (button.id == 200)
            {
                this.mc.gameSettings.saveOptions();
                this.mc.displayGuiScreen(this.field_146505_f);
            }
            else if (button.id == 201)
            {
                this.mc.gameSettings.setOptionValue(GameSettings.Options.SHOW_SUBTITLES, 1);
                button.displayString = this.mc.gameSettings.getKeyBinding(GameSettings.Options.SHOW_SUBTITLES);
                this.mc.gameSettings.saveOptions();
            }
        }
    }

    /**
     * Draws the screen and all the components in it. Args : mouseX, mouseY, renderPartialTicks
     */
    public void drawScreen(int mouseX, int mouseY, float partialTicks)
    {
        this.drawDefaultBackground();
        this.drawCenteredString(this.fontRendererObj, this.field_146507_a, this.width / 2, 15, 16777215);
        super.drawScreen(mouseX, mouseY, partialTicks);
    }

    protected String func_184097_a(SoundCategory p_184097_1_)
    {
        float f = this.game_settings_4.func_186711_a(p_184097_1_);
        return f == 0.0F ? this.field_146508_h : (int)(f * 100.0F) + "%";
    }

    class Button extends GuiButton
    {
        private final SoundCategory field_184063_r;
        private final String field_146152_s;
        public float field_146156_o = 1.0F;
        public boolean field_146155_p;

        public Button(int p_i46744_2_, int p_i46744_3_, int p_i46744_4_, SoundCategory p_i46744_5_, boolean p_i46744_6_)
        {
            super(p_i46744_2_, p_i46744_3_, p_i46744_4_, p_i46744_6_ ? 310 : 150, 20, "");
            this.field_184063_r = p_i46744_5_;
            this.field_146152_s = I18n.format("soundCategory." + p_i46744_5_.getName(), new Object[0]);
            this.displayString = this.field_146152_s + ": " + GuiScreenOptionsSounds.this.func_184097_a(p_i46744_5_);
            this.field_146156_o = GuiScreenOptionsSounds.this.game_settings_4.func_186711_a(p_i46744_5_);
        }

        protected int getHoverState(boolean mouseOver)
        {
            return 0;
        }

        protected void mouseDragged(Minecraft mc, int mouseX, int mouseY)
        {
            if (this.visible)
            {
                if (this.field_146155_p)
                {
                    this.field_146156_o = (float)(mouseX - (this.xPosition + 4)) / (float)(this.width - 8);
                    this.field_146156_o = MathHelper.clamp_float(this.field_146156_o, 0.0F, 1.0F);
                    mc.gameSettings.func_186712_a(this.field_184063_r, this.field_146156_o);
                    mc.gameSettings.saveOptions();
                    this.displayString = this.field_146152_s + ": " + GuiScreenOptionsSounds.this.func_184097_a(this.field_184063_r);
                }

                GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
                this.drawTexturedModalRect(this.xPosition + (int)(this.field_146156_o * (float)(this.width - 8)), this.yPosition, 0, 66, 4, 20);
                this.drawTexturedModalRect(this.xPosition + (int)(this.field_146156_o * (float)(this.width - 8)) + 4, this.yPosition, 196, 66, 4, 20);
            }
        }

        public boolean mousePressed(Minecraft mc, int mouseX, int mouseY)
        {
            if (super.mousePressed(mc, mouseX, mouseY))
            {
                this.field_146156_o = (float)(mouseX - (this.xPosition + 4)) / (float)(this.width - 8);
                this.field_146156_o = MathHelper.clamp_float(this.field_146156_o, 0.0F, 1.0F);
                mc.gameSettings.func_186712_a(this.field_184063_r, this.field_146156_o);
                mc.gameSettings.saveOptions();
                this.displayString = this.field_146152_s + ": " + GuiScreenOptionsSounds.this.func_184097_a(this.field_184063_r);
                this.field_146155_p = true;
                return true;
            }
            else
            {
                return false;
            }
        }

        public void playPressSound(SoundHandler soundHandlerIn)
        {
        }

        public void mouseReleased(int mouseX, int mouseY)
        {
            if (this.field_146155_p)
            {
                GuiScreenOptionsSounds.this.mc.getSoundHandler().playSound(PositionedSoundRecord.func_184371_a(SoundEvents.ui_button_click, 1.0F));
            }

            this.field_146155_p = false;
        }
    }
}
