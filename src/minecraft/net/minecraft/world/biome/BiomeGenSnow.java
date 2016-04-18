package net.minecraft.world.biome;

import java.util.Random;
import net.minecraft.entity.passive.EntityRabbit;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.gen.feature.WorldGenAbstractTree;
import net.minecraft.world.gen.feature.WorldGenIcePath;
import net.minecraft.world.gen.feature.WorldGenIceSpike;
import net.minecraft.world.gen.feature.WorldGenTaiga2;

public class BiomeGenSnow extends BiomeGenBase
{
    private boolean field_150615_aC;
    private WorldGenIceSpike field_150616_aD = new WorldGenIceSpike();
    private WorldGenIcePath field_150617_aE = new WorldGenIcePath(4);

    public BiomeGenSnow(boolean p_i46706_1_, BiomeGenBase.BiomeProperties properties)
    {
        super(properties);
        this.field_150615_aC = p_i46706_1_;

        if (p_i46706_1_)
        {
            this.topBlock = Blocks.snow.getDefaultState();
        }

        this.spawnableCreatureList.clear();
        this.spawnableCreatureList.add(new BiomeGenBase.SpawnListEntry(EntityRabbit.class, 4, 2, 3));
    }

    public void decorate(World worldIn, Random rand, BlockPos pos)
    {
        if (this.field_150615_aC)
        {
            for (int i = 0; i < 3; ++i)
            {
                int j = rand.nextInt(16) + 8;
                int k = rand.nextInt(16) + 8;
                this.field_150616_aD.generate(worldIn, rand, worldIn.getHeight(pos.add(j, 0, k)));
            }

            for (int l = 0; l < 2; ++l)
            {
                int i1 = rand.nextInt(16) + 8;
                int j1 = rand.nextInt(16) + 8;
                this.field_150617_aE.generate(worldIn, rand, worldIn.getHeight(pos.add(i1, 0, j1)));
            }
        }

        super.decorate(worldIn, rand, pos);
    }

    public WorldGenAbstractTree genBigTreeChance(Random rand)
    {
        return new WorldGenTaiga2(false);
    }
}
