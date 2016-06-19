package net.minecraft.world.gen.structure;

import com.google.common.collect.Lists;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Map.Entry;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.ChunkCoordIntPair;
import net.minecraft.world.World;
import net.minecraft.world.biome.BiomeGenBase;

public class MapGenStronghold extends MapGenStructure
{
    private List<BiomeGenBase> field_151546_e;

    /**
     * is spawned false and set true once the defined BiomeGenBases were compared with the present ones
     */
    private boolean ranBiomeCheck;
    private ChunkCoordIntPair[] structureCoords;
    private double field_82671_h;
    private int field_82672_i;

    public MapGenStronghold()
    {
        this.structureCoords = new ChunkCoordIntPair[128];
        this.field_82671_h = 32.0D;
        this.field_82672_i = 3;
        this.field_151546_e = Lists.<BiomeGenBase>newArrayList();

        for (BiomeGenBase biomegenbase : BiomeGenBase.biomeRegistry)
        {
            if (biomegenbase != null && biomegenbase.getBaseHeight() > 0.0F)
            {
                this.field_151546_e.add(biomegenbase);
            }
        }
    }

    public MapGenStronghold(Map<String, String> p_i2068_1_)
    {
        this();

        for (Entry<String, String> entry : p_i2068_1_.entrySet())
        {
            if (((String)entry.getKey()).equals("distance"))
            {
                this.field_82671_h = MathHelper.parseDoubleWithDefaultAndMax((String)entry.getValue(), this.field_82671_h, 1.0D);
            }
            else if (((String)entry.getKey()).equals("count"))
            {
                this.structureCoords = new ChunkCoordIntPair[MathHelper.parseIntWithDefaultAndMax((String)entry.getValue(), this.structureCoords.length, 1)];
            }
            else if (((String)entry.getKey()).equals("spread"))
            {
                this.field_82672_i = MathHelper.parseIntWithDefaultAndMax((String)entry.getValue(), this.field_82672_i, 1);
            }
        }
    }

    public String getStructureName()
    {
        return "Stronghold";
    }

    public BlockPos getClosestStrongholdPos(World worldIn, BlockPos pos)
    {
        if (!this.ranBiomeCheck)
        {
            this.func_189104_c();
            this.ranBiomeCheck = true;
        }

        BlockPos blockpos = null;
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos(0, 0, 0);
        double d0 = Double.MAX_VALUE;

        for (ChunkCoordIntPair chunkcoordintpair : this.structureCoords)
        {
            blockpos$mutableblockpos.set((chunkcoordintpair.chunkXPos << 4) + 8, 32, (chunkcoordintpair.chunkZPos << 4) + 8);
            double d1 = blockpos$mutableblockpos.distanceSq(pos);

            if (blockpos == null)
            {
                blockpos = new BlockPos(blockpos$mutableblockpos);
                d0 = d1;
            }
            else if (d1 < d0)
            {
                blockpos = new BlockPos(blockpos$mutableblockpos);
                d0 = d1;
            }
        }

        return blockpos;
    }

    protected boolean canSpawnStructureAtCoords(int chunkX, int chunkZ)
    {
        if (!this.ranBiomeCheck)
        {
            this.func_189104_c();
            this.ranBiomeCheck = true;
        }

        for (ChunkCoordIntPair chunkcoordintpair : this.structureCoords)
        {
            if (chunkX == chunkcoordintpair.chunkXPos && chunkZ == chunkcoordintpair.chunkZPos)
            {
                return true;
            }
        }

        return false;
    }

    private void func_189104_c()
    {
        this.initializeStructureData(this.worldObj);
        int i = 0;

        for (StructureStart structurestart : this.structureMap.values())
        {
            if (i < this.structureCoords.length)
            {
                this.structureCoords[i++] = new ChunkCoordIntPair(structurestart.getChunkPosX(), structurestart.getChunkPosZ());
            }
        }

        Random random = new Random();
        random.setSeed(this.worldObj.getSeed());
        double d1 = random.nextDouble() * Math.PI * 2.0D;
        int j = 0;
        int k = 0;
        int l = this.structureMap.size();

        if (l < this.structureCoords.length)
        {
            for (int i1 = 0; i1 < this.structureCoords.length; ++i1)
            {
                double d0 = 4.0D * this.field_82671_h + this.field_82671_h * (double)j * 6.0D + (random.nextDouble() - 0.5D) * this.field_82671_h * 2.5D;
                int j1 = (int)Math.round(Math.cos(d1) * d0);
                int k1 = (int)Math.round(Math.sin(d1) * d0);
                BlockPos blockpos = this.worldObj.getWorldChunkManager().findBiomePosition((j1 << 4) + 8, (k1 << 4) + 8, 112, this.field_151546_e, random);

                if (blockpos != null)
                {
                    j1 = blockpos.getX() >> 4;
                    k1 = blockpos.getZ() >> 4;
                }

                if (i1 >= l)
                {
                    this.structureCoords[i1] = new ChunkCoordIntPair(j1, k1);
                }

                d1 += (Math.PI * 2D) / (double)this.field_82672_i;
                ++k;

                if (k == this.field_82672_i)
                {
                    ++j;
                    k = 0;
                    this.field_82672_i += 2 * this.field_82672_i / (j + 1);
                    this.field_82672_i = Math.min(this.field_82672_i, this.structureCoords.length - i1);
                    d1 += random.nextDouble() * Math.PI * 2.0D;
                }
            }
        }
    }

    protected List<BlockPos> getCoordList()
    {
        List<BlockPos> list = Lists.<BlockPos>newArrayList();

        for (ChunkCoordIntPair chunkcoordintpair : this.structureCoords)
        {
            if (chunkcoordintpair != null)
            {
                list.add(chunkcoordintpair.getCenterBlock(64));
            }
        }

        return list;
    }

    protected StructureStart getStructureStart(int chunkX, int chunkZ)
    {
        MapGenStronghold.Start mapgenstronghold$start;

        for (mapgenstronghold$start = new MapGenStronghold.Start(this.worldObj, this.rand, chunkX, chunkZ); mapgenstronghold$start.func_186161_c().isEmpty() || ((StructureStrongholdPieces.Stairs2)mapgenstronghold$start.func_186161_c().get(0)).strongholdPortalRoom == null; mapgenstronghold$start = new MapGenStronghold.Start(this.worldObj, this.rand, chunkX, chunkZ))
        {
            ;
        }

        return mapgenstronghold$start;
    }

    public static class Start extends StructureStart
    {
        public Start()
        {
        }

        public Start(World worldIn, Random p_i2067_2_, int p_i2067_3_, int p_i2067_4_)
        {
            super(p_i2067_3_, p_i2067_4_);
            StructureStrongholdPieces.prepareStructurePieces();
            StructureStrongholdPieces.Stairs2 structurestrongholdpieces$stairs2 = new StructureStrongholdPieces.Stairs2(0, p_i2067_2_, (p_i2067_3_ << 4) + 2, (p_i2067_4_ << 4) + 2);
            this.components.add(structurestrongholdpieces$stairs2);
            structurestrongholdpieces$stairs2.buildComponent(structurestrongholdpieces$stairs2, this.components, p_i2067_2_);
            List<StructureComponent> list = structurestrongholdpieces$stairs2.field_75026_c;

            while (!list.isEmpty())
            {
                int i = p_i2067_2_.nextInt(list.size());
                StructureComponent structurecomponent = (StructureComponent)list.remove(i);
                structurecomponent.buildComponent(structurestrongholdpieces$stairs2, this.components, p_i2067_2_);
            }

            this.updateBoundingBox();
            this.markAvailableHeight(worldIn, p_i2067_2_, 10);
        }
    }
}
