package net.minecraft.util.datafix.fixes;

import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.util.datafix.IFixableData;

public class RidingToPassengers implements IFixableData
{
    public int getFixVersion()
    {
        return 135;
    }

    public NBTTagCompound fixTagCompound(NBTTagCompound compound)
    {
        while (compound.hasKey("Riding", 10))
        {
            NBTTagCompound nbttagcompound = this.func_188220_b(compound);
            this.func_188219_a(compound, nbttagcompound);
            compound = nbttagcompound;
        }

        return compound;
    }

    protected void func_188219_a(NBTTagCompound p_188219_1_, NBTTagCompound p_188219_2_)
    {
        NBTTagList nbttaglist = new NBTTagList();
        nbttaglist.appendTag(p_188219_1_);
        p_188219_2_.setTag("Passengers", nbttaglist);
    }

    protected NBTTagCompound func_188220_b(NBTTagCompound p_188220_1_)
    {
        NBTTagCompound nbttagcompound = p_188220_1_.getCompoundTag("Riding");
        p_188220_1_.removeTag("Riding");
        return nbttagcompound;
    }
}
