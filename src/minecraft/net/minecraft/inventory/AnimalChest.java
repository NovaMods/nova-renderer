package net.minecraft.inventory;

import net.minecraft.util.text.ITextComponent;

public class AnimalChest extends InventoryBasic
{
    public AnimalChest(String inventoryName, int slotCount)
    {
        super(inventoryName, false, slotCount);
    }

    public AnimalChest(ITextComponent invTitle, int slotCount)
    {
        super(invTitle, slotCount);
    }
}
