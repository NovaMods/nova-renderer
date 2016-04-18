package net.minecraft.inventory;

import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemHoe;
import net.minecraft.item.ItemPickaxe;
import net.minecraft.item.ItemStack;
import net.minecraft.item.ItemSword;
import net.minecraft.item.crafting.CraftingManager;
import net.minecraft.stats.AchievementList;

public class SlotCrafting extends Slot
{
    /** The craft matrix inventory linked to this result slot. */
    private final InventoryCrafting craftMatrix;

    /** The player that is using the GUI where this slot resides. */
    private final EntityPlayer thePlayer;

    /**
     * The number of items that have been crafted so far. Gets passed to ItemStack.onCrafting before being reset.
     */
    private int amountCrafted;

    public SlotCrafting(EntityPlayer player, InventoryCrafting craftingInventory, IInventory p_i45790_3_, int slotIndex, int xPosition, int yPosition)
    {
        super(p_i45790_3_, slotIndex, xPosition, yPosition);
        this.thePlayer = player;
        this.craftMatrix = craftingInventory;
    }

    /**
     * Check if the stack is a valid item for this slot. Always true beside for the armor slots.
     */
    public boolean isItemValid(ItemStack stack)
    {
        return false;
    }

    /**
     * Decrease the size of the stack in slot (first int arg) by the amount of the second int arg. Returns the new
     * stack.
     */
    public ItemStack decrStackSize(int amount)
    {
        if (this.getHasStack())
        {
            this.amountCrafted += Math.min(amount, this.getStack().stackSize);
        }

        return super.decrStackSize(amount);
    }

    /**
     * the itemStack passed in is the output - ie, iron ingots, and pickaxes, not ore and wood. Typically increases an
     * internal count then calls onCrafting(item).
     */
    protected void onCrafting(ItemStack stack, int amount)
    {
        this.amountCrafted += amount;
        this.onCrafting(stack);
    }

    /**
     * the itemStack passed in is the output - ie, iron ingots, and pickaxes, not ore and wood.
     */
    protected void onCrafting(ItemStack stack)
    {
        if (this.amountCrafted > 0)
        {
            stack.onCrafting(this.thePlayer.worldObj, this.thePlayer, this.amountCrafted);
        }

        this.amountCrafted = 0;

        if (stack.getItem() == Item.getItemFromBlock(Blocks.crafting_table))
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187984_h);
        }

        if (stack.getItem() instanceof ItemPickaxe)
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187985_i);
        }

        if (stack.getItem() == Item.getItemFromBlock(Blocks.furnace))
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187986_j);
        }

        if (stack.getItem() instanceof ItemHoe)
        {
            this.thePlayer.triggerAchievement(AchievementList.buildHoe);
        }

        if (stack.getItem() == Items.bread)
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187988_m);
        }

        if (stack.getItem() == Items.cake)
        {
            this.thePlayer.triggerAchievement(AchievementList.bakeCake);
        }

        if (stack.getItem() instanceof ItemPickaxe && ((ItemPickaxe)stack.getItem()).getToolMaterial() != Item.ToolMaterial.WOOD)
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187989_o);
        }

        if (stack.getItem() instanceof ItemSword)
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187991_r);
        }

        if (stack.getItem() == Item.getItemFromBlock(Blocks.enchanting_table))
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187972_E);
        }

        if (stack.getItem() == Item.getItemFromBlock(Blocks.bookshelf))
        {
            this.thePlayer.triggerAchievement(AchievementList.field_187974_G);
        }
    }

    public void onPickupFromSlot(EntityPlayer playerIn, ItemStack stack)
    {
        this.onCrafting(stack);
        ItemStack[] aitemstack = CraftingManager.getInstance().func_180303_b(this.craftMatrix, playerIn.worldObj);

        for (int i = 0; i < aitemstack.length; ++i)
        {
            ItemStack itemstack = this.craftMatrix.getStackInSlot(i);
            ItemStack itemstack1 = aitemstack[i];

            if (itemstack != null)
            {
                this.craftMatrix.decrStackSize(i, 1);
                itemstack = this.craftMatrix.getStackInSlot(i);
            }

            if (itemstack1 != null)
            {
                if (itemstack == null)
                {
                    this.craftMatrix.setInventorySlotContents(i, itemstack1);
                }
                else if (ItemStack.areItemsEqual(itemstack, itemstack1) && ItemStack.areItemStackTagsEqual(itemstack, itemstack1))
                {
                    itemstack1.stackSize += itemstack.stackSize;
                    this.craftMatrix.setInventorySlotContents(i, itemstack1);
                }
                else if (!this.thePlayer.inventory.addItemStackToInventory(itemstack1))
                {
                    this.thePlayer.dropPlayerItemWithRandomChoice(itemstack1, false);
                }
            }
        }
    }
}
