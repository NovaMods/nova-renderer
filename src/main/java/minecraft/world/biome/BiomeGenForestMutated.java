package net.minecraft.world.biome;

import java.util.Random;
import net.minecraft.world.gen.feature.WorldGenAbstractTree;

public class BiomeGenForestMutated extends BiomeGenForest
{
    public BiomeGenForestMutated(BiomeGenBase.BiomeProperties properties)
    {
        super(BiomeGenForest.Type.BIRCH, properties);
    }

    public WorldGenAbstractTree genBigTreeChance(Random rand)
    {
        return rand.nextBoolean() ? BiomeGenForest.field_150629_aC : BiomeGenForest.field_150630_aD;
    }
}
