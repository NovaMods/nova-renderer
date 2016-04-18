package net.minecraft.util.datafix.walkers;

import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.datafix.DataFixesManager;
import net.minecraft.util.datafix.IDataFixer;

public class ItemStackData extends Filtered
{
    private final String[] field_188274_a;

    public ItemStackData(String p_i46827_1_, String... p_i46827_2_)
    {
        super("id", p_i46827_1_);
        this.field_188274_a = p_i46827_2_;
    }

    NBTTagCompound func_188271_b(IDataFixer p_188271_1_, NBTTagCompound p_188271_2_, int p_188271_3_)
    {
        int i = 0;

        for (int j = this.field_188274_a.length; i < j; ++i)
        {
            p_188271_2_ = DataFixesManager.func_188277_a(p_188271_1_, p_188271_2_, p_188271_3_, this.field_188274_a[i]);
        }

        return p_188271_2_;
    }
}
