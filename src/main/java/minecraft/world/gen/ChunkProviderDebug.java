package net.minecraft.world.gen;

import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.EnumCreatureType;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;
import net.minecraft.world.biome.BiomeGenBase;
import net.minecraft.world.chunk.Chunk;
import net.minecraft.world.chunk.ChunkPrimer;
import net.minecraft.world.chunk.IChunkGenerator;

public class ChunkProviderDebug implements IChunkGenerator
{
    private static final List<IBlockState> field_177464_a = Lists.<IBlockState>newArrayList();
    private static final int field_177462_b;
    private static final int field_181039_c;
    protected static final IBlockState AIR_DEFAULT_STATE = Blocks.air.getDefaultState();
    protected static final IBlockState BARRIER_DEFAULT_STATE = Blocks.barrier.getDefaultState();
    private final World world;

    public ChunkProviderDebug(World worldIn)
    {
        this.world = worldIn;
    }

    public Chunk provideChunk(int x, int z)
    {
        ChunkPrimer chunkprimer = new ChunkPrimer();

        for (int i = 0; i < 16; ++i)
        {
            for (int j = 0; j < 16; ++j)
            {
                int k = x * 16 + i;
                int l = z * 16 + j;
                chunkprimer.setBlockState(i, 60, j, BARRIER_DEFAULT_STATE);
                IBlockState iblockstate = func_177461_b(k, l);

                if (iblockstate != null)
                {
                    chunkprimer.setBlockState(i, 70, j, iblockstate);
                }
            }
        }

        Chunk chunk = new Chunk(this.world, chunkprimer, x, z);
        chunk.generateSkylightMap();
        BiomeGenBase[] abiomegenbase = this.world.getWorldChunkManager().loadBlockGeneratorData((BiomeGenBase[])null, x * 16, z * 16, 16, 16);
        byte[] abyte = chunk.getBiomeArray();

        for (int i1 = 0; i1 < abyte.length; ++i1)
        {
            abyte[i1] = (byte)BiomeGenBase.getIdForBiome(abiomegenbase[i1]);
        }

        chunk.generateSkylightMap();
        return chunk;
    }

    public static IBlockState func_177461_b(int p_177461_0_, int p_177461_1_)
    {
        IBlockState iblockstate = AIR_DEFAULT_STATE;

        if (p_177461_0_ > 0 && p_177461_1_ > 0 && p_177461_0_ % 2 != 0 && p_177461_1_ % 2 != 0)
        {
            p_177461_0_ = p_177461_0_ / 2;
            p_177461_1_ = p_177461_1_ / 2;

            if (p_177461_0_ <= field_177462_b && p_177461_1_ <= field_181039_c)
            {
                int i = MathHelper.abs_int(p_177461_0_ * field_177462_b + p_177461_1_);

                if (i < field_177464_a.size())
                {
                    iblockstate = (IBlockState)field_177464_a.get(i);
                }
            }
        }

        return iblockstate;
    }

    public void populate(int x, int z)
    {
    }

    public boolean func_185933_a(Chunk chunkIn, int x, int z)
    {
        return false;
    }

    public List<BiomeGenBase.SpawnListEntry> getPossibleCreatures(EnumCreatureType creatureType, BlockPos pos)
    {
        BiomeGenBase biomegenbase = this.world.getBiomeGenForCoords(pos);
        return biomegenbase.getSpawnableList(creatureType);
    }

    public BlockPos getStrongholdGen(World worldIn, String structureName, BlockPos position)
    {
        return null;
    }

    public void recreateStructures(Chunk chunkIn, int x, int z)
    {
    }

    static
    {
        for (Block block : Block.blockRegistry)
        {
            field_177464_a.addAll(block.getBlockState().getValidStates());
        }

        field_177462_b = MathHelper.ceiling_float_int(MathHelper.sqrt_float((float)field_177464_a.size()));
        field_181039_c = MathHelper.ceiling_float_int((float)field_177464_a.size() / (float)field_177462_b);
    }
}
