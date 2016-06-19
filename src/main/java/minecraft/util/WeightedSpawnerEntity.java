package net.minecraft.util;

import net.minecraft.nbt.NBTTagCompound;

public class WeightedSpawnerEntity extends WeightedRandom.Item
{
    private final NBTTagCompound field_185279_b;

    public WeightedSpawnerEntity()
    {
        super(1);
        this.field_185279_b = new NBTTagCompound();
        this.field_185279_b.setString("id", "Pig");
    }

    public WeightedSpawnerEntity(NBTTagCompound p_i46715_1_)
    {
        this(p_i46715_1_.hasKey("Weight", 99) ? p_i46715_1_.getInteger("Weight") : 1, p_i46715_1_.getCompoundTag("Entity"));
    }

    public WeightedSpawnerEntity(int p_i46716_1_, NBTTagCompound p_i46716_2_)
    {
        super(p_i46716_1_);
        this.field_185279_b = p_i46716_2_;
    }

    public NBTTagCompound func_185278_a()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        nbttagcompound.setTag("Entity", this.field_185279_b);
        nbttagcompound.setInteger("Weight", this.itemWeight);
        return nbttagcompound;
    }

    public NBTTagCompound func_185277_b()
    {
        return this.field_185279_b;
    }
}
