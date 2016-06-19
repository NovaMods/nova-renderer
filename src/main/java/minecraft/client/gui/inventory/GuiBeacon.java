package net.minecraft.client.gui.inventory;

import io.netty.buffer.Unpooled;
import java.io.IOException;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.RenderHelper;
import net.minecraft.client.resources.I18n;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.inventory.ContainerBeacon;
import net.minecraft.inventory.IInventory;
import net.minecraft.item.ItemStack;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.client.CPacketCloseWindow;
import net.minecraft.network.play.client.CPacketCustomPayload;
import net.minecraft.potion.Potion;
import net.minecraft.tileentity.TileEntityBeacon;
import net.minecraft.util.ResourceLocation;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GuiBeacon extends GuiContainer
{
    private static final Logger logger = LogManager.getLogger();
    private static final ResourceLocation beaconGuiTextures = new ResourceLocation("textures/gui/container/beacon.png");
    private IInventory tileBeacon;
    private GuiBeacon.ConfirmButton beaconConfirmButton;
    private boolean buttonsNotDrawn;

    public GuiBeacon(InventoryPlayer playerInventory, IInventory tileBeaconIn)
    {
        super(new ContainerBeacon(playerInventory, tileBeaconIn));
        this.tileBeacon = tileBeaconIn;
        this.xSize = 230;
        this.ySize = 219;
    }

    /**
     * Adds the buttons (and other controls) to the screen in question. Called when the GUI is displayed and when the
     * window resizes, the buttonList is cleared beforehand.
     */
    public void initGui()
    {
        super.initGui();
        this.buttonList.add(this.beaconConfirmButton = new GuiBeacon.ConfirmButton(-1, this.guiLeft + 164, this.guiTop + 107));
        this.buttonList.add(new GuiBeacon.CancelButton(-2, this.guiLeft + 190, this.guiTop + 107));
        this.buttonsNotDrawn = true;
        this.beaconConfirmButton.enabled = false;
    }

    /**
     * Called from the main game loop to update the screen.
     */
    public void updateScreen()
    {
        super.updateScreen();
        int i = this.tileBeacon.getField(0);
        Potion potion = Potion.getPotionById(this.tileBeacon.getField(1));
        Potion potion1 = Potion.getPotionById(this.tileBeacon.getField(2));

        if (this.buttonsNotDrawn && i >= 0)
        {
            this.buttonsNotDrawn = false;
            int j = 100;

            for (int k = 0; k <= 2; ++k)
            {
                int l = TileEntityBeacon.effectsList[k].length;
                int i1 = l * 22 + (l - 1) * 2;

                for (int j1 = 0; j1 < l; ++j1)
                {
                    Potion potion2 = TileEntityBeacon.effectsList[k][j1];
                    GuiBeacon.PowerButton guibeacon$powerbutton = new GuiBeacon.PowerButton(j++, this.guiLeft + 76 + j1 * 24 - i1 / 2, this.guiTop + 22 + k * 25, potion2, k);
                    this.buttonList.add(guibeacon$powerbutton);

                    if (k >= i)
                    {
                        guibeacon$powerbutton.enabled = false;
                    }
                    else if (potion2 == potion)
                    {
                        guibeacon$powerbutton.func_146140_b(true);
                    }
                }
            }

            int k1 = 3;
            int l1 = TileEntityBeacon.effectsList[k1].length + 1;
            int i2 = l1 * 22 + (l1 - 1) * 2;

            for (int j2 = 0; j2 < l1 - 1; ++j2)
            {
                Potion potion3 = TileEntityBeacon.effectsList[k1][j2];
                GuiBeacon.PowerButton guibeacon$powerbutton2 = new GuiBeacon.PowerButton(j++, this.guiLeft + 167 + j2 * 24 - i2 / 2, this.guiTop + 47, potion3, k1);
                this.buttonList.add(guibeacon$powerbutton2);

                if (k1 >= i)
                {
                    guibeacon$powerbutton2.enabled = false;
                }
                else if (potion3 == potion1)
                {
                    guibeacon$powerbutton2.func_146140_b(true);
                }
            }

            if (potion != null)
            {
                GuiBeacon.PowerButton guibeacon$powerbutton1 = new GuiBeacon.PowerButton(j++, this.guiLeft + 167 + (l1 - 1) * 24 - i2 / 2, this.guiTop + 47, potion, k1);
                this.buttonList.add(guibeacon$powerbutton1);

                if (k1 >= i)
                {
                    guibeacon$powerbutton1.enabled = false;
                }
                else if (potion == potion1)
                {
                    guibeacon$powerbutton1.func_146140_b(true);
                }
            }
        }

        this.beaconConfirmButton.enabled = this.tileBeacon.getStackInSlot(0) != null && potion != null;
    }

    /**
     * Called by the controls from the buttonList when activated. (Mouse pressed for buttons)
     */
    protected void actionPerformed(GuiButton button) throws IOException
    {
        if (button.id == -2)
        {
            this.mc.thePlayer.sendQueue.addToSendQueue(new CPacketCloseWindow(this.mc.thePlayer.openContainer.windowId));
            this.mc.displayGuiScreen((GuiScreen)null);
        }
        else if (button.id == -1)
        {
            String s = "MC|Beacon";
            PacketBuffer packetbuffer = new PacketBuffer(Unpooled.buffer());
            packetbuffer.writeInt(this.tileBeacon.getField(1));
            packetbuffer.writeInt(this.tileBeacon.getField(2));
            this.mc.getNetHandler().addToSendQueue(new CPacketCustomPayload(s, packetbuffer));
            this.mc.thePlayer.sendQueue.addToSendQueue(new CPacketCloseWindow(this.mc.thePlayer.openContainer.windowId));
            this.mc.displayGuiScreen((GuiScreen)null);
        }
        else if (button instanceof GuiBeacon.PowerButton)
        {
            GuiBeacon.PowerButton guibeacon$powerbutton = (GuiBeacon.PowerButton)button;

            if (guibeacon$powerbutton.func_146141_c())
            {
                return;
            }

            int i = Potion.getIdFromPotion(guibeacon$powerbutton.field_184066_p);

            if (guibeacon$powerbutton.field_146148_q < 3)
            {
                this.tileBeacon.setField(1, i);
            }
            else
            {
                this.tileBeacon.setField(2, i);
            }

            this.buttonList.clear();
            this.initGui();
            this.updateScreen();
        }
    }

    /**
     * Draw the foreground layer for the GuiContainer (everything in front of the items). Args : mouseX, mouseY
     */
    protected void drawGuiContainerForegroundLayer(int mouseX, int mouseY)
    {
        RenderHelper.disableStandardItemLighting();
        this.drawCenteredString(this.fontRendererObj, I18n.format("tile.beacon.primary", new Object[0]), 62, 10, 14737632);
        this.drawCenteredString(this.fontRendererObj, I18n.format("tile.beacon.secondary", new Object[0]), 169, 10, 14737632);

        for (GuiButton guibutton : this.buttonList)
        {
            if (guibutton.isMouseOver())
            {
                guibutton.drawButtonForegroundLayer(mouseX - this.guiLeft, mouseY - this.guiTop);
                break;
            }
        }

        RenderHelper.enableGUIStandardItemLighting();
    }

    /**
     * Args : renderPartialTicks, mouseX, mouseY
     */
    protected void drawGuiContainerBackgroundLayer(float partialTicks, int mouseX, int mouseY)
    {
        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
        this.mc.getTextureManager().bindTexture(beaconGuiTextures);
        int i = (this.width - this.xSize) / 2;
        int j = (this.height - this.ySize) / 2;
        this.drawTexturedModalRect(i, j, 0, 0, this.xSize, this.ySize);
        this.itemRender.zLevel = 100.0F;
        this.itemRender.renderItemAndEffectIntoGUI(new ItemStack(Items.emerald), i + 42, j + 109);
        this.itemRender.renderItemAndEffectIntoGUI(new ItemStack(Items.diamond), i + 42 + 22, j + 109);
        this.itemRender.renderItemAndEffectIntoGUI(new ItemStack(Items.gold_ingot), i + 42 + 44, j + 109);
        this.itemRender.renderItemAndEffectIntoGUI(new ItemStack(Items.iron_ingot), i + 42 + 66, j + 109);
        this.itemRender.zLevel = 0.0F;
    }

    static class Button extends GuiButton
    {
        private final ResourceLocation field_146145_o;
        private final int field_146144_p;
        private final int field_146143_q;
        private boolean field_146142_r;

        protected Button(int p_i1077_1_, int p_i1077_2_, int p_i1077_3_, ResourceLocation p_i1077_4_, int p_i1077_5_, int p_i1077_6_)
        {
            super(p_i1077_1_, p_i1077_2_, p_i1077_3_, 22, 22, "");
            this.field_146145_o = p_i1077_4_;
            this.field_146144_p = p_i1077_5_;
            this.field_146143_q = p_i1077_6_;
        }

        public void drawButton(Minecraft mc, int mouseX, int mouseY)
        {
            if (this.visible)
            {
                mc.getTextureManager().bindTexture(GuiBeacon.beaconGuiTextures);
                GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
                this.hovered = mouseX >= this.xPosition && mouseY >= this.yPosition && mouseX < this.xPosition + this.width && mouseY < this.yPosition + this.height;
                int i = 219;
                int j = 0;

                if (!this.enabled)
                {
                    j += this.width * 2;
                }
                else if (this.field_146142_r)
                {
                    j += this.width * 1;
                }
                else if (this.hovered)
                {
                    j += this.width * 3;
                }

                this.drawTexturedModalRect(this.xPosition, this.yPosition, j, i, this.width, this.height);

                if (!GuiBeacon.beaconGuiTextures.equals(this.field_146145_o))
                {
                    mc.getTextureManager().bindTexture(this.field_146145_o);
                }

                this.drawTexturedModalRect(this.xPosition + 2, this.yPosition + 2, this.field_146144_p, this.field_146143_q, 18, 18);
            }
        }

        public boolean func_146141_c()
        {
            return this.field_146142_r;
        }

        public void func_146140_b(boolean p_146140_1_)
        {
            this.field_146142_r = p_146140_1_;
        }
    }

    class CancelButton extends GuiBeacon.Button
    {
        public CancelButton(int p_i1074_2_, int p_i1074_3_, int p_i1074_4_)
        {
            super(p_i1074_2_, p_i1074_3_, p_i1074_4_, GuiBeacon.beaconGuiTextures, 112, 220);
        }

        public void drawButtonForegroundLayer(int mouseX, int mouseY)
        {
            GuiBeacon.this.drawCreativeTabHoveringText(I18n.format("gui.cancel", new Object[0]), mouseX, mouseY);
        }
    }

    class ConfirmButton extends GuiBeacon.Button
    {
        public ConfirmButton(int p_i1075_2_, int p_i1075_3_, int p_i1075_4_)
        {
            super(p_i1075_2_, p_i1075_3_, p_i1075_4_, GuiBeacon.beaconGuiTextures, 90, 220);
        }

        public void drawButtonForegroundLayer(int mouseX, int mouseY)
        {
            GuiBeacon.this.drawCreativeTabHoveringText(I18n.format("gui.done", new Object[0]), mouseX, mouseY);
        }
    }

    class PowerButton extends GuiBeacon.Button
    {
        private final Potion field_184066_p;
        private final int field_146148_q;

        public PowerButton(int p_i47045_2_, int p_i47045_3_, int p_i47045_4_, Potion p_i47045_5_, int p_i47045_6_)
        {
            super(p_i47045_2_, p_i47045_3_, p_i47045_4_, GuiContainer.inventoryBackground, p_i47045_5_.getStatusIconIndex() % 8 * 18, 198 + p_i47045_5_.getStatusIconIndex() / 8 * 18);
            this.field_184066_p = p_i47045_5_;
            this.field_146148_q = p_i47045_6_;
        }

        public void drawButtonForegroundLayer(int mouseX, int mouseY)
        {
            String s = I18n.format(this.field_184066_p.getName(), new Object[0]);

            if (this.field_146148_q >= 3 && this.field_184066_p != MobEffects.regeneration)
            {
                s = s + " II";
            }

            GuiBeacon.this.drawCreativeTabHoveringText(s, mouseX, mouseY);
        }
    }
}
