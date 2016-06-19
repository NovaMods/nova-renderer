package net.minecraft.client.gui;

import com.google.common.base.Splitter;
import com.google.common.collect.Lists;
import java.io.IOException;
import net.minecraft.client.resources.I18n;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GuiWorldSelection extends GuiScreen implements GuiYesNoCallback
{
    private static final Logger field_184868_g = LogManager.getLogger();
    protected GuiScreen field_184864_a;
    protected String field_184867_f = "Select world";
    private String field_184869_h;
    private GuiButton deleteButton;
    private GuiButton selectButton;
    private GuiButton renameButton;
    private GuiButton field_184865_t;
    private GuiListWorldSelection field_184866_u;

    public GuiWorldSelection(GuiScreen p_i46592_1_)
    {
        this.field_184864_a = p_i46592_1_;
    }

    /**
     * Adds the buttons (and other controls) to the screen in question. Called when the GUI is displayed and when the
     * window resizes, the buttonList is cleared beforehand.
     */
    public void initGui()
    {
        this.field_184867_f = I18n.format("selectWorld.title", new Object[0]);
        this.field_184866_u = new GuiListWorldSelection(this, this.mc, this.width, this.height, 32, this.height - 64, 36);
        this.func_184862_a();
    }

    /**
     * Handles mouse input.
     */
    public void handleMouseInput() throws IOException
    {
        super.handleMouseInput();
        this.field_184866_u.handleMouseInput();
    }

    public void func_184862_a()
    {
        this.buttonList.add(this.selectButton = new GuiButton(1, this.width / 2 - 154, this.height - 52, 150, 20, I18n.format("selectWorld.select", new Object[0])));
        this.buttonList.add(new GuiButton(3, this.width / 2 + 4, this.height - 52, 150, 20, I18n.format("selectWorld.create", new Object[0])));
        this.buttonList.add(this.renameButton = new GuiButton(4, this.width / 2 - 154, this.height - 28, 72, 20, I18n.format("selectWorld.edit", new Object[0])));
        this.buttonList.add(this.deleteButton = new GuiButton(2, this.width / 2 - 76, this.height - 28, 72, 20, I18n.format("selectWorld.delete", new Object[0])));
        this.buttonList.add(this.field_184865_t = new GuiButton(5, this.width / 2 + 4, this.height - 28, 72, 20, I18n.format("selectWorld.recreate", new Object[0])));
        this.buttonList.add(new GuiButton(0, this.width / 2 + 82, this.height - 28, 72, 20, I18n.format("gui.cancel", new Object[0])));
        this.selectButton.enabled = false;
        this.deleteButton.enabled = false;
        this.renameButton.enabled = false;
        this.field_184865_t.enabled = false;
    }

    /**
     * Called by the controls from the buttonList when activated. (Mouse pressed for buttons)
     */
    protected void actionPerformed(GuiButton button) throws IOException
    {
        if (button.enabled)
        {
            GuiListWorldSelectionEntry guilistworldselectionentry = this.field_184866_u.func_186794_f();

            if (button.id == 2)
            {
                if (guilistworldselectionentry != null)
                {
                    guilistworldselectionentry.func_186776_b();
                }
            }
            else if (button.id == 1)
            {
                if (guilistworldselectionentry != null)
                {
                    guilistworldselectionentry.func_186774_a();
                }
            }
            else if (button.id == 3)
            {
                this.mc.displayGuiScreen(new GuiCreateWorld(this));
            }
            else if (button.id == 4)
            {
                if (guilistworldselectionentry != null)
                {
                    guilistworldselectionentry.func_186778_c();
                }
            }
            else if (button.id == 0)
            {
                this.mc.displayGuiScreen(this.field_184864_a);
            }
            else if (button.id == 5 && guilistworldselectionentry != null)
            {
                guilistworldselectionentry.func_186779_d();
            }
        }
    }

    /**
     * Draws the screen and all the components in it. Args : mouseX, mouseY, renderPartialTicks
     */
    public void drawScreen(int mouseX, int mouseY, float partialTicks)
    {
        this.field_184869_h = null;
        this.field_184866_u.drawScreen(mouseX, mouseY, partialTicks);
        this.drawCenteredString(this.fontRendererObj, this.field_184867_f, this.width / 2, 20, 16777215);
        super.drawScreen(mouseX, mouseY, partialTicks);

        if (this.field_184869_h != null)
        {
            this.drawHoveringText(Lists.newArrayList(Splitter.on("\n").split(this.field_184869_h)), mouseX, mouseY);
        }
    }

    /**
     * Called when the mouse is clicked. Args : mouseX, mouseY, clickedButton
     */
    protected void mouseClicked(int mouseX, int mouseY, int mouseButton) throws IOException
    {
        super.mouseClicked(mouseX, mouseY, mouseButton);
        this.field_184866_u.mouseClicked(mouseX, mouseY, mouseButton);
    }

    /**
     * Called when a mouse button is released.  Args : mouseX, mouseY, releaseButton
     */
    protected void mouseReleased(int mouseX, int mouseY, int state)
    {
        super.mouseReleased(mouseX, mouseY, state);
        this.field_184866_u.mouseReleased(mouseX, mouseY, state);
    }

    public void func_184861_a(String p_184861_1_)
    {
        this.field_184869_h = p_184861_1_;
    }

    public void func_184863_a(GuiListWorldSelectionEntry p_184863_1_)
    {
        boolean flag = p_184863_1_ != null;
        this.selectButton.enabled = flag;
        this.deleteButton.enabled = flag;
        this.renameButton.enabled = flag;
        this.field_184865_t.enabled = flag;
    }
}
