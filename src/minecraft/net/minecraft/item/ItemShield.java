package net.minecraft.item;

import java.util.List;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.World;

public class ItemShield extends Item
{
    public ItemShield()
    {
        this.maxStackSize = 1;
        this.setCreativeTab(CreativeTabs.tabCombat);
        this.setMaxDamage(336);
        this.addPropertyOverride(new ResourceLocation("blocking"), new IItemPropertyGetter()
        {
            public float apply(ItemStack stack, World worldIn, EntityLivingBase entityIn)
            {
                return entityIn != null && entityIn.func_184587_cr() && entityIn.func_184607_cu() == stack ? 1.0F : 0.0F;
            }
        });
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        return super.onItemUse(stack, playerIn, worldIn, pos, side, hitX, hitY, hitZ, p_180614_9_);
    }

    public String getItemStackDisplayName(ItemStack stack)
    {
        if (stack.getSubCompound("BlockEntityTag", false) != null)
        {
            String s = "item.shield.";
            EnumDyeColor enumdyecolor = ItemBanner.getBaseColor(stack);
            s = s + enumdyecolor.getUnlocalizedName() + ".name";
            return I18n.translateToLocal(s);
        }
        else
        {
            return I18n.translateToLocal("item.shield.name");
        }
    }

    /**
     * allows items to add custom lines of information to the mouseover description
     */
    public void addInformation(ItemStack stack, EntityPlayer playerIn, List<String> tooltip, boolean advanced)
    {
        ItemBanner.func_185054_a(stack, tooltip);
    }

    /**
     * returns a list of items with the same ID, but different meta (eg: dye returns 16 items)
     */
    public void getSubItems(Item itemIn, CreativeTabs tab, List<ItemStack> subItems)
    {
        ItemStack itemstack = new ItemStack(itemIn, 1, 0);
        subItems.add(itemstack);
    }

    /**
     * gets the CreativeTab this item is displayed on
     */
    public CreativeTabs getCreativeTab()
    {
        return CreativeTabs.tabCombat;
    }

    /**
     * returns the action that specifies what animation to play when the items is being used
     */
    public EnumAction getItemUseAction(ItemStack stack)
    {
        return EnumAction.BLOCK;
    }

    /**
     * How long it takes to use or consume an item
     */
    public int getMaxItemUseDuration(ItemStack stack)
    {
        return 72000;
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        playerIn.func_184598_c(hand);
        return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
    }

    /**
     * Return whether this item is repairable in an anvil.
     */
    public boolean getIsRepairable(ItemStack toRepair, ItemStack repair)
    {
        return repair.getItem() == Item.getItemFromBlock(Blocks.planks) ? true : super.getIsRepairable(toRepair, repair);
    }
}
