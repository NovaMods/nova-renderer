package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.item.ItemStack;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityEnchantmentTable;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockEnchantmentTable extends BlockContainer
{
    protected static final AxisAlignedBB field_185567_a = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.75D, 1.0D);

    protected BlockEnchantmentTable()
    {
        super(Material.rock, MapColor.redColor);
        this.setLightOpacity(0);
        this.setCreativeTab(CreativeTabs.tabDecorations);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185567_a;
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        super.randomDisplayTick(worldIn, pos, state, rand);

        for (int i = -2; i <= 2; ++i)
        {
            for (int j = -2; j <= 2; ++j)
            {
                if (i > -2 && i < 2 && j == -1)
                {
                    j = 2;
                }

                if (rand.nextInt(16) == 0)
                {
                    for (int k = 0; k <= 1; ++k)
                    {
                        BlockPos blockpos = state.add(i, k, j);

                        if (pos.getBlockState(blockpos).getBlock() == Blocks.bookshelf)
                        {
                            if (!pos.isAirBlock(state.add(i / 2, 0, j / 2)))
                            {
                                break;
                            }

                            pos.spawnParticle(EnumParticleTypes.ENCHANTMENT_TABLE, (double)state.getX() + 0.5D, (double)state.getY() + 2.0D, (double)state.getZ() + 0.5D, (double)((float)i + rand.nextFloat()) - 0.5D, (double)((float)k - rand.nextFloat() - 1.0F), (double)((float)j + rand.nextFloat()) - 0.5D, new int[0]);
                        }
                    }
                }
            }
        }
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    /**
     * The type of render function called. 3 for standard block models, 2 for TESR's, 1 for liquids, -1 is no render
     */
    public EnumBlockRenderType getRenderType(IBlockState state)
    {
        return EnumBlockRenderType.MODEL;
    }

    /**
     * Returns a new instance of a block's tile entity class. Called on placing the block.
     */
    public TileEntity createNewTileEntity(World worldIn, int meta)
    {
        return new TileEntityEnchantmentTable();
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (worldIn.isRemote)
        {
            return true;
        }
        else
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityEnchantmentTable)
            {
                playerIn.displayGui((TileEntityEnchantmentTable)tileentity);
            }

            return true;
        }
    }

    /**
     * Called by ItemBlocks after a block is set in the world, to allow post-place logic
     */
    public void onBlockPlacedBy(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase placer, ItemStack stack)
    {
        super.onBlockPlacedBy(worldIn, pos, state, placer, stack);

        if (stack.hasDisplayName())
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityEnchantmentTable)
            {
                ((TileEntityEnchantmentTable)tileentity).setCustomName(stack.getDisplayName());
            }
        }
    }
}
