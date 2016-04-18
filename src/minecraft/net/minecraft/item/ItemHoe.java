package net.minecraft.item;

import com.google.common.collect.Multimap;
import net.minecraft.block.Block;
import net.minecraft.block.BlockDirt;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemHoe extends Item
{
    private final float field_185072_b;
    protected Item.ToolMaterial theToolMaterial;

    public ItemHoe(Item.ToolMaterial material)
    {
        this.theToolMaterial = material;
        this.maxStackSize = 1;
        this.setMaxDamage(material.getMaxUses());
        this.setCreativeTab(CreativeTabs.tabTools);
        this.field_185072_b = material.getDamageVsEntity() + 1.0F;
    }

    @SuppressWarnings("incomplete-switch")

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        if (!playerIn.canPlayerEdit(pos.offset(hitX), hitX, stack))
        {
            return EnumActionResult.FAIL;
        }
        else
        {
            IBlockState iblockstate = worldIn.getBlockState(pos);
            Block block = iblockstate.getBlock();

            if (hitX != EnumFacing.DOWN && worldIn.getBlockState(pos.up()).getMaterial() == Material.air)
            {
                if (block == Blocks.grass || block == Blocks.grass_path)
                {
                    this.func_185071_a(stack, playerIn, worldIn, pos, Blocks.farmland.getDefaultState());
                    return EnumActionResult.SUCCESS;
                }

                if (block == Blocks.dirt)
                {
                    switch ((BlockDirt.DirtType)iblockstate.getValue(BlockDirt.VARIANT))
                    {
                        case DIRT:
                            this.func_185071_a(stack, playerIn, worldIn, pos, Blocks.farmland.getDefaultState());
                            return EnumActionResult.SUCCESS;

                        case COARSE_DIRT:
                            this.func_185071_a(stack, playerIn, worldIn, pos, Blocks.dirt.getDefaultState().withProperty(BlockDirt.VARIANT, BlockDirt.DirtType.DIRT));
                            return EnumActionResult.SUCCESS;
                    }
                }
            }

            return EnumActionResult.PASS;
        }
    }

    /**
     * Current implementations of this method in child classes do not use the entry argument beside ev. They just raise
     * the damage on the stack.
     */
    public boolean hitEntity(ItemStack stack, EntityLivingBase target, EntityLivingBase attacker)
    {
        stack.damageItem(1, attacker);
        return true;
    }

    protected void func_185071_a(ItemStack p_185071_1_, EntityPlayer p_185071_2_, World p_185071_3_, BlockPos p_185071_4_, IBlockState p_185071_5_)
    {
        p_185071_3_.func_184133_a(p_185071_2_, p_185071_4_, SoundEvents.item_hoe_till, SoundCategory.BLOCKS, 1.0F, 1.0F);

        if (!p_185071_3_.isRemote)
        {
            p_185071_3_.setBlockState(p_185071_4_, p_185071_5_, 11);
            p_185071_1_.damageItem(1, p_185071_2_);
        }
    }

    /**
     * Returns True is the item is renderer in full 3D when hold.
     */
    public boolean isFull3D()
    {
        return true;
    }

    /**
     * Returns the name of the material this tool is made from as it is declared in EnumToolMaterial (meaning diamond
     * would return "EMERALD")
     */
    public String getMaterialName()
    {
        return this.theToolMaterial.toString();
    }

    public Multimap<String, AttributeModifier> getItemAttributeModifiers(EntityEquipmentSlot p_111205_1_)
    {
        Multimap<String, AttributeModifier> multimap = super.getItemAttributeModifiers(p_111205_1_);

        if (p_111205_1_ == EntityEquipmentSlot.MAINHAND)
        {
            multimap.put(SharedMonsterAttributes.ATTACK_DAMAGE.getAttributeUnlocalizedName(), new AttributeModifier(itemModifierUUID, "Weapon modifier", 0.0D, 0));
            multimap.put(SharedMonsterAttributes.ATTACK_SPEED.getAttributeUnlocalizedName(), new AttributeModifier(field_185050_h, "Weapon modifier", (double)(this.field_185072_b - 4.0F), 0));
        }

        return multimap;
    }
}
