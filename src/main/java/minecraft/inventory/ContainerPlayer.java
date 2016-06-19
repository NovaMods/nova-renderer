package net.minecraft.inventory;

import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.item.ItemArmor;
import net.minecraft.item.ItemStack;
import net.minecraft.item.crafting.CraftingManager;

public class ContainerPlayer extends Container
{
    private static final EntityEquipmentSlot[] field_185003_h = new EntityEquipmentSlot[] {EntityEquipmentSlot.HEAD, EntityEquipmentSlot.CHEST, EntityEquipmentSlot.LEGS, EntityEquipmentSlot.FEET};

    /** The crafting matrix inventory. */
    public InventoryCrafting craftMatrix = new InventoryCrafting(this, 2, 2);
    public IInventory craftResult = new InventoryCraftResult();

    /** Determines if inventory manipulation should be handled. */
    public boolean isLocalWorld;
    private final EntityPlayer thePlayer;

    public ContainerPlayer(final InventoryPlayer playerInventory, boolean localWorld, EntityPlayer player)
    {
        this.isLocalWorld = localWorld;
        this.thePlayer = player;
        this.addSlotToContainer(new SlotCrafting(playerInventory.player, this.craftMatrix, this.craftResult, 0, 154, 28));

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                this.addSlotToContainer(new Slot(this.craftMatrix, j + i * 2, 98 + j * 18, 18 + i * 18));
            }
        }

        for (int k = 0; k < 4; ++k)
        {
            final EntityEquipmentSlot entityequipmentslot = field_185003_h[k];
            this.addSlotToContainer(new Slot(playerInventory, 36 + (3 - k), 8, 8 + k * 18)
            {
                public int getSlotStackLimit()
                {
                    return 1;
                }
                public boolean isItemValid(ItemStack stack)
                {
                    if (stack == null)
                    {
                        return false;
                    }
                    else
                    {
                        EntityEquipmentSlot entityequipmentslot1 = EntityLiving.func_184640_d(stack);
                        return entityequipmentslot1 == entityequipmentslot;
                    }
                }
                public String getSlotTexture()
                {
                    return ItemArmor.EMPTY_SLOT_NAMES[entityequipmentslot.func_188454_b()];
                }
            });
        }

        for (int l = 0; l < 3; ++l)
        {
            for (int j1 = 0; j1 < 9; ++j1)
            {
                this.addSlotToContainer(new Slot(playerInventory, j1 + (l + 1) * 9, 8 + j1 * 18, 84 + l * 18));
            }
        }

        for (int i1 = 0; i1 < 9; ++i1)
        {
            this.addSlotToContainer(new Slot(playerInventory, i1, 8 + i1 * 18, 142));
        }

        this.addSlotToContainer(new Slot(playerInventory, 40, 77, 62)
        {
            public boolean isItemValid(ItemStack stack)
            {
                return super.isItemValid(stack);
            }
            public String getSlotTexture()
            {
                return "minecraft:items/empty_armor_slot_shield";
            }
        });
        this.onCraftMatrixChanged(this.craftMatrix);
    }

    /**
     * Callback for when the crafting matrix is changed.
     */
    public void onCraftMatrixChanged(IInventory inventoryIn)
    {
        this.craftResult.setInventorySlotContents(0, CraftingManager.getInstance().findMatchingRecipe(this.craftMatrix, this.thePlayer.worldObj));
    }

    /**
     * Called when the container is closed.
     */
    public void onContainerClosed(EntityPlayer playerIn)
    {
        super.onContainerClosed(playerIn);

        for (int i = 0; i < 4; ++i)
        {
            ItemStack itemstack = this.craftMatrix.removeStackFromSlot(i);

            if (itemstack != null)
            {
                playerIn.dropPlayerItemWithRandomChoice(itemstack, false);
            }
        }

        this.craftResult.setInventorySlotContents(0, (ItemStack)null);
    }

    public boolean canInteractWith(EntityPlayer playerIn)
    {
        return true;
    }

    /**
     * Take a stack from the specified inventory slot.
     */
    public ItemStack transferStackInSlot(EntityPlayer playerIn, int index)
    {
        ItemStack itemstack = null;
        Slot slot = (Slot)this.inventorySlots.get(index);

        if (slot != null && slot.getHasStack())
        {
            ItemStack itemstack1 = slot.getStack();
            itemstack = itemstack1.copy();
            EntityEquipmentSlot entityequipmentslot = EntityLiving.func_184640_d(itemstack);

            if (index == 0)
            {
                if (!this.mergeItemStack(itemstack1, 9, 45, true))
                {
                    return null;
                }

                slot.onSlotChange(itemstack1, itemstack);
            }
            else if (index >= 1 && index < 5)
            {
                if (!this.mergeItemStack(itemstack1, 9, 45, false))
                {
                    return null;
                }
            }
            else if (index >= 5 && index < 9)
            {
                if (!this.mergeItemStack(itemstack1, 9, 45, false))
                {
                    return null;
                }
            }
            else if (entityequipmentslot.func_188453_a() == EntityEquipmentSlot.Type.ARMOR && !((Slot)this.inventorySlots.get(8 - entityequipmentslot.func_188454_b())).getHasStack())
            {
                int i = 8 - entityequipmentslot.func_188454_b();

                if (!this.mergeItemStack(itemstack1, i, i + 1, false))
                {
                    return null;
                }
            }
            else if (index >= 9 && index < 36)
            {
                if (!this.mergeItemStack(itemstack1, 36, 45, false))
                {
                    return null;
                }
            }
            else if (index >= 36 && index < 45)
            {
                if (!this.mergeItemStack(itemstack1, 9, 36, false))
                {
                    return null;
                }
            }
            else if (!this.mergeItemStack(itemstack1, 9, 45, false))
            {
                return null;
            }

            if (itemstack1.stackSize == 0)
            {
                slot.putStack((ItemStack)null);
            }
            else
            {
                slot.onSlotChanged();
            }

            if (itemstack1.stackSize == itemstack.stackSize)
            {
                return null;
            }

            slot.onPickupFromSlot(playerIn, itemstack1);
        }

        return itemstack;
    }

    /**
     * Called to determine if the current slot is valid for the stack merging (double-click) code. The stack passed in
     * is null for the initial slot that was double-clicked.
     */
    public boolean canMergeSlot(ItemStack stack, Slot slotIn)
    {
        return slotIn.inventory != this.craftResult && super.canMergeSlot(stack, slotIn);
    }
}
