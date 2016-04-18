package net.minecraft.item;

import com.google.common.collect.Sets;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;

public class ItemPickaxe extends ItemTool
{
    private static final Set<Block> EFFECTIVE_ON = Sets.newHashSet(new Block[] {Blocks.activator_rail, Blocks.coal_ore, Blocks.cobblestone, Blocks.detector_rail, Blocks.diamond_block, Blocks.diamond_ore, Blocks.double_stone_slab, Blocks.golden_rail, Blocks.gold_block, Blocks.gold_ore, Blocks.ice, Blocks.iron_block, Blocks.iron_ore, Blocks.lapis_block, Blocks.lapis_ore, Blocks.lit_redstone_ore, Blocks.mossy_cobblestone, Blocks.netherrack, Blocks.packed_ice, Blocks.rail, Blocks.redstone_ore, Blocks.sandstone, Blocks.red_sandstone, Blocks.stone, Blocks.stone_slab, Blocks.stone_button, Blocks.stone_pressure_plate});

    protected ItemPickaxe(Item.ToolMaterial material)
    {
        super(1.0F, -2.8F, material, EFFECTIVE_ON);
    }

    /**
     * Check whether this Item can harvest the given Block
     */
    public boolean canHarvestBlock(IBlockState blockIn)
    {
        Block block = blockIn.getBlock();

        if (block == Blocks.obsidian)
        {
            return this.toolMaterial.getHarvestLevel() == 3;
        }
        else if (block != Blocks.diamond_block && block != Blocks.diamond_ore)
        {
            if (block != Blocks.emerald_ore && block != Blocks.emerald_block)
            {
                if (block != Blocks.gold_block && block != Blocks.gold_ore)
                {
                    if (block != Blocks.iron_block && block != Blocks.iron_ore)
                    {
                        if (block != Blocks.lapis_block && block != Blocks.lapis_ore)
                        {
                            if (block != Blocks.redstone_ore && block != Blocks.lit_redstone_ore)
                            {
                                Material material = blockIn.getMaterial();
                                return material == Material.rock ? true : (material == Material.iron ? true : material == Material.anvil);
                            }
                            else
                            {
                                return this.toolMaterial.getHarvestLevel() >= 2;
                            }
                        }
                        else
                        {
                            return this.toolMaterial.getHarvestLevel() >= 1;
                        }
                    }
                    else
                    {
                        return this.toolMaterial.getHarvestLevel() >= 1;
                    }
                }
                else
                {
                    return this.toolMaterial.getHarvestLevel() >= 2;
                }
            }
            else
            {
                return this.toolMaterial.getHarvestLevel() >= 2;
            }
        }
        else
        {
            return this.toolMaterial.getHarvestLevel() >= 2;
        }
    }

    public float getStrVsBlock(ItemStack stack, IBlockState state)
    {
        Material material = state.getMaterial();
        return material != Material.iron && material != Material.anvil && material != Material.rock ? super.getStrVsBlock(stack, state) : this.efficiencyOnProperMaterial;
    }
}
