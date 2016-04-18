package net.minecraft.entity.item;

import java.util.Random;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.InventoryHelper;
import net.minecraft.inventory.ItemStackHelper;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.ILockableContainer;
import net.minecraft.world.LockCode;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;
import net.minecraft.world.storage.loot.ILootContainer;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.LootTable;

public abstract class EntityMinecartContainer extends EntityMinecart implements ILockableContainer, ILootContainer
{
    private ItemStack[] minecartContainerItems = new ItemStack[36];

    /**
     * When set to true, the minecart will drop all items when setDead() is called. When false (such as when travelling
     * dimensions) it preserves its contents.
     */
    private boolean dropContentsWhenDead = true;
    private ResourceLocation field_184290_c;
    private long field_184291_d;

    public EntityMinecartContainer(World worldIn)
    {
        super(worldIn);
    }

    public EntityMinecartContainer(World worldIn, double x, double y, double z)
    {
        super(worldIn, x, y, z);
    }

    public void killMinecart(DamageSource source)
    {
        super.killMinecart(source);

        if (this.worldObj.getGameRules().getBoolean("doEntityDrops"))
        {
            InventoryHelper.dropInventoryItems(this.worldObj, this, this);
        }
    }

    /**
     * Returns the stack in the given slot.
     */
    public ItemStack getStackInSlot(int index)
    {
        this.func_184288_f((EntityPlayer)null);
        return this.minecartContainerItems[index];
    }

    /**
     * Removes up to a specified number of items from an inventory slot and returns them in a new stack.
     */
    public ItemStack decrStackSize(int index, int count)
    {
        this.func_184288_f((EntityPlayer)null);
        return ItemStackHelper.func_188382_a(this.minecartContainerItems, index, count);
    }

    /**
     * Removes a stack from the given slot and returns it.
     */
    public ItemStack removeStackFromSlot(int index)
    {
        this.func_184288_f((EntityPlayer)null);

        if (this.minecartContainerItems[index] != null)
        {
            ItemStack itemstack = this.minecartContainerItems[index];
            this.minecartContainerItems[index] = null;
            return itemstack;
        }
        else
        {
            return null;
        }
    }

    /**
     * Sets the given item stack to the specified slot in the inventory (can be crafting or armor sections).
     */
    public void setInventorySlotContents(int index, ItemStack stack)
    {
        this.func_184288_f((EntityPlayer)null);
        this.minecartContainerItems[index] = stack;

        if (stack != null && stack.stackSize > this.getInventoryStackLimit())
        {
            stack.stackSize = this.getInventoryStackLimit();
        }
    }

    /**
     * For tile entities, ensures the chunk containing the tile entity is saved to disk later - the game won't think it
     * hasn't changed and skip it.
     */
    public void markDirty()
    {
    }

    /**
     * Do not make give this method the name canInteractWith because it clashes with Container
     */
    public boolean isUseableByPlayer(EntityPlayer player)
    {
        return this.isDead ? false : player.getDistanceSqToEntity(this) <= 64.0D;
    }

    public void openInventory(EntityPlayer player)
    {
    }

    public void closeInventory(EntityPlayer player)
    {
    }

    /**
     * Returns true if automation is allowed to insert the given stack (ignoring stack size) into the given slot.
     */
    public boolean isItemValidForSlot(int index, ItemStack stack)
    {
        return true;
    }

    /**
     * Get the name of this object. For players this returns their username
     */
    public String getName()
    {
        return this.hasCustomName() ? this.getCustomNameTag() : "container.minecart";
    }

    /**
     * Returns the maximum stack size for a inventory slot. Seems to always be 64, possibly will be extended.
     */
    public int getInventoryStackLimit()
    {
        return 64;
    }

    public Entity changeDimension(int dimensionIn)
    {
        this.dropContentsWhenDead = false;
        return super.changeDimension(dimensionIn);
    }

    /**
     * Will get destroyed next tick.
     */
    public void setDead()
    {
        if (this.dropContentsWhenDead)
        {
            InventoryHelper.dropInventoryItems(this.worldObj, this, this);
        }

        super.setDead();
    }

    public void func_184174_b(boolean p_184174_1_)
    {
        this.dropContentsWhenDead = p_184174_1_;
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);

        if (this.field_184290_c != null)
        {
            tagCompound.setString("LootTable", this.field_184290_c.toString());

            if (this.field_184291_d != 0L)
            {
                tagCompound.setLong("LootTableSeed", this.field_184291_d);
            }
        }
        else
        {
            NBTTagList nbttaglist = new NBTTagList();

            for (int i = 0; i < this.minecartContainerItems.length; ++i)
            {
                if (this.minecartContainerItems[i] != null)
                {
                    NBTTagCompound nbttagcompound = new NBTTagCompound();
                    nbttagcompound.setByte("Slot", (byte)i);
                    this.minecartContainerItems[i].writeToNBT(nbttagcompound);
                    nbttaglist.appendTag(nbttagcompound);
                }
            }

            tagCompound.setTag("Items", nbttaglist);
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.minecartContainerItems = new ItemStack[this.getSizeInventory()];

        if (tagCompund.hasKey("LootTable", 8))
        {
            this.field_184290_c = new ResourceLocation(tagCompund.getString("LootTable"));
            this.field_184291_d = tagCompund.getLong("LootTableSeed");
        }
        else
        {
            NBTTagList nbttaglist = tagCompund.getTagList("Items", 10);

            for (int i = 0; i < nbttaglist.tagCount(); ++i)
            {
                NBTTagCompound nbttagcompound = nbttaglist.getCompoundTagAt(i);
                int j = nbttagcompound.getByte("Slot") & 255;

                if (j >= 0 && j < this.minecartContainerItems.length)
                {
                    this.minecartContainerItems[j] = ItemStack.loadItemStackFromNBT(nbttagcompound);
                }
            }
        }
    }

    public boolean func_184230_a(EntityPlayer p_184230_1_, ItemStack p_184230_2_, EnumHand p_184230_3_)
    {
        if (!this.worldObj.isRemote)
        {
            p_184230_1_.displayGUIChest(this);
        }

        return true;
    }

    protected void applyDrag()
    {
        float f = 0.98F;

        if (this.field_184290_c == null)
        {
            int i = 15 - Container.calcRedstoneFromInventory(this);
            f += (float)i * 0.001F;
        }

        this.motionX *= (double)f;
        this.motionY *= 0.0D;
        this.motionZ *= (double)f;
    }

    public int getField(int id)
    {
        return 0;
    }

    public void setField(int id, int value)
    {
    }

    public int getFieldCount()
    {
        return 0;
    }

    public boolean isLocked()
    {
        return false;
    }

    public void setLockCode(LockCode code)
    {
    }

    public LockCode getLockCode()
    {
        return LockCode.EMPTY_CODE;
    }

    public void func_184288_f(EntityPlayer p_184288_1_)
    {
        if (this.field_184290_c != null)
        {
            LootTable loottable = this.worldObj.getLootTableManager().func_186521_a(this.field_184290_c);
            this.field_184290_c = null;
            Random random;

            if (this.field_184291_d == 0L)
            {
                random = new Random();
            }
            else
            {
                random = new Random(this.field_184291_d);
            }

            LootContext.Builder lootcontext$builder = new LootContext.Builder((WorldServer)this.worldObj);

            if (p_184288_1_ != null)
            {
                lootcontext$builder.withLuck(p_184288_1_.getLuck());
            }

            loottable.func_186460_a(this, random, lootcontext$builder.build());
        }
    }

    public void clear()
    {
        this.func_184288_f((EntityPlayer)null);

        for (int i = 0; i < this.minecartContainerItems.length; ++i)
        {
            this.minecartContainerItems[i] = null;
        }
    }

    public void func_184289_a(ResourceLocation p_184289_1_, long p_184289_2_)
    {
        this.field_184290_c = p_184289_1_;
        this.field_184291_d = p_184289_2_;
    }

    public ResourceLocation getLootTable()
    {
        return this.field_184290_c;
    }
}
