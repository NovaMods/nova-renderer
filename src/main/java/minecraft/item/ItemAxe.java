package net.minecraft.item;

import com.google.common.collect.Sets;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;

public class ItemAxe extends ItemTool
{
    private static final Set<Block> EFFECTIVE_ON = Sets.newHashSet(new Block[] {Blocks.planks, Blocks.bookshelf, Blocks.log, Blocks.log2, Blocks.chest, Blocks.pumpkin, Blocks.lit_pumpkin, Blocks.melon_block, Blocks.ladder, Blocks.wooden_button, Blocks.wooden_pressure_plate});
    private static final float[] field_185066_m = new float[] {6.0F, 8.0F, 8.0F, 8.0F, 6.0F};
    private static final float[] field_185067_n = new float[] { -3.2F, -3.2F, -3.1F, -3.0F, -3.0F};

    protected ItemAxe(Item.ToolMaterial material)
    {
        super(material, EFFECTIVE_ON);
        this.damageVsEntity = field_185066_m[material.ordinal()];
        this.field_185065_c = field_185067_n[material.ordinal()];
    }

    public float getStrVsBlock(ItemStack stack, IBlockState state)
    {
        Material material = state.getMaterial();
        return material != Material.wood && material != Material.plants && material != Material.vine ? super.getStrVsBlock(stack, state) : this.efficiencyOnProperMaterial;
    }
}
