package net.minecraft.world.biome;

import java.util.Random;
import net.minecraft.block.BlockDirt;
import net.minecraft.block.BlockDoublePlant;
import net.minecraft.block.BlockTallGrass;
import net.minecraft.entity.passive.EntityRabbit;
import net.minecraft.entity.passive.EntityWolf;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.chunk.ChunkPrimer;
import net.minecraft.world.gen.feature.WorldGenAbstractTree;
import net.minecraft.world.gen.feature.WorldGenBlockBlob;
import net.minecraft.world.gen.feature.WorldGenMegaPineTree;
import net.minecraft.world.gen.feature.WorldGenTaiga1;
import net.minecraft.world.gen.feature.WorldGenTaiga2;
import net.minecraft.world.gen.feature.WorldGenTallGrass;
import net.minecraft.world.gen.feature.WorldGenerator;

public class BiomeGenTaiga extends BiomeGenBase
{
    private static final WorldGenTaiga1 field_150639_aC = new WorldGenTaiga1();
    private static final WorldGenTaiga2 field_150640_aD = new WorldGenTaiga2(false);
    private static final WorldGenMegaPineTree field_150641_aE = new WorldGenMegaPineTree(false, false);
    private static final WorldGenMegaPineTree field_150642_aF = new WorldGenMegaPineTree(false, true);
    private static final WorldGenBlockBlob field_150643_aG = new WorldGenBlockBlob(Blocks.mossy_cobblestone, 0);
    private BiomeGenTaiga.Type field_150644_aH;

    public BiomeGenTaiga(BiomeGenTaiga.Type p_i46694_1_, BiomeGenBase.BiomeProperties properties)
    {
        super(properties);
        this.field_150644_aH = p_i46694_1_;
        this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityWolf.class, 8, 4, 4));
        this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityRabbit.class, 4, 2, 3));
        this.theBiomeDecorator.treesPerChunk = 10;

        if (p_i46694_1_ != BiomeGenTaiga.Type.MEGA && p_i46694_1_ != BiomeGenTaiga.Type.MEGA_SPRUCE)
        {
            this.theBiomeDecorator.grassPerChunk = 1;
            this.theBiomeDecorator.mushroomsPerChunk = 1;
        }
        else
        {
            this.theBiomeDecorator.grassPerChunk = 7;
            this.theBiomeDecorator.deadBushPerChunk = 1;
            this.theBiomeDecorator.mushroomsPerChunk = 3;
        }
    }

    public WorldGenAbstractTree genBigTreeChance(Random rand)
    {
        return (WorldGenAbstractTree)((this.field_150644_aH == BiomeGenTaiga.Type.MEGA || this.field_150644_aH == BiomeGenTaiga.Type.MEGA_SPRUCE) && rand.nextInt(3) == 0 ? (this.field_150644_aH != BiomeGenTaiga.Type.MEGA_SPRUCE && rand.nextInt(13) != 0 ? field_150641_aE : field_150642_aF) : (rand.nextInt(3) == 0 ? field_150639_aC : field_150640_aD));
    }

    /**
     * Gets a WorldGen appropriate for this biome.
     */
    public WorldGenerator getRandomWorldGenForGrass(Random rand)
    {
        return rand.nextInt(5) > 0 ? new WorldGenTallGrass(BlockTallGrass.EnumType.FERN) : new WorldGenTallGrass(BlockTallGrass.EnumType.GRASS);
    }

    public void decorate(World worldIn, Random rand, BlockPos pos)
    {
        if (this.field_150644_aH == BiomeGenTaiga.Type.MEGA || this.field_150644_aH == BiomeGenTaiga.Type.MEGA_SPRUCE)
        {
            int i = rand.nextInt(3);

            for (int j = 0; j < i; ++j)
            {
                int k = rand.nextInt(16) + 8;
                int l = rand.nextInt(16) + 8;
                BlockPos blockpos = worldIn.getHeight(pos.add(k, 0, l));
                field_150643_aG.generate(worldIn, rand, blockpos);
            }
        }

        DOUBLE_PLANT_GENERATOR.setPlantType(BlockDoublePlant.EnumPlantType.FERN);

        for (int i1 = 0; i1 < 7; ++i1)
        {
            int j1 = rand.nextInt(16) + 8;
            int k1 = rand.nextInt(16) + 8;
            int l1 = rand.nextInt(worldIn.getHeight(pos.add(j1, 0, k1)).getY() + 32);
            DOUBLE_PLANT_GENERATOR.generate(worldIn, rand, pos.add(j1, l1, k1));
        }

        super.decorate(worldIn, rand, pos);
    }

    public void genTerrainBlocks(World worldIn, Random rand, ChunkPrimer chunkPrimerIn, int x, int z, double noiseVal)
    {
        if (this.field_150644_aH == BiomeGenTaiga.Type.MEGA || this.field_150644_aH == BiomeGenTaiga.Type.MEGA_SPRUCE)
        {
            this.topBlock = Blocks.grass.getDefaultState();
            this.fillerBlock = Blocks.dirt.getDefaultState();

            if (noiseVal > 1.75D)
            {
                this.topBlock = Blocks.dirt.getDefaultState().withProperty(BlockDirt.VARIANT, BlockDirt.DirtType.COARSE_DIRT);
            }
            else if (noiseVal > -0.95D)
            {
                this.topBlock = Blocks.dirt.getDefaultState().withProperty(BlockDirt.VARIANT, BlockDirt.DirtType.PODZOL);
            }
        }

        this.generateBiomeTerrain(worldIn, rand, chunkPrimerIn, x, z, noiseVal);
    }

    public static enum Type
    {
        NORMAL,
        MEGA,
        MEGA_SPRUCE;
    }
}
