package net.minecraft.world.biome;

import java.util.Random;
import net.minecraft.block.BlockDoublePlant;
import net.minecraft.block.BlockFlower;
import net.minecraft.entity.passive.EntityRabbit;
import net.minecraft.entity.passive.EntityWolf;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;
import net.minecraft.world.gen.feature.WorldGenAbstractTree;
import net.minecraft.world.gen.feature.WorldGenBigMushroom;
import net.minecraft.world.gen.feature.WorldGenBirchTree;
import net.minecraft.world.gen.feature.WorldGenCanopyTree;

public class BiomeGenForest extends BiomeGenBase
{
    protected static final WorldGenBirchTree field_150629_aC = new WorldGenBirchTree(false, true);
    protected static final WorldGenBirchTree field_150630_aD = new WorldGenBirchTree(false, false);
    protected static final WorldGenCanopyTree field_150631_aE = new WorldGenCanopyTree(false);
    private BiomeGenForest.Type field_150632_aF;

    public BiomeGenForest(BiomeGenForest.Type p_i46708_1_, BiomeGenBase.BiomeProperties properties)
    {
        super(properties);
        this.field_150632_aF = p_i46708_1_;
        this.theBiomeDecorator.treesPerChunk = 10;
        this.theBiomeDecorator.grassPerChunk = 2;

        if (this.field_150632_aF == BiomeGenForest.Type.FLOWER)
        {
            this.theBiomeDecorator.treesPerChunk = 6;
            this.theBiomeDecorator.flowersPerChunk = 100;
            this.theBiomeDecorator.grassPerChunk = 1;
            this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityRabbit.class, 4, 2, 3));
        }

        if (this.field_150632_aF == BiomeGenForest.Type.NORMAL)
        {
            this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityWolf.class, 5, 4, 4));
        }

        if (this.field_150632_aF == BiomeGenForest.Type.ROOFED)
        {
            this.theBiomeDecorator.treesPerChunk = -999;
        }
    }

    public WorldGenAbstractTree genBigTreeChance(Random rand)
    {
        return (WorldGenAbstractTree)(this.field_150632_aF == BiomeGenForest.Type.ROOFED && rand.nextInt(3) > 0 ? field_150631_aE : (this.field_150632_aF != BiomeGenForest.Type.BIRCH && rand.nextInt(5) != 0 ? (rand.nextInt(10) == 0 ? worldGeneratorBigTree : worldGeneratorTrees) : field_150630_aD));
    }

    public BlockFlower.EnumFlowerType pickRandomFlower(Random rand, BlockPos pos)
    {
        if (this.field_150632_aF == BiomeGenForest.Type.FLOWER)
        {
            double d0 = MathHelper.clamp_double((1.0D + GRASS_COLOR_NOISE.func_151601_a((double)pos.getX() / 48.0D, (double)pos.getZ() / 48.0D)) / 2.0D, 0.0D, 0.9999D);
            BlockFlower.EnumFlowerType blockflower$enumflowertype = BlockFlower.EnumFlowerType.values()[(int)(d0 * (double)BlockFlower.EnumFlowerType.values().length)];
            return blockflower$enumflowertype == BlockFlower.EnumFlowerType.BLUE_ORCHID ? BlockFlower.EnumFlowerType.POPPY : blockflower$enumflowertype;
        }
        else
        {
            return super.pickRandomFlower(rand, pos);
        }
    }

    public void decorate(World worldIn, Random rand, BlockPos pos)
    {
        if (this.field_150632_aF == BiomeGenForest.Type.ROOFED)
        {
            this.func_185379_b(worldIn, rand, pos);
        }

        int i = rand.nextInt(5) - 3;

        if (this.field_150632_aF == BiomeGenForest.Type.FLOWER)
        {
            i += 2;
        }

        this.func_185378_a(worldIn, rand, pos, i);
        super.decorate(worldIn, rand, pos);
    }

    protected void func_185379_b(World p_185379_1_, Random p_185379_2_, BlockPos p_185379_3_)
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                int k = i * 4 + 1 + 8 + p_185379_2_.nextInt(3);
                int l = j * 4 + 1 + 8 + p_185379_2_.nextInt(3);
                BlockPos blockpos = p_185379_1_.getHeight(p_185379_3_.add(k, 0, l));

                if (p_185379_2_.nextInt(20) == 0)
                {
                    WorldGenBigMushroom worldgenbigmushroom = new WorldGenBigMushroom();
                    worldgenbigmushroom.generate(p_185379_1_, p_185379_2_, blockpos);
                }
                else
                {
                    WorldGenAbstractTree worldgenabstracttree = this.genBigTreeChance(p_185379_2_);
                    worldgenabstracttree.func_175904_e();

                    if (worldgenabstracttree.generate(p_185379_1_, p_185379_2_, blockpos))
                    {
                        worldgenabstracttree.func_180711_a(p_185379_1_, p_185379_2_, blockpos);
                    }
                }
            }
        }
    }

    protected void func_185378_a(World p_185378_1_, Random p_185378_2_, BlockPos p_185378_3_, int p_185378_4_)
    {
        for (int i = 0; i < p_185378_4_; ++i)
        {
            int j = p_185378_2_.nextInt(3);

            if (j == 0)
            {
                DOUBLE_PLANT_GENERATOR.setPlantType(BlockDoublePlant.EnumPlantType.SYRINGA);
            }
            else if (j == 1)
            {
                DOUBLE_PLANT_GENERATOR.setPlantType(BlockDoublePlant.EnumPlantType.ROSE);
            }
            else if (j == 2)
            {
                DOUBLE_PLANT_GENERATOR.setPlantType(BlockDoublePlant.EnumPlantType.PAEONIA);
            }

            for (int k = 0; k < 5; ++k)
            {
                int l = p_185378_2_.nextInt(16) + 8;
                int i1 = p_185378_2_.nextInt(16) + 8;
                int j1 = p_185378_2_.nextInt(p_185378_1_.getHeight(p_185378_3_.add(l, 0, i1)).getY() + 32);

                if (DOUBLE_PLANT_GENERATOR.generate(p_185378_1_, p_185378_2_, new BlockPos(p_185378_3_.getX() + l, j1, p_185378_3_.getZ() + i1)))
                {
                    break;
                }
            }
        }
    }

    public Class <? extends BiomeGenBase > getBiomeClass()
    {
        return BiomeGenForest.class;
    }

    public int getGrassColorAtPos(BlockPos pos)
    {
        int i = super.getGrassColorAtPos(pos);
        return this.field_150632_aF == BiomeGenForest.Type.ROOFED ? (i & 16711422) + 2634762 >> 1 : i;
    }

    public static enum Type
    {
        NORMAL,
        FLOWER,
        BIRCH,
        ROOFED;
    }
}
