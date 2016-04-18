package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.item.EntityFallingBlock;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class BlockFalling extends Block
{
    public static boolean fallInstantly;

    public BlockFalling()
    {
        super(Material.sand);
        this.setCreativeTab(CreativeTabs.tabBlock);
    }

    public BlockFalling(Material materialIn)
    {
        super(materialIn);
    }

    public void onBlockAdded(World worldIn, BlockPos pos, IBlockState state)
    {
        worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn));
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn));
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (!worldIn.isRemote)
        {
            this.checkFallable(worldIn, pos);
        }
    }

    private void checkFallable(World worldIn, BlockPos pos)
    {
        if (func_185759_i(worldIn.getBlockState(pos.down())) && pos.getY() >= 0)
        {
            int i = 32;

            if (!fallInstantly && worldIn.isAreaLoaded(pos.add(-i, -i, -i), pos.add(i, i, i)))
            {
                if (!worldIn.isRemote)
                {
                    EntityFallingBlock entityfallingblock = new EntityFallingBlock(worldIn, (double)pos.getX() + 0.5D, (double)pos.getY(), (double)pos.getZ() + 0.5D, worldIn.getBlockState(pos));
                    this.onStartFalling(entityfallingblock);
                    worldIn.spawnEntityInWorld(entityfallingblock);
                }
            }
            else
            {
                worldIn.setBlockToAir(pos);
                BlockPos blockpos;

                for (blockpos = pos.down(); func_185759_i(worldIn.getBlockState(blockpos)) && blockpos.getY() > 0; blockpos = blockpos.down())
                {
                    ;
                }

                if (blockpos.getY() > 0)
                {
                    worldIn.setBlockState(blockpos.up(), this.getDefaultState());
                }
            }
        }
    }

    protected void onStartFalling(EntityFallingBlock fallingEntity)
    {
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return 2;
    }

    public static boolean func_185759_i(IBlockState p_185759_0_)
    {
        Block block = p_185759_0_.getBlock();
        Material material = p_185759_0_.getMaterial();
        return block == Blocks.fire || material == Material.air || material == Material.water || material == Material.lava;
    }

    public void onEndFalling(World worldIn, BlockPos pos)
    {
    }
}
