package net.minecraft.client.renderer.entity;

import net.minecraft.client.renderer.RenderItem;
import net.minecraft.entity.projectile.EntityPotion;
import net.minecraft.init.Items;
import net.minecraft.item.ItemStack;

public class RenderPotion extends RenderSnowball<EntityPotion>
{
    public RenderPotion(RenderManager renderManagerIn, RenderItem itemRendererIn)
    {
        super(renderManagerIn, Items.potionitem, itemRendererIn);
    }

    public ItemStack func_177082_d(EntityPotion entityIn)
    {
        return entityIn.func_184543_l();
    }
}
