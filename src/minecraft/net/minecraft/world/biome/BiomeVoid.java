package net.minecraft.world.biome;

public class BiomeVoid extends BiomeGenBase
{
    public BiomeVoid(BiomeGenBase.BiomeProperties properties)
    {
        super(properties);
        this.spawnableMonsterList.clear();
        this.spawnableCreatureList.clear();
        this.spawnableWaterCreatureList.clear();
        this.spawnableCaveCreatureList.clear();
        this.theBiomeDecorator = new BiomeVoidDecorator();
    }

    public boolean ignorePlayerSpawnSuitability()
    {
        return true;
    }
}
