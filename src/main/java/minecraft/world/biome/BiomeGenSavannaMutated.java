package net.minecraft.world.biome;

import java.util.Random;
import net.minecraft.block.BlockDirt;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.chunk.ChunkPrimer;

public class BiomeGenSavannaMutated extends BiomeGenSavanna
{
    public BiomeGenSavannaMutated(BiomeGenBase.BiomeProperties properties)
    {
        super(properties);
        this.theBiomeDecorator.treesPerChunk = 2;
        this.theBiomeDecorator.flowersPerChunk = 2;
        this.theBiomeDecorator.grassPerChunk = 5;
    }

    public void genTerrainBlocks(World worldIn, Random rand, ChunkPrimer chunkPrimerIn, int x, int z, double noiseVal)
    {
        this.topBlock = Blocks.grass.getDefaultState();
        this.fillerBlock = Blocks.dirt.getDefaultState();

        if (noiseVal > 1.75D)
        {
            this.topBlock = Blocks.stone.getDefaultState();
            this.fillerBlock = Blocks.stone.getDefaultState();
        }
        else if (noiseVal > -0.5D)
        {
            this.topBlock = Blocks.dirt.getDefaultState().withProperty(BlockDirt.VARIANT, BlockDirt.DirtType.COARSE_DIRT);
        }

        this.generateBiomeTerrain(worldIn, rand, chunkPrimerIn, x, z, noiseVal);
    }

    public void decorate(World worldIn, Random rand, BlockPos pos)
    {
        this.theBiomeDecorator.decorate(worldIn, rand, this, pos);
    }
}
