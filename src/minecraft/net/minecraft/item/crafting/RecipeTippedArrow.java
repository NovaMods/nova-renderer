package net.minecraft.item.crafting;

import net.minecraft.init.Items;
import net.minecraft.inventory.InventoryCrafting;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.potion.PotionUtils;
import net.minecraft.world.World;

class RecipeTippedArrow implements IRecipe
{
    private static final ItemStack[] field_185255_a = new ItemStack[9];

    /**
     * Used to check if a recipe matches current crafting inventory
     */
    public boolean matches(InventoryCrafting inv, World worldIn)
    {
        if (inv.getWidth() == 3 && inv.getHeight() == 3)
        {
            for (int i = 0; i < inv.getWidth(); ++i)
            {
                for (int j = 0; j < inv.getHeight(); ++j)
                {
                    ItemStack itemstack = inv.getStackInRowAndColumn(i, j);

                    if (itemstack == null)
                    {
                        return false;
                    }

                    Item item = itemstack.getItem();

                    if (i == 1 && j == 1)
                    {
                        if (item != Items.lingering_potion)
                        {
                            return false;
                        }
                    }
                    else if (item != Items.arrow)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * Returns an Item that is the result of this recipe
     */
    public ItemStack getCraftingResult(InventoryCrafting inv)
    {
        ItemStack itemstack = inv.getStackInRowAndColumn(1, 1);

        if (itemstack != null && itemstack.getItem() == Items.lingering_potion)
        {
            ItemStack itemstack1 = new ItemStack(Items.tipped_arrow, 8);
            PotionUtils.func_185188_a(itemstack1, PotionUtils.func_185191_c(itemstack));
            PotionUtils.func_185184_a(itemstack1, PotionUtils.func_185190_b(itemstack));
            return itemstack1;
        }
        else
        {
            return null;
        }
    }

    /**
     * Returns the size of the recipe area
     */
    public int getRecipeSize()
    {
        return 9;
    }

    public ItemStack getRecipeOutput()
    {
        return null;
    }

    public ItemStack[] getRemainingItems(InventoryCrafting inv)
    {
        return field_185255_a;
    }
}
