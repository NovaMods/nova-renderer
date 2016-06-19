package net.minecraft.block;

import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;

public class BlockCompressedPowered extends Block
{
    public BlockCompressedPowered(Material p_i46386_1_, MapColor p_i46386_2_)
    {
        super(p_i46386_1_, p_i46386_2_);
    }

    /**
     * Can this block provide power. Only wire currently seems to have this change based on its state.
     */
    public boolean canProvidePower(IBlockState state)
    {
        return true;
    }

    public int getWeakPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return 15;
    }
}
