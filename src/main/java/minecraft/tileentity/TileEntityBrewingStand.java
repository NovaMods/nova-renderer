package net.minecraft.tileentity;

import java.util.Arrays;
import net.minecraft.block.BlockBrewingStand;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.init.Items;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.ContainerBrewingStand;
import net.minecraft.inventory.ISidedInventory;
import net.minecraft.inventory.InventoryHelper;
import net.minecraft.inventory.ItemStackHelper;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.potion.PotionHelper;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ITickable;
import net.minecraft.util.math.BlockPos;

public class TileEntityBrewingStand extends TileEntityLockable implements ITickable, ISidedInventory
{
    /** an array of the input slot indices */
    private static final int[] inputSlots = new int[] {3};
    private static final int[] field_184277_f = new int[] {0, 1, 2, 3};

    /** an array of the output slot indices */
    private static final int[] outputSlots = new int[] {0, 1, 2, 4};

    /** The ItemStacks currently placed in the slots of the brewing stand */
    private ItemStack[] brewingItemStacks = new ItemStack[5];
    private int brewTime;

    /**
     * an integer with each bit specifying whether that slot of the stand contains a potion
     */
    private boolean[] filledSlots;

    /**
     * used to check if the current ingredient has been removed from the brewing stand during brewing
     */
    private Item ingredientID;
    private String customName;
    private int fuel;

    /**
     * Get the name of this object. For players this returns their username
     */
    public String getName()
    {
        return this.hasCustomName() ? this.customName : "container.brewing";
    }

    /**
     * Returns true if this thing is named
     */
    public boolean hasCustomName()
    {
        return this.customName != null && !this.customName.isEmpty();
    }

    public void setName(String name)
    {
        this.customName = name;
    }

    /**
     * Returns the number of slots in the inventory.
     */
    public int getSizeInventory()
    {
        return this.brewingItemStacks.length;
    }

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        if (this.fuel <= 0 && this.brewingItemStacks[4] != null && this.brewingItemStacks[4].getItem() == Items.blaze_powder)
        {
            this.fuel = 20;
            --this.brewingItemStacks[4].stackSize;

            if (this.brewingItemStacks[4].stackSize <= 0)
            {
                this.brewingItemStacks[4] = null;
            }

            this.markDirty();
        }

        boolean flag = this.canBrew();
        boolean flag1 = this.brewTime > 0;

        if (flag1)
        {
            --this.brewTime;
            boolean flag2 = this.brewTime == 0;

            if (flag2 && flag)
            {
                this.brewPotions();
                this.markDirty();
            }
            else if (!flag)
            {
                this.brewTime = 0;
                this.markDirty();
            }
            else if (this.ingredientID != this.brewingItemStacks[3].getItem())
            {
                this.brewTime = 0;
                this.markDirty();
            }
        }
        else if (flag && this.fuel > 0)
        {
            --this.fuel;
            this.brewTime = 400;
            this.ingredientID = this.brewingItemStacks[3].getItem();
            this.markDirty();
        }

        if (!this.worldObj.isRemote)
        {
            boolean[] aboolean = this.func_174902_m();

            if (!Arrays.equals(aboolean, this.filledSlots))
            {
                this.filledSlots = aboolean;
                IBlockState iblockstate = this.worldObj.getBlockState(this.getPos());

                if (!(iblockstate.getBlock() instanceof BlockBrewingStand))
                {
                    return;
                }

                for (int i = 0; i < BlockBrewingStand.HAS_BOTTLE.length; ++i)
                {
                    iblockstate = iblockstate.withProperty(BlockBrewingStand.HAS_BOTTLE[i], Boolean.valueOf(aboolean[i]));
                }

                this.worldObj.setBlockState(this.pos, iblockstate, 2);
            }
        }
    }

    public boolean[] func_174902_m()
    {
        boolean[] aboolean = new boolean[3];

        for (int i = 0; i < 3; ++i)
        {
            if (this.brewingItemStacks[i] != null)
            {
                aboolean[i] = true;
            }
        }

        return aboolean;
    }

    private boolean canBrew()
    {
        if (this.brewingItemStacks[3] != null && this.brewingItemStacks[3].stackSize > 0)
        {
            ItemStack itemstack = this.brewingItemStacks[3];

            if (!PotionHelper.func_185205_a(itemstack))
            {
                return false;
            }
            else
            {
                for (int i = 0; i < 3; ++i)
                {
                    ItemStack itemstack1 = this.brewingItemStacks[i];

                    if (itemstack1 != null && PotionHelper.func_185208_a(itemstack1, itemstack))
                    {
                        return true;
                    }
                }

                return false;
            }
        }
        else
        {
            return false;
        }
    }

    private void brewPotions()
    {
        ItemStack itemstack = this.brewingItemStacks[3];

        for (int i = 0; i < 3; ++i)
        {
            this.brewingItemStacks[i] = PotionHelper.func_185212_d(itemstack, this.brewingItemStacks[i]);
        }

        --itemstack.stackSize;
        BlockPos blockpos = this.getPos();

        if (itemstack.getItem().hasContainerItem())
        {
            ItemStack itemstack1 = new ItemStack(itemstack.getItem().getContainerItem());

            if (itemstack.stackSize <= 0)
            {
                itemstack = itemstack1;
            }
            else
            {
                InventoryHelper.spawnItemStack(this.worldObj, (double)blockpos.getX(), (double)blockpos.getY(), (double)blockpos.getZ(), itemstack1);
            }
        }

        if (itemstack.stackSize <= 0)
        {
            itemstack = null;
        }

        this.brewingItemStacks[3] = itemstack;
        this.worldObj.playAuxSFX(1035, blockpos, 0);
    }

    public void readFromNBT(NBTTagCompound compound)
    {
        super.readFromNBT(compound);
        NBTTagList nbttaglist = compound.getTagList("Items", 10);
        this.brewingItemStacks = new ItemStack[this.getSizeInventory()];

        for (int i = 0; i < nbttaglist.tagCount(); ++i)
        {
            NBTTagCompound nbttagcompound = nbttaglist.getCompoundTagAt(i);
            int j = nbttagcompound.getByte("Slot");

            if (j >= 0 && j < this.brewingItemStacks.length)
            {
                this.brewingItemStacks[j] = ItemStack.loadItemStackFromNBT(nbttagcompound);
            }
        }

        this.brewTime = compound.getShort("BrewTime");

        if (compound.hasKey("CustomName", 8))
        {
            this.customName = compound.getString("CustomName");
        }

        this.fuel = compound.getByte("Fuel");
    }

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);
        compound.setShort("BrewTime", (short)this.brewTime);
        NBTTagList nbttaglist = new NBTTagList();

        for (int i = 0; i < this.brewingItemStacks.length; ++i)
        {
            if (this.brewingItemStacks[i] != null)
            {
                NBTTagCompound nbttagcompound = new NBTTagCompound();
                nbttagcompound.setByte("Slot", (byte)i);
                this.brewingItemStacks[i].writeToNBT(nbttagcompound);
                nbttaglist.appendTag(nbttagcompound);
            }
        }

        compound.setTag("Items", nbttaglist);

        if (this.hasCustomName())
        {
            compound.setString("CustomName", this.customName);
        }

        compound.setByte("Fuel", (byte)this.fuel);
    }

    /**
     * Returns the stack in the given slot.
     */
    public ItemStack getStackInSlot(int index)
    {
        return index >= 0 && index < this.brewingItemStacks.length ? this.brewingItemStacks[index] : null;
    }

    /**
     * Removes up to a specified number of items from an inventory slot and returns them in a new stack.
     */
    public ItemStack decrStackSize(int index, int count)
    {
        return ItemStackHelper.func_188382_a(this.brewingItemStacks, index, count);
    }

    /**
     * Removes a stack from the given slot and returns it.
     */
    public ItemStack removeStackFromSlot(int index)
    {
        return ItemStackHelper.func_188383_a(this.brewingItemStacks, index);
    }

    /**
     * Sets the given item stack to the specified slot in the inventory (can be crafting or armor sections).
     */
    public void setInventorySlotContents(int index, ItemStack stack)
    {
        if (index >= 0 && index < this.brewingItemStacks.length)
        {
            this.brewingItemStacks[index] = stack;
        }
    }

    /**
     * Returns the maximum stack size for a inventory slot. Seems to always be 64, possibly will be extended.
     */
    public int getInventoryStackLimit()
    {
        return 64;
    }

    /**
     * Do not make give this method the name canInteractWith because it clashes with Container
     */
    public boolean isUseableByPlayer(EntityPlayer player)
    {
        return this.worldObj.getTileEntity(this.pos) != this ? false : player.getDistanceSq((double)this.pos.getX() + 0.5D, (double)this.pos.getY() + 0.5D, (double)this.pos.getZ() + 0.5D) <= 64.0D;
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
        if (index == 3)
        {
            return PotionHelper.func_185205_a(stack);
        }
        else
        {
            Item item = stack.getItem();
            return index == 4 ? item == Items.blaze_powder : item == Items.potionitem || item == Items.splash_potion || item == Items.lingering_potion || item == Items.glass_bottle;
        }
    }

    public int[] getSlotsForFace(EnumFacing side)
    {
        return side == EnumFacing.UP ? inputSlots : (side == EnumFacing.DOWN ? field_184277_f : outputSlots);
    }

    /**
     * Returns true if automation can insert the given item in the given slot from the given side. Args: slot, item,
     * side
     */
    public boolean canInsertItem(int index, ItemStack itemStackIn, EnumFacing direction)
    {
        return this.isItemValidForSlot(index, itemStackIn);
    }

    /**
     * Returns true if automation can extract the given item in the given slot from the given side. Args: slot, item,
     * side
     */
    public boolean canExtractItem(int index, ItemStack stack, EnumFacing direction)
    {
        return index == 3 ? stack.getItem() == Items.glass_bottle : true;
    }

    public String getGuiID()
    {
        return "minecraft:brewing_stand";
    }

    public Container createContainer(InventoryPlayer playerInventory, EntityPlayer playerIn)
    {
        return new ContainerBrewingStand(playerInventory, this);
    }

    public int getField(int id)
    {
        switch (id)
        {
            case 0:
                return this.brewTime;

            case 1:
                return this.fuel;

            default:
                return 0;
        }
    }

    public void setField(int id, int value)
    {
        switch (id)
        {
            case 0:
                this.brewTime = value;
                break;

            case 1:
                this.fuel = value;
        }
    }

    public int getFieldCount()
    {
        return 2;
    }

    public void clear()
    {
        Arrays.fill(this.brewingItemStacks, (Object)null);
    }
}
