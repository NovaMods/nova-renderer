package net.minecraft.item;

import com.google.common.collect.Multimap;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemTool extends Item
{
    private Set<Block> effectiveBlocks;
    protected float efficiencyOnProperMaterial;

    /** Damage versus entities. */
    protected float damageVsEntity;
    protected float field_185065_c;

    /** The material this tool is made from. */
    protected Item.ToolMaterial toolMaterial;

    protected ItemTool(float p_i46745_1_, float p_i46745_2_, Item.ToolMaterial p_i46745_3_, Set<Block> p_i46745_4_)
    {
        this.efficiencyOnProperMaterial = 4.0F;
        this.toolMaterial = p_i46745_3_;
        this.effectiveBlocks = p_i46745_4_;
        this.maxStackSize = 1;
        this.setMaxDamage(p_i46745_3_.getMaxUses());
        this.efficiencyOnProperMaterial = p_i46745_3_.getEfficiencyOnProperMaterial();
        this.damageVsEntity = p_i46745_1_ + p_i46745_3_.getDamageVsEntity();
        this.field_185065_c = p_i46745_2_;
        this.setCreativeTab(CreativeTabs.tabTools);
    }

    protected ItemTool(Item.ToolMaterial p_i46746_1_, Set<Block> p_i46746_2_)
    {
        this(0.0F, 0.0F, p_i46746_1_, p_i46746_2_);
    }

    public float getStrVsBlock(ItemStack stack, IBlockState state)
    {
        return this.effectiveBlocks.contains(state.getBlock()) ? this.efficiencyOnProperMaterial : 1.0F;
    }

    /**
     * Current implementations of this method in child classes do not use the entry argument beside ev. They just raise
     * the damage on the stack.
     */
    public boolean hitEntity(ItemStack stack, EntityLivingBase target, EntityLivingBase attacker)
    {
        stack.damageItem(2, attacker);
        return true;
    }

    /**
     * Called when a Block is destroyed using this Item. Return true to trigger the "Use Item" statistic.
     */
    public boolean onBlockDestroyed(ItemStack stack, World worldIn, IBlockState blockIn, BlockPos pos, EntityLivingBase playerIn)
    {
        if ((double)blockIn.getBlockHardness(worldIn, pos) != 0.0D)
        {
            stack.damageItem(1, playerIn);
        }

        return true;
    }

    /**
     * Returns True is the item is renderer in full 3D when hold.
     */
    public boolean isFull3D()
    {
        return true;
    }

    public Item.ToolMaterial getToolMaterial()
    {
        return this.toolMaterial;
    }

    /**
     * Return the enchantability factor of the item, most of the time is based on material.
     */
    public int getItemEnchantability()
    {
        return this.toolMaterial.getEnchantability();
    }

    /**
     * Return the name for this tool's material.
     */
    public String getToolMaterialName()
    {
        return this.toolMaterial.toString();
    }

    /**
     * Return whether this item is repairable in an anvil.
     */
    public boolean getIsRepairable(ItemStack toRepair, ItemStack repair)
    {
        return this.toolMaterial.getRepairItem() == repair.getItem() ? true : super.getIsRepairable(toRepair, repair);
    }

    public Multimap<String, AttributeModifier> getItemAttributeModifiers(EntityEquipmentSlot p_111205_1_)
    {
        Multimap<String, AttributeModifier> multimap = super.getItemAttributeModifiers(p_111205_1_);

        if (p_111205_1_ == EntityEquipmentSlot.MAINHAND)
        {
            multimap.put(SharedMonsterAttributes.ATTACK_DAMAGE.getAttributeUnlocalizedName(), new AttributeModifier(itemModifierUUID, "Tool modifier", (double)this.damageVsEntity, 0));
            multimap.put(SharedMonsterAttributes.ATTACK_SPEED.getAttributeUnlocalizedName(), new AttributeModifier(field_185050_h, "Tool modifier", (double)this.field_185065_c, 0));
        }

        return multimap;
    }
}
