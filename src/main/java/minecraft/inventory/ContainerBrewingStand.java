package net.minecraft.inventory;

import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.init.Items;
import net.minecraft.init.PotionTypes;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.potion.PotionHelper;
import net.minecraft.potion.PotionUtils;
import net.minecraft.stats.AchievementList;

public class ContainerBrewingStand extends Container
{
    private IInventory tileBrewingStand;

    /** Instance of Slot. */
    private final Slot theSlot;
    private int field_184998_g;
    private int field_184999_h;

    public ContainerBrewingStand(InventoryPlayer playerInventory, IInventory tileBrewingStandIn)
    {
        this.tileBrewingStand = tileBrewingStandIn;
        this.addSlotToContainer(new ContainerBrewingStand.Potion(playerInventory.player, tileBrewingStandIn, 0, 56, 51));
        this.addSlotToContainer(new ContainerBrewingStand.Potion(playerInventory.player, tileBrewingStandIn, 1, 79, 58));
        this.addSlotToContainer(new ContainerBrewingStand.Potion(playerInventory.player, tileBrewingStandIn, 2, 102, 51));
        this.theSlot = this.addSlotToContainer(new ContainerBrewingStand.Ingredient(tileBrewingStandIn, 3, 79, 17));
        this.addSlotToContainer(new ContainerBrewingStand.Fuel(tileBrewingStandIn, 4, 17, 17));

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                this.addSlotToContainer(new Slot(playerInventory, j + i * 9 + 9, 8 + j * 18, 84 + i * 18));
            }
        }

        for (int k = 0; k < 9; ++k)
        {
            this.addSlotToContainer(new Slot(playerInventory, k, 8 + k * 18, 142));
        }
    }

    public void onCraftGuiOpened(ICrafting listener)
    {
        super.onCraftGuiOpened(listener);
        listener.sendAllWindowProperties(this, this.tileBrewingStand);
    }

    /**
     * Looks for changes made in the container, sends them to every listener.
     */
    public void detectAndSendChanges()
    {
        super.detectAndSendChanges();

        for (int i = 0; i < this.crafters.size(); ++i)
        {
            ICrafting icrafting = (ICrafting)this.crafters.get(i);

            if (this.field_184998_g != this.tileBrewingStand.getField(0))
            {
                icrafting.sendProgressBarUpdate(this, 0, this.tileBrewingStand.getField(0));
            }

            if (this.field_184999_h != this.tileBrewingStand.getField(1))
            {
                icrafting.sendProgressBarUpdate(this, 1, this.tileBrewingStand.getField(1));
            }
        }

        this.field_184998_g = this.tileBrewingStand.getField(0);
        this.field_184999_h = this.tileBrewingStand.getField(1);
    }

    public void updateProgressBar(int id, int data)
    {
        this.tileBrewingStand.setField(id, data);
    }

    public boolean canInteractWith(EntityPlayer playerIn)
    {
        return this.tileBrewingStand.isUseableByPlayer(playerIn);
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

            if ((index < 0 || index > 2) && index != 3 && index != 4)
            {
                if (!this.theSlot.getHasStack() && this.theSlot.isItemValid(itemstack1))
                {
                    if (!this.mergeItemStack(itemstack1, 3, 4, false))
                    {
                        return null;
                    }
                }
                else if (ContainerBrewingStand.Potion.canHoldPotion(itemstack))
                {
                    if (!this.mergeItemStack(itemstack1, 0, 3, false))
                    {
                        return null;
                    }
                }
                else if (ContainerBrewingStand.Fuel.func_185004_b_(itemstack))
                {
                    if (!this.mergeItemStack(itemstack1, 4, 5, false))
                    {
                        return null;
                    }
                }
                else if (index >= 5 && index < 32)
                {
                    if (!this.mergeItemStack(itemstack1, 32, 41, false))
                    {
                        return null;
                    }
                }
                else if (index >= 32 && index < 41)
                {
                    if (!this.mergeItemStack(itemstack1, 5, 32, false))
                    {
                        return null;
                    }
                }
                else if (!this.mergeItemStack(itemstack1, 5, 41, false))
                {
                    return null;
                }
            }
            else
            {
                if (!this.mergeItemStack(itemstack1, 5, 41, true))
                {
                    return null;
                }

                slot.onSlotChange(itemstack1, itemstack);
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

    static class Fuel extends Slot
    {
        public Fuel(IInventory p_i47070_1_, int p_i47070_2_, int p_i47070_3_, int p_i47070_4_)
        {
            super(p_i47070_1_, p_i47070_2_, p_i47070_3_, p_i47070_4_);
        }

        public boolean isItemValid(ItemStack stack)
        {
            return func_185004_b_(stack);
        }

        public static boolean func_185004_b_(ItemStack p_185004_0_)
        {
            return p_185004_0_ != null && p_185004_0_.getItem() == Items.blaze_powder;
        }

        public int getSlotStackLimit()
        {
            return 64;
        }
    }

    static class Ingredient extends Slot
    {
        public Ingredient(IInventory p_i47069_1_, int p_i47069_2_, int p_i47069_3_, int p_i47069_4_)
        {
            super(p_i47069_1_, p_i47069_2_, p_i47069_3_, p_i47069_4_);
        }

        public boolean isItemValid(ItemStack stack)
        {
            return stack != null && PotionHelper.func_185205_a(stack);
        }

        public int getSlotStackLimit()
        {
            return 64;
        }
    }

    static class Potion extends Slot
    {
        private EntityPlayer player;

        public Potion(EntityPlayer playerIn, IInventory inventoryIn, int index, int xPosition, int yPosition)
        {
            super(inventoryIn, index, xPosition, yPosition);
            this.player = playerIn;
        }

        public boolean isItemValid(ItemStack stack)
        {
            return canHoldPotion(stack);
        }

        public int getSlotStackLimit()
        {
            return 1;
        }

        public void onPickupFromSlot(EntityPlayer playerIn, ItemStack stack)
        {
            if (PotionUtils.func_185191_c(stack) != PotionTypes.water)
            {
                this.player.triggerAchievement(AchievementList.field_187970_B);
            }

            super.onPickupFromSlot(playerIn, stack);
        }

        public static boolean canHoldPotion(ItemStack stack)
        {
            if (stack == null)
            {
                return false;
            }
            else
            {
                Item item = stack.getItem();
                return item == Items.potionitem || item == Items.glass_bottle || item == Items.splash_potion || item == Items.lingering_potion;
            }
        }
    }
}
