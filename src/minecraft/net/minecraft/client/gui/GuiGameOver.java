package net.minecraft.client.gui;

import java.io.IOException;
import net.minecraft.client.multiplayer.WorldClient;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.resources.I18n;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextFormatting;

public class GuiGameOver extends GuiScreen implements GuiYesNoCallback
{
    /**
     * The integer value containing the number of ticks that have passed since the player's death
     */
    private int enableButtonsTimer;
    private final ITextComponent field_184871_f;

    public GuiGameOver(ITextComponent p_i46598_1_)
    {
        this.field_184871_f = p_i46598_1_;
    }

    /**
     * Adds the buttons (and other controls) to the screen in question. Called when the GUI is displayed and when the
     * window resizes, the buttonList is cleared beforehand.
     */
    public void initGui()
    {
        this.buttonList.clear();
        this.enableButtonsTimer = 0;

        if (this.mc.theWorld.getWorldInfo().isHardcoreModeEnabled())
        {
            this.buttonList.add(new GuiButton(0, this.width / 2 - 100, this.height / 4 + 72, I18n.format("deathScreen.spectate", new Object[0])));
            this.buttonList.add(new GuiButton(1, this.width / 2 - 100, this.height / 4 + 96, I18n.format("deathScreen." + (this.mc.isIntegratedServerRunning() ? "deleteWorld" : "leaveServer"), new Object[0])));
        }
        else
        {
            this.buttonList.add(new GuiButton(0, this.width / 2 - 100, this.height / 4 + 72, I18n.format("deathScreen.respawn", new Object[0])));
            this.buttonList.add(new GuiButton(1, this.width / 2 - 100, this.height / 4 + 96, I18n.format("deathScreen.titleScreen", new Object[0])));

            if (this.mc.getSession() == null)
            {
                ((GuiButton)this.buttonList.get(1)).enabled = false;
            }
        }

        for (GuiButton guibutton : this.buttonList)
        {
            guibutton.enabled = false;
        }
    }

    /**
     * Fired when a key is typed (except F11 which toggles full screen). This is the equivalent of
     * KeyListener.keyTyped(KeyEvent e). Args : character (character on the key), keyCode (lwjgl Keyboard key code)
     */
    protected void keyTyped(char typedChar, int keyCode) throws IOException
    {
    }

    /**
     * Called by the controls from the buttonList when activated. (Mouse pressed for buttons)
     */
    protected void actionPerformed(GuiButton button) throws IOException
    {
        switch (button.id)
        {
            case 0:
                this.mc.thePlayer.respawnPlayer();
                this.mc.displayGuiScreen((GuiScreen)null);
                break;

            case 1:
                if (this.mc.theWorld.getWorldInfo().isHardcoreModeEnabled())
                {
                    this.mc.displayGuiScreen(new GuiMainMenu());
                }
                else
                {
                    GuiYesNo guiyesno = new GuiYesNo(this, I18n.format("deathScreen.quit.confirm", new Object[0]), "", I18n.format("deathScreen.titleScreen", new Object[0]), I18n.format("deathScreen.respawn", new Object[0]), 0);
                    this.mc.displayGuiScreen(guiyesno);
                    guiyesno.setButtonDelay(20);
                }
        }
    }

    public void confirmClicked(boolean result, int id)
    {
        if (result)
        {
            if (this.mc.theWorld != null)
            {
                this.mc.theWorld.sendQuittingDisconnectingPacket();
            }

            this.mc.loadWorld((WorldClient)null);
            this.mc.displayGuiScreen(new GuiMainMenu());
        }
        else
        {
            this.mc.thePlayer.respawnPlayer();
            this.mc.displayGuiScreen((GuiScreen)null);
        }
    }

    /**
     * Draws the screen and all the components in it. Args : mouseX, mouseY, renderPartialTicks
     */
    public void drawScreen(int mouseX, int mouseY, float partialTicks)
    {
        boolean flag = this.mc.theWorld.getWorldInfo().isHardcoreModeEnabled();
        this.drawGradientRect(0, 0, this.width, this.height, 1615855616, -1602211792);
        GlStateManager.pushMatrix();
        GlStateManager.scale(2.0F, 2.0F, 2.0F);
        this.drawCenteredString(this.fontRendererObj, flag ? I18n.format("deathScreen.title.hardcore", new Object[0]) : I18n.format("deathScreen.title", new Object[0]), this.width / 2 / 2, 30, 16777215);
        GlStateManager.popMatrix();

        if (this.field_184871_f != null)
        {
            this.drawCenteredString(this.fontRendererObj, this.field_184871_f.getFormattedText(), this.width / 2, 85, 16777215);
        }

        this.drawCenteredString(this.fontRendererObj, I18n.format("deathScreen.score", new Object[0]) + ": " + TextFormatting.YELLOW + this.mc.thePlayer.getScore(), this.width / 2, 100, 16777215);

        if (this.field_184871_f != null && mouseY > 85 && mouseY < 85 + this.fontRendererObj.FONT_HEIGHT)
        {
            ITextComponent itextcomponent = this.func_184870_b(mouseX);

            if (itextcomponent != null && itextcomponent.getChatStyle().getChatHoverEvent() != null)
            {
                this.handleComponentHover(itextcomponent, mouseX, mouseY);
            }
        }

        super.drawScreen(mouseX, mouseY, partialTicks);
    }

    public ITextComponent func_184870_b(int p_184870_1_)
    {
        if (this.field_184871_f == null)
        {
            return null;
        }
        else
        {
            int i = this.mc.fontRendererObj.getStringWidth(this.field_184871_f.getFormattedText());
            int j = this.width / 2 - i / 2;
            int k = this.width / 2 + i / 2;
            int l = j;

            if (p_184870_1_ >= j && p_184870_1_ <= k)
            {
                for (ITextComponent itextcomponent : this.field_184871_f)
                {
                    l += this.mc.fontRendererObj.getStringWidth(GuiUtilRenderComponents.func_178909_a(itextcomponent.getUnformattedTextForChat(), false));

                    if (l > p_184870_1_)
                    {
                        return itextcomponent;
                    }
                }

                return null;
            }
            else
            {
                return null;
            }
        }
    }

    /**
     * Returns true if this GUI should pause the game when it is displayed in single-player
     */
    public boolean doesGuiPauseGame()
    {
        return false;
    }

    /**
     * Called from the main game loop to update the screen.
     */
    public void updateScreen()
    {
        super.updateScreen();
        ++this.enableButtonsTimer;

        if (this.enableButtonsTimer == 20)
        {
            for (GuiButton guibutton : this.buttonList)
            {
                guibutton.enabled = true;
            }
        }
    }
}
