package net.minecraft.world.gen.feature;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityChest;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class WorldGeneratorBonusChest extends WorldGenerator
{
    public boolean generate(World worldIn, Random rand, BlockPos position)
    {
        IBlockState iblockstate;

        while (((iblockstate = worldIn.getBlockState(position)).getMaterial() == Material.air || iblockstate.getMaterial() == Material.leaves) && position.getY() > 1)
        {
            position = position.down();
        }

        if (position.getY() < 1)
        {
            return false;
        }
        else
        {
            position = position.up();

            for (int i = 0; i < 4; ++i)
            {
                BlockPos blockpos = position.add(rand.nextInt(4) - rand.nextInt(4), rand.nextInt(3) - rand.nextInt(3), rand.nextInt(4) - rand.nextInt(4));

                if (worldIn.isAirBlock(blockpos) && worldIn.getBlockState(blockpos.down()).func_185896_q())
                {
                    worldIn.setBlockState(blockpos, Blocks.chest.getDefaultState(), 2);
                    TileEntity tileentity = worldIn.getTileEntity(blockpos);

                    if (tileentity instanceof TileEntityChest)
                    {
                        ((TileEntityChest)tileentity).func_184287_a(LootTableList.CHESTS_SPAWN_BONUS_CHEST, rand.nextLong());
                    }

                    BlockPos blockpos1 = blockpos.east();
                    BlockPos blockpos2 = blockpos.west();
                    BlockPos blockpos3 = blockpos.north();
                    BlockPos blockpos4 = blockpos.south();

                    if (worldIn.isAirBlock(blockpos2) && worldIn.getBlockState(blockpos2.down()).func_185896_q())
                    {
                        worldIn.setBlockState(blockpos2, Blocks.torch.getDefaultState(), 2);
                    }

                    if (worldIn.isAirBlock(blockpos1) && worldIn.getBlockState(blockpos1.down()).func_185896_q())
                    {
                        worldIn.setBlockState(blockpos1, Blocks.torch.getDefaultState(), 2);
                    }

                    if (worldIn.isAirBlock(blockpos3) && worldIn.getBlockState(blockpos3.down()).func_185896_q())
                    {
                        worldIn.setBlockState(blockpos3, Blocks.torch.getDefaultState(), 2);
                    }

                    if (worldIn.isAirBlock(blockpos4) && worldIn.getBlockState(blockpos4.down()).func_185896_q())
                    {
                        worldIn.setBlockState(blockpos4, Blocks.torch.getDefaultState(), 2);
                    }

                    return true;
                }
            }

            return false;
        }
    }
}
