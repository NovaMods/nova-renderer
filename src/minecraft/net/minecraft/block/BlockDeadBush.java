package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockDeadBush extends BlockBush
{
    protected static final AxisAlignedBB field_185516_a = new AxisAlignedBB(0.09999999403953552D, 0.0D, 0.09999999403953552D, 0.8999999761581421D, 0.800000011920929D, 0.8999999761581421D);

    protected BlockDeadBush()
    {
        super(Material.vine);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185516_a;
    }

    /**
     * Get the MapColor for this Block and the given BlockState
     */
    public MapColor getMapColor(IBlockState state)
    {
        return MapColor.woodColor;
    }

    protected boolean func_185514_i(IBlockState p_185514_1_)
    {
        return p_185514_1_.getBlock() == Blocks.sand || p_185514_1_.getBlock() == Blocks.hardened_clay || p_185514_1_.getBlock() == Blocks.stained_hardened_clay || p_185514_1_.getBlock() == Blocks.dirt;
    }

    /**
     * Whether this Block can be replaced directly by other blocks (true for e.g. tall grass)
     */
    public boolean isReplaceable(IBlockAccess worldIn, BlockPos pos)
    {
        return true;
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return random.nextInt(3);
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.stick;
    }

    public void harvestBlock(World worldIn, EntityPlayer player, BlockPos pos, IBlockState state, TileEntity te, ItemStack p_180657_6_)
    {
        if (!worldIn.isRemote && p_180657_6_ != null && p_180657_6_.getItem() == Items.shears)
        {
            player.triggerAchievement(StatList.func_188055_a(this));
            spawnAsEntity(worldIn, pos, new ItemStack(Blocks.deadbush, 1, 0));
        }
        else
        {
            super.harvestBlock(worldIn, player, pos, state, te, p_180657_6_);
        }
    }
}
