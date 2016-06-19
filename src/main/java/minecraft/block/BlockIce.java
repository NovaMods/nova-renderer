package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Enchantments;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.EnumSkyBlock;
import net.minecraft.world.World;

public class BlockIce extends BlockBreakable
{
    public BlockIce()
    {
        super(Material.ice, false);
        this.slipperiness = 0.98F;
        this.setTickRandomly(true);
        this.setCreativeTab(CreativeTabs.tabBlock);
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.TRANSLUCENT;
    }

    public void harvestBlock(World worldIn, EntityPlayer player, BlockPos pos, IBlockState state, TileEntity te, ItemStack p_180657_6_)
    {
        player.triggerAchievement(StatList.func_188055_a(this));
        player.addExhaustion(0.025F);

        if (this.canSilkHarvest() && EnchantmentHelper.getEnchantmentLevel(Enchantments.silkTouch, p_180657_6_) > 0)
        {
            ItemStack itemstack = this.createStackedBlock(state);

            if (itemstack != null)
            {
                spawnAsEntity(worldIn, pos, itemstack);
            }
        }
        else
        {
            if (worldIn.provider.doesWaterVaporize())
            {
                worldIn.setBlockToAir(pos);
                return;
            }

            int i = EnchantmentHelper.getEnchantmentLevel(Enchantments.fortune, p_180657_6_);
            this.dropBlockAsItem(worldIn, pos, state, i);
            Material material = worldIn.getBlockState(pos.down()).getMaterial();

            if (material.blocksMovement() || material.isLiquid())
            {
                worldIn.setBlockState(pos, Blocks.flowing_water.getDefaultState());
            }
        }
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return 0;
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (worldIn.getLightFor(EnumSkyBlock.BLOCK, pos) > 11 - this.getDefaultState().getLightOpacity())
        {
            this.func_185679_b(worldIn, pos);
        }
    }

    protected void func_185679_b(World p_185679_1_, BlockPos p_185679_2_)
    {
        if (p_185679_1_.provider.doesWaterVaporize())
        {
            p_185679_1_.setBlockToAir(p_185679_2_);
        }
        else
        {
            this.dropBlockAsItem(p_185679_1_, p_185679_2_, p_185679_1_.getBlockState(p_185679_2_), 0);
            p_185679_1_.setBlockState(p_185679_2_, Blocks.water.getDefaultState());
            p_185679_1_.notifyBlockOfStateChange(p_185679_2_, Blocks.water);
        }
    }

    public EnumPushReaction getMobilityFlag(IBlockState state)
    {
        return EnumPushReaction.NORMAL;
    }
}
