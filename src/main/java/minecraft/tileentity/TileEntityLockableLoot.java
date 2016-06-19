package net.minecraft.tileentity;

import java.util.Random;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.WorldServer;
import net.minecraft.world.storage.loot.ILootContainer;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.LootTable;

public abstract class TileEntityLockableLoot extends TileEntityLockable implements ILootContainer
{
    protected ResourceLocation lootTable;
    protected long lootTableSeed;

    protected boolean func_184283_b(NBTTagCompound p_184283_1_)
    {
        if (p_184283_1_.hasKey("LootTable", 8))
        {
            this.lootTable = new ResourceLocation(p_184283_1_.getString("LootTable"));
            this.lootTableSeed = p_184283_1_.getLong("LootTableSeed");
            return true;
        }
        else
        {
            return false;
        }
    }

    protected boolean func_184282_c(NBTTagCompound p_184282_1_)
    {
        if (this.lootTable != null)
        {
            p_184282_1_.setString("LootTable", this.lootTable.toString());

            if (this.lootTableSeed != 0L)
            {
                p_184282_1_.setLong("LootTableSeed", this.lootTableSeed);
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    protected void func_184281_d(EntityPlayer p_184281_1_)
    {
        if (this.lootTable != null)
        {
            LootTable loottable = this.worldObj.getLootTableManager().func_186521_a(this.lootTable);
            this.lootTable = null;
            Random random;

            if (this.lootTableSeed == 0L)
            {
                random = new Random();
            }
            else
            {
                random = new Random(this.lootTableSeed);
            }

            LootContext.Builder lootcontext$builder = new LootContext.Builder((WorldServer)this.worldObj);

            if (p_184281_1_ != null)
            {
                lootcontext$builder.withLuck(p_184281_1_.getLuck());
            }

            loottable.func_186460_a(this, random, lootcontext$builder.build());
        }
    }

    public ResourceLocation getLootTable()
    {
        return this.lootTable;
    }
}
