package net.minecraft.util.datafix.walkers;

import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.datafix.IDataFixer;
import net.minecraft.util.datafix.IDataWalker;

public abstract class Filtered implements IDataWalker
{
    private final String field_188272_a;
    private final String field_188273_b;

    public Filtered(String p_i46825_1_, String p_i46825_2_)
    {
        this.field_188272_a = p_i46825_1_;
        this.field_188273_b = p_i46825_2_;
    }

    public NBTTagCompound func_188266_a(IDataFixer p_188266_1_, NBTTagCompound p_188266_2_, int p_188266_3_)
    {
        if (p_188266_2_.getString(this.field_188272_a).equals(this.field_188273_b))
        {
            p_188266_2_ = this.func_188271_b(p_188266_1_, p_188266_2_, p_188266_3_);
        }

        return p_188266_2_;
    }

    abstract NBTTagCompound func_188271_b(IDataFixer p_188271_1_, NBTTagCompound p_188271_2_, int p_188271_3_);
}
