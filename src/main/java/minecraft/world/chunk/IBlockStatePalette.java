package net.minecraft.world.chunk;

import net.minecraft.block.state.IBlockState;
import net.minecraft.network.PacketBuffer;

public interface IBlockStatePalette
{
    int func_186041_a(IBlockState p_186041_1_);

    IBlockState func_186039_a(int p_186039_1_);

    void func_186038_a(PacketBuffer p_186038_1_);

    void func_186037_b(PacketBuffer p_186037_1_);

    int func_186040_a();
}
