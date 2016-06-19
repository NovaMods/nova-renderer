package net.minecraft.client.gui;

import java.io.IOException;
import net.minecraft.client.renderer.OpenGlHelper;
import net.minecraft.client.resources.I18n;
import net.minecraft.world.storage.ISaveFormat;
import net.minecraft.world.storage.WorldInfo;
import org.apache.commons.io.FileUtils;
import org.lwjgl.input.Keyboard;

public class GuiWorldEdit extends GuiScreen
{
    private final GuiScreen field_184858_a;
    private GuiTextField field_184859_f;
    private final String field_184860_g;

    public GuiWorldEdit(GuiScreen p_i46593_1_, String p_i46593_2_)
    {
        this.field_184858_a = p_i46593_1_;
        this.field_184860_g = p_i46593_2_;
    }

    /**
     * Called from the main game loop to update the screen.
     */
    public void updateScreen()
    {
        this.field_184859_f.updateCursorCounter();
    }

    /**
     * Adds the buttons (and other controls) to the screen in question. Called when the GUI is displayed and when the
     * window resizes, the buttonList is cleared beforehand.
     */
    public void initGui()
    {
        Keyboard.enableRepeatEvents(true);
        this.buttonList.clear();
        GuiButton guibutton;
        this.buttonList.add(guibutton = new GuiButton(3, this.width / 2 - 100, this.height / 4 + 24 + 12, I18n.format("selectWorld.edit.resetIcon", new Object[0])));
        this.buttonList.add(new GuiButton(4, this.width / 2 - 100, this.height / 4 + 48 + 12, I18n.format("selectWorld.edit.openFolder", new Object[0])));
        this.buttonList.add(new GuiButton(0, this.width / 2 - 100, this.height / 4 + 96 + 12, I18n.format("selectWorld.edit.save", new Object[0])));
        this.buttonList.add(new GuiButton(1, this.width / 2 - 100, this.height / 4 + 120 + 12, I18n.format("gui.cancel", new Object[0])));
        guibutton.enabled = this.mc.getSaveLoader().func_186352_b(this.field_184860_g, "icon.png").isFile();
        ISaveFormat isaveformat = this.mc.getSaveLoader();
        WorldInfo worldinfo = isaveformat.getWorldInfo(this.field_184860_g);
        String s = worldinfo.getWorldName();
        this.field_184859_f = new GuiTextField(2, this.fontRendererObj, this.width / 2 - 100, 60, 200, 20);
        this.field_184859_f.setFocused(true);
        this.field_184859_f.setText(s);
    }

    /**
     * Called when the screen is unloaded. Used to disable keyboard repeat events
     */
    public void onGuiClosed()
    {
        Keyboard.enableRepeatEvents(false);
    }

    /**
     * Called by the controls from the buttonList when activated. (Mouse pressed for buttons)
     */
    protected void actionPerformed(GuiButton button) throws IOException
    {
        if (button.enabled)
        {
            if (button.id == 1)
            {
                this.mc.displayGuiScreen(this.field_184858_a);
            }
            else if (button.id == 0)
            {
                ISaveFormat isaveformat = this.mc.getSaveLoader();
                isaveformat.renameWorld(this.field_184860_g, this.field_184859_f.getText().trim());
                this.mc.displayGuiScreen(this.field_184858_a);
            }
            else if (button.id == 3)
            {
                ISaveFormat isaveformat1 = this.mc.getSaveLoader();
                FileUtils.deleteQuietly(isaveformat1.func_186352_b(this.field_184860_g, "icon.png"));
                button.enabled = false;
            }
            else if (button.id == 4)
            {
                ISaveFormat isaveformat2 = this.mc.getSaveLoader();
                OpenGlHelper.func_188786_a(isaveformat2.func_186352_b(this.field_184860_g, "icon.png").getParentFile());
            }
        }
    }

    /**
     * Fired when a key is typed (except F11 which toggles full screen). This is the equivalent of
     * KeyListener.keyTyped(KeyEvent e). Args : character (character on the key), keyCode (lwjgl Keyboard key code)
     */
    protected void keyTyped(char typedChar, int keyCode) throws IOException
    {
        this.field_184859_f.textboxKeyTyped(typedChar, keyCode);
        ((GuiButton)this.buttonList.get(2)).enabled = !this.field_184859_f.getText().trim().isEmpty();

        if (keyCode == 28 || keyCode == 156)
        {
            this.actionPerformed((GuiButton)this.buttonList.get(2));
        }
    }

    /**
     * Called when the mouse is clicked. Args : mouseX, mouseY, clickedButton
     */
    protected void mouseClicked(int mouseX, int mouseY, int mouseButton) throws IOException
    {
        super.mouseClicked(mouseX, mouseY, mouseButton);
        this.field_184859_f.mouseClicked(mouseX, mouseY, mouseButton);
    }

    /**
     * Draws the screen and all the components in it. Args : mouseX, mouseY, renderPartialTicks
     */
    public void drawScreen(int mouseX, int mouseY, float partialTicks)
    {
        this.drawDefaultBackground();
        this.drawCenteredString(this.fontRendererObj, I18n.format("selectWorld.edit.title", new Object[0]), this.width / 2, 20, 16777215);
        this.drawString(this.fontRendererObj, I18n.format("selectWorld.enterName", new Object[0]), this.width / 2 - 100, 47, 10526880);
        this.field_184859_f.drawTextBox();
        super.drawScreen(mouseX, mouseY, partialTicks);
    }
}
