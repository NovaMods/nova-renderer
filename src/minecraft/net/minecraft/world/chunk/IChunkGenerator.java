package net.minecraft.world.chunk;

import java.util.List;
import net.minecraft.entity.EnumCreatureType;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.biome.BiomeGenBase;

public interface IChunkGenerator
{
    Chunk provideChunk(int x, int z);

    void populate(int x, int z);

    boolean func_185933_a(Chunk chunkIn, int x, int z);

    List<BiomeGenBase.SpawnListEntry> getPossibleCreatures(EnumCreatureType creatureType, BlockPos pos);

    BlockPos getStrongholdGen(World worldIn, String structureName, BlockPos position);

    void recreateStructures(Chunk chunkIn, int x, int z);
}
