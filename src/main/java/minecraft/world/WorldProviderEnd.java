package net.minecraft.world;

import net.minecraft.init.Biomes;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.biome.BiomeProviderSingle;
import net.minecraft.world.chunk.IChunkGenerator;
import net.minecraft.world.end.DragonFightManager;
import net.minecraft.world.gen.ChunkProviderEnd;

public class WorldProviderEnd extends WorldProvider
{
    private DragonFightManager dragonFightManager = null;

    /**
     * creates a new world chunk manager for WorldProvider
     */
    public void registerWorldChunkManager()
    {
        this.worldChunkMgr = new BiomeProviderSingle(Biomes.sky);
        this.hasNoSky = true;
        NBTTagCompound nbttagcompound = this.worldObj.getWorldInfo().func_186347_a(DimensionType.THE_END);
        this.dragonFightManager = this.worldObj instanceof WorldServer ? new DragonFightManager((WorldServer)this.worldObj, nbttagcompound.getCompoundTag("DragonFight")) : null;
    }

    public IChunkGenerator createChunkGenerator()
    {
        return new ChunkProviderEnd(this.worldObj, this.worldObj.getWorldInfo().isMapFeaturesEnabled(), this.worldObj.getSeed());
    }

    /**
     * Calculates the angle of sun and moon in the sky relative to a specified time (usually worldTime)
     */
    public float calculateCelestialAngle(long worldTime, float partialTicks)
    {
        return 0.0F;
    }

    /**
     * Returns array with sunrise/sunset colors
     */
    public float[] calcSunriseSunsetColors(float celestialAngle, float partialTicks)
    {
        return null;
    }

    /**
     * Return Vec3D with biome specific fog color
     */
    public Vec3d getFogColor(float p_76562_1_, float p_76562_2_)
    {
        int i = 10518688;
        float f = MathHelper.cos(p_76562_1_ * ((float)Math.PI * 2F)) * 2.0F + 0.5F;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        float f1 = (float)(i >> 16 & 255) / 255.0F;
        float f2 = (float)(i >> 8 & 255) / 255.0F;
        float f3 = (float)(i & 255) / 255.0F;
        f1 = f1 * (f * 0.0F + 0.15F);
        f2 = f2 * (f * 0.0F + 0.15F);
        f3 = f3 * (f * 0.0F + 0.15F);
        return new Vec3d((double)f1, (double)f2, (double)f3);
    }

    public boolean isSkyColored()
    {
        return false;
    }

    /**
     * True if the player can respawn in this dimension (true = overworld, false = nether).
     */
    public boolean canRespawnHere()
    {
        return false;
    }

    /**
     * Returns 'true' if in the "main surface world", but 'false' if in the Nether or End dimensions.
     */
    public boolean isSurfaceWorld()
    {
        return false;
    }

    /**
     * the y level at which clouds are rendered.
     */
    public float getCloudHeight()
    {
        return 8.0F;
    }

    /**
     * Will check if the x, z position specified is alright to be set as the map spawn point
     */
    public boolean canCoordinateBeSpawn(int x, int z)
    {
        return this.worldObj.func_184141_c(new BlockPos(x, 0, z)).getMaterial().blocksMovement();
    }

    public BlockPos getSpawnCoordinate()
    {
        return new BlockPos(100, 50, 0);
    }

    public int getAverageGroundLevel()
    {
        return 50;
    }

    /**
     * Returns true if the given X,Z coordinate should show environmental fog.
     */
    public boolean doesXZShowFog(int x, int z)
    {
        return false;
    }

    public DimensionType getDimensionType()
    {
        return DimensionType.THE_END;
    }

    public void func_186057_q()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();

        if (this.dragonFightManager != null)
        {
            nbttagcompound.setTag("DragonFight", this.dragonFightManager.func_186088_a());
        }

        this.worldObj.getWorldInfo().func_186345_a(DimensionType.THE_END, nbttagcompound);
    }

    public void func_186059_r()
    {
        if (this.dragonFightManager != null)
        {
            this.dragonFightManager.func_186105_b();
        }
    }

    public DragonFightManager getDragonFightManager()
    {
        return this.dragonFightManager;
    }
}
