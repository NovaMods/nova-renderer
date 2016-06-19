package net.minecraft.client.gui;

import net.minecraft.client.Minecraft;
import net.minecraft.realms.RealmsClickableScrolledSelectionList;
import net.minecraft.realms.Tezzelator;
import org.lwjgl.input.Mouse;

public class GuiClickableScrolledSelectionListProxy extends GuiSlot
{
    private final RealmsClickableScrolledSelectionList proxy;

    public GuiClickableScrolledSelectionListProxy(RealmsClickableScrolledSelectionList selectionList, int p_i45526_2_, int p_i45526_3_, int p_i45526_4_, int p_i45526_5_, int p_i45526_6_)
    {
        super(Minecraft.getMinecraft(), p_i45526_2_, p_i45526_3_, p_i45526_4_, p_i45526_5_, p_i45526_6_);
        this.proxy = selectionList;
    }

    protected int getSize()
    {
        return this.proxy.getItemCount();
    }

    /**
     * The element in the slot that was clicked, boolean for whether it was double clicked or not
     */
    protected void elementClicked(int slotIndex, boolean isDoubleClick, int mouseX, int mouseY)
    {
        this.proxy.selectItem(slotIndex, isDoubleClick, mouseX, mouseY);
    }

    /**
     * Returns true if the element passed in is currently selected
     */
    protected boolean isSelected(int slotIndex)
    {
        return this.proxy.isSelectedItem(slotIndex);
    }

    protected void drawBackground()
    {
        this.proxy.renderBackground();
    }

    protected void drawSlot(int entryID, int p_180791_2_, int p_180791_3_, int p_180791_4_, int mouseXIn, int mouseYIn)
    {
        this.proxy.renderItem(entryID, p_180791_2_, p_180791_3_, p_180791_4_, mouseXIn, mouseYIn);
    }

    public int width()
    {
        return super.width;
    }

    public int mouseY()
    {
        return super.mouseY;
    }

    public int mouseX()
    {
        return super.mouseX;
    }

    /**
     * Return the height of the content being scrolled
     */
    protected int getContentHeight()
    {
        return this.proxy.getMaxPosition();
    }

    protected int getScrollBarX()
    {
        return this.proxy.getScrollbarPosition();
    }

    public void handleMouseInput()
    {
        super.handleMouseInput();

        if (this.scrollMultiplier > 0.0F && Mouse.getEventButtonState())
        {
            this.proxy.customMouseEvent(this.top, this.bottom, this.headerPadding, this.amountScrolled, this.slotHeight);
        }
    }

    public void renderSelected(int p_178043_1_, int p_178043_2_, int p_178043_3_, Tezzelator p_178043_4_)
    {
        this.proxy.renderSelected(p_178043_1_, p_178043_2_, p_178043_3_, p_178043_4_);
    }

    /**
     * Draws the selection box around the selected slot element.
     */
    protected void drawSelectionBox(int p_148120_1_, int p_148120_2_, int mouseXIn, int mouseYIn)
    {
        int i = this.getSize();

        for (int j = 0; j < i; ++j)
        {
            int k = p_148120_2_ + j * this.slotHeight + this.headerPadding;
            int l = this.slotHeight - 4;

            if (k > this.bottom || k + l < this.top)
            {
                this.func_178040_a(j, p_148120_1_, k);
            }

            if (this.showSelectionBox && this.isSelected(j))
            {
                this.renderSelected(this.width, k, l, Tezzelator.instance);
            }

            this.drawSlot(j, p_148120_1_, k, l, mouseXIn, mouseYIn);
        }
    }
}
