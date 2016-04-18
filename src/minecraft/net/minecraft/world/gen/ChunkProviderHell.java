package net.minecraft.world.gen;

import java.util.List;
import java.util.Random;
import net.minecraft.block.BlockFalling;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.block.state.pattern.BlockMatcher;
import net.minecraft.entity.EnumCreatureType;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.ChunkCoordIntPair;
import net.minecraft.world.World;
import net.minecraft.world.biome.BiomeGenBase;
import net.minecraft.world.chunk.Chunk;
import net.minecraft.world.chunk.ChunkPrimer;
import net.minecraft.world.chunk.IChunkGenerator;
import net.minecraft.world.gen.feature.WorldGenBush;
import net.minecraft.world.gen.feature.WorldGenFire;
import net.minecraft.world.gen.feature.WorldGenGlowStone1;
import net.minecraft.world.gen.feature.WorldGenGlowStone2;
import net.minecraft.world.gen.feature.WorldGenHellLava;
import net.minecraft.world.gen.feature.WorldGenMinable;
import net.minecraft.world.gen.feature.WorldGenerator;
import net.minecraft.world.gen.structure.MapGenNetherBridge;

public class ChunkProviderHell implements IChunkGenerator
{
    protected static final IBlockState AIR_DEFAULT_STATE = Blocks.air.getDefaultState();
    protected static final IBlockState NETHERRACK_DEFAULT_STATE = Blocks.netherrack.getDefaultState();
    protected static final IBlockState BEDROCK_DEFAULT_STATE = Blocks.bedrock.getDefaultState();
    protected static final IBlockState LAVA_DEFAULT_STATE = Blocks.lava.getDefaultState();
    protected static final IBlockState GRAVEL_DEFAULT_STATE = Blocks.gravel.getDefaultState();
    protected static final IBlockState SOUL_SAND_DEFAULT_STATE = Blocks.soul_sand.getDefaultState();
    private final World world;
    private final boolean field_185953_o;
    private final Random rand;

    /**
     * Holds the noise used to determine whether slowsand can be generated at a location
     */
    private double[] slowsandNoise = new double[256];
    private double[] gravelNoise = new double[256];
    private double[] field_185955_s = new double[256];
    private double[] field_185956_t;
    private final NoiseGeneratorOctaves field_185957_u;
    private final NoiseGeneratorOctaves field_185958_v;
    private final NoiseGeneratorOctaves field_185959_w;

    /** Determines whether slowsand or gravel can be generated at a location */
    private final NoiseGeneratorOctaves slowsandGravelNoiseGen;

    /**
     * Determines whether something other than nettherack can be generated at a location
     */
    private final NoiseGeneratorOctaves netherrackExculsivityNoiseGen;
    public final NoiseGeneratorOctaves field_185946_g;
    public final NoiseGeneratorOctaves field_185947_h;
    private final WorldGenFire field_177470_t = new WorldGenFire();
    private final WorldGenGlowStone1 field_177469_u = new WorldGenGlowStone1();
    private final WorldGenGlowStone2 field_177468_v = new WorldGenGlowStone2();
    private final WorldGenerator field_177467_w = new WorldGenMinable(Blocks.quartz_ore.getDefaultState(), 14, BlockMatcher.forBlock(Blocks.netherrack));
    private final WorldGenHellLava field_177473_x = new WorldGenHellLava(Blocks.flowing_lava, true);
    private final WorldGenHellLava field_177472_y = new WorldGenHellLava(Blocks.flowing_lava, false);
    private final WorldGenBush field_177471_z = new WorldGenBush(Blocks.brown_mushroom);
    private final WorldGenBush field_177465_A = new WorldGenBush(Blocks.red_mushroom);
    private final MapGenNetherBridge genNetherBridge = new MapGenNetherBridge();
    private final MapGenBase field_185939_I = new MapGenCavesHell();
    double[] field_185948_i;
    double[] field_185949_j;
    double[] field_185950_k;
    double[] noiseData4;
    double[] field_185951_m;

    public ChunkProviderHell(World worldIn, boolean p_i45637_2_, long seed)
    {
        this.world = worldIn;
        this.field_185953_o = p_i45637_2_;
        this.rand = new Random(seed);
        this.field_185957_u = new NoiseGeneratorOctaves(this.rand, 16);
        this.field_185958_v = new NoiseGeneratorOctaves(this.rand, 16);
        this.field_185959_w = new NoiseGeneratorOctaves(this.rand, 8);
        this.slowsandGravelNoiseGen = new NoiseGeneratorOctaves(this.rand, 4);
        this.netherrackExculsivityNoiseGen = new NoiseGeneratorOctaves(this.rand, 4);
        this.field_185946_g = new NoiseGeneratorOctaves(this.rand, 10);
        this.field_185947_h = new NoiseGeneratorOctaves(this.rand, 16);
        worldIn.setSeaLevel(63);
    }

    public void func_185936_a(int p_185936_1_, int p_185936_2_, ChunkPrimer p_185936_3_)
    {
        int i = 4;
        int j = this.world.getSeaLevel() / 2 + 1;
        int k = i + 1;
        int l = 17;
        int i1 = i + 1;
        this.field_185956_t = this.func_185938_a(this.field_185956_t, p_185936_1_ * i, 0, p_185936_2_ * i, k, l, i1);

        for (int j1 = 0; j1 < i; ++j1)
        {
            for (int k1 = 0; k1 < i; ++k1)
            {
                for (int l1 = 0; l1 < 16; ++l1)
                {
                    double d0 = 0.125D;
                    double d1 = this.field_185956_t[((j1 + 0) * i1 + k1 + 0) * l + l1 + 0];
                    double d2 = this.field_185956_t[((j1 + 0) * i1 + k1 + 1) * l + l1 + 0];
                    double d3 = this.field_185956_t[((j1 + 1) * i1 + k1 + 0) * l + l1 + 0];
                    double d4 = this.field_185956_t[((j1 + 1) * i1 + k1 + 1) * l + l1 + 0];
                    double d5 = (this.field_185956_t[((j1 + 0) * i1 + k1 + 0) * l + l1 + 1] - d1) * d0;
                    double d6 = (this.field_185956_t[((j1 + 0) * i1 + k1 + 1) * l + l1 + 1] - d2) * d0;
                    double d7 = (this.field_185956_t[((j1 + 1) * i1 + k1 + 0) * l + l1 + 1] - d3) * d0;
                    double d8 = (this.field_185956_t[((j1 + 1) * i1 + k1 + 1) * l + l1 + 1] - d4) * d0;

                    for (int i2 = 0; i2 < 8; ++i2)
                    {
                        double d9 = 0.25D;
                        double d10 = d1;
                        double d11 = d2;
                        double d12 = (d3 - d1) * d9;
                        double d13 = (d4 - d2) * d9;

                        for (int j2 = 0; j2 < 4; ++j2)
                        {
                            double d14 = 0.25D;
                            double d15 = d10;
                            double d16 = (d11 - d10) * d14;

                            for (int k2 = 0; k2 < 4; ++k2)
                            {
                                IBlockState iblockstate = null;

                                if (l1 * 8 + i2 < j)
                                {
                                    iblockstate = LAVA_DEFAULT_STATE;
                                }

                                if (d15 > 0.0D)
                                {
                                    iblockstate = NETHERRACK_DEFAULT_STATE;
                                }

                                int l2 = j2 + j1 * 4;
                                int i3 = i2 + l1 * 8;
                                int j3 = k2 + k1 * 4;
                                p_185936_3_.setBlockState(l2, i3, j3, iblockstate);
                                d15 += d16;
                            }

                            d10 += d12;
                            d11 += d13;
                        }

                        d1 += d5;
                        d2 += d6;
                        d3 += d7;
                        d4 += d8;
                    }
                }
            }
        }
    }

    public void func_185937_b(int p_185937_1_, int p_185937_2_, ChunkPrimer p_185937_3_)
    {
        int i = this.world.getSeaLevel() + 1;
        double d0 = 0.03125D;
        this.slowsandNoise = this.slowsandGravelNoiseGen.generateNoiseOctaves(this.slowsandNoise, p_185937_1_ * 16, p_185937_2_ * 16, 0, 16, 16, 1, d0, d0, 1.0D);
        this.gravelNoise = this.slowsandGravelNoiseGen.generateNoiseOctaves(this.gravelNoise, p_185937_1_ * 16, 109, p_185937_2_ * 16, 16, 1, 16, d0, 1.0D, d0);
        this.field_185955_s = this.netherrackExculsivityNoiseGen.generateNoiseOctaves(this.field_185955_s, p_185937_1_ * 16, p_185937_2_ * 16, 0, 16, 16, 1, d0 * 2.0D, d0 * 2.0D, d0 * 2.0D);

        for (int j = 0; j < 16; ++j)
        {
            for (int k = 0; k < 16; ++k)
            {
                boolean flag = this.slowsandNoise[j + k * 16] + this.rand.nextDouble() * 0.2D > 0.0D;
                boolean flag1 = this.gravelNoise[j + k * 16] + this.rand.nextDouble() * 0.2D > 0.0D;
                int l = (int)(this.field_185955_s[j + k * 16] / 3.0D + 3.0D + this.rand.nextDouble() * 0.25D);
                int i1 = -1;
                IBlockState iblockstate = NETHERRACK_DEFAULT_STATE;
                IBlockState iblockstate1 = NETHERRACK_DEFAULT_STATE;

                for (int j1 = 127; j1 >= 0; --j1)
                {
                    if (j1 < 127 - this.rand.nextInt(5) && j1 > this.rand.nextInt(5))
                    {
                        IBlockState iblockstate2 = p_185937_3_.getBlockState(k, j1, j);

                        if (iblockstate2.getBlock() != null && iblockstate2.getMaterial() != Material.air)
                        {
                            if (iblockstate2.getBlock() == Blocks.netherrack)
                            {
                                if (i1 == -1)
                                {
                                    if (l <= 0)
                                    {
                                        iblockstate = AIR_DEFAULT_STATE;
                                        iblockstate1 = NETHERRACK_DEFAULT_STATE;
                                    }
                                    else if (j1 >= i - 4 && j1 <= i + 1)
                                    {
                                        iblockstate = NETHERRACK_DEFAULT_STATE;
                                        iblockstate1 = NETHERRACK_DEFAULT_STATE;

                                        if (flag1)
                                        {
                                            iblockstate = GRAVEL_DEFAULT_STATE;
                                            iblockstate1 = NETHERRACK_DEFAULT_STATE;
                                        }

                                        if (flag)
                                        {
                                            iblockstate = SOUL_SAND_DEFAULT_STATE;
                                            iblockstate1 = SOUL_SAND_DEFAULT_STATE;
                                        }
                                    }

                                    if (j1 < i && (iblockstate == null || iblockstate.getMaterial() == Material.air))
                                    {
                                        iblockstate = LAVA_DEFAULT_STATE;
                                    }

                                    i1 = l;

                                    if (j1 >= i - 1)
                                    {
                                        p_185937_3_.setBlockState(k, j1, j, iblockstate);
                                    }
                                    else
                                    {
                                        p_185937_3_.setBlockState(k, j1, j, iblockstate1);
                                    }
                                }
                                else if (i1 > 0)
                                {
                                    --i1;
                                    p_185937_3_.setBlockState(k, j1, j, iblockstate1);
                                }
                            }
                        }
                        else
                        {
                            i1 = -1;
                        }
                    }
                    else
                    {
                        p_185937_3_.setBlockState(k, j1, j, BEDROCK_DEFAULT_STATE);
                    }
                }
            }
        }
    }

    public Chunk provideChunk(int x, int z)
    {
        this.rand.setSeed((long)x * 341873128712L + (long)z * 132897987541L);
        ChunkPrimer chunkprimer = new ChunkPrimer();
        this.func_185936_a(x, z, chunkprimer);
        this.func_185937_b(x, z, chunkprimer);
        this.field_185939_I.func_186125_a(this.world, x, z, chunkprimer);

        if (this.field_185953_o)
        {
            this.genNetherBridge.func_186125_a(this.world, x, z, chunkprimer);
        }

        Chunk chunk = new Chunk(this.world, chunkprimer, x, z);
        BiomeGenBase[] abiomegenbase = this.world.getWorldChunkManager().loadBlockGeneratorData((BiomeGenBase[])null, x * 16, z * 16, 16, 16);
        byte[] abyte = chunk.getBiomeArray();

        for (int i = 0; i < abyte.length; ++i)
        {
            abyte[i] = (byte)BiomeGenBase.getIdForBiome(abiomegenbase[i]);
        }

        chunk.resetRelightChecks();
        return chunk;
    }

    private double[] func_185938_a(double[] p_185938_1_, int p_185938_2_, int p_185938_3_, int p_185938_4_, int p_185938_5_, int p_185938_6_, int p_185938_7_)
    {
        if (p_185938_1_ == null)
        {
            p_185938_1_ = new double[p_185938_5_ * p_185938_6_ * p_185938_7_];
        }

        double d0 = 684.412D;
        double d1 = 2053.236D;
        this.noiseData4 = this.field_185946_g.generateNoiseOctaves(this.noiseData4, p_185938_2_, p_185938_3_, p_185938_4_, p_185938_5_, 1, p_185938_7_, 1.0D, 0.0D, 1.0D);
        this.field_185951_m = this.field_185947_h.generateNoiseOctaves(this.field_185951_m, p_185938_2_, p_185938_3_, p_185938_4_, p_185938_5_, 1, p_185938_7_, 100.0D, 0.0D, 100.0D);
        this.field_185948_i = this.field_185959_w.generateNoiseOctaves(this.field_185948_i, p_185938_2_, p_185938_3_, p_185938_4_, p_185938_5_, p_185938_6_, p_185938_7_, d0 / 80.0D, d1 / 60.0D, d0 / 80.0D);
        this.field_185949_j = this.field_185957_u.generateNoiseOctaves(this.field_185949_j, p_185938_2_, p_185938_3_, p_185938_4_, p_185938_5_, p_185938_6_, p_185938_7_, d0, d1, d0);
        this.field_185950_k = this.field_185958_v.generateNoiseOctaves(this.field_185950_k, p_185938_2_, p_185938_3_, p_185938_4_, p_185938_5_, p_185938_6_, p_185938_7_, d0, d1, d0);
        int i = 0;
        double[] adouble = new double[p_185938_6_];

        for (int j = 0; j < p_185938_6_; ++j)
        {
            adouble[j] = Math.cos((double)j * Math.PI * 6.0D / (double)p_185938_6_) * 2.0D;
            double d2 = (double)j;

            if (j > p_185938_6_ / 2)
            {
                d2 = (double)(p_185938_6_ - 1 - j);
            }

            if (d2 < 4.0D)
            {
                d2 = 4.0D - d2;
                adouble[j] -= d2 * d2 * d2 * 10.0D;
            }
        }

        for (int l = 0; l < p_185938_5_; ++l)
        {
            for (int i1 = 0; i1 < p_185938_7_; ++i1)
            {
                double d3 = 0.0D;

                for (int k = 0; k < p_185938_6_; ++k)
                {
                    double d4 = 0.0D;
                    double d5 = adouble[k];
                    double d6 = this.field_185949_j[i] / 512.0D;
                    double d7 = this.field_185950_k[i] / 512.0D;
                    double d8 = (this.field_185948_i[i] / 10.0D + 1.0D) / 2.0D;

                    if (d8 < 0.0D)
                    {
                        d4 = d6;
                    }
                    else if (d8 > 1.0D)
                    {
                        d4 = d7;
                    }
                    else
                    {
                        d4 = d6 + (d7 - d6) * d8;
                    }

                    d4 = d4 - d5;

                    if (k > p_185938_6_ - 4)
                    {
                        double d9 = (double)((float)(k - (p_185938_6_ - 4)) / 3.0F);
                        d4 = d4 * (1.0D - d9) + -10.0D * d9;
                    }

                    if ((double)k < d3)
                    {
                        double d10 = (d3 - (double)k) / 4.0D;
                        d10 = MathHelper.clamp_double(d10, 0.0D, 1.0D);
                        d4 = d4 * (1.0D - d10) + -10.0D * d10;
                    }

                    p_185938_1_[i] = d4;
                    ++i;
                }
            }
        }

        return p_185938_1_;
    }

    public void populate(int x, int z)
    {
        BlockFalling.fallInstantly = true;
        BlockPos blockpos = new BlockPos(x * 16, 0, z * 16);
        ChunkCoordIntPair chunkcoordintpair = new ChunkCoordIntPair(x, z);
        this.genNetherBridge.generateStructure(this.world, this.rand, chunkcoordintpair);

        for (int i = 0; i < 8; ++i)
        {
            this.field_177472_y.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16) + 8, this.rand.nextInt(120) + 4, this.rand.nextInt(16) + 8));
        }

        for (int j = 0; j < this.rand.nextInt(this.rand.nextInt(10) + 1) + 1; ++j)
        {
            this.field_177470_t.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16) + 8, this.rand.nextInt(120) + 4, this.rand.nextInt(16) + 8));
        }

        for (int k = 0; k < this.rand.nextInt(this.rand.nextInt(10) + 1); ++k)
        {
            this.field_177469_u.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16) + 8, this.rand.nextInt(120) + 4, this.rand.nextInt(16) + 8));
        }

        for (int l = 0; l < 10; ++l)
        {
            this.field_177468_v.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16) + 8, this.rand.nextInt(128), this.rand.nextInt(16) + 8));
        }

        if (this.rand.nextBoolean())
        {
            this.field_177471_z.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16) + 8, this.rand.nextInt(128), this.rand.nextInt(16) + 8));
        }

        if (this.rand.nextBoolean())
        {
            this.field_177465_A.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16) + 8, this.rand.nextInt(128), this.rand.nextInt(16) + 8));
        }

        for (int i1 = 0; i1 < 16; ++i1)
        {
            this.field_177467_w.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16), this.rand.nextInt(108) + 10, this.rand.nextInt(16)));
        }

        for (int j1 = 0; j1 < 16; ++j1)
        {
            this.field_177473_x.generate(this.world, this.rand, blockpos.add(this.rand.nextInt(16), this.rand.nextInt(108) + 10, this.rand.nextInt(16)));
        }

        BlockFalling.fallInstantly = false;
    }

    public boolean func_185933_a(Chunk chunkIn, int x, int z)
    {
        return false;
    }

    public List<BiomeGenBase.SpawnListEntry> getPossibleCreatures(EnumCreatureType creatureType, BlockPos pos)
    {
        if (creatureType == EnumCreatureType.MONSTER)
        {
            if (this.genNetherBridge.func_175795_b(pos))
            {
                return this.genNetherBridge.getSpawnList();
            }

            if (this.genNetherBridge.isPositionInStructure(this.world, pos) && this.world.getBlockState(pos.down()).getBlock() == Blocks.nether_brick)
            {
                return this.genNetherBridge.getSpawnList();
            }
        }

        BiomeGenBase biomegenbase = this.world.getBiomeGenForCoords(pos);
        return biomegenbase.getSpawnableList(creatureType);
    }

    public BlockPos getStrongholdGen(World worldIn, String structureName, BlockPos position)
    {
        return null;
    }

    public void recreateStructures(Chunk chunkIn, int x, int z)
    {
        this.genNetherBridge.func_186125_a(this.world, x, z, (ChunkPrimer)null);
    }
}
