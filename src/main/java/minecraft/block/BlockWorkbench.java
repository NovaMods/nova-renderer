package net.minecraft.block;

import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.ContainerWorkbench;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.world.IInteractionObject;
import net.minecraft.world.World;

public class BlockWorkbench extends Block
{
    protected BlockWorkbench()
    {
        super(Material.wood);
        this.setCreativeTab(CreativeTabs.tabDecorations);
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (worldIn.isRemote)
        {
            return true;
        }
        else
        {
            playerIn.displayGui(new BlockWorkbench.InterfaceCraftingTable(worldIn, pos));
            playerIn.triggerAchievement(StatList.field_188062_ab);
            return true;
        }
    }

    public static class InterfaceCraftingTable implements IInteractionObject
    {
        private final World world;
        private final BlockPos position;

        public InterfaceCraftingTable(World worldIn, BlockPos pos)
        {
            this.world = worldIn;
            this.position = pos;
        }

        public String getName()
        {
            return null;
        }

        public boolean hasCustomName()
        {
            return false;
        }

        public ITextComponent getDisplayName()
        {
            return new TextComponentTranslation(Blocks.crafting_table.getUnlocalizedName() + ".name", new Object[0]);
        }

        public Container createContainer(InventoryPlayer playerInventory, EntityPlayer playerIn)
        {
            return new ContainerWorkbench(playerInventory, this.world, this.position);
        }

        public String getGuiID()
        {
            return "minecraft:crafting_table";
        }
    }
}
