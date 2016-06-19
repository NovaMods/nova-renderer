package net.minecraft.world.chunk;

import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;

public class ChunkPrimer
{
    private static final IBlockState defaultState = Blocks.air.getDefaultState();
    private final char[] data = new char[65536];

    public IBlockState getBlockState(int x, int y, int z)
    {
        IBlockState iblockstate = (IBlockState)Block.BLOCK_STATE_IDS.getByValue(this.data[func_186137_b(x, y, z)]);
        return iblockstate == null ? defaultState : iblockstate;
    }

    public void setBlockState(int x, int y, int z, IBlockState state)
    {
        this.data[func_186137_b(x, y, z)] = (char)Block.BLOCK_STATE_IDS.get(state);
    }

    private static int func_186137_b(int x, int y, int z)
    {
        return x << 12 | z << 8 | y;
    }

    public int func_186138_a(int p_186138_1_, int p_186138_2_)
    {
        int i = (p_186138_1_ << 12 | p_186138_2_ << 8) + 256 - 1;

        for (int j = 255; j >= 0; --j)
        {
            IBlockState iblockstate = (IBlockState)Block.BLOCK_STATE_IDS.getByValue(this.data[i + j]);

            if (iblockstate != null && iblockstate != defaultState)
            {
                return j;
            }
        }

        return 0;
    }
}
