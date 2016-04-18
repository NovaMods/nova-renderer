package net.minecraft.world.chunk;

import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.network.PacketBuffer;
import net.minecraft.util.IntIdentityHashBiMap;

public class BlockStatePaletteHashMap implements IBlockStatePalette
{
    private final IntIdentityHashBiMap<IBlockState> field_186046_a;
    private final IBlockStatePaletteResizer field_186047_b;
    private final int field_186048_c;

    public BlockStatePaletteHashMap(int p_i47089_1_, IBlockStatePaletteResizer p_i47089_2_)
    {
        this.field_186048_c = p_i47089_1_;
        this.field_186047_b = p_i47089_2_;
        this.field_186046_a = new IntIdentityHashBiMap(1 << p_i47089_1_);
    }

    public int func_186041_a(IBlockState p_186041_1_)
    {
        int i = this.field_186046_a.func_186815_a(p_186041_1_);

        if (i == -1)
        {
            i = this.field_186046_a.func_186808_c(p_186041_1_);

            if (i >= 1 << this.field_186048_c)
            {
                i = this.field_186047_b.func_186008_a(this.field_186048_c + 1, p_186041_1_);
            }
        }

        return i;
    }

    public IBlockState func_186039_a(int p_186039_1_)
    {
        return (IBlockState)this.field_186046_a.func_186813_a(p_186039_1_);
    }

    public void func_186038_a(PacketBuffer p_186038_1_)
    {
        this.field_186046_a.func_186812_a();
        int i = p_186038_1_.readVarIntFromBuffer();

        for (int j = 0; j < i; ++j)
        {
            this.field_186046_a.func_186808_c(Block.BLOCK_STATE_IDS.getByValue(p_186038_1_.readVarIntFromBuffer()));
        }
    }

    public void func_186037_b(PacketBuffer p_186037_1_)
    {
        int i = this.field_186046_a.func_186810_b();
        p_186037_1_.writeVarIntToBuffer(i);

        for (int j = 0; j < i; ++j)
        {
            p_186037_1_.writeVarIntToBuffer(Block.BLOCK_STATE_IDS.get(this.field_186046_a.func_186813_a(j)));
        }
    }

    public int func_186040_a()
    {
        int i = PacketBuffer.getVarIntSize(this.field_186046_a.func_186810_b());

        for (int j = 0; j < this.field_186046_a.func_186810_b(); ++j)
        {
            i += PacketBuffer.getVarIntSize(Block.BLOCK_STATE_IDS.get(this.field_186046_a.func_186813_a(j)));
        }

        return i;
    }
}
