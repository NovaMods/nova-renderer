package net.minecraft.world.biome;

import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import java.util.Collections;
import java.util.List;
import java.util.Random;
import java.util.Set;
import net.minecraft.block.BlockFlower;
import net.minecraft.block.BlockSand;
import net.minecraft.block.BlockTallGrass;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EnumCreatureType;
import net.minecraft.entity.monster.EntityCreeper;
import net.minecraft.entity.monster.EntityEnderman;
import net.minecraft.entity.monster.EntitySkeleton;
import net.minecraft.entity.monster.EntitySlime;
import net.minecraft.entity.monster.EntitySpider;
import net.minecraft.entity.monster.EntityWitch;
import net.minecraft.entity.monster.EntityZombie;
import net.minecraft.entity.passive.EntityBat;
import net.minecraft.entity.passive.EntityChicken;
import net.minecraft.entity.passive.EntityCow;
import net.minecraft.entity.passive.EntityPig;
import net.minecraft.entity.passive.EntitySheep;
import net.minecraft.entity.passive.EntitySquid;
import net.minecraft.init.Biomes;
import net.minecraft.init.Blocks;
import net.minecraft.util.ObjectIntIdentityMap;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.WeightedRandom;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.registry.RegistryNamespaced;
import net.minecraft.world.ColorizerFoliage;
import net.minecraft.world.ColorizerGrass;
import net.minecraft.world.World;
import net.minecraft.world.chunk.ChunkPrimer;
import net.minecraft.world.gen.NoiseGeneratorPerlin;
import net.minecraft.world.gen.feature.WorldGenAbstractTree;
import net.minecraft.world.gen.feature.WorldGenBigTree;
import net.minecraft.world.gen.feature.WorldGenDoublePlant;
import net.minecraft.world.gen.feature.WorldGenSwamp;
import net.minecraft.world.gen.feature.WorldGenTallGrass;
import net.minecraft.world.gen.feature.WorldGenTrees;
import net.minecraft.world.gen.feature.WorldGenerator;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public abstract class BiomeGenBase
{
    private static final Logger logger = LogManager.getLogger();
    protected static final IBlockState STONE = Blocks.stone.getDefaultState();
    protected static final IBlockState AIR = Blocks.air.getDefaultState();
    protected static final IBlockState BEDROCK = Blocks.bedrock.getDefaultState();
    protected static final IBlockState GRAVEL = Blocks.gravel.getDefaultState();
    protected static final IBlockState RED_SANDSTONE = Blocks.red_sandstone.getDefaultState();
    protected static final IBlockState SANDSTONE = Blocks.sandstone.getDefaultState();
    protected static final IBlockState ICE = Blocks.ice.getDefaultState();
    protected static final IBlockState WATER = Blocks.water.getDefaultState();
    public static final Set<BiomeGenBase> explorationBiomesList = Sets.<BiomeGenBase>newHashSet();
    public static final ObjectIntIdentityMap<BiomeGenBase> MUTATION_TO_BASE_ID_MAP = new ObjectIntIdentityMap();
    protected static final NoiseGeneratorPerlin temperatureNoise = new NoiseGeneratorPerlin(new Random(1234L), 1);
    protected static final NoiseGeneratorPerlin GRASS_COLOR_NOISE = new NoiseGeneratorPerlin(new Random(2345L), 1);
    protected static final WorldGenDoublePlant DOUBLE_PLANT_GENERATOR = new WorldGenDoublePlant();

    /** The tree generator. */
    protected static final WorldGenTrees worldGeneratorTrees = new WorldGenTrees(false);

    /** The big tree generator. */
    protected static final WorldGenBigTree worldGeneratorBigTree = new WorldGenBigTree(false);

    /** The swamp tree generator. */
    protected static final WorldGenSwamp worldGeneratorSwamp = new WorldGenSwamp();
    public static final RegistryNamespaced<ResourceLocation, BiomeGenBase> biomeRegistry = new RegistryNamespaced();
    private final String biomeName;

    /** The base height of this biome. Default 0.1. */
    private final float baseHeight;

    /** The variation from the base height of the biome. Default 0.3. */
    private final float heightVariation;

    /** The temperature of this biome. */
    private final float temperature;

    /** The rainfall in this biome. */
    private final float rainfall;

    /** Color tint applied to water depending on biome */
    private final int waterColor;

    /** Set to true if snow is enabled for this biome. */
    private final boolean enableSnow;

    /**
     * Is true (default) if the biome support rain (desert and nether can't have rain)
     */
    private final boolean enableRain;

    /** The unique identifier of the biome for which this is a mutation of. */
    private final String baseBiomeRegName;

    /** The block expected to be on the top of this biome */
    public IBlockState topBlock = Blocks.grass.getDefaultState();

    /** The block to fill spots in when not on the top */
    public IBlockState fillerBlock = Blocks.dirt.getDefaultState();

    /** The biome decorator. */
    public BiomeDecorator theBiomeDecorator;
    protected List<BiomeGenBase.SpawnListEntry> spawnableMonsterList = Lists.<BiomeGenBase.SpawnListEntry>newArrayList();
    protected List<BiomeGenBase.SpawnListEntry> spawnableCreatureList = Lists.<BiomeGenBase.SpawnListEntry>newArrayList();
    protected List<BiomeGenBase.SpawnListEntry> spawnableWaterCreatureList = Lists.<BiomeGenBase.SpawnListEntry>newArrayList();
    protected List<BiomeGenBase.SpawnListEntry> spawnableCaveCreatureList = Lists.<BiomeGenBase.SpawnListEntry>newArrayList();

    public static int getIdForBiome(BiomeGenBase biome)
    {
        return biomeRegistry.getIDForObject(biome);
    }

    public static BiomeGenBase getBiomeForId(int id)
    {
        return (BiomeGenBase)biomeRegistry.getObjectById(id);
    }

    public static BiomeGenBase getMutationForBiome(BiomeGenBase biome)
    {
        return (BiomeGenBase)MUTATION_TO_BASE_ID_MAP.getByValue(getIdForBiome(biome));
    }

    protected BiomeGenBase(BiomeGenBase.BiomeProperties properties)
    {
        this.biomeName = properties.biomeName;
        this.baseHeight = properties.baseHeight;
        this.heightVariation = properties.heightVariation;
        this.temperature = properties.temperature;
        this.rainfall = properties.rainfall;
        this.waterColor = properties.waterColor;
        this.enableSnow = properties.enableSnow;
        this.enableRain = properties.enableRain;
        this.baseBiomeRegName = properties.baseBiomeRegName;
        this.theBiomeDecorator = this.createBiomeDecorator();
        this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntitySheep.class, 12, 4, 4));
        this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityPig.class, 10, 4, 4));
        this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityChicken.class, 10, 4, 4));
        this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityCow.class, 8, 4, 4));
        this.spawnableMonsterList.add(new BiomeGenBase.SpawnListEntry(EntitySpider.class, 100, 4, 4));
        this.spawnableMonsterList.add(new BiomeGenBase.SpawnListEntry(EntityZombie.class, 100, 4, 4));
        this.spawnableMonsterList.add(new BiomeGenBase.SpawnListEntry(EntitySkeleton.class, 100, 4, 4));
        this.spawnableMonsterList.add(new BiomeGenBase.SpawnListEntry(EntityCreeper.class, 100, 4, 4));
        this.spawnableMonsterList.add(new BiomeGenBase.SpawnListEntry(EntitySlime.class, 100, 4, 4));
        this.spawnableMonsterList.add(new BiomeGenBase.SpawnListEntry(EntityEnderman.class, 10, 1, 4));
        this.spawnableMonsterList.add(new BiomeGenBase.SpawnListEntry(EntityWitch.class, 5, 1, 1));
        this.spawnableWaterCreatureList.add(new BiomeGenBase.SpawnListEntry(EntitySquid.class, 10, 4, 4));
        this.spawnableCaveCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityBat.class, 10, 8, 8));
    }

    /**
     * Allocate a new BiomeDecorator for this BiomeGenBase
     */
    protected BiomeDecorator createBiomeDecorator()
    {
        return new BiomeDecorator();
    }

    public boolean isMutation()
    {
        return this.baseBiomeRegName != null;
    }

    public WorldGenAbstractTree genBigTreeChance(Random rand)
    {
        return (WorldGenAbstractTree)(rand.nextInt(10) == 0 ? worldGeneratorBigTree : worldGeneratorTrees);
    }

    /**
     * Gets a WorldGen appropriate for this biome.
     */
    public WorldGenerator getRandomWorldGenForGrass(Random rand)
    {
        return new WorldGenTallGrass(BlockTallGrass.EnumType.GRASS);
    }

    public BlockFlower.EnumFlowerType pickRandomFlower(Random rand, BlockPos pos)
    {
        return rand.nextInt(3) > 0 ? BlockFlower.EnumFlowerType.DANDELION : BlockFlower.EnumFlowerType.POPPY;
    }

    /**
     * takes temperature, returns color
     */
    public int getSkyColorByTemp(float p_76731_1_)
    {
        p_76731_1_ = p_76731_1_ / 3.0F;
        p_76731_1_ = MathHelper.clamp_float(p_76731_1_, -1.0F, 1.0F);
        return MathHelper.hsvToRGB(0.62222224F - p_76731_1_ * 0.05F, 0.5F + p_76731_1_ * 0.1F, 1.0F);
    }

    public List<BiomeGenBase.SpawnListEntry> getSpawnableList(EnumCreatureType creatureType)
    {
        switch (creatureType)
        {
            case MONSTER:
                return this.spawnableMonsterList;

            case CREATURE:
                return this.spawnableCreatureList;

            case WATER_CREATURE:
                return this.spawnableWaterCreatureList;

            case AMBIENT:
                return this.spawnableCaveCreatureList;

            default:
                return Collections.<BiomeGenBase.SpawnListEntry>emptyList();
        }
    }

    /**
     * Returns true if the biome have snowfall instead a normal rain.
     */
    public boolean getEnableSnow()
    {
        return this.isSnowyBiome();
    }

    /**
     * Check if rain can occur in biome
     */
    public boolean canRain()
    {
        return this.isSnowyBiome() ? false : this.enableRain;
    }

    /**
     * Checks to see if the rainfall level of the biome is extremely high
     */
    public boolean isHighHumidity()
    {
        return this.getRainfall() > 0.85F;
    }

    /**
     * returns the chance a creature has to spawn.
     */
    public float getSpawningChance()
    {
        return 0.1F;
    }

    /**
     * Gets a floating point representation of this biome's temperature
     */
    public final float getFloatTemperature(BlockPos pos)
    {
        if (pos.getY() > 64)
        {
            float f = (float)(temperatureNoise.func_151601_a((double)((float)pos.getX() / 8.0F), (double)((float)pos.getZ() / 8.0F)) * 4.0D);
            return this.getTemperature() - (f + (float)pos.getY() - 64.0F) * 0.05F / 30.0F;
        }
        else
        {
            return this.getTemperature();
        }
    }

    public void decorate(World worldIn, Random rand, BlockPos pos)
    {
        this.theBiomeDecorator.decorate(worldIn, rand, this, pos);
    }

    public int getGrassColorAtPos(BlockPos pos)
    {
        double d0 = (double)MathHelper.clamp_float(this.getFloatTemperature(pos), 0.0F, 1.0F);
        double d1 = (double)MathHelper.clamp_float(this.getRainfall(), 0.0F, 1.0F);
        return ColorizerGrass.getGrassColor(d0, d1);
    }

    public int getFoliageColorAtPos(BlockPos pos)
    {
        double d0 = (double)MathHelper.clamp_float(this.getFloatTemperature(pos), 0.0F, 1.0F);
        double d1 = (double)MathHelper.clamp_float(this.getRainfall(), 0.0F, 1.0F);
        return ColorizerFoliage.getFoliageColor(d0, d1);
    }

    public void genTerrainBlocks(World worldIn, Random rand, ChunkPrimer chunkPrimerIn, int x, int z, double noiseVal)
    {
        this.generateBiomeTerrain(worldIn, rand, chunkPrimerIn, x, z, noiseVal);
    }

    /**
     * Given x, z coordinates, we count down all the y positions starting at 255 and working our way down. When we hit a
     * non-air block, we replace it with this.topBlock (default grass, descendants may set otherwise), and then a
     * relatively shallow layer of blocks of type this.fillerBlock (default dirt). A random set of blocks below y == 5
     * (but always including y == 0) is replaced with bedrock.
     *  
     * If we don't hit non-air until somewhat below sea level, we top with gravel and fill down with stone.
     *  
     * If this.fillerBlock is red sand, we replace some of that with red sandstone.
     */
    public final void generateBiomeTerrain(World worldIn, Random rand, ChunkPrimer chunkPrimerIn, int x, int z, double noiseVal)
    {
        int i = worldIn.getSeaLevel();
        IBlockState iblockstate = this.topBlock;
        IBlockState iblockstate1 = this.fillerBlock;
        int j = -1;
        int k = (int)(noiseVal / 3.0D + 3.0D + rand.nextDouble() * 0.25D);
        int l = x & 15;
        int i1 = z & 15;
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos();

        for (int j1 = 255; j1 >= 0; --j1)
        {
            if (j1 <= rand.nextInt(5))
            {
                chunkPrimerIn.setBlockState(i1, j1, l, BEDROCK);
            }
            else
            {
                IBlockState iblockstate2 = chunkPrimerIn.getBlockState(i1, j1, l);

                if (iblockstate2.getMaterial() == Material.air)
                {
                    j = -1;
                }
                else if (iblockstate2.getBlock() == Blocks.stone)
                {
                    if (j == -1)
                    {
                        if (k <= 0)
                        {
                            iblockstate = AIR;
                            iblockstate1 = STONE;
                        }
                        else if (j1 >= i - 4 && j1 <= i + 1)
                        {
                            iblockstate = this.topBlock;
                            iblockstate1 = this.fillerBlock;
                        }

                        if (j1 < i && (iblockstate == null || iblockstate.getMaterial() == Material.air))
                        {
                            if (this.getFloatTemperature(blockpos$mutableblockpos.set(x, j1, z)) < 0.15F)
                            {
                                iblockstate = ICE;
                            }
                            else
                            {
                                iblockstate = WATER;
                            }
                        }

                        j = k;

                        if (j1 >= i - 1)
                        {
                            chunkPrimerIn.setBlockState(i1, j1, l, iblockstate);
                        }
                        else if (j1 < i - 7 - k)
                        {
                            iblockstate = AIR;
                            iblockstate1 = STONE;
                            chunkPrimerIn.setBlockState(i1, j1, l, GRAVEL);
                        }
                        else
                        {
                            chunkPrimerIn.setBlockState(i1, j1, l, iblockstate1);
                        }
                    }
                    else if (j > 0)
                    {
                        --j;
                        chunkPrimerIn.setBlockState(i1, j1, l, iblockstate1);

                        if (j == 0 && iblockstate1.getBlock() == Blocks.sand)
                        {
                            j = rand.nextInt(4) + Math.max(0, j1 - 63);
                            iblockstate1 = iblockstate1.getValue(BlockSand.VARIANT) == BlockSand.EnumType.RED_SAND ? RED_SANDSTONE : SANDSTONE;
                        }
                    }
                }
            }
        }
    }

    public Class <? extends BiomeGenBase > getBiomeClass()
    {
        return this.getClass();
    }

    public BiomeGenBase.TempCategory getTempCategory()
    {
        return (double)this.getTemperature() < 0.2D ? BiomeGenBase.TempCategory.COLD : ((double)this.getTemperature() < 1.0D ? BiomeGenBase.TempCategory.MEDIUM : BiomeGenBase.TempCategory.WARM);
    }

    /**
     * return the biome specified by biomeID, or 0 (ocean) if out of bounds
     */
    public static BiomeGenBase getBiome(int id)
    {
        return getBiomeFromBiomeList(id, (BiomeGenBase)null);
    }

    public static BiomeGenBase getBiomeFromBiomeList(int biomeId, BiomeGenBase biome)
    {
        BiomeGenBase biomegenbase = getBiomeForId(biomeId);
        return biomegenbase == null ? biome : biomegenbase;
    }

    public boolean ignorePlayerSpawnSuitability()
    {
        return false;
    }

    public final float getBaseHeight()
    {
        return this.baseHeight;
    }

    /**
     * Gets a floating point representation of this biome's rainfall
     */
    public final float getRainfall()
    {
        return this.rainfall;
    }

    public final String getBiomeName()
    {
        return this.biomeName;
    }

    public final float getHeightVariation()
    {
        return this.heightVariation;
    }

    public final float getTemperature()
    {
        return this.temperature;
    }

    public final int getWaterColor()
    {
        return this.waterColor;
    }

    public final boolean isSnowyBiome()
    {
        return this.enableSnow;
    }

    /**
     * Registers all of the vanilla biomes.
     */
    public static void registerBiomes()
    {
        registerBiome(0, "ocean", new BiomeGenOcean((new BiomeGenBase.BiomeProperties("Ocean")).setBaseHeight(-1.0F).setHeightVariation(0.1F)));
        registerBiome(1, "plains", new BiomeGenPlains(false, (new BiomeGenBase.BiomeProperties("Plains")).setBaseHeight(0.125F).setHeightVariation(0.05F).setTemperature(0.8F).setRainfall(0.4F)));
        registerBiome(2, "desert", new BiomeGenDesert((new BiomeGenBase.BiomeProperties("Desert")).setBaseHeight(0.125F).setHeightVariation(0.05F).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(3, "extreme_hills", new BiomeGenHills(BiomeGenHills.Type.NORMAL, (new BiomeGenBase.BiomeProperties("Extreme Hills")).setBaseHeight(1.0F).setHeightVariation(0.5F).setTemperature(0.2F).setRainfall(0.3F)));
        registerBiome(4, "forest", new BiomeGenForest(BiomeGenForest.Type.NORMAL, (new BiomeGenBase.BiomeProperties("Forest")).setTemperature(0.7F).setRainfall(0.8F)));
        registerBiome(5, "taiga", new BiomeGenTaiga(BiomeGenTaiga.Type.NORMAL, (new BiomeGenBase.BiomeProperties("Taiga")).setBaseHeight(0.2F).setHeightVariation(0.2F).setTemperature(0.25F).setRainfall(0.8F)));
        registerBiome(6, "swampland", new BiomeGenSwamp((new BiomeGenBase.BiomeProperties("Swampland")).setBaseHeight(-0.2F).setHeightVariation(0.1F).setTemperature(0.8F).setRainfall(0.9F).setWaterColor(14745518)));
        registerBiome(7, "river", new BiomeGenRiver((new BiomeGenBase.BiomeProperties("River")).setBaseHeight(-0.5F).setHeightVariation(0.0F)));
        registerBiome(8, "hell", new BiomeGenHell((new BiomeGenBase.BiomeProperties("Hell")).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(9, "sky", new BiomeGenEnd((new BiomeGenBase.BiomeProperties("The End")).setRainDisabled()));
        registerBiome(10, "frozen_ocean", new BiomeGenOcean((new BiomeGenBase.BiomeProperties("FrozenOcean")).setBaseHeight(-1.0F).setHeightVariation(0.1F).setTemperature(0.0F).setRainfall(0.5F).setSnowEnabled()));
        registerBiome(11, "frozen_river", new BiomeGenRiver((new BiomeGenBase.BiomeProperties("FrozenRiver")).setBaseHeight(-0.5F).setHeightVariation(0.0F).setTemperature(0.0F).setRainfall(0.5F).setSnowEnabled()));
        registerBiome(12, "ice_flats", new BiomeGenSnow(false, (new BiomeGenBase.BiomeProperties("Ice Plains")).setBaseHeight(0.125F).setHeightVariation(0.05F).setTemperature(0.0F).setRainfall(0.5F).setSnowEnabled()));
        registerBiome(13, "ice_mountains", new BiomeGenSnow(false, (new BiomeGenBase.BiomeProperties("Ice Mountains")).setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(0.0F).setRainfall(0.5F).setSnowEnabled()));
        registerBiome(14, "mushroom_island", new BiomeGenMushroomIsland((new BiomeGenBase.BiomeProperties("MushroomIsland")).setBaseHeight(0.2F).setHeightVariation(0.3F).setTemperature(0.9F).setRainfall(1.0F)));
        registerBiome(15, "mushroom_island_shore", new BiomeGenMushroomIsland((new BiomeGenBase.BiomeProperties("MushroomIslandShore")).setBaseHeight(0.0F).setHeightVariation(0.025F).setTemperature(0.9F).setRainfall(1.0F)));
        registerBiome(16, "beaches", new BiomeGenBeach((new BiomeGenBase.BiomeProperties("Beach")).setBaseHeight(0.0F).setHeightVariation(0.025F).setTemperature(0.8F).setRainfall(0.4F)));
        registerBiome(17, "desert_hills", new BiomeGenDesert((new BiomeGenBase.BiomeProperties("DesertHills")).setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(18, "forest_hills", new BiomeGenForest(BiomeGenForest.Type.NORMAL, (new BiomeGenBase.BiomeProperties("ForestHills")).setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(0.7F).setRainfall(0.8F)));
        registerBiome(19, "taiga_hills", new BiomeGenTaiga(BiomeGenTaiga.Type.NORMAL, (new BiomeGenBase.BiomeProperties("TaigaHills")).setTemperature(0.25F).setRainfall(0.8F).setBaseHeight(0.45F).setHeightVariation(0.3F)));
        registerBiome(20, "smaller_extreme_hills", new BiomeGenHills(BiomeGenHills.Type.EXTRA_TREES, (new BiomeGenBase.BiomeProperties("Extreme Hills Edge")).setBaseHeight(0.8F).setHeightVariation(0.3F).setTemperature(0.2F).setRainfall(0.3F)));
        registerBiome(21, "jungle", new BiomeGenJungle(false, (new BiomeGenBase.BiomeProperties("Jungle")).setTemperature(0.95F).setRainfall(0.9F)));
        registerBiome(22, "jungle_hills", new BiomeGenJungle(false, (new BiomeGenBase.BiomeProperties("JungleHills")).setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(0.95F).setRainfall(0.9F)));
        registerBiome(23, "jungle_edge", new BiomeGenJungle(true, (new BiomeGenBase.BiomeProperties("JungleEdge")).setTemperature(0.95F).setRainfall(0.8F)));
        registerBiome(24, "deep_ocean", new BiomeGenOcean((new BiomeGenBase.BiomeProperties("Deep Ocean")).setBaseHeight(-1.8F).setHeightVariation(0.1F)));
        registerBiome(25, "stone_beach", new BiomeGenStoneBeach((new BiomeGenBase.BiomeProperties("Stone Beach")).setBaseHeight(0.1F).setHeightVariation(0.8F).setTemperature(0.2F).setRainfall(0.3F)));
        registerBiome(26, "cold_beach", new BiomeGenBeach((new BiomeGenBase.BiomeProperties("Cold Beach")).setBaseHeight(0.0F).setHeightVariation(0.025F).setTemperature(0.05F).setRainfall(0.3F).setSnowEnabled()));
        registerBiome(27, "birch_forest", new BiomeGenForest(BiomeGenForest.Type.BIRCH, (new BiomeGenBase.BiomeProperties("Birch Forest")).setTemperature(0.6F).setRainfall(0.6F)));
        registerBiome(28, "birch_forest_hills", new BiomeGenForest(BiomeGenForest.Type.BIRCH, (new BiomeGenBase.BiomeProperties("Birch Forest Hills")).setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(0.6F).setRainfall(0.6F)));
        registerBiome(29, "roofed_forest", new BiomeGenForest(BiomeGenForest.Type.ROOFED, (new BiomeGenBase.BiomeProperties("Roofed Forest")).setTemperature(0.7F).setRainfall(0.8F)));
        registerBiome(30, "taiga_cold", new BiomeGenTaiga(BiomeGenTaiga.Type.NORMAL, (new BiomeGenBase.BiomeProperties("Cold Taiga")).setBaseHeight(0.2F).setHeightVariation(0.2F).setTemperature(-0.5F).setRainfall(0.4F).setSnowEnabled()));
        registerBiome(31, "taiga_cold_hills", new BiomeGenTaiga(BiomeGenTaiga.Type.NORMAL, (new BiomeGenBase.BiomeProperties("Cold Taiga Hills")).setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(-0.5F).setRainfall(0.4F).setSnowEnabled()));
        registerBiome(32, "redwood_taiga", new BiomeGenTaiga(BiomeGenTaiga.Type.MEGA, (new BiomeGenBase.BiomeProperties("Mega Taiga")).setTemperature(0.3F).setRainfall(0.8F).setBaseHeight(0.2F).setHeightVariation(0.2F)));
        registerBiome(33, "redwood_taiga_hills", new BiomeGenTaiga(BiomeGenTaiga.Type.MEGA, (new BiomeGenBase.BiomeProperties("Mega Taiga Hills")).setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(0.3F).setRainfall(0.8F)));
        registerBiome(34, "extreme_hills_with_trees", new BiomeGenHills(BiomeGenHills.Type.EXTRA_TREES, (new BiomeGenBase.BiomeProperties("Extreme Hills+")).setBaseHeight(1.0F).setHeightVariation(0.5F).setTemperature(0.2F).setRainfall(0.3F)));
        registerBiome(35, "savanna", new BiomeGenSavanna((new BiomeGenBase.BiomeProperties("Savanna")).setBaseHeight(0.125F).setHeightVariation(0.05F).setTemperature(1.2F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(36, "savanna_rock", new BiomeGenSavanna((new BiomeGenBase.BiomeProperties("Savanna Plateau")).setBaseHeight(1.5F).setHeightVariation(0.025F).setTemperature(1.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(37, "mesa", new BiomeGenMesa(false, false, (new BiomeGenBase.BiomeProperties("Mesa")).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(38, "mesa_rock", new BiomeGenMesa(false, true, (new BiomeGenBase.BiomeProperties("Mesa Plateau F")).setBaseHeight(1.5F).setHeightVariation(0.025F).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(39, "mesa_clear_rock", new BiomeGenMesa(false, false, (new BiomeGenBase.BiomeProperties("Mesa Plateau")).setBaseHeight(1.5F).setHeightVariation(0.025F).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(127, "void", new BiomeVoid((new BiomeGenBase.BiomeProperties("The Void")).setRainDisabled()));
        registerBiome(129, "mutated_plains", new BiomeGenPlains(true, (new BiomeGenBase.BiomeProperties("Sunflower Plains")).setBaseBiome("plains").setBaseHeight(0.125F).setHeightVariation(0.05F).setTemperature(0.8F).setRainfall(0.4F)));
        registerBiome(130, "mutated_desert", new BiomeGenDesert((new BiomeGenBase.BiomeProperties("Desert M")).setBaseBiome("desert").setBaseHeight(0.225F).setHeightVariation(0.25F).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(131, "mutated_extreme_hills", new BiomeGenHills(BiomeGenHills.Type.MUTATED, (new BiomeGenBase.BiomeProperties("Extreme Hills M")).setBaseBiome("extreme_hills").setBaseHeight(1.0F).setHeightVariation(0.5F).setTemperature(0.2F).setRainfall(0.3F)));
        registerBiome(132, "mutated_forest", new BiomeGenForest(BiomeGenForest.Type.FLOWER, (new BiomeGenBase.BiomeProperties("Flower Forest")).setBaseBiome("forest").setHeightVariation(0.4F).setTemperature(0.7F).setRainfall(0.8F)));
        registerBiome(133, "mutated_taiga", new BiomeGenTaiga(BiomeGenTaiga.Type.NORMAL, (new BiomeGenBase.BiomeProperties("Taiga M")).setBaseBiome("taiga").setBaseHeight(0.3F).setHeightVariation(0.4F).setTemperature(0.25F).setRainfall(0.8F)));
        registerBiome(134, "mutated_swampland", new BiomeGenSwamp((new BiomeGenBase.BiomeProperties("Swampland M")).setBaseBiome("swampland").setBaseHeight(-0.1F).setHeightVariation(0.3F).setTemperature(0.8F).setRainfall(0.9F).setWaterColor(14745518)));
        registerBiome(140, "mutated_ice_flats", new BiomeGenSnow(true, (new BiomeGenBase.BiomeProperties("Ice Plains Spikes")).setBaseBiome("ice_flats").setBaseHeight(0.425F).setHeightVariation(0.45000002F).setTemperature(0.0F).setRainfall(0.5F).setSnowEnabled()));
        registerBiome(149, "mutated_jungle", new BiomeGenJungle(false, (new BiomeGenBase.BiomeProperties("Jungle M")).setBaseBiome("jungle").setBaseHeight(0.2F).setHeightVariation(0.4F).setTemperature(0.95F).setRainfall(0.9F)));
        registerBiome(151, "mutated_jungle_edge", new BiomeGenJungle(true, (new BiomeGenBase.BiomeProperties("JungleEdge M")).setBaseBiome("jungle_edge").setBaseHeight(0.2F).setHeightVariation(0.4F).setTemperature(0.95F).setRainfall(0.8F)));
        registerBiome(155, "mutated_birch_forest", new BiomeGenForestMutated((new BiomeGenBase.BiomeProperties("Birch Forest M")).setBaseBiome("birch_forest").setBaseHeight(0.2F).setHeightVariation(0.4F).setTemperature(0.6F).setRainfall(0.6F)));
        registerBiome(156, "mutated_birch_forest_hills", new BiomeGenForestMutated((new BiomeGenBase.BiomeProperties("Birch Forest Hills M")).setBaseBiome("birch_forest").setBaseHeight(0.55F).setHeightVariation(0.5F).setTemperature(0.6F).setRainfall(0.6F)));
        registerBiome(157, "mutated_roofed_forest", new BiomeGenForest(BiomeGenForest.Type.ROOFED, (new BiomeGenBase.BiomeProperties("Roofed Forest M")).setBaseBiome("roofed_forest").setBaseHeight(0.2F).setHeightVariation(0.4F).setTemperature(0.7F).setRainfall(0.8F)));
        registerBiome(158, "mutated_taiga_cold", new BiomeGenTaiga(BiomeGenTaiga.Type.NORMAL, (new BiomeGenBase.BiomeProperties("Cold Taiga M")).setBaseBiome("taiga_cold").setBaseHeight(0.3F).setHeightVariation(0.4F).setTemperature(-0.5F).setRainfall(0.4F).setSnowEnabled()));
        registerBiome(160, "mutated_redwood_taiga", new BiomeGenTaiga(BiomeGenTaiga.Type.MEGA_SPRUCE, (new BiomeGenBase.BiomeProperties("Mega Spruce Taiga")).setBaseBiome("redwood_taiga").setBaseHeight(0.2F).setHeightVariation(0.2F).setTemperature(0.25F).setRainfall(0.8F)));
        registerBiome(161, "mutated_redwood_taiga_hills", new BiomeGenTaiga(BiomeGenTaiga.Type.MEGA_SPRUCE, (new BiomeGenBase.BiomeProperties("Redwood Taiga Hills M")).setBaseBiome("redwood_taiga_hills").setBaseHeight(0.2F).setHeightVariation(0.2F).setTemperature(0.25F).setRainfall(0.8F)));
        registerBiome(162, "mutated_extreme_hills_with_trees", new BiomeGenHills(BiomeGenHills.Type.MUTATED, (new BiomeGenBase.BiomeProperties("Extreme Hills+ M")).setBaseBiome("extreme_hills_with_trees").setBaseHeight(1.0F).setHeightVariation(0.5F).setTemperature(0.2F).setRainfall(0.3F)));
        registerBiome(163, "mutated_savanna", new BiomeGenSavannaMutated((new BiomeGenBase.BiomeProperties("Savanna M")).setBaseBiome("savanna").setBaseHeight(0.3625F).setHeightVariation(1.225F).setTemperature(1.1F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(164, "mutated_savanna_rock", new BiomeGenSavannaMutated((new BiomeGenBase.BiomeProperties("Savanna Plateau M")).setBaseBiome("savanna_rock").setBaseHeight(1.05F).setHeightVariation(1.2125001F).setTemperature(1.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(165, "mutated_mesa", new BiomeGenMesa(true, false, (new BiomeGenBase.BiomeProperties("Mesa (Bryce)")).setBaseBiome("mesa").setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(166, "mutated_mesa_rock", new BiomeGenMesa(false, true, (new BiomeGenBase.BiomeProperties("Mesa Plateau F M")).setBaseBiome("mesa_rock").setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        registerBiome(167, "mutated_mesa_clear_rock", new BiomeGenMesa(false, false, (new BiomeGenBase.BiomeProperties("Mesa Plateau M")).setBaseBiome("mesa_clear_rock").setBaseHeight(0.45F).setHeightVariation(0.3F).setTemperature(2.0F).setRainfall(0.0F).setRainDisabled()));
        Collections.addAll(explorationBiomesList, new BiomeGenBase[] {Biomes.ocean, Biomes.plains, Biomes.desert, Biomes.extremeHills, Biomes.forest, Biomes.taiga, Biomes.swampland, Biomes.river, Biomes.frozenRiver, Biomes.icePlains, Biomes.iceMountains, Biomes.mushroomIsland, Biomes.mushroomIslandShore, Biomes.beach, Biomes.desertHills, Biomes.forestHills, Biomes.taigaHills, Biomes.jungle, Biomes.jungleHills, Biomes.jungleEdge, Biomes.deepOcean, Biomes.stoneBeach, Biomes.coldBeach, Biomes.birchForest, Biomes.birchForestHills, Biomes.roofedForest, Biomes.coldTaiga, Biomes.coldTaigaHills, Biomes.megaTaiga, Biomes.megaTaigaHills, Biomes.extremeHillsPlus, Biomes.savanna, Biomes.savannaPlateau, Biomes.mesa, Biomes.mesaPlateau_F, Biomes.mesaPlateau});
    }

    /**
     * Registers a new biome into the registry.
     *  
     * @param id The numeric Id for the biome.
     * @param name The name to register the biome under.
     * @param biome The biome to register.
     */
    private static void registerBiome(int id, String name, BiomeGenBase biome)
    {
        biomeRegistry.register(id, new ResourceLocation(name), biome);

        if (biome.isMutation())
        {
            MUTATION_TO_BASE_ID_MAP.put(biome, getIdForBiome((BiomeGenBase)biomeRegistry.getObject(new ResourceLocation(biome.baseBiomeRegName))));
        }
    }

    public static class BiomeProperties
    {
        private final String biomeName;
        private float baseHeight = 0.1F;
        private float heightVariation = 0.2F;
        private float temperature = 0.5F;
        private float rainfall = 0.5F;
        private int waterColor = 16777215;
        private boolean enableSnow;
        private boolean enableRain = true;
        private String baseBiomeRegName;

        public BiomeProperties(String nameIn)
        {
            this.biomeName = nameIn;
        }

        protected BiomeGenBase.BiomeProperties setTemperature(float temperatureIn)
        {
            if (temperatureIn > 0.1F && temperatureIn < 0.2F)
            {
                throw new IllegalArgumentException("Please avoid temperatures in the range 0.1 - 0.2 because of snow");
            }
            else
            {
                this.temperature = temperatureIn;
                return this;
            }
        }

        protected BiomeGenBase.BiomeProperties setRainfall(float rainfallIn)
        {
            this.rainfall = rainfallIn;
            return this;
        }

        protected BiomeGenBase.BiomeProperties setBaseHeight(float baseHeightIn)
        {
            this.baseHeight = baseHeightIn;
            return this;
        }

        protected BiomeGenBase.BiomeProperties setHeightVariation(float heightVariationIn)
        {
            this.heightVariation = heightVariationIn;
            return this;
        }

        protected BiomeGenBase.BiomeProperties setRainDisabled()
        {
            this.enableRain = false;
            return this;
        }

        protected BiomeGenBase.BiomeProperties setSnowEnabled()
        {
            this.enableSnow = true;
            return this;
        }

        protected BiomeGenBase.BiomeProperties setWaterColor(int waterColorIn)
        {
            this.waterColor = waterColorIn;
            return this;
        }

        protected BiomeGenBase.BiomeProperties setBaseBiome(String nameIn)
        {
            this.baseBiomeRegName = nameIn;
            return this;
        }
    }

    public static class SpawnListEntry extends WeightedRandom.Item
    {
        public Class <? extends EntityLiving > entityClass;
        public int minGroupCount;
        public int maxGroupCount;

        public SpawnListEntry(Class <? extends EntityLiving > entityclassIn, int weight, int groupCountMin, int groupCountMax)
        {
            super(weight);
            this.entityClass = entityclassIn;
            this.minGroupCount = groupCountMin;
            this.maxGroupCount = groupCountMax;
        }

        public String toString()
        {
            return this.entityClass.getSimpleName() + "*(" + this.minGroupCount + "-" + this.maxGroupCount + "):" + this.itemWeight;
        }
    }

    public static enum TempCategory
    {
        OCEAN,
        COLD,
        MEDIUM,
        WARM;
    }
}
