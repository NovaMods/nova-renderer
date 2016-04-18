package net.minecraft.world;

public class WorldProviderSurface extends WorldProvider
{
    public DimensionType getDimensionType()
    {
        return DimensionType.OVERWORLD;
    }

    public boolean func_186056_c(int x, int z)
    {
        return !this.worldObj.isSpawnChunk(x, z);
    }
}
