package net.minecraft.client.gui.inventory;

import io.netty.buffer.Unpooled;
import java.io.IOException;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.gui.GuiTextField;
import net.minecraft.client.resources.I18n;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.client.CPacketCustomPayload;
import net.minecraft.tileentity.CommandBlockBaseLogic;
import net.minecraft.util.ITabCompleter;
import net.minecraft.util.TabCompleter;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.ITextComponent;
import org.lwjgl.input.Keyboard;

public class GuiEditCommandBlockMinecart extends GuiScreen implements ITabCompleter
{
    private GuiTextField field_184088_a;
    private GuiTextField field_184092_f;
    private final CommandBlockBaseLogic field_184093_g;
    private GuiButton field_184094_h;
    private GuiButton field_184095_i;
    private GuiButton field_184089_r;
    private boolean field_184090_s;
    private TabCompleter field_184091_t;

    public GuiEditCommandBlockMinecart(CommandBlockBaseLogic p_i46595_1_)
    {
        this.field_184093_g = p_i46595_1_;
    }

    /**
     * Called from the main game loop to update the screen.
     */
    public void updateScreen()
    {
        this.field_184088_a.updateCursorCounter();
    }

    /**
     * Adds the buttons (and other controls) to the screen in question. Called when the GUI is displayed and when the
     * window resizes, the buttonList is cleared beforehand.
     */
    public void initGui()
    {
        Keyboard.enableRepeatEvents(true);
        this.buttonList.clear();
        this.buttonList.add(this.field_184094_h = new GuiButton(0, this.width / 2 - 4 - 150, this.height / 4 + 120 + 12, 150, 20, I18n.format("gui.done", new Object[0])));
        this.buttonList.add(this.field_184095_i = new GuiButton(1, this.width / 2 + 4, this.height / 4 + 120 + 12, 150, 20, I18n.format("gui.cancel", new Object[0])));
        this.buttonList.add(this.field_184089_r = new GuiButton(4, this.width / 2 + 150 - 20, 150, 20, 20, "O"));
        this.field_184088_a = new GuiTextField(2, this.fontRendererObj, this.width / 2 - 150, 50, 300, 20);
        this.field_184088_a.setMaxStringLength(32500);
        this.field_184088_a.setFocused(true);
        this.field_184088_a.setText(this.field_184093_g.getCommand());
        this.field_184092_f = new GuiTextField(3, this.fontRendererObj, this.width / 2 - 150, 150, 276, 20);
        this.field_184092_f.setMaxStringLength(32500);
        this.field_184092_f.setEnabled(false);
        this.field_184092_f.setText("-");
        this.field_184090_s = this.field_184093_g.shouldTrackOutput();
        this.func_184087_a();
        this.field_184094_h.enabled = !this.field_184088_a.getText().trim().isEmpty();
        this.field_184091_t = new TabCompleter(this.field_184088_a, true)
        {
            public BlockPos func_186839_b()
            {
                return GuiEditCommandBlockMinecart.this.field_184093_g.getPosition();
            }
        };
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
                this.field_184093_g.setTrackOutput(this.field_184090_s);
                this.mc.displayGuiScreen((GuiScreen)null);
            }
            else if (button.id == 0)
            {
                PacketBuffer packetbuffer = new PacketBuffer(Unpooled.buffer());
                packetbuffer.writeByte(this.field_184093_g.func_145751_f());
                this.field_184093_g.func_145757_a(packetbuffer);
                packetbuffer.writeString(this.field_184088_a.getText());
                packetbuffer.writeBoolean(this.field_184093_g.shouldTrackOutput());
                this.mc.getNetHandler().addToSendQueue(new CPacketCustomPayload("MC|AdvCmd", packetbuffer));

                if (!this.field_184093_g.shouldTrackOutput())
                {
                    this.field_184093_g.setLastOutput((ITextComponent)null);
                }

                this.mc.displayGuiScreen((GuiScreen)null);
            }
            else if (button.id == 4)
            {
                this.field_184093_g.setTrackOutput(!this.field_184093_g.shouldTrackOutput());
                this.func_184087_a();
            }
        }
    }

    /**
     * Fired when a key is typed (except F11 which toggles full screen). This is the equivalent of
     * KeyListener.keyTyped(KeyEvent e). Args : character (character on the key), keyCode (lwjgl Keyboard key code)
     */
    protected void keyTyped(char typedChar, int keyCode) throws IOException
    {
        this.field_184091_t.func_186843_d();

        if (keyCode == 15)
        {
            this.field_184091_t.func_186841_a();
        }
        else
        {
            this.field_184091_t.func_186842_c();
        }

        this.field_184088_a.textboxKeyTyped(typedChar, keyCode);
        this.field_184092_f.textboxKeyTyped(typedChar, keyCode);
        this.field_184094_h.enabled = !this.field_184088_a.getText().trim().isEmpty();

        if (keyCode != 28 && keyCode != 156)
        {
            if (keyCode == 1)
            {
                this.actionPerformed(this.field_184095_i);
            }
        }
        else
        {
            this.actionPerformed(this.field_184094_h);
        }
    }

    /**
     * Called when the mouse is clicked. Args : mouseX, mouseY, clickedButton
     */
    protected void mouseClicked(int mouseX, int mouseY, int mouseButton) throws IOException
    {
        super.mouseClicked(mouseX, mouseY, mouseButton);
        this.field_184088_a.mouseClicked(mouseX, mouseY, mouseButton);
        this.field_184092_f.mouseClicked(mouseX, mouseY, mouseButton);
    }

    /**
     * Draws the screen and all the components in it. Args : mouseX, mouseY, renderPartialTicks
     */
    public void drawScreen(int mouseX, int mouseY, float partialTicks)
    {
        this.drawDefaultBackground();
        this.drawCenteredString(this.fontRendererObj, I18n.format("advMode.setCommand", new Object[0]), this.width / 2, 20, 16777215);
        this.drawString(this.fontRendererObj, I18n.format("advMode.command", new Object[0]), this.width / 2 - 150, 37, 10526880);
        this.field_184088_a.drawTextBox();
        int i = 75;
        int j = 0;
        this.drawString(this.fontRendererObj, I18n.format("advMode.nearestPlayer", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, I18n.format("advMode.randomPlayer", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, I18n.format("advMode.allPlayers", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, I18n.format("advMode.allEntities", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, "", this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);

        if (!this.field_184092_f.getText().isEmpty())
        {
            i = i + j * this.fontRendererObj.FONT_HEIGHT + 16;
            this.drawString(this.fontRendererObj, I18n.format("advMode.previousOutput", new Object[0]), this.width / 2 - 150, i, 10526880);
            this.field_184092_f.drawTextBox();
        }

        super.drawScreen(mouseX, mouseY, partialTicks);
    }

    private void func_184087_a()
    {
        if (this.field_184093_g.shouldTrackOutput())
        {
            this.field_184089_r.displayString = "O";

            if (this.field_184093_g.getLastOutput() != null)
            {
                this.field_184092_f.setText(this.field_184093_g.getLastOutput().getUnformattedText());
            }
        }
        else
        {
            this.field_184089_r.displayString = "X";
            this.field_184092_f.setText("-");
        }
    }

    public void func_184072_a(String... p_184072_1_)
    {
        this.field_184091_t.func_186840_a(p_184072_1_);
    }
}
