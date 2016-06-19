package net.minecraft.world.chunk;

import com.google.common.base.Predicate;
import com.google.common.collect.Maps;
import com.google.common.collect.Queues;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentLinkedQueue;
import net.minecraft.block.Block;
import net.minecraft.block.ITileEntityProvider;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.entity.Entity;
import net.minecraft.init.Biomes;
import net.minecraft.init.Blocks;
import net.minecraft.network.PacketBuffer;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.ClassInheritanceMultiMap;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ReportedException;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.ChunkCoordIntPair;
import net.minecraft.world.EnumSkyBlock;
import net.minecraft.world.World;
import net.minecraft.world.WorldType;
import net.minecraft.world.biome.BiomeGenBase;
import net.minecraft.world.biome.BiomeProvider;
import net.minecraft.world.chunk.storage.ExtendedBlockStorage;
import net.minecraft.world.gen.ChunkProviderDebug;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class Chunk
{
    private static final Logger logger = LogManager.getLogger();
    public static final ExtendedBlockStorage NULL_BLOCK_STORAGE = null;

    /**
     * Used to store block IDs, block MSBs, Sky-light maps, Block-light maps, and metadata. Each entry corresponds to a
     * logical segment of 16x16x16 blocks, stacked vertically.
     */
    private final ExtendedBlockStorage[] storageArrays;

    /**
     * Contains a 16x16 mapping on the X/Z plane of the biome ID to which each colum belongs.
     */
    private final byte[] blockBiomeArray;

    /**
     * A map, similar to heightMap, that tracks how far down precipitation can fall.
     */
    private final int[] precipitationHeightMap;

    /** Which columns need their skylightMaps updated. */
    private final boolean[] updateSkylightColumns;

    /** Whether or not this Chunk is currently loaded into the World */
    private boolean isChunkLoaded;

    /** Reference to the World object. */
    private final World worldObj;
    private final int[] heightMap;

    /** The x coordinate of the chunk. */
    public final int xPosition;

    /** The z coordinate of the chunk. */
    public final int zPosition;
    private boolean isGapLightingUpdated;
    private final Map<BlockPos, TileEntity> chunkTileEntityMap;
    private final ClassInheritanceMultiMap<Entity>[] entityLists;

    /** Boolean value indicating if the terrain is populated. */
    private boolean isTerrainPopulated;
    private boolean isLightPopulated;
    private boolean field_150815_m;

    /**
     * Set to true if the chunk has been modified and needs to be updated internally.
     */
    private boolean isModified;

    /**
     * Whether this Chunk has any Entities and thus requires saving on every tick
     */
    private boolean hasEntities;

    /** The time according to World.worldTime when this chunk was last saved */
    private long lastSaveTime;

    /** Lowest value in the heightmap. */
    private int heightMapMinimum;

    /** the cumulative number of ticks players have been in this chunk */
    private long inhabitedTime;

    /**
     * Contains the current round-robin relight check index, and is implied as the relight check location as well.
     */
    private int queuedLightChecks;
    private ConcurrentLinkedQueue<BlockPos> tileEntityPosQueue;

    public Chunk(World worldIn, int x, int z)
    {
        this.storageArrays = new ExtendedBlockStorage[16];
        this.blockBiomeArray = new byte[256];
        this.precipitationHeightMap = new int[256];
        this.updateSkylightColumns = new boolean[256];
        this.chunkTileEntityMap = Maps.<BlockPos, TileEntity>newHashMap();
        this.queuedLightChecks = 4096;
        this.tileEntityPosQueue = Queues.<BlockPos>newConcurrentLinkedQueue();
        this.entityLists = (ClassInheritanceMultiMap[])(new ClassInheritanceMultiMap[16]);
        this.worldObj = worldIn;
        this.xPosition = x;
        this.zPosition = z;
        this.heightMap = new int[256];

        for (int i = 0; i < this.entityLists.length; ++i)
        {
            this.entityLists[i] = new ClassInheritanceMultiMap(Entity.class);
        }

        Arrays.fill((int[])this.precipitationHeightMap, (int) - 999);
        Arrays.fill(this.blockBiomeArray, (byte) - 1);
    }

    public Chunk(World worldIn, ChunkPrimer primer, int x, int z)
    {
        this(worldIn, x, z);
        int i = 256;
        boolean flag = !worldIn.provider.getHasNoSky();

        for (int j = 0; j < 16; ++j)
        {
            for (int k = 0; k < 16; ++k)
            {
                for (int l = 0; l < i; ++l)
                {
                    IBlockState iblockstate = primer.getBlockState(j, l, k);

                    if (iblockstate.getMaterial() != Material.air)
                    {
                        int i1 = l >> 4;

                        if (this.storageArrays[i1] == NULL_BLOCK_STORAGE)
                        {
                            this.storageArrays[i1] = new ExtendedBlockStorage(i1 << 4, flag);
                        }

                        this.storageArrays[i1].set(j, l & 15, k, iblockstate);
                    }
                }
            }
        }
    }

    /**
     * Checks whether the chunk is at the X/Z location specified
     */
    public boolean isAtLocation(int x, int z)
    {
        return x == this.xPosition && z == this.zPosition;
    }

    public int getHeight(BlockPos pos)
    {
        return this.getHeightValue(pos.getX() & 15, pos.getZ() & 15);
    }

    /**
     * Returns the value in the height map at this x, z coordinate in the chunk
     */
    public int getHeightValue(int x, int z)
    {
        return this.heightMap[z << 4 | x];
    }

    private ExtendedBlockStorage func_186031_y()
    {
        for (int i = this.storageArrays.length - 1; i >= 0; --i)
        {
            if (this.storageArrays[i] != NULL_BLOCK_STORAGE)
            {
                return this.storageArrays[i];
            }
        }

        return null;
    }

    /**
     * Returns the topmost ExtendedBlockStorage instance for this Chunk that actually contains a block.
     */
    public int getTopFilledSegment()
    {
        ExtendedBlockStorage extendedblockstorage = this.func_186031_y();
        return extendedblockstorage == null ? 0 : extendedblockstorage.getYLocation();
    }

    /**
     * Returns the ExtendedBlockStorage array for this Chunk.
     */
    public ExtendedBlockStorage[] getBlockStorageArray()
    {
        return this.storageArrays;
    }

    /**
     * Generates the height map for a chunk from scratch
     */
    protected void generateHeightMap()
    {
        int i = this.getTopFilledSegment();
        this.heightMapMinimum = Integer.MAX_VALUE;

        for (int j = 0; j < 16; ++j)
        {
            for (int k = 0; k < 16; ++k)
            {
                this.precipitationHeightMap[j + (k << 4)] = -999;

                for (int l = i + 16; l > 0; --l)
                {
                    IBlockState iblockstate = this.getBlockState(j, l - 1, k);

                    if (iblockstate.getLightOpacity() != 0)
                    {
                        this.heightMap[k << 4 | j] = l;

                        if (l < this.heightMapMinimum)
                        {
                            this.heightMapMinimum = l;
                        }

                        break;
                    }
                }
            }
        }

        this.isModified = true;
    }

    /**
     * Generates the initial skylight map for the chunk upon generation or load.
     */
    public void generateSkylightMap()
    {
        int i = this.getTopFilledSegment();
        this.heightMapMinimum = Integer.MAX_VALUE;

        for (int j = 0; j < 16; ++j)
        {
            for (int k = 0; k < 16; ++k)
            {
                this.precipitationHeightMap[j + (k << 4)] = -999;

                for (int l = i + 16; l > 0; --l)
                {
                    if (this.getBlockLightOpacity(j, l - 1, k) != 0)
                    {
                        this.heightMap[k << 4 | j] = l;

                        if (l < this.heightMapMinimum)
                        {
                            this.heightMapMinimum = l;
                        }

                        break;
                    }
                }

                if (!this.worldObj.provider.getHasNoSky())
                {
                    int k1 = 15;
                    int i1 = i + 16 - 1;

                    while (true)
                    {
                        int j1 = this.getBlockLightOpacity(j, i1, k);

                        if (j1 == 0 && k1 != 15)
                        {
                            j1 = 1;
                        }

                        k1 -= j1;

                        if (k1 > 0)
                        {
                            ExtendedBlockStorage extendedblockstorage = this.storageArrays[i1 >> 4];

                            if (extendedblockstorage != NULL_BLOCK_STORAGE)
                            {
                                extendedblockstorage.setExtSkylightValue(j, i1 & 15, k, k1);
                                this.worldObj.notifyLightSet(new BlockPos((this.xPosition << 4) + j, i1, (this.zPosition << 4) + k));
                            }
                        }

                        --i1;

                        if (i1 <= 0 || k1 <= 0)
                        {
                            break;
                        }
                    }
                }
            }
        }

        this.isModified = true;
    }

    /**
     * Propagates a given sky-visible block's light value downward and upward to neighboring blocks as necessary.
     */
    private void propagateSkylightOcclusion(int x, int z)
    {
        this.updateSkylightColumns[x + z * 16] = true;
        this.isGapLightingUpdated = true;
    }

    private void recheckGaps(boolean p_150803_1_)
    {
        this.worldObj.theProfiler.startSection("recheckGaps");

        if (this.worldObj.isAreaLoaded(new BlockPos(this.xPosition * 16 + 8, 0, this.zPosition * 16 + 8), 16))
        {
            for (int i = 0; i < 16; ++i)
            {
                for (int j = 0; j < 16; ++j)
                {
                    if (this.updateSkylightColumns[i + j * 16])
                    {
                        this.updateSkylightColumns[i + j * 16] = false;
                        int k = this.getHeightValue(i, j);
                        int l = this.xPosition * 16 + i;
                        int i1 = this.zPosition * 16 + j;
                        int j1 = Integer.MAX_VALUE;

                        for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
                        {
                            j1 = Math.min(j1, this.worldObj.getChunksLowestHorizon(l + enumfacing.getFrontOffsetX(), i1 + enumfacing.getFrontOffsetZ()));
                        }

                        this.checkSkylightNeighborHeight(l, i1, j1);

                        for (EnumFacing enumfacing1 : EnumFacing.Plane.HORIZONTAL)
                        {
                            this.checkSkylightNeighborHeight(l + enumfacing1.getFrontOffsetX(), i1 + enumfacing1.getFrontOffsetZ(), k);
                        }

                        if (p_150803_1_)
                        {
                            this.worldObj.theProfiler.endSection();
                            return;
                        }
                    }
                }
            }

            this.isGapLightingUpdated = false;
        }

        this.worldObj.theProfiler.endSection();
    }

    /**
     * Checks the height of a block next to a sky-visible block and schedules a lighting update as necessary.
     */
    private void checkSkylightNeighborHeight(int x, int z, int maxValue)
    {
        int i = this.worldObj.getHeight(new BlockPos(x, 0, z)).getY();

        if (i > maxValue)
        {
            this.updateSkylightNeighborHeight(x, z, maxValue, i + 1);
        }
        else if (i < maxValue)
        {
            this.updateSkylightNeighborHeight(x, z, i, maxValue + 1);
        }
    }

    private void updateSkylightNeighborHeight(int x, int z, int startY, int endY)
    {
        if (endY > startY && this.worldObj.isAreaLoaded(new BlockPos(x, 0, z), 16))
        {
            for (int i = startY; i < endY; ++i)
            {
                this.worldObj.checkLightFor(EnumSkyBlock.SKY, new BlockPos(x, i, z));
            }

            this.isModified = true;
        }
    }

    /**
     * Initiates the recalculation of both the block-light and sky-light for a given block inside a chunk.
     */
    private void relightBlock(int x, int y, int z)
    {
        int i = this.heightMap[z << 4 | x] & 255;
        int j = i;

        if (y > i)
        {
            j = y;
        }

        while (j > 0 && this.getBlockLightOpacity(x, j - 1, z) == 0)
        {
            --j;
        }

        if (j != i)
        {
            this.worldObj.markBlocksDirtyVertical(x + this.xPosition * 16, z + this.zPosition * 16, j, i);
            this.heightMap[z << 4 | x] = j;
            int k = this.xPosition * 16 + x;
            int l = this.zPosition * 16 + z;

            if (!this.worldObj.provider.getHasNoSky())
            {
                if (j < i)
                {
                    for (int j1 = j; j1 < i; ++j1)
                    {
                        ExtendedBlockStorage extendedblockstorage2 = this.storageArrays[j1 >> 4];

                        if (extendedblockstorage2 != NULL_BLOCK_STORAGE)
                        {
                            extendedblockstorage2.setExtSkylightValue(x, j1 & 15, z, 15);
                            this.worldObj.notifyLightSet(new BlockPos((this.xPosition << 4) + x, j1, (this.zPosition << 4) + z));
                        }
                    }
                }
                else
                {
                    for (int i1 = i; i1 < j; ++i1)
                    {
                        ExtendedBlockStorage extendedblockstorage = this.storageArrays[i1 >> 4];

                        if (extendedblockstorage != NULL_BLOCK_STORAGE)
                        {
                            extendedblockstorage.setExtSkylightValue(x, i1 & 15, z, 0);
                            this.worldObj.notifyLightSet(new BlockPos((this.xPosition << 4) + x, i1, (this.zPosition << 4) + z));
                        }
                    }
                }

                int k1 = 15;

                while (j > 0 && k1 > 0)
                {
                    --j;
                    int i2 = this.getBlockLightOpacity(x, j, z);

                    if (i2 == 0)
                    {
                        i2 = 1;
                    }

                    k1 -= i2;

                    if (k1 < 0)
                    {
                        k1 = 0;
                    }

                    ExtendedBlockStorage extendedblockstorage1 = this.storageArrays[j >> 4];

                    if (extendedblockstorage1 != NULL_BLOCK_STORAGE)
                    {
                        extendedblockstorage1.setExtSkylightValue(x, j & 15, z, k1);
                    }
                }
            }

            int l1 = this.heightMap[z << 4 | x];
            int j2 = i;
            int k2 = l1;

            if (l1 < i)
            {
                j2 = l1;
                k2 = i;
            }

            if (l1 < this.heightMapMinimum)
            {
                this.heightMapMinimum = l1;
            }

            if (!this.worldObj.provider.getHasNoSky())
            {
                for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
                {
                    this.updateSkylightNeighborHeight(k + enumfacing.getFrontOffsetX(), l + enumfacing.getFrontOffsetZ(), j2, k2);
                }

                this.updateSkylightNeighborHeight(k, l, j2, k2);
            }

            this.isModified = true;
        }
    }

    public int getBlockLightOpacity(BlockPos pos)
    {
        return this.getBlockState(pos).getLightOpacity();
    }

    private int getBlockLightOpacity(int x, int y, int z)
    {
        return this.getBlockState(x, y, z).getLightOpacity();
    }

    public IBlockState getBlockState(BlockPos pos)
    {
        return this.getBlockState(pos.getX(), pos.getY(), pos.getZ());
    }

    public IBlockState getBlockState(final int p_186032_1_, final int p_186032_2_, final int p_186032_3_)
    {
        if (this.worldObj.getWorldType() == WorldType.DEBUG_WORLD)
        {
            IBlockState iblockstate = null;

            if (p_186032_2_ == 60)
            {
                iblockstate = Blocks.barrier.getDefaultState();
            }

            if (p_186032_2_ == 70)
            {
                iblockstate = ChunkProviderDebug.func_177461_b(p_186032_1_, p_186032_3_);
            }

            return iblockstate == null ? Blocks.air.getDefaultState() : iblockstate;
        }
        else
        {
            try
            {
                if (p_186032_2_ >= 0 && p_186032_2_ >> 4 < this.storageArrays.length)
                {
                    ExtendedBlockStorage extendedblockstorage = this.storageArrays[p_186032_2_ >> 4];

                    if (extendedblockstorage != NULL_BLOCK_STORAGE)
                    {
                        return extendedblockstorage.get(p_186032_1_ & 15, p_186032_2_ & 15, p_186032_3_ & 15);
                    }
                }

                return Blocks.air.getDefaultState();
            }
            catch (Throwable throwable)
            {
                CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Getting block state");
                CrashReportCategory crashreportcategory = crashreport.makeCategory("Block being got");
                crashreportcategory.addCrashSectionCallable("Location", new Callable<String>()
                {
                    public String call() throws Exception
                    {
                        return CrashReportCategory.func_184876_a(p_186032_1_, p_186032_2_, p_186032_3_);
                    }
                });
                throw new ReportedException(crashreport);
            }
        }
    }

    public IBlockState setBlockState(BlockPos pos, IBlockState state)
    {
        int i = pos.getX() & 15;
        int j = pos.getY();
        int k = pos.getZ() & 15;
        int l = k << 4 | i;

        if (j >= this.precipitationHeightMap[l] - 1)
        {
            this.precipitationHeightMap[l] = -999;
        }

        int i1 = this.heightMap[l];
        IBlockState iblockstate = this.getBlockState(pos);

        if (iblockstate == state)
        {
            return null;
        }
        else
        {
            Block block = state.getBlock();
            Block block1 = iblockstate.getBlock();
            ExtendedBlockStorage extendedblockstorage = this.storageArrays[j >> 4];
            boolean flag = false;

            if (extendedblockstorage == NULL_BLOCK_STORAGE)
            {
                if (block == Blocks.air)
                {
                    return null;
                }

                extendedblockstorage = this.storageArrays[j >> 4] = new ExtendedBlockStorage(j >> 4 << 4, !this.worldObj.provider.getHasNoSky());
                flag = j >= i1;
            }

            extendedblockstorage.set(i, j & 15, k, state);

            if (block1 != block)
            {
                if (!this.worldObj.isRemote)
                {
                    block1.breakBlock(this.worldObj, pos, iblockstate);
                }
                else if (block1 instanceof ITileEntityProvider)
                {
                    this.worldObj.removeTileEntity(pos);
                }
            }

            if (extendedblockstorage.get(i, j & 15, k).getBlock() != block)
            {
                return null;
            }
            else
            {
                if (flag)
                {
                    this.generateSkylightMap();
                }
                else
                {
                    int j1 = state.getLightOpacity();
                    int k1 = iblockstate.getLightOpacity();

                    if (j1 > 0)
                    {
                        if (j >= i1)
                        {
                            this.relightBlock(i, j + 1, k);
                        }
                    }
                    else if (j == i1 - 1)
                    {
                        this.relightBlock(i, j, k);
                    }

                    if (j1 != k1 && (j1 < k1 || this.getLightFor(EnumSkyBlock.SKY, pos) > 0 || this.getLightFor(EnumSkyBlock.BLOCK, pos) > 0))
                    {
                        this.propagateSkylightOcclusion(i, k);
                    }
                }

                if (block1 instanceof ITileEntityProvider)
                {
                    TileEntity tileentity = this.getTileEntity(pos, Chunk.EnumCreateEntityType.CHECK);

                    if (tileentity != null)
                    {
                        tileentity.updateContainingBlockInfo();
                    }
                }

                if (!this.worldObj.isRemote && block1 != block)
                {
                    block.onBlockAdded(this.worldObj, pos, state);
                }

                if (block instanceof ITileEntityProvider)
                {
                    TileEntity tileentity1 = this.getTileEntity(pos, Chunk.EnumCreateEntityType.CHECK);

                    if (tileentity1 == null)
                    {
                        tileentity1 = ((ITileEntityProvider)block).createNewTileEntity(this.worldObj, block.getMetaFromState(state));
                        this.worldObj.setTileEntity(pos, tileentity1);
                    }

                    if (tileentity1 != null)
                    {
                        tileentity1.updateContainingBlockInfo();
                    }
                }

                this.isModified = true;
                return iblockstate;
            }
        }
    }

    public int getLightFor(EnumSkyBlock p_177413_1_, BlockPos pos)
    {
        int i = pos.getX() & 15;
        int j = pos.getY();
        int k = pos.getZ() & 15;
        ExtendedBlockStorage extendedblockstorage = this.storageArrays[j >> 4];
        return extendedblockstorage == NULL_BLOCK_STORAGE ? (this.canSeeSky(pos) ? p_177413_1_.defaultLightValue : 0) : (p_177413_1_ == EnumSkyBlock.SKY ? (this.worldObj.provider.getHasNoSky() ? 0 : extendedblockstorage.getExtSkylightValue(i, j & 15, k)) : (p_177413_1_ == EnumSkyBlock.BLOCK ? extendedblockstorage.getExtBlocklightValue(i, j & 15, k) : p_177413_1_.defaultLightValue));
    }

    public void setLightFor(EnumSkyBlock p_177431_1_, BlockPos pos, int value)
    {
        int i = pos.getX() & 15;
        int j = pos.getY();
        int k = pos.getZ() & 15;
        ExtendedBlockStorage extendedblockstorage = this.storageArrays[j >> 4];

        if (extendedblockstorage == NULL_BLOCK_STORAGE)
        {
            extendedblockstorage = this.storageArrays[j >> 4] = new ExtendedBlockStorage(j >> 4 << 4, !this.worldObj.provider.getHasNoSky());
            this.generateSkylightMap();
        }

        this.isModified = true;

        if (p_177431_1_ == EnumSkyBlock.SKY)
        {
            if (!this.worldObj.provider.getHasNoSky())
            {
                extendedblockstorage.setExtSkylightValue(i, j & 15, k, value);
            }
        }
        else if (p_177431_1_ == EnumSkyBlock.BLOCK)
        {
            extendedblockstorage.setExtBlocklightValue(i, j & 15, k, value);
        }
    }

    public int getLightSubtracted(BlockPos pos, int amount)
    {
        int i = pos.getX() & 15;
        int j = pos.getY();
        int k = pos.getZ() & 15;
        ExtendedBlockStorage extendedblockstorage = this.storageArrays[j >> 4];

        if (extendedblockstorage == NULL_BLOCK_STORAGE)
        {
            return !this.worldObj.provider.getHasNoSky() && amount < EnumSkyBlock.SKY.defaultLightValue ? EnumSkyBlock.SKY.defaultLightValue - amount : 0;
        }
        else
        {
            int l = this.worldObj.provider.getHasNoSky() ? 0 : extendedblockstorage.getExtSkylightValue(i, j & 15, k);
            l = l - amount;
            int i1 = extendedblockstorage.getExtBlocklightValue(i, j & 15, k);

            if (i1 > l)
            {
                l = i1;
            }

            return l;
        }
    }

    /**
     * Adds an entity to the chunk. Args: entity
     */
    public void addEntity(Entity entityIn)
    {
        this.hasEntities = true;
        int i = MathHelper.floor_double(entityIn.posX / 16.0D);
        int j = MathHelper.floor_double(entityIn.posZ / 16.0D);

        if (i != this.xPosition || j != this.zPosition)
        {
            logger.warn("Wrong location! (" + i + ", " + j + ") should be (" + this.xPosition + ", " + this.zPosition + "), " + entityIn, new Object[] {entityIn});
            entityIn.setDead();
        }

        int k = MathHelper.floor_double(entityIn.posY / 16.0D);

        if (k < 0)
        {
            k = 0;
        }

        if (k >= this.entityLists.length)
        {
            k = this.entityLists.length - 1;
        }

        entityIn.addedToChunk = true;
        entityIn.chunkCoordX = this.xPosition;
        entityIn.chunkCoordY = k;
        entityIn.chunkCoordZ = this.zPosition;
        this.entityLists[k].add(entityIn);
    }

    /**
     * removes entity using its y chunk coordinate as its index
     */
    public void removeEntity(Entity entityIn)
    {
        this.removeEntityAtIndex(entityIn, entityIn.chunkCoordY);
    }

    /**
     * Removes entity at the specified index from the entity array.
     */
    public void removeEntityAtIndex(Entity entityIn, int p_76608_2_)
    {
        if (p_76608_2_ < 0)
        {
            p_76608_2_ = 0;
        }

        if (p_76608_2_ >= this.entityLists.length)
        {
            p_76608_2_ = this.entityLists.length - 1;
        }

        this.entityLists[p_76608_2_].remove(entityIn);
    }

    public boolean canSeeSky(BlockPos pos)
    {
        int i = pos.getX() & 15;
        int j = pos.getY();
        int k = pos.getZ() & 15;
        return j >= this.heightMap[k << 4 | i];
    }

    private TileEntity createNewTileEntity(BlockPos pos)
    {
        IBlockState iblockstate = this.getBlockState(pos);
        Block block = iblockstate.getBlock();
        return !block.hasTileEntity() ? null : ((ITileEntityProvider)block).createNewTileEntity(this.worldObj, iblockstate.getBlock().getMetaFromState(iblockstate));
    }

    public TileEntity getTileEntity(BlockPos pos, Chunk.EnumCreateEntityType p_177424_2_)
    {
        TileEntity tileentity = (TileEntity)this.chunkTileEntityMap.get(pos);

        if (tileentity == null)
        {
            if (p_177424_2_ == Chunk.EnumCreateEntityType.IMMEDIATE)
            {
                tileentity = this.createNewTileEntity(pos);
                this.worldObj.setTileEntity(pos, tileentity);
            }
            else if (p_177424_2_ == Chunk.EnumCreateEntityType.QUEUED)
            {
                this.tileEntityPosQueue.add(pos);
            }
        }
        else if (tileentity.isInvalid())
        {
            this.chunkTileEntityMap.remove(pos);
            return null;
        }

        return tileentity;
    }

    public void addTileEntity(TileEntity tileEntityIn)
    {
        this.addTileEntity(tileEntityIn.getPos(), tileEntityIn);

        if (this.isChunkLoaded)
        {
            this.worldObj.addTileEntity(tileEntityIn);
        }
    }

    public void addTileEntity(BlockPos pos, TileEntity tileEntityIn)
    {
        tileEntityIn.setWorldObj(this.worldObj);
        tileEntityIn.setPos(pos);

        if (this.getBlockState(pos).getBlock() instanceof ITileEntityProvider)
        {
            if (this.chunkTileEntityMap.containsKey(pos))
            {
                ((TileEntity)this.chunkTileEntityMap.get(pos)).invalidate();
            }

            tileEntityIn.validate();
            this.chunkTileEntityMap.put(pos, tileEntityIn);
        }
    }

    public void removeTileEntity(BlockPos pos)
    {
        if (this.isChunkLoaded)
        {
            TileEntity tileentity = (TileEntity)this.chunkTileEntityMap.remove(pos);

            if (tileentity != null)
            {
                tileentity.invalidate();
            }
        }
    }

    /**
     * Called when this Chunk is loaded by the ChunkProvider
     */
    public void onChunkLoad()
    {
        this.isChunkLoaded = true;
        this.worldObj.addTileEntities(this.chunkTileEntityMap.values());

        for (int i = 0; i < this.entityLists.length; ++i)
        {
            for (Entity entity : this.entityLists[i])
            {
                entity.onChunkLoad();
            }

            this.worldObj.loadEntities(this.entityLists[i]);
        }
    }

    /**
     * Called when this Chunk is unloaded by the ChunkProvider
     */
    public void onChunkUnload()
    {
        this.isChunkLoaded = false;

        for (TileEntity tileentity : this.chunkTileEntityMap.values())
        {
            this.worldObj.markTileEntityForRemoval(tileentity);
        }

        for (int i = 0; i < this.entityLists.length; ++i)
        {
            this.worldObj.unloadEntities(this.entityLists[i]);
        }
    }

    /**
     * Sets the isModified flag for this Chunk
     */
    public void setChunkModified()
    {
        this.isModified = true;
    }

    /**
     * Fills the given list of all entities that intersect within the given bounding box that aren't the passed entity.
     */
    public void getEntitiesWithinAABBForEntity(Entity entityIn, AxisAlignedBB aabb, List<Entity> listToFill, Predicate <? super Entity > p_177414_4_)
    {
        int i = MathHelper.floor_double((aabb.minY - 2.0D) / 16.0D);
        int j = MathHelper.floor_double((aabb.maxY + 2.0D) / 16.0D);
        i = MathHelper.clamp_int(i, 0, this.entityLists.length - 1);
        j = MathHelper.clamp_int(j, 0, this.entityLists.length - 1);

        for (int k = i; k <= j; ++k)
        {
            if (!this.entityLists[k].isEmpty())
            {
                for (Entity entity : this.entityLists[k])
                {
                    if (entity.getEntityBoundingBox().intersectsWith(aabb) && entity != entityIn)
                    {
                        if (p_177414_4_ == null || p_177414_4_.apply(entity))
                        {
                            listToFill.add(entity);
                        }

                        Entity[] aentity = entity.getParts();

                        if (aentity != null)
                        {
                            for (int l = 0; l < aentity.length; ++l)
                            {
                                entity = aentity[l];

                                if (entity != entityIn && entity.getEntityBoundingBox().intersectsWith(aabb) && (p_177414_4_ == null || p_177414_4_.apply(entity)))
                                {
                                    listToFill.add(entity);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    public <T extends Entity> void getEntitiesOfTypeWithinAAAB(Class <? extends T > entityClass, AxisAlignedBB aabb, List<T> listToFill, Predicate <? super T > p_177430_4_)
    {
        int i = MathHelper.floor_double((aabb.minY - 2.0D) / 16.0D);
        int j = MathHelper.floor_double((aabb.maxY + 2.0D) / 16.0D);
        i = MathHelper.clamp_int(i, 0, this.entityLists.length - 1);
        j = MathHelper.clamp_int(j, 0, this.entityLists.length - 1);

        for (int k = i; k <= j; ++k)
        {
            for (T t : this.entityLists[k].getByClass(entityClass))
            {
                if (t.getEntityBoundingBox().intersectsWith(aabb) && (p_177430_4_ == null || p_177430_4_.apply(t)))
                {
                    listToFill.add(t);
                }
            }
        }
    }

    /**
     * Returns true if this Chunk needs to be saved
     */
    public boolean needsSaving(boolean p_76601_1_)
    {
        if (p_76601_1_)
        {
            if (this.hasEntities && this.worldObj.getTotalWorldTime() != this.lastSaveTime || this.isModified)
            {
                return true;
            }
        }
        else if (this.hasEntities && this.worldObj.getTotalWorldTime() >= this.lastSaveTime + 600L)
        {
            return true;
        }

        return this.isModified;
    }

    public Random getRandomWithSeed(long seed)
    {
        return new Random(this.worldObj.getSeed() + (long)(this.xPosition * this.xPosition * 4987142) + (long)(this.xPosition * 5947611) + (long)(this.zPosition * this.zPosition) * 4392871L + (long)(this.zPosition * 389711) ^ seed);
    }

    public boolean isEmpty()
    {
        return false;
    }

    public void func_186030_a(IChunkProvider p_186030_1_, IChunkGenerator p_186030_2_)
    {
        Chunk chunk = p_186030_1_.getLoadedChunk(this.xPosition, this.zPosition - 1);
        Chunk chunk1 = p_186030_1_.getLoadedChunk(this.xPosition + 1, this.zPosition);
        Chunk chunk2 = p_186030_1_.getLoadedChunk(this.xPosition, this.zPosition + 1);
        Chunk chunk3 = p_186030_1_.getLoadedChunk(this.xPosition - 1, this.zPosition);

        if (chunk1 != null && chunk2 != null && p_186030_1_.getLoadedChunk(this.xPosition + 1, this.zPosition + 1) != null)
        {
            this.func_186034_a(p_186030_2_);
        }

        if (chunk3 != null && chunk2 != null && p_186030_1_.getLoadedChunk(this.xPosition - 1, this.zPosition + 1) != null)
        {
            chunk3.func_186034_a(p_186030_2_);
        }

        if (chunk != null && chunk1 != null && p_186030_1_.getLoadedChunk(this.xPosition + 1, this.zPosition - 1) != null)
        {
            chunk.func_186034_a(p_186030_2_);
        }

        if (chunk != null && chunk3 != null)
        {
            Chunk chunk4 = p_186030_1_.getLoadedChunk(this.xPosition - 1, this.zPosition - 1);

            if (chunk4 != null)
            {
                chunk4.func_186034_a(p_186030_2_);
            }
        }
    }

    protected void func_186034_a(IChunkGenerator p_186034_1_)
    {
        if (this.isTerrainPopulated())
        {
            if (p_186034_1_.func_185933_a(this, this.xPosition, this.zPosition))
            {
                this.setChunkModified();
            }
        }
        else
        {
            this.func_150809_p();
            p_186034_1_.populate(this.xPosition, this.zPosition);
            this.setChunkModified();
        }
    }

    public BlockPos getPrecipitationHeight(BlockPos pos)
    {
        int i = pos.getX() & 15;
        int j = pos.getZ() & 15;
        int k = i | j << 4;
        BlockPos blockpos = new BlockPos(pos.getX(), this.precipitationHeightMap[k], pos.getZ());

        if (blockpos.getY() == -999)
        {
            int l = this.getTopFilledSegment() + 15;
            blockpos = new BlockPos(pos.getX(), l, pos.getZ());
            int i1 = -1;

            while (blockpos.getY() > 0 && i1 == -1)
            {
                IBlockState iblockstate = this.getBlockState(blockpos);
                Material material = iblockstate.getMaterial();

                if (!material.blocksMovement() && !material.isLiquid())
                {
                    blockpos = blockpos.down();
                }
                else
                {
                    i1 = blockpos.getY() + 1;
                }
            }

            this.precipitationHeightMap[k] = i1;
        }

        return new BlockPos(pos.getX(), this.precipitationHeightMap[k], pos.getZ());
    }

    public void func_150804_b(boolean p_150804_1_)
    {
        if (this.isGapLightingUpdated && !this.worldObj.provider.getHasNoSky() && !p_150804_1_)
        {
            this.recheckGaps(this.worldObj.isRemote);
        }

        this.field_150815_m = true;

        if (!this.isLightPopulated && this.isTerrainPopulated)
        {
            this.func_150809_p();
        }

        while (!this.tileEntityPosQueue.isEmpty())
        {
            BlockPos blockpos = (BlockPos)this.tileEntityPosQueue.poll();

            if (this.getTileEntity(blockpos, Chunk.EnumCreateEntityType.CHECK) == null && this.getBlockState(blockpos).getBlock().hasTileEntity())
            {
                TileEntity tileentity = this.createNewTileEntity(blockpos);
                this.worldObj.setTileEntity(blockpos, tileentity);
                this.worldObj.markBlockRangeForRenderUpdate(blockpos, blockpos);
            }
        }
    }

    public boolean isPopulated()
    {
        return this.field_150815_m && this.isTerrainPopulated && this.isLightPopulated;
    }

    public boolean func_186035_j()
    {
        return this.field_150815_m;
    }

    /**
     * Gets a ChunkCoordIntPair representing the Chunk's position.
     */
    public ChunkCoordIntPair getChunkCoordIntPair()
    {
        return new ChunkCoordIntPair(this.xPosition, this.zPosition);
    }

    /**
     * Returns whether the ExtendedBlockStorages containing levels (in blocks) from arg 1 to arg 2 are fully empty
     * (true) or not (false).
     */
    public boolean getAreLevelsEmpty(int startY, int endY)
    {
        if (startY < 0)
        {
            startY = 0;
        }

        if (endY >= 256)
        {
            endY = 255;
        }

        for (int i = startY; i <= endY; i += 16)
        {
            ExtendedBlockStorage extendedblockstorage = this.storageArrays[i >> 4];

            if (extendedblockstorage != NULL_BLOCK_STORAGE && !extendedblockstorage.isEmpty())
            {
                return false;
            }
        }

        return true;
    }

    public void setStorageArrays(ExtendedBlockStorage[] newStorageArrays)
    {
        if (this.storageArrays.length != newStorageArrays.length)
        {
            logger.warn("Could not set level chunk sections, array length is " + newStorageArrays.length + " instead of " + this.storageArrays.length);
        }
        else
        {
            System.arraycopy(newStorageArrays, 0, this.storageArrays, 0, this.storageArrays.length);
        }
    }

    public void fillChunk(PacketBuffer p_186033_1_, int p_186033_2_, boolean p_186033_3_)
    {
        boolean flag = !this.worldObj.provider.getHasNoSky();

        for (int i = 0; i < this.storageArrays.length; ++i)
        {
            ExtendedBlockStorage extendedblockstorage = this.storageArrays[i];

            if ((p_186033_2_ & 1 << i) == 0)
            {
                if (p_186033_3_ && extendedblockstorage != NULL_BLOCK_STORAGE)
                {
                    this.storageArrays[i] = NULL_BLOCK_STORAGE;
                }
            }
            else
            {
                if (extendedblockstorage == NULL_BLOCK_STORAGE)
                {
                    extendedblockstorage = new ExtendedBlockStorage(i << 4, flag);
                    this.storageArrays[i] = extendedblockstorage;
                }

                extendedblockstorage.func_186049_g().func_186010_a(p_186033_1_);
                p_186033_1_.readBytes(extendedblockstorage.getBlocklightArray().getData());

                if (flag)
                {
                    p_186033_1_.readBytes(extendedblockstorage.getSkylightArray().getData());
                }
            }
        }

        if (p_186033_3_)
        {
            p_186033_1_.readBytes(this.blockBiomeArray);
        }

        for (int j = 0; j < this.storageArrays.length; ++j)
        {
            if (this.storageArrays[j] != NULL_BLOCK_STORAGE && (p_186033_2_ & 1 << j) != 0)
            {
                this.storageArrays[j].removeInvalidBlocks();
            }
        }

        this.isLightPopulated = true;
        this.isTerrainPopulated = true;
        this.generateHeightMap();

        for (TileEntity tileentity : this.chunkTileEntityMap.values())
        {
            tileentity.updateContainingBlockInfo();
        }
    }

    public BiomeGenBase getBiome(BlockPos pos, BiomeProvider chunkManager)
    {
        int i = pos.getX() & 15;
        int j = pos.getZ() & 15;
        int k = this.blockBiomeArray[j << 4 | i] & 255;

        if (k == 255)
        {
            BiomeGenBase biomegenbase = chunkManager.getBiomeGenerator(pos, Biomes.plains);
            k = BiomeGenBase.getIdForBiome(biomegenbase);
            this.blockBiomeArray[j << 4 | i] = (byte)(k & 255);
        }

        BiomeGenBase biomegenbase1 = BiomeGenBase.getBiome(k);
        return biomegenbase1 == null ? Biomes.plains : biomegenbase1;
    }

    /**
     * Returns an array containing a 16x16 mapping on the X/Z of block positions in this Chunk to biome IDs.
     */
    public byte[] getBiomeArray()
    {
        return this.blockBiomeArray;
    }

    /**
     * Accepts a 256-entry array that contains a 16x16 mapping on the X/Z plane of block positions in this Chunk to
     * biome IDs.
     */
    public void setBiomeArray(byte[] biomeArray)
    {
        if (this.blockBiomeArray.length != biomeArray.length)
        {
            logger.warn("Could not set level chunk biomes, array length is " + biomeArray.length + " instead of " + this.blockBiomeArray.length);
        }
        else
        {
            for (int i = 0; i < this.blockBiomeArray.length; ++i)
            {
                this.blockBiomeArray[i] = biomeArray[i];
            }
        }
    }

    /**
     * Resets the relight check index to 0 for this Chunk.
     */
    public void resetRelightChecks()
    {
        this.queuedLightChecks = 0;
    }

    /**
     * Called once-per-chunk-per-tick, and advances the round-robin relight check index by up to 8 blocks at a time. In
     * a worst-case scenario, can potentially take up to 25.6 seconds, calculated via (4096/8)/20, to re-check all
     * blocks in a chunk, which may explain lagging light updates on initial world generation.
     */
    public void enqueueRelightChecks()
    {
        if (this.queuedLightChecks < 4096)
        {
            BlockPos blockpos = new BlockPos(this.xPosition << 4, 0, this.zPosition << 4);

            for (int i = 0; i < 8; ++i)
            {
                if (this.queuedLightChecks >= 4096)
                {
                    return;
                }

                int j = this.queuedLightChecks % 16;
                int k = this.queuedLightChecks / 16 % 16;
                int l = this.queuedLightChecks / 256;
                ++this.queuedLightChecks;

                for (int i1 = 0; i1 < 16; ++i1)
                {
                    BlockPos blockpos1 = blockpos.add(k, (j << 4) + i1, l);
                    boolean flag = i1 == 0 || i1 == 15 || k == 0 || k == 15 || l == 0 || l == 15;

                    if (this.storageArrays[j] == NULL_BLOCK_STORAGE && flag || this.storageArrays[j] != NULL_BLOCK_STORAGE && this.storageArrays[j].get(k, i1, l).getMaterial() == Material.air)
                    {
                        for (EnumFacing enumfacing : EnumFacing.values())
                        {
                            BlockPos blockpos2 = blockpos1.offset(enumfacing);

                            if (this.worldObj.getBlockState(blockpos2).getlightValue() > 0)
                            {
                                this.worldObj.checkLight(blockpos2);
                            }
                        }

                        this.worldObj.checkLight(blockpos1);
                    }
                }
            }
        }
    }

    public void func_150809_p()
    {
        this.isTerrainPopulated = true;
        this.isLightPopulated = true;
        BlockPos blockpos = new BlockPos(this.xPosition << 4, 0, this.zPosition << 4);

        if (!this.worldObj.provider.getHasNoSky())
        {
            if (this.worldObj.isAreaLoaded(blockpos.add(-1, 0, -1), blockpos.add(16, this.worldObj.getSeaLevel(), 16)))
            {
                label92:

                for (int i = 0; i < 16; ++i)
                {
                    for (int j = 0; j < 16; ++j)
                    {
                        if (!this.func_150811_f(i, j))
                        {
                            this.isLightPopulated = false;
                            break label92;
                        }
                    }
                }

                if (this.isLightPopulated)
                {
                    for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
                    {
                        int k = enumfacing.getAxisDirection() == EnumFacing.AxisDirection.POSITIVE ? 16 : 1;
                        this.worldObj.getChunkFromBlockCoords(blockpos.offset(enumfacing, k)).func_180700_a(enumfacing.getOpposite());
                    }

                    this.func_177441_y();
                }
            }
            else
            {
                this.isLightPopulated = false;
            }
        }
    }

    private void func_177441_y()
    {
        for (int i = 0; i < this.updateSkylightColumns.length; ++i)
        {
            this.updateSkylightColumns[i] = true;
        }

        this.recheckGaps(false);
    }

    private void func_180700_a(EnumFacing facing)
    {
        if (this.isTerrainPopulated)
        {
            if (facing == EnumFacing.EAST)
            {
                for (int i = 0; i < 16; ++i)
                {
                    this.func_150811_f(15, i);
                }
            }
            else if (facing == EnumFacing.WEST)
            {
                for (int j = 0; j < 16; ++j)
                {
                    this.func_150811_f(0, j);
                }
            }
            else if (facing == EnumFacing.SOUTH)
            {
                for (int k = 0; k < 16; ++k)
                {
                    this.func_150811_f(k, 15);
                }
            }
            else if (facing == EnumFacing.NORTH)
            {
                for (int l = 0; l < 16; ++l)
                {
                    this.func_150811_f(l, 0);
                }
            }
        }
    }

    private boolean func_150811_f(int x, int z)
    {
        int i = this.getTopFilledSegment();
        boolean flag = false;
        boolean flag1 = false;
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos((this.xPosition << 4) + x, 0, (this.zPosition << 4) + z);

        for (int j = i + 16 - 1; j > this.worldObj.getSeaLevel() || j > 0 && !flag1; --j)
        {
            blockpos$mutableblockpos.set(blockpos$mutableblockpos.getX(), j, blockpos$mutableblockpos.getZ());
            int k = this.getBlockLightOpacity(blockpos$mutableblockpos);

            if (k == 255 && blockpos$mutableblockpos.getY() < this.worldObj.getSeaLevel())
            {
                flag1 = true;
            }

            if (!flag && k > 0)
            {
                flag = true;
            }
            else if (flag && k == 0 && !this.worldObj.checkLight(blockpos$mutableblockpos))
            {
                return false;
            }
        }

        for (int l = blockpos$mutableblockpos.getY(); l > 0; --l)
        {
            blockpos$mutableblockpos.set(blockpos$mutableblockpos.getX(), l, blockpos$mutableblockpos.getZ());

            if (this.getBlockState(blockpos$mutableblockpos).getlightValue() > 0)
            {
                this.worldObj.checkLight(blockpos$mutableblockpos);
            }
        }

        return true;
    }

    public boolean isLoaded()
    {
        return this.isChunkLoaded;
    }

    public void setChunkLoaded(boolean loaded)
    {
        this.isChunkLoaded = loaded;
    }

    public World getWorld()
    {
        return this.worldObj;
    }

    public int[] getHeightMap()
    {
        return this.heightMap;
    }

    public void setHeightMap(int[] newHeightMap)
    {
        if (this.heightMap.length != newHeightMap.length)
        {
            logger.warn("Could not set level chunk heightmap, array length is " + newHeightMap.length + " instead of " + this.heightMap.length);
        }
        else
        {
            for (int i = 0; i < this.heightMap.length; ++i)
            {
                this.heightMap[i] = newHeightMap[i];
            }
        }
    }

    public Map<BlockPos, TileEntity> getTileEntityMap()
    {
        return this.chunkTileEntityMap;
    }

    public ClassInheritanceMultiMap<Entity>[] getEntityLists()
    {
        return this.entityLists;
    }

    public boolean isTerrainPopulated()
    {
        return this.isTerrainPopulated;
    }

    public void setTerrainPopulated(boolean terrainPopulated)
    {
        this.isTerrainPopulated = terrainPopulated;
    }

    public boolean isLightPopulated()
    {
        return this.isLightPopulated;
    }

    public void setLightPopulated(boolean lightPopulated)
    {
        this.isLightPopulated = lightPopulated;
    }

    public void setModified(boolean modified)
    {
        this.isModified = modified;
    }

    public void setHasEntities(boolean hasEntitiesIn)
    {
        this.hasEntities = hasEntitiesIn;
    }

    public void setLastSaveTime(long saveTime)
    {
        this.lastSaveTime = saveTime;
    }

    public int getLowestHeight()
    {
        return this.heightMapMinimum;
    }

    public long getInhabitedTime()
    {
        return this.inhabitedTime;
    }

    public void setInhabitedTime(long newInhabitedTime)
    {
        this.inhabitedTime = newInhabitedTime;
    }

    public static enum EnumCreateEntityType
    {
        IMMEDIATE,
        QUEUED,
        CHECK;
    }
}
