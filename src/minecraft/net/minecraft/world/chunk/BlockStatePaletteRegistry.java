package net.minecraft.world.chunk;

import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;
import net.minecraft.network.PacketBuffer;

public class BlockStatePaletteRegistry implements IBlockStatePalette
{
    public int func_186041_a(IBlockState p_186041_1_)
    {
        int i = Block.BLOCK_STATE_IDS.get(p_186041_1_);
        return i == -1 ? 0 : i;
    }

    public IBlockState func_186039_a(int p_186039_1_)
    {
        IBlockState iblockstate = (IBlockState)Block.BLOCK_STATE_IDS.getByValue(p_186039_1_);
        return iblockstate == null ? Blocks.air.getDefaultState() : iblockstate;
    }

    public void func_186038_a(PacketBuffer p_186038_1_)
    {
        p_186038_1_.readVarIntFromBuffer();
    }

    public void func_186037_b(PacketBuffer p_186037_1_)
    {
        p_186037_1_.writeVarIntToBuffer(0);
    }

    public int func_186040_a()
    {
        return PacketBuffer.getVarIntSize(0);
    }
}
