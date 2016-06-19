package net.minecraft.client.gui;

import com.google.common.collect.Lists;
import java.util.Collections;
import java.util.List;
import net.minecraft.client.AnvilConverterException;
import net.minecraft.client.Minecraft;
import net.minecraft.world.storage.ISaveFormat;
import net.minecraft.world.storage.SaveFormatComparator;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GuiListWorldSelection extends GuiListExtended
{
    private static final Logger field_186797_u = LogManager.getLogger();
    private final GuiWorldSelection field_186798_v;
    private final List<GuiListWorldSelectionEntry> field_186799_w = Lists.<GuiListWorldSelectionEntry>newArrayList();
    private int field_186800_x = -1;

    public GuiListWorldSelection(GuiWorldSelection p_i46590_1_, Minecraft p_i46590_2_, int p_i46590_3_, int p_i46590_4_, int p_i46590_5_, int p_i46590_6_, int p_i46590_7_)
    {
        super(p_i46590_2_, p_i46590_3_, p_i46590_4_, p_i46590_5_, p_i46590_6_, p_i46590_7_);
        this.field_186798_v = p_i46590_1_;
        this.func_186795_e();
    }

    public void func_186795_e()
    {
        ISaveFormat isaveformat = this.mc.getSaveLoader();
        List<SaveFormatComparator> list;

        try
        {
            list = isaveformat.getSaveList();
        }
        catch (AnvilConverterException anvilconverterexception)
        {
            field_186797_u.error((String)"Couldn\'t load level list", (Throwable)anvilconverterexception);
            this.mc.displayGuiScreen(new GuiErrorScreen("Unable to load worlds", anvilconverterexception.getMessage()));
            return;
        }

        Collections.sort(list);

        for (SaveFormatComparator saveformatcomparator : list)
        {
            this.field_186799_w.add(new GuiListWorldSelectionEntry(this, saveformatcomparator, this.mc.getSaveLoader()));
        }
    }

    /**
     * Gets the IGuiListEntry object for the given index
     */
    public GuiListWorldSelectionEntry getListEntry(int index)
    {
        return (GuiListWorldSelectionEntry)this.field_186799_w.get(index);
    }

    protected int getSize()
    {
        return this.field_186799_w.size();
    }

    protected int getScrollBarX()
    {
        return super.getScrollBarX() + 20;
    }

    /**
     * Gets the width of the list
     */
    public int getListWidth()
    {
        return super.getListWidth() + 50;
    }

    public void func_186792_d(int p_186792_1_)
    {
        this.field_186800_x = p_186792_1_;
        this.field_186798_v.func_184863_a(this.func_186794_f());
    }

    /**
     * Returns true if the element passed in is currently selected
     */
    protected boolean isSelected(int slotIndex)
    {
        return slotIndex == this.field_186800_x;
    }

    public GuiListWorldSelectionEntry func_186794_f()
    {
        return this.field_186800_x >= 0 && this.field_186800_x < this.getSize() ? this.getListEntry(this.field_186800_x) : null;
    }

    public GuiWorldSelection func_186796_g()
    {
        return this.field_186798_v;
    }
}
