package net.minecraft.client.gui;

import io.netty.buffer.Unpooled;
import java.io.IOException;
import net.minecraft.client.resources.I18n;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.client.CPacketCustomPayload;
import net.minecraft.tileentity.CommandBlockBaseLogic;
import net.minecraft.tileentity.TileEntityCommandBlock;
import net.minecraft.util.ITabCompleter;
import net.minecraft.util.TabCompleter;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.ITextComponent;
import org.lwjgl.input.Keyboard;

public class GuiCommandBlock extends GuiScreen implements ITabCompleter
{
    /** Text field containing the command block's command. */
    private GuiTextField commandTextField;
    private GuiTextField previousOutputTextField;
    private final TileEntityCommandBlock field_184078_g;

    /** "Done" button for the GUI. */
    private GuiButton doneBtn;
    private GuiButton cancelBtn;
    private GuiButton field_175390_s;
    private GuiButton field_184079_s;
    private GuiButton field_184080_t;
    private GuiButton field_184081_u;
    private boolean field_175389_t;
    private TileEntityCommandBlock.Mode field_184082_w = TileEntityCommandBlock.Mode.REDSTONE;
    private TabCompleter field_184083_x;
    private boolean field_184084_y;
    private boolean field_184085_z;

    public GuiCommandBlock(TileEntityCommandBlock p_i46596_1_)
    {
        this.field_184078_g = p_i46596_1_;
    }

    /**
     * Called from the main game loop to update the screen.
     */
    public void updateScreen()
    {
        this.commandTextField.updateCursorCounter();
    }

    /**
     * Adds the buttons (and other controls) to the screen in question. Called when the GUI is displayed and when the
     * window resizes, the buttonList is cleared beforehand.
     */
    public void initGui()
    {
        final CommandBlockBaseLogic commandblockbaselogic = this.field_184078_g.getCommandBlockLogic();
        Keyboard.enableRepeatEvents(true);
        this.buttonList.clear();
        this.buttonList.add(this.doneBtn = new GuiButton(0, this.width / 2 - 4 - 150, this.height / 4 + 120 + 12, 150, 20, I18n.format("gui.done", new Object[0])));
        this.buttonList.add(this.cancelBtn = new GuiButton(1, this.width / 2 + 4, this.height / 4 + 120 + 12, 150, 20, I18n.format("gui.cancel", new Object[0])));
        this.buttonList.add(this.field_175390_s = new GuiButton(4, this.width / 2 + 150 - 20, 135, 20, 20, "O"));
        this.buttonList.add(this.field_184079_s = new GuiButton(5, this.width / 2 - 50 - 100 - 4, 165, 100, 20, I18n.format("advMode.mode.sequence", new Object[0])));
        this.buttonList.add(this.field_184080_t = new GuiButton(6, this.width / 2 - 50, 165, 100, 20, I18n.format("advMode.mode.unconditional", new Object[0])));
        this.buttonList.add(this.field_184081_u = new GuiButton(7, this.width / 2 + 50 + 4, 165, 100, 20, I18n.format("advMode.mode.redstoneTriggered", new Object[0])));
        this.commandTextField = new GuiTextField(2, this.fontRendererObj, this.width / 2 - 150, 50, 300, 20);
        this.commandTextField.setMaxStringLength(32500);
        this.commandTextField.setFocused(true);
        this.previousOutputTextField = new GuiTextField(3, this.fontRendererObj, this.width / 2 - 150, 135, 276, 20);
        this.previousOutputTextField.setMaxStringLength(32500);
        this.previousOutputTextField.setEnabled(false);
        this.previousOutputTextField.setText("-");
        this.doneBtn.enabled = false;
        this.field_175390_s.enabled = false;
        this.field_184079_s.enabled = false;
        this.field_184080_t.enabled = false;
        this.field_184081_u.enabled = false;
        this.field_184083_x = new TabCompleter(this.commandTextField, true)
        {
            public BlockPos func_186839_b()
            {
                return commandblockbaselogic.getPosition();
            }
        };
    }

    public void func_184075_a()
    {
        CommandBlockBaseLogic commandblockbaselogic = this.field_184078_g.getCommandBlockLogic();
        this.commandTextField.setText(commandblockbaselogic.getCommand());
        this.field_175389_t = commandblockbaselogic.shouldTrackOutput();
        this.field_184082_w = this.field_184078_g.func_184251_i();
        this.field_184084_y = this.field_184078_g.func_184258_j();
        this.field_184085_z = this.field_184078_g.isAuto();
        this.func_175388_a();
        this.func_184073_g();
        this.func_184077_i();
        this.func_184076_j();
        this.doneBtn.enabled = true;
        this.field_175390_s.enabled = true;
        this.field_184079_s.enabled = true;
        this.field_184080_t.enabled = true;
        this.field_184081_u.enabled = true;
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
            CommandBlockBaseLogic commandblockbaselogic = this.field_184078_g.getCommandBlockLogic();

            if (button.id == 1)
            {
                commandblockbaselogic.setTrackOutput(this.field_175389_t);
                this.mc.displayGuiScreen((GuiScreen)null);
            }
            else if (button.id == 0)
            {
                PacketBuffer packetbuffer = new PacketBuffer(Unpooled.buffer());
                commandblockbaselogic.func_145757_a(packetbuffer);
                packetbuffer.writeString(this.commandTextField.getText());
                packetbuffer.writeBoolean(commandblockbaselogic.shouldTrackOutput());
                packetbuffer.writeString(this.field_184082_w.name());
                packetbuffer.writeBoolean(this.field_184084_y);
                packetbuffer.writeBoolean(this.field_184085_z);
                this.mc.getNetHandler().addToSendQueue(new CPacketCustomPayload("MC|AutoCmd", packetbuffer));

                if (!commandblockbaselogic.shouldTrackOutput())
                {
                    commandblockbaselogic.setLastOutput((ITextComponent)null);
                }

                this.mc.displayGuiScreen((GuiScreen)null);
            }
            else if (button.id == 4)
            {
                commandblockbaselogic.setTrackOutput(!commandblockbaselogic.shouldTrackOutput());
                this.func_175388_a();
            }
            else if (button.id == 5)
            {
                this.func_184074_h();
                this.func_184073_g();
            }
            else if (button.id == 6)
            {
                this.field_184084_y = !this.field_184084_y;
                this.func_184077_i();
            }
            else if (button.id == 7)
            {
                this.field_184085_z = !this.field_184085_z;
                this.func_184076_j();
            }
        }
    }

    /**
     * Fired when a key is typed (except F11 which toggles full screen). This is the equivalent of
     * KeyListener.keyTyped(KeyEvent e). Args : character (character on the key), keyCode (lwjgl Keyboard key code)
     */
    protected void keyTyped(char typedChar, int keyCode) throws IOException
    {
        this.field_184083_x.func_186843_d();

        if (keyCode == 15)
        {
            this.field_184083_x.func_186841_a();
        }
        else
        {
            this.field_184083_x.func_186842_c();
        }

        this.commandTextField.textboxKeyTyped(typedChar, keyCode);
        this.previousOutputTextField.textboxKeyTyped(typedChar, keyCode);

        if (keyCode != 28 && keyCode != 156)
        {
            if (keyCode == 1)
            {
                this.actionPerformed(this.cancelBtn);
            }
        }
        else
        {
            this.actionPerformed(this.doneBtn);
        }
    }

    /**
     * Called when the mouse is clicked. Args : mouseX, mouseY, clickedButton
     */
    protected void mouseClicked(int mouseX, int mouseY, int mouseButton) throws IOException
    {
        super.mouseClicked(mouseX, mouseY, mouseButton);
        this.commandTextField.mouseClicked(mouseX, mouseY, mouseButton);
        this.previousOutputTextField.mouseClicked(mouseX, mouseY, mouseButton);
    }

    /**
     * Draws the screen and all the components in it. Args : mouseX, mouseY, renderPartialTicks
     */
    public void drawScreen(int mouseX, int mouseY, float partialTicks)
    {
        this.drawDefaultBackground();
        this.drawCenteredString(this.fontRendererObj, I18n.format("advMode.setCommand", new Object[0]), this.width / 2, 20, 16777215);
        this.drawString(this.fontRendererObj, I18n.format("advMode.command", new Object[0]), this.width / 2 - 150, 37, 10526880);
        this.commandTextField.drawTextBox();
        int i = 75;
        int j = 0;
        this.drawString(this.fontRendererObj, I18n.format("advMode.nearestPlayer", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, I18n.format("advMode.randomPlayer", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, I18n.format("advMode.allPlayers", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, I18n.format("advMode.allEntities", new Object[0]), this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);
        this.drawString(this.fontRendererObj, "", this.width / 2 - 150, i + j++ * this.fontRendererObj.FONT_HEIGHT, 10526880);

        if (!this.previousOutputTextField.getText().isEmpty())
        {
            i = i + j * this.fontRendererObj.FONT_HEIGHT + 1;
            this.drawString(this.fontRendererObj, I18n.format("advMode.previousOutput", new Object[0]), this.width / 2 - 150, i, 10526880);
            this.previousOutputTextField.drawTextBox();
        }

        super.drawScreen(mouseX, mouseY, partialTicks);
    }

    private void func_175388_a()
    {
        CommandBlockBaseLogic commandblockbaselogic = this.field_184078_g.getCommandBlockLogic();

        if (commandblockbaselogic.shouldTrackOutput())
        {
            this.field_175390_s.displayString = "O";

            if (commandblockbaselogic.getLastOutput() != null)
            {
                this.previousOutputTextField.setText(commandblockbaselogic.getLastOutput().getUnformattedText());
            }
        }
        else
        {
            this.field_175390_s.displayString = "X";
            this.previousOutputTextField.setText("-");
        }
    }

    private void func_184073_g()
    {
        switch (this.field_184082_w)
        {
            case SEQUENCE:
                this.field_184079_s.displayString = I18n.format("advMode.mode.sequence", new Object[0]);
                break;

            case AUTO:
                this.field_184079_s.displayString = I18n.format("advMode.mode.auto", new Object[0]);
                break;

            case REDSTONE:
                this.field_184079_s.displayString = I18n.format("advMode.mode.redstone", new Object[0]);
        }
    }

    private void func_184074_h()
    {
        switch (this.field_184082_w)
        {
            case SEQUENCE:
                this.field_184082_w = TileEntityCommandBlock.Mode.AUTO;
                break;

            case AUTO:
                this.field_184082_w = TileEntityCommandBlock.Mode.REDSTONE;
                break;

            case REDSTONE:
                this.field_184082_w = TileEntityCommandBlock.Mode.SEQUENCE;
        }
    }

    private void func_184077_i()
    {
        if (this.field_184084_y)
        {
            this.field_184080_t.displayString = I18n.format("advMode.mode.conditional", new Object[0]);
        }
        else
        {
            this.field_184080_t.displayString = I18n.format("advMode.mode.unconditional", new Object[0]);
        }
    }

    private void func_184076_j()
    {
        if (this.field_184085_z)
        {
            this.field_184081_u.displayString = I18n.format("advMode.mode.autoexec.bat", new Object[0]);
        }
        else
        {
            this.field_184081_u.displayString = I18n.format("advMode.mode.redstoneTriggered", new Object[0]);
        }
    }

    public void func_184072_a(String... p_184072_1_)
    {
        this.field_184083_x.func_186840_a(p_184072_1_);
    }
}
