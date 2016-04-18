package net.minecraft.inventory;

import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import java.util.List;
import java.util.Set;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.math.MathHelper;

public abstract class Container
{
    public List<ItemStack> inventoryItemStacks = Lists.<ItemStack>newArrayList();
    public List<Slot> inventorySlots = Lists.<Slot>newArrayList();
    public int windowId;
    private short transactionID;

    /**
     * The current drag mode (0 : evenly split, 1 : one item by slot, 2 : not used ?)
     */
    private int dragMode = -1;

    /** The current drag event (0 : start, 1 : add slot : 2 : end) */
    private int dragEvent;
    private final Set<Slot> dragSlots = Sets.<Slot>newHashSet();
    protected List<ICrafting> crafters = Lists.<ICrafting>newArrayList();
    private Set<EntityPlayer> playerList = Sets.<EntityPlayer>newHashSet();

    /**
     * Adds an item slot to this container
     */
    protected Slot addSlotToContainer(Slot slotIn)
    {
        slotIn.slotNumber = this.inventorySlots.size();
        this.inventorySlots.add(slotIn);
        this.inventoryItemStacks.add((ItemStack)null);
        return slotIn;
    }

    public void onCraftGuiOpened(ICrafting listener)
    {
        if (this.crafters.contains(listener))
        {
            throw new IllegalArgumentException("Listener already listening");
        }
        else
        {
            this.crafters.add(listener);
            listener.updateCraftingInventory(this, this.getInventory());
            this.detectAndSendChanges();
        }
    }

    /**
     * Remove the given Listener. Method name is for legacy.
     */
    public void removeCraftingFromCrafters(ICrafting listeners)
    {
        this.crafters.remove(listeners);
    }

    public List<ItemStack> getInventory()
    {
        List<ItemStack> list = Lists.<ItemStack>newArrayList();

        for (int i = 0; i < this.inventorySlots.size(); ++i)
        {
            list.add(((Slot)this.inventorySlots.get(i)).getStack());
        }

        return list;
    }

    /**
     * Looks for changes made in the container, sends them to every listener.
     */
    public void detectAndSendChanges()
    {
        for (int i = 0; i < this.inventorySlots.size(); ++i)
        {
            ItemStack itemstack = ((Slot)this.inventorySlots.get(i)).getStack();
            ItemStack itemstack1 = (ItemStack)this.inventoryItemStacks.get(i);

            if (!ItemStack.areItemStacksEqual(itemstack1, itemstack))
            {
                itemstack1 = itemstack == null ? null : itemstack.copy();
                this.inventoryItemStacks.set(i, itemstack1);

                for (int j = 0; j < this.crafters.size(); ++j)
                {
                    ((ICrafting)this.crafters.get(j)).sendSlotContents(this, i, itemstack1);
                }
            }
        }
    }

    /**
     * Handles the given Button-click on the server, currently only used by enchanting. Name is for legacy.
     */
    public boolean enchantItem(EntityPlayer playerIn, int id)
    {
        return false;
    }

    public Slot getSlotFromInventory(IInventory inv, int slotIn)
    {
        for (int i = 0; i < this.inventorySlots.size(); ++i)
        {
            Slot slot = (Slot)this.inventorySlots.get(i);

            if (slot.isHere(inv, slotIn))
            {
                return slot;
            }
        }

        return null;
    }

    public Slot getSlot(int slotId)
    {
        return (Slot)this.inventorySlots.get(slotId);
    }

    /**
     * Take a stack from the specified inventory slot.
     */
    public ItemStack transferStackInSlot(EntityPlayer playerIn, int index)
    {
        Slot slot = (Slot)this.inventorySlots.get(index);
        return slot != null ? slot.getStack() : null;
    }

    public ItemStack func_184996_a(int p_184996_1_, int p_184996_2_, ClickType p_184996_3_, EntityPlayer p_184996_4_)
    {
        ItemStack itemstack = null;
        InventoryPlayer inventoryplayer = p_184996_4_.inventory;

        if (p_184996_3_ == ClickType.QUICK_CRAFT)
        {
            int i = this.dragEvent;
            this.dragEvent = getDragEvent(p_184996_2_);

            if ((i != 1 || this.dragEvent != 2) && i != this.dragEvent)
            {
                this.resetDrag();
            }
            else if (inventoryplayer.getItemStack() == null)
            {
                this.resetDrag();
            }
            else if (this.dragEvent == 0)
            {
                this.dragMode = extractDragMode(p_184996_2_);

                if (isValidDragMode(this.dragMode, p_184996_4_))
                {
                    this.dragEvent = 1;
                    this.dragSlots.clear();
                }
                else
                {
                    this.resetDrag();
                }
            }
            else if (this.dragEvent == 1)
            {
                Slot slot = (Slot)this.inventorySlots.get(p_184996_1_);

                if (slot != null && canAddItemToSlot(slot, inventoryplayer.getItemStack(), true) && slot.isItemValid(inventoryplayer.getItemStack()) && inventoryplayer.getItemStack().stackSize > this.dragSlots.size() && this.canDragIntoSlot(slot))
                {
                    this.dragSlots.add(slot);
                }
            }
            else if (this.dragEvent == 2)
            {
                if (!this.dragSlots.isEmpty())
                {
                    ItemStack itemstack3 = inventoryplayer.getItemStack().copy();
                    int j = inventoryplayer.getItemStack().stackSize;

                    for (Slot slot1 : this.dragSlots)
                    {
                        if (slot1 != null && canAddItemToSlot(slot1, inventoryplayer.getItemStack(), true) && slot1.isItemValid(inventoryplayer.getItemStack()) && inventoryplayer.getItemStack().stackSize >= this.dragSlots.size() && this.canDragIntoSlot(slot1))
                        {
                            ItemStack itemstack1 = itemstack3.copy();
                            int k = slot1.getHasStack() ? slot1.getStack().stackSize : 0;
                            computeStackSize(this.dragSlots, this.dragMode, itemstack1, k);

                            if (itemstack1.stackSize > itemstack1.getMaxStackSize())
                            {
                                itemstack1.stackSize = itemstack1.getMaxStackSize();
                            }

                            if (itemstack1.stackSize > slot1.getItemStackLimit(itemstack1))
                            {
                                itemstack1.stackSize = slot1.getItemStackLimit(itemstack1);
                            }

                            j -= itemstack1.stackSize - k;
                            slot1.putStack(itemstack1);
                        }
                    }

                    itemstack3.stackSize = j;

                    if (itemstack3.stackSize <= 0)
                    {
                        itemstack3 = null;
                    }

                    inventoryplayer.setItemStack(itemstack3);
                }

                this.resetDrag();
            }
            else
            {
                this.resetDrag();
            }
        }
        else if (this.dragEvent != 0)
        {
            this.resetDrag();
        }
        else if ((p_184996_3_ == ClickType.PICKUP || p_184996_3_ == ClickType.QUICK_MOVE) && (p_184996_2_ == 0 || p_184996_2_ == 1))
        {
            if (p_184996_1_ == -999)
            {
                if (inventoryplayer.getItemStack() != null)
                {
                    if (p_184996_2_ == 0)
                    {
                        p_184996_4_.dropPlayerItemWithRandomChoice(inventoryplayer.getItemStack(), true);
                        inventoryplayer.setItemStack((ItemStack)null);
                    }

                    if (p_184996_2_ == 1)
                    {
                        p_184996_4_.dropPlayerItemWithRandomChoice(inventoryplayer.getItemStack().splitStack(1), true);

                        if (inventoryplayer.getItemStack().stackSize == 0)
                        {
                            inventoryplayer.setItemStack((ItemStack)null);
                        }
                    }
                }
            }
            else if (p_184996_3_ == ClickType.QUICK_MOVE)
            {
                if (p_184996_1_ < 0)
                {
                    return null;
                }

                Slot slot6 = (Slot)this.inventorySlots.get(p_184996_1_);

                if (slot6 != null && slot6.canTakeStack(p_184996_4_))
                {
                    ItemStack itemstack8 = slot6.getStack();

                    if (itemstack8 != null && itemstack8.stackSize <= 0)
                    {
                        itemstack = itemstack8.copy();
                        slot6.putStack((ItemStack)null);
                    }

                    ItemStack itemstack11 = this.transferStackInSlot(p_184996_4_, p_184996_1_);

                    if (itemstack11 != null)
                    {
                        Item item = itemstack11.getItem();
                        itemstack = itemstack11.copy();

                        if (slot6.getStack() != null && slot6.getStack().getItem() == item)
                        {
                            this.retrySlotClick(p_184996_1_, p_184996_2_, true, p_184996_4_);
                        }
                    }
                }
            }
            else
            {
                if (p_184996_1_ < 0)
                {
                    return null;
                }

                Slot slot7 = (Slot)this.inventorySlots.get(p_184996_1_);

                if (slot7 != null)
                {
                    ItemStack itemstack9 = slot7.getStack();
                    ItemStack itemstack12 = inventoryplayer.getItemStack();

                    if (itemstack9 != null)
                    {
                        itemstack = itemstack9.copy();
                    }

                    if (itemstack9 == null)
                    {
                        if (itemstack12 != null && slot7.isItemValid(itemstack12))
                        {
                            int l2 = p_184996_2_ == 0 ? itemstack12.stackSize : 1;

                            if (l2 > slot7.getItemStackLimit(itemstack12))
                            {
                                l2 = slot7.getItemStackLimit(itemstack12);
                            }

                            slot7.putStack(itemstack12.splitStack(l2));

                            if (itemstack12.stackSize == 0)
                            {
                                inventoryplayer.setItemStack((ItemStack)null);
                            }
                        }
                    }
                    else if (slot7.canTakeStack(p_184996_4_))
                    {
                        if (itemstack12 == null)
                        {
                            if (itemstack9.stackSize > 0)
                            {
                                int k2 = p_184996_2_ == 0 ? itemstack9.stackSize : (itemstack9.stackSize + 1) / 2;
                                inventoryplayer.setItemStack(slot7.decrStackSize(k2));

                                if (itemstack9.stackSize <= 0)
                                {
                                    slot7.putStack((ItemStack)null);
                                }

                                slot7.onPickupFromSlot(p_184996_4_, inventoryplayer.getItemStack());
                            }
                            else
                            {
                                slot7.putStack((ItemStack)null);
                                inventoryplayer.setItemStack((ItemStack)null);
                            }
                        }
                        else if (slot7.isItemValid(itemstack12))
                        {
                            if (itemstack9.getItem() == itemstack12.getItem() && itemstack9.getMetadata() == itemstack12.getMetadata() && ItemStack.areItemStackTagsEqual(itemstack9, itemstack12))
                            {
                                int j2 = p_184996_2_ == 0 ? itemstack12.stackSize : 1;

                                if (j2 > slot7.getItemStackLimit(itemstack12) - itemstack9.stackSize)
                                {
                                    j2 = slot7.getItemStackLimit(itemstack12) - itemstack9.stackSize;
                                }

                                if (j2 > itemstack12.getMaxStackSize() - itemstack9.stackSize)
                                {
                                    j2 = itemstack12.getMaxStackSize() - itemstack9.stackSize;
                                }

                                itemstack12.splitStack(j2);

                                if (itemstack12.stackSize == 0)
                                {
                                    inventoryplayer.setItemStack((ItemStack)null);
                                }

                                itemstack9.stackSize += j2;
                            }
                            else if (itemstack12.stackSize <= slot7.getItemStackLimit(itemstack12))
                            {
                                slot7.putStack(itemstack12);
                                inventoryplayer.setItemStack(itemstack9);
                            }
                        }
                        else if (itemstack9.getItem() == itemstack12.getItem() && itemstack12.getMaxStackSize() > 1 && (!itemstack9.getHasSubtypes() || itemstack9.getMetadata() == itemstack12.getMetadata()) && ItemStack.areItemStackTagsEqual(itemstack9, itemstack12))
                        {
                            int i2 = itemstack9.stackSize;

                            if (i2 > 0 && i2 + itemstack12.stackSize <= itemstack12.getMaxStackSize())
                            {
                                itemstack12.stackSize += i2;
                                itemstack9 = slot7.decrStackSize(i2);

                                if (itemstack9.stackSize == 0)
                                {
                                    slot7.putStack((ItemStack)null);
                                }

                                slot7.onPickupFromSlot(p_184996_4_, inventoryplayer.getItemStack());
                            }
                        }
                    }

                    slot7.onSlotChanged();
                }
            }
        }
        else if (p_184996_3_ == ClickType.SWAP && p_184996_2_ >= 0 && p_184996_2_ < 9)
        {
            Slot slot5 = (Slot)this.inventorySlots.get(p_184996_1_);
            ItemStack itemstack7 = inventoryplayer.getStackInSlot(p_184996_2_);

            if (itemstack7 != null && itemstack7.stackSize <= 0)
            {
                itemstack7 = null;
                inventoryplayer.setInventorySlotContents(p_184996_2_, (ItemStack)null);
            }

            ItemStack itemstack10 = slot5.getStack();

            if (itemstack7 != null || itemstack10 != null)
            {
                if (itemstack7 == null)
                {
                    if (slot5.canTakeStack(p_184996_4_))
                    {
                        inventoryplayer.setInventorySlotContents(p_184996_2_, itemstack10);
                        slot5.putStack((ItemStack)null);
                        slot5.onPickupFromSlot(p_184996_4_, itemstack10);
                    }
                }
                else if (itemstack10 == null)
                {
                    if (slot5.isItemValid(itemstack7))
                    {
                        int k1 = slot5.getItemStackLimit(itemstack7);

                        if (itemstack7.stackSize > k1)
                        {
                            slot5.putStack(itemstack7.splitStack(k1));
                        }
                        else
                        {
                            slot5.putStack(itemstack7);
                            inventoryplayer.setInventorySlotContents(p_184996_2_, (ItemStack)null);
                        }
                    }
                }
                else if (slot5.canTakeStack(p_184996_4_) && slot5.isItemValid(itemstack7))
                {
                    int l1 = slot5.getItemStackLimit(itemstack7);

                    if (itemstack7.stackSize > l1)
                    {
                        slot5.putStack(itemstack7.splitStack(l1));
                        slot5.onPickupFromSlot(p_184996_4_, itemstack10);

                        if (!inventoryplayer.addItemStackToInventory(itemstack10))
                        {
                            p_184996_4_.dropPlayerItemWithRandomChoice(itemstack10, true);
                        }
                    }
                    else
                    {
                        slot5.putStack(itemstack7);
                        inventoryplayer.setInventorySlotContents(p_184996_2_, itemstack10);
                        slot5.onPickupFromSlot(p_184996_4_, itemstack10);
                    }
                }
            }
        }
        else if (p_184996_3_ == ClickType.CLONE && p_184996_4_.capabilities.isCreativeMode && inventoryplayer.getItemStack() == null && p_184996_1_ >= 0)
        {
            Slot slot4 = (Slot)this.inventorySlots.get(p_184996_1_);

            if (slot4 != null && slot4.getHasStack())
            {
                if (slot4.getStack().stackSize > 0)
                {
                    ItemStack itemstack6 = slot4.getStack().copy();
                    itemstack6.stackSize = itemstack6.getMaxStackSize();
                    inventoryplayer.setItemStack(itemstack6);
                }
                else
                {
                    slot4.putStack((ItemStack)null);
                }
            }
        }
        else if (p_184996_3_ == ClickType.THROW && inventoryplayer.getItemStack() == null && p_184996_1_ >= 0)
        {
            Slot slot3 = (Slot)this.inventorySlots.get(p_184996_1_);

            if (slot3 != null && slot3.getHasStack() && slot3.canTakeStack(p_184996_4_))
            {
                ItemStack itemstack5 = slot3.decrStackSize(p_184996_2_ == 0 ? 1 : slot3.getStack().stackSize);
                slot3.onPickupFromSlot(p_184996_4_, itemstack5);
                p_184996_4_.dropPlayerItemWithRandomChoice(itemstack5, true);
            }
        }
        else if (p_184996_3_ == ClickType.PICKUP_ALL && p_184996_1_ >= 0)
        {
            Slot slot2 = (Slot)this.inventorySlots.get(p_184996_1_);
            ItemStack itemstack4 = inventoryplayer.getItemStack();

            if (itemstack4 != null && (slot2 == null || !slot2.getHasStack() || !slot2.canTakeStack(p_184996_4_)))
            {
                int i1 = p_184996_2_ == 0 ? 0 : this.inventorySlots.size() - 1;
                int j1 = p_184996_2_ == 0 ? 1 : -1;

                for (int i3 = 0; i3 < 2; ++i3)
                {
                    for (int j3 = i1; j3 >= 0 && j3 < this.inventorySlots.size() && itemstack4.stackSize < itemstack4.getMaxStackSize(); j3 += j1)
                    {
                        Slot slot8 = (Slot)this.inventorySlots.get(j3);

                        if (slot8.getHasStack() && canAddItemToSlot(slot8, itemstack4, true) && slot8.canTakeStack(p_184996_4_) && this.canMergeSlot(itemstack4, slot8) && (i3 != 0 || slot8.getStack().stackSize != slot8.getStack().getMaxStackSize()))
                        {
                            int l = Math.min(itemstack4.getMaxStackSize() - itemstack4.stackSize, slot8.getStack().stackSize);
                            ItemStack itemstack2 = slot8.decrStackSize(l);
                            itemstack4.stackSize += l;

                            if (itemstack2.stackSize <= 0)
                            {
                                slot8.putStack((ItemStack)null);
                            }

                            slot8.onPickupFromSlot(p_184996_4_, itemstack2);
                        }
                    }
                }
            }

            this.detectAndSendChanges();
        }

        return itemstack;
    }

    /**
     * Called to determine if the current slot is valid for the stack merging (double-click) code. The stack passed in
     * is null for the initial slot that was double-clicked.
     */
    public boolean canMergeSlot(ItemStack stack, Slot slotIn)
    {
        return true;
    }

    /**
     * Retries slotClick() in case of failure
     */
    protected void retrySlotClick(int slotId, int clickedButton, boolean mode, EntityPlayer playerIn)
    {
        this.func_184996_a(slotId, clickedButton, ClickType.QUICK_MOVE, playerIn);
    }

    /**
     * Called when the container is closed.
     */
    public void onContainerClosed(EntityPlayer playerIn)
    {
        InventoryPlayer inventoryplayer = playerIn.inventory;

        if (inventoryplayer.getItemStack() != null)
        {
            playerIn.dropPlayerItemWithRandomChoice(inventoryplayer.getItemStack(), false);
            inventoryplayer.setItemStack((ItemStack)null);
        }
    }

    /**
     * Callback for when the crafting matrix is changed.
     */
    public void onCraftMatrixChanged(IInventory inventoryIn)
    {
        this.detectAndSendChanges();
    }

    /**
     * args: slotID, itemStack to put in slot
     */
    public void putStackInSlot(int slotID, ItemStack stack)
    {
        this.getSlot(slotID).putStack(stack);
    }

    /**
     * places itemstacks in first x slots, x being aitemstack.lenght
     */
    public void putStacksInSlots(ItemStack[] p_75131_1_)
    {
        for (int i = 0; i < p_75131_1_.length; ++i)
        {
            this.getSlot(i).putStack(p_75131_1_[i]);
        }
    }

    public void updateProgressBar(int id, int data)
    {
    }

    /**
     * Gets a unique transaction ID. Parameter is unused.
     */
    public short getNextTransactionID(InventoryPlayer p_75136_1_)
    {
        ++this.transactionID;
        return this.transactionID;
    }

    /**
     * gets whether or not the player can craft in this inventory or not
     */
    public boolean getCanCraft(EntityPlayer p_75129_1_)
    {
        return !this.playerList.contains(p_75129_1_);
    }

    /**
     * sets whether the player can craft in this inventory or not
     */
    public void setCanCraft(EntityPlayer p_75128_1_, boolean p_75128_2_)
    {
        if (p_75128_2_)
        {
            this.playerList.remove(p_75128_1_);
        }
        else
        {
            this.playerList.add(p_75128_1_);
        }
    }

    public abstract boolean canInteractWith(EntityPlayer playerIn);

    /**
     * Merges provided ItemStack with the first avaliable one in the container/player inventor between minIndex
     * (included) and maxIndex (excluded). Args : stack, minIndex, maxIndex, negativDirection. /!\ the Container
     * implementation do not check if the item is valid for the slot
     */
    protected boolean mergeItemStack(ItemStack stack, int startIndex, int endIndex, boolean reverseDirection)
    {
        boolean flag = false;
        int i = startIndex;

        if (reverseDirection)
        {
            i = endIndex - 1;
        }

        if (stack.isStackable())
        {
            while (stack.stackSize > 0 && (!reverseDirection && i < endIndex || reverseDirection && i >= startIndex))
            {
                Slot slot = (Slot)this.inventorySlots.get(i);
                ItemStack itemstack = slot.getStack();

                if (itemstack != null && func_184997_a(stack, itemstack))
                {
                    int j = itemstack.stackSize + stack.stackSize;

                    if (j <= stack.getMaxStackSize())
                    {
                        stack.stackSize = 0;
                        itemstack.stackSize = j;
                        slot.onSlotChanged();
                        flag = true;
                    }
                    else if (itemstack.stackSize < stack.getMaxStackSize())
                    {
                        stack.stackSize -= stack.getMaxStackSize() - itemstack.stackSize;
                        itemstack.stackSize = stack.getMaxStackSize();
                        slot.onSlotChanged();
                        flag = true;
                    }
                }

                if (reverseDirection)
                {
                    --i;
                }
                else
                {
                    ++i;
                }
            }
        }

        if (stack.stackSize > 0)
        {
            if (reverseDirection)
            {
                i = endIndex - 1;
            }
            else
            {
                i = startIndex;
            }

            while (!reverseDirection && i < endIndex || reverseDirection && i >= startIndex)
            {
                Slot slot1 = (Slot)this.inventorySlots.get(i);
                ItemStack itemstack1 = slot1.getStack();

                if (itemstack1 == null)
                {
                    slot1.putStack(stack.copy());
                    slot1.onSlotChanged();
                    stack.stackSize = 0;
                    flag = true;
                    break;
                }

                if (reverseDirection)
                {
                    --i;
                }
                else
                {
                    ++i;
                }
            }
        }

        return flag;
    }

    private static boolean func_184997_a(ItemStack p_184997_0_, ItemStack p_184997_1_)
    {
        return p_184997_1_.getItem() == p_184997_0_.getItem() && (!p_184997_0_.getHasSubtypes() || p_184997_0_.getMetadata() == p_184997_1_.getMetadata()) && ItemStack.areItemStackTagsEqual(p_184997_0_, p_184997_1_);
    }

    /**
     * Extracts the drag mode. Args : eventButton. Return (0 : evenly split, 1 : one item by slot, 2 : not used ?)
     */
    public static int extractDragMode(int p_94529_0_)
    {
        return p_94529_0_ >> 2 & 3;
    }

    /**
     * Args : clickedButton, Returns (0 : start drag, 1 : add slot, 2 : end drag)
     */
    public static int getDragEvent(int p_94532_0_)
    {
        return p_94532_0_ & 3;
    }

    public static int func_94534_d(int p_94534_0_, int p_94534_1_)
    {
        return p_94534_0_ & 3 | (p_94534_1_ & 3) << 2;
    }

    public static boolean isValidDragMode(int dragModeIn, EntityPlayer player)
    {
        return dragModeIn == 0 ? true : (dragModeIn == 1 ? true : dragModeIn == 2 && player.capabilities.isCreativeMode);
    }

    /**
     * Reset the drag fields
     */
    protected void resetDrag()
    {
        this.dragEvent = 0;
        this.dragSlots.clear();
    }

    /**
     * Checks if it's possible to add the given itemstack to the given slot.
     */
    public static boolean canAddItemToSlot(Slot slotIn, ItemStack stack, boolean stackSizeMatters)
    {
        boolean flag = slotIn == null || !slotIn.getHasStack();

        if (slotIn != null && slotIn.getHasStack() && stack != null && stack.isItemEqual(slotIn.getStack()) && ItemStack.areItemStackTagsEqual(slotIn.getStack(), stack))
        {
            flag |= slotIn.getStack().stackSize + (stackSizeMatters ? 0 : stack.stackSize) <= stack.getMaxStackSize();
        }

        return flag;
    }

    /**
     * Compute the new stack size, Returns the stack with the new size. Args : dragSlots, dragMode, dragStack,
     * slotStackSize
     */
    public static void computeStackSize(Set<Slot> p_94525_0_, int p_94525_1_, ItemStack p_94525_2_, int p_94525_3_)
    {
        switch (p_94525_1_)
        {
            case 0:
                p_94525_2_.stackSize = MathHelper.floor_float((float)p_94525_2_.stackSize / (float)p_94525_0_.size());
                break;

            case 1:
                p_94525_2_.stackSize = 1;
                break;

            case 2:
                p_94525_2_.stackSize = p_94525_2_.getItem().getItemStackLimit();
        }

        p_94525_2_.stackSize += p_94525_3_;
    }

    /**
     * Returns true if the player can "drag-spilt" items into this slot,. returns true by default. Called to check if
     * the slot can be added to a list of Slots to split the held ItemStack across.
     */
    public boolean canDragIntoSlot(Slot p_94531_1_)
    {
        return true;
    }

    /**
     * Like the version that takes an inventory. If the given TileEntity is not an Inventory, 0 is returned instead.
     */
    public static int calcRedstone(TileEntity te)
    {
        return te instanceof IInventory ? calcRedstoneFromInventory((IInventory)te) : 0;
    }

    public static int calcRedstoneFromInventory(IInventory inv)
    {
        if (inv == null)
        {
            return 0;
        }
        else
        {
            int i = 0;
            float f = 0.0F;

            for (int j = 0; j < inv.getSizeInventory(); ++j)
            {
                ItemStack itemstack = inv.getStackInSlot(j);

                if (itemstack != null)
                {
                    f += (float)itemstack.stackSize / (float)Math.min(inv.getInventoryStackLimit(), itemstack.getMaxStackSize());
                    ++i;
                }
            }

            f = f / (float)inv.getSizeInventory();
            return MathHelper.floor_float(f * 14.0F) + (i > 0 ? 1 : 0);
        }
    }
}
