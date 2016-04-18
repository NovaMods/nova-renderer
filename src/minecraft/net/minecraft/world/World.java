package net.minecraft.world;

import com.google.common.base.Function;
import com.google.common.base.Objects;
import com.google.common.base.Predicate;
import com.google.common.collect.Lists;
import java.util.Calendar;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.UUID;
import java.util.concurrent.Callable;
import net.minecraft.block.Block;
import net.minecraft.block.BlockLiquid;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.init.Biomes;
import net.minecraft.init.Blocks;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.Packet;
import net.minecraft.pathfinding.PathWorldListener;
import net.minecraft.profiler.Profiler;
import net.minecraft.scoreboard.Scoreboard;
import net.minecraft.server.MinecraftServer;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.EntitySelectors;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ITickable;
import net.minecraft.util.IntHashMap;
import net.minecraft.util.ReportedException;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.math.Vec3d;
import net.minecraft.village.VillageCollection;
import net.minecraft.world.biome.BiomeGenBase;
import net.minecraft.world.biome.BiomeProvider;
import net.minecraft.world.border.WorldBorder;
import net.minecraft.world.chunk.Chunk;
import net.minecraft.world.chunk.IChunkProvider;
import net.minecraft.world.gen.structure.StructureBoundingBox;
import net.minecraft.world.storage.ISaveHandler;
import net.minecraft.world.storage.MapStorage;
import net.minecraft.world.storage.WorldInfo;
import net.minecraft.world.storage.loot.LootTableManager;

public abstract class World implements IBlockAccess
{
    private int seaLevel = 63;

    /**
     * boolean; if true updates scheduled by scheduleBlockUpdate happen immediately
     */
    protected boolean scheduledUpdatesAreImmediate;
    public final List<Entity> loadedEntityList = Lists.<Entity>newArrayList();
    protected final List<Entity> unloadedEntityList = Lists.<Entity>newArrayList();
    public final List<TileEntity> loadedTileEntityList = Lists.<TileEntity>newArrayList();
    public final List<TileEntity> tickableTileEntities = Lists.<TileEntity>newArrayList();
    private final List<TileEntity> addedTileEntityList = Lists.<TileEntity>newArrayList();
    private final List<TileEntity> tileEntitiesToBeRemoved = Lists.<TileEntity>newArrayList();
    public final List<EntityPlayer> playerEntities = Lists.<EntityPlayer>newArrayList();
    public final List<Entity> weatherEffects = Lists.<Entity>newArrayList();
    protected final IntHashMap<Entity> entitiesById = new IntHashMap();
    private long cloudColour = 16777215L;

    /** How much light is subtracted from full daylight */
    private int skylightSubtracted;

    /**
     * Contains the current Linear Congruential Generator seed for block updates. Used with an A value of 3 and a C
     * value of 0x3c6ef35f, producing a highly planar series of values ill-suited for choosing random blocks in a
     * 16x128x16 field.
     */
    protected int updateLCG = (new Random()).nextInt();

    /**
     * magic number used to generate fast random numbers for 3d distribution within a chunk
     */
    protected final int DIST_HASH_MAGIC = 1013904223;
    protected float prevRainingStrength;
    protected float rainingStrength;
    protected float prevThunderingStrength;
    protected float thunderingStrength;

    /**
     * Set to 2 whenever a lightning bolt is generated in SSP. Decrements if > 0 in updateWeather(). Value appears to be
     * unused.
     */
    private int lastLightningBolt;

    /** RNG for World. */
    public final Random rand = new Random();

    /** The WorldProvider instance that World uses. */
    public final WorldProvider provider;
    protected PathWorldListener pathListener = new PathWorldListener();
    protected List<IWorldEventListener> worldAccesses;

    /** Handles chunk operations and caching */
    protected IChunkProvider chunkProvider;
    protected final ISaveHandler saveHandler;

    /**
     * holds information about a world (size on disk, time, spawn point, seed, ...)
     */
    protected WorldInfo worldInfo;

    /**
     * if set, this flag forces a request to load a chunk to load the chunk rather than defaulting to the world's
     * chunkprovider's dummy if possible
     */
    protected boolean findingSpawnPoint;
    protected MapStorage mapStorage;
    protected VillageCollection villageCollectionObj;
    protected LootTableManager lootTable;
    public final Profiler theProfiler;
    private final Calendar theCalendar;
    protected Scoreboard worldScoreboard;

    /**
     * True if the world is a 'slave' client; changes will not be saved or propagated from this world. For example,
     * server worlds have this set to false, client worlds have this set to true.
     */
    public final boolean isRemote;

    /** indicates if enemies are spawned or not */
    protected boolean spawnHostileMobs;

    /** A flag indicating whether we should spawn peaceful mobs. */
    protected boolean spawnPeacefulMobs;
    private boolean processingLoadedTiles;
    private final WorldBorder worldBorder;

    /**
     * is a temporary list of blocks and light values used when updating light levels. Holds up to 32x32x32 blocks (the
     * maximum influence of a light source.) Every element is a packed bit value: 0000000000LLLLzzzzzzyyyyyyxxxxxx. The
     * 4-bit L is a light level used when darkening blocks. 6-bit numbers x, y and z represent the block's offset from
     * the original block, plus 32 (i.e. value of 31 would mean a -1 offset
     */
    int[] lightUpdateBlockList;

    protected World(ISaveHandler saveHandlerIn, WorldInfo info, WorldProvider providerIn, Profiler profilerIn, boolean client)
    {
        this.worldAccesses = Lists.newArrayList(new IWorldEventListener[] {this.pathListener});
        this.theCalendar = Calendar.getInstance();
        this.worldScoreboard = new Scoreboard();
        this.spawnHostileMobs = true;
        this.spawnPeacefulMobs = true;
        this.lightUpdateBlockList = new int[32768];
        this.saveHandler = saveHandlerIn;
        this.theProfiler = profilerIn;
        this.worldInfo = info;
        this.provider = providerIn;
        this.isRemote = client;
        this.worldBorder = providerIn.getWorldBorder();
    }

    public World init()
    {
        return this;
    }

    public BiomeGenBase getBiomeGenForCoords(final BlockPos pos)
    {
        if (this.isBlockLoaded(pos))
        {
            Chunk chunk = this.getChunkFromBlockCoords(pos);

            try
            {
                return chunk.getBiome(pos, this.provider.getWorldChunkManager());
            }
            catch (Throwable throwable)
            {
                CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Getting biome");
                CrashReportCategory crashreportcategory = crashreport.makeCategory("Coordinates of biome request");
                crashreportcategory.addCrashSectionCallable("Location", new Callable<String>()
                {
                    public String call() throws Exception
                    {
                        return CrashReportCategory.getCoordinateInfo(pos);
                    }
                });
                throw new ReportedException(crashreport);
            }
        }
        else
        {
            return this.provider.getWorldChunkManager().getBiomeGenerator(pos, Biomes.plains);
        }
    }

    public BiomeProvider getWorldChunkManager()
    {
        return this.provider.getWorldChunkManager();
    }

    /**
     * Creates the chunk provider for this world. Called in the constructor. Retrieves provider from worldProvider?
     */
    protected abstract IChunkProvider createChunkProvider();

    public void initialize(WorldSettings settings)
    {
        this.worldInfo.setServerInitialized(true);
    }

    public MinecraftServer getMinecraftServer()
    {
        return null;
    }

    /**
     * Sets a new spawn location by finding an uncovered block at a random (x,z) location in the chunk.
     */
    public void setInitialSpawnLocation()
    {
        this.setSpawnPoint(new BlockPos(8, 64, 8));
    }

    public IBlockState func_184141_c(BlockPos p_184141_1_)
    {
        BlockPos blockpos;

        for (blockpos = new BlockPos(p_184141_1_.getX(), this.getSeaLevel(), p_184141_1_.getZ()); !this.isAirBlock(blockpos.up()); blockpos = blockpos.up())
        {
            ;
        }

        return this.getBlockState(blockpos);
    }

    /**
     * Check if the given BlockPos has valid coordinates
     */
    private boolean isValid(BlockPos pos)
    {
        return pos.getX() >= -30000000 && pos.getZ() >= -30000000 && pos.getX() < 30000000 && pos.getZ() < 30000000 && pos.getY() >= 0 && pos.getY() < 256;
    }

    /**
     * Checks to see if an air block exists at the provided location. Note that this only checks to see if the blocks
     * material is set to air, meaning it is possible for non-vanilla blocks to still pass this check.
     */
    public boolean isAirBlock(BlockPos pos)
    {
        return this.getBlockState(pos).getMaterial() == Material.air;
    }

    public boolean isBlockLoaded(BlockPos pos)
    {
        return this.isBlockLoaded(pos, true);
    }

    public boolean isBlockLoaded(BlockPos pos, boolean allowEmpty)
    {
        return !this.isValid(pos) ? false : this.isChunkLoaded(pos.getX() >> 4, pos.getZ() >> 4, allowEmpty);
    }

    public boolean isAreaLoaded(BlockPos center, int radius)
    {
        return this.isAreaLoaded(center, radius, true);
    }

    public boolean isAreaLoaded(BlockPos center, int radius, boolean allowEmpty)
    {
        return this.isAreaLoaded(center.getX() - radius, center.getY() - radius, center.getZ() - radius, center.getX() + radius, center.getY() + radius, center.getZ() + radius, allowEmpty);
    }

    public boolean isAreaLoaded(BlockPos from, BlockPos to)
    {
        return this.isAreaLoaded(from, to, true);
    }

    public boolean isAreaLoaded(BlockPos from, BlockPos to, boolean allowEmpty)
    {
        return this.isAreaLoaded(from.getX(), from.getY(), from.getZ(), to.getX(), to.getY(), to.getZ(), allowEmpty);
    }

    public boolean isAreaLoaded(StructureBoundingBox box)
    {
        return this.isAreaLoaded(box, true);
    }

    public boolean isAreaLoaded(StructureBoundingBox box, boolean allowEmpty)
    {
        return this.isAreaLoaded(box.minX, box.minY, box.minZ, box.maxX, box.maxY, box.maxZ, allowEmpty);
    }

    private boolean isAreaLoaded(int xStart, int yStart, int zStart, int xEnd, int yEnd, int zEnd, boolean allowEmpty)
    {
        if (yEnd >= 0 && yStart < 256)
        {
            xStart = xStart >> 4;
            zStart = zStart >> 4;
            xEnd = xEnd >> 4;
            zEnd = zEnd >> 4;

            for (int i = xStart; i <= xEnd; ++i)
            {
                for (int j = zStart; j <= zEnd; ++j)
                {
                    if (!this.isChunkLoaded(i, j, allowEmpty))
                    {
                        return false;
                    }
                }
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    protected abstract boolean isChunkLoaded(int x, int z, boolean allowEmpty);

    public Chunk getChunkFromBlockCoords(BlockPos pos)
    {
        return this.getChunkFromChunkCoords(pos.getX() >> 4, pos.getZ() >> 4);
    }

    /**
     * Returns back a chunk looked up by chunk coordinates Args: x, y
     */
    public Chunk getChunkFromChunkCoords(int chunkX, int chunkZ)
    {
        return this.chunkProvider.func_186025_d(chunkX, chunkZ);
    }

    /**
     * Sets the block state at a given location. Flag 1 will cause a block update. Flag 2 will send the change to
     * clients (you almost always want this). Flag 4 prevents the block from being re-rendered, if this is a client
     * world. Flags can be added together.
     */
    public boolean setBlockState(BlockPos pos, IBlockState newState, int flags)
    {
        if (!this.isValid(pos))
        {
            return false;
        }
        else if (!this.isRemote && this.worldInfo.getTerrainType() == WorldType.DEBUG_WORLD)
        {
            return false;
        }
        else
        {
            Chunk chunk = this.getChunkFromBlockCoords(pos);
            Block block = newState.getBlock();
            IBlockState iblockstate = chunk.setBlockState(pos, newState);

            if (iblockstate == null)
            {
                return false;
            }
            else
            {
                if (newState.getLightOpacity() != iblockstate.getLightOpacity() || newState.getlightValue() != iblockstate.getlightValue())
                {
                    this.theProfiler.startSection("checkLight");
                    this.checkLight(pos);
                    this.theProfiler.endSection();
                }

                if ((flags & 2) != 0 && (!this.isRemote || (flags & 4) == 0) && chunk.isPopulated())
                {
                    this.notifyBlockUpdate(pos, iblockstate, newState, flags);
                }

                if (!this.isRemote && (flags & 1) != 0)
                {
                    this.notifyNeighborsRespectDebug(pos, iblockstate.getBlock());

                    if (newState.hasComparatorInputOverride())
                    {
                        this.updateComparatorOutputLevel(pos, block);
                    }
                }

                return true;
            }
        }
    }

    public boolean setBlockToAir(BlockPos pos)
    {
        return this.setBlockState(pos, Blocks.air.getDefaultState(), 3);
    }

    /**
     * Sets a block to air, but also plays the sound and particles and can spawn drops
     */
    public boolean destroyBlock(BlockPos pos, boolean dropBlock)
    {
        IBlockState iblockstate = this.getBlockState(pos);
        Block block = iblockstate.getBlock();

        if (iblockstate.getMaterial() == Material.air)
        {
            return false;
        }
        else
        {
            this.playAuxSFX(2001, pos, Block.getStateId(iblockstate));

            if (dropBlock)
            {
                block.dropBlockAsItem(this, pos, iblockstate, 0);
            }

            return this.setBlockState(pos, Blocks.air.getDefaultState(), 3);
        }
    }

    /**
     * Convenience method to update the block on both the client and server
     */
    public boolean setBlockState(BlockPos pos, IBlockState state)
    {
        return this.setBlockState(pos, state, 3);
    }

    public void notifyBlockUpdate(BlockPos pos, IBlockState oldState, IBlockState newState, int flags)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).notifyBlockUpdate(this, pos, oldState, newState, flags);
        }
    }

    public void notifyNeighborsRespectDebug(BlockPos pos, Block blockType)
    {
        if (this.worldInfo.getTerrainType() != WorldType.DEBUG_WORLD)
        {
            this.notifyNeighborsOfStateChange(pos, blockType);
        }
    }

    /**
     * marks a vertical line of blocks as dirty
     */
    public void markBlocksDirtyVertical(int x1, int z1, int x2, int z2)
    {
        if (x2 > z2)
        {
            int i = z2;
            z2 = x2;
            x2 = i;
        }

        if (!this.provider.getHasNoSky())
        {
            for (int j = x2; j <= z2; ++j)
            {
                this.checkLightFor(EnumSkyBlock.SKY, new BlockPos(x1, j, z1));
            }
        }

        this.markBlockRangeForRenderUpdate(x1, x2, z1, x1, z2, z1);
    }

    public void markBlockRangeForRenderUpdate(BlockPos rangeMin, BlockPos rangeMax)
    {
        this.markBlockRangeForRenderUpdate(rangeMin.getX(), rangeMin.getY(), rangeMin.getZ(), rangeMax.getX(), rangeMax.getY(), rangeMax.getZ());
    }

    public void markBlockRangeForRenderUpdate(int x1, int y1, int z1, int x2, int y2, int z2)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).markBlockRangeForRenderUpdate(x1, y1, z1, x2, y2, z2);
        }
    }

    public void notifyNeighborsOfStateChange(BlockPos pos, Block blockType)
    {
        this.notifyBlockOfStateChange(pos.west(), blockType);
        this.notifyBlockOfStateChange(pos.east(), blockType);
        this.notifyBlockOfStateChange(pos.down(), blockType);
        this.notifyBlockOfStateChange(pos.up(), blockType);
        this.notifyBlockOfStateChange(pos.north(), blockType);
        this.notifyBlockOfStateChange(pos.south(), blockType);
    }

    public void notifyNeighborsOfStateExcept(BlockPos pos, Block blockType, EnumFacing skipSide)
    {
        if (skipSide != EnumFacing.WEST)
        {
            this.notifyBlockOfStateChange(pos.west(), blockType);
        }

        if (skipSide != EnumFacing.EAST)
        {
            this.notifyBlockOfStateChange(pos.east(), blockType);
        }

        if (skipSide != EnumFacing.DOWN)
        {
            this.notifyBlockOfStateChange(pos.down(), blockType);
        }

        if (skipSide != EnumFacing.UP)
        {
            this.notifyBlockOfStateChange(pos.up(), blockType);
        }

        if (skipSide != EnumFacing.NORTH)
        {
            this.notifyBlockOfStateChange(pos.north(), blockType);
        }

        if (skipSide != EnumFacing.SOUTH)
        {
            this.notifyBlockOfStateChange(pos.south(), blockType);
        }
    }

    public void notifyBlockOfStateChange(BlockPos pos, final Block blockIn)
    {
        if (!this.isRemote)
        {
            IBlockState iblockstate = this.getBlockState(pos);

            try
            {
                iblockstate.getBlock().onNeighborBlockChange(this, pos, iblockstate, blockIn);
            }
            catch (Throwable throwable)
            {
                CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Exception while updating neighbours");
                CrashReportCategory crashreportcategory = crashreport.makeCategory("Block being updated");
                crashreportcategory.addCrashSectionCallable("Source block type", new Callable<String>()
                {
                    public String call() throws Exception
                    {
                        try
                        {
                            return String.format("ID #%d (%s // %s)", new Object[] {Integer.valueOf(Block.getIdFromBlock(blockIn)), blockIn.getUnlocalizedName(), blockIn.getClass().getCanonicalName()});
                        }
                        catch (Throwable var2)
                        {
                            return "ID #" + Block.getIdFromBlock(blockIn);
                        }
                    }
                });
                CrashReportCategory.addBlockInfo(crashreportcategory, pos, iblockstate);
                throw new ReportedException(crashreport);
            }
        }
    }

    public boolean isBlockTickPending(BlockPos pos, Block blockType)
    {
        return false;
    }

    public boolean canSeeSky(BlockPos pos)
    {
        return this.getChunkFromBlockCoords(pos).canSeeSky(pos);
    }

    public boolean canBlockSeeSky(BlockPos pos)
    {
        if (pos.getY() >= this.getSeaLevel())
        {
            return this.canSeeSky(pos);
        }
        else
        {
            BlockPos blockpos = new BlockPos(pos.getX(), this.getSeaLevel(), pos.getZ());

            if (!this.canSeeSky(blockpos))
            {
                return false;
            }
            else
            {
                for (blockpos = blockpos.down(); blockpos.getY() > pos.getY(); blockpos = blockpos.down())
                {
                    IBlockState iblockstate = this.getBlockState(blockpos);

                    if (iblockstate.getLightOpacity() > 0 && !iblockstate.getMaterial().isLiquid())
                    {
                        return false;
                    }
                }

                return true;
            }
        }
    }

    public int getLight(BlockPos pos)
    {
        if (pos.getY() < 0)
        {
            return 0;
        }
        else
        {
            if (pos.getY() >= 256)
            {
                pos = new BlockPos(pos.getX(), 255, pos.getZ());
            }

            return this.getChunkFromBlockCoords(pos).getLightSubtracted(pos, 0);
        }
    }

    public int getLightFromNeighbors(BlockPos pos)
    {
        return this.getLight(pos, true);
    }

    public int getLight(BlockPos pos, boolean checkNeighbors)
    {
        if (pos.getX() >= -30000000 && pos.getZ() >= -30000000 && pos.getX() < 30000000 && pos.getZ() < 30000000)
        {
            if (checkNeighbors && this.getBlockState(pos).useNeighborBrightness())
            {
                int i1 = this.getLight(pos.up(), false);
                int i = this.getLight(pos.east(), false);
                int j = this.getLight(pos.west(), false);
                int k = this.getLight(pos.south(), false);
                int l = this.getLight(pos.north(), false);

                if (i > i1)
                {
                    i1 = i;
                }

                if (j > i1)
                {
                    i1 = j;
                }

                if (k > i1)
                {
                    i1 = k;
                }

                if (l > i1)
                {
                    i1 = l;
                }

                return i1;
            }
            else if (pos.getY() < 0)
            {
                return 0;
            }
            else
            {
                if (pos.getY() >= 256)
                {
                    pos = new BlockPos(pos.getX(), 255, pos.getZ());
                }

                Chunk chunk = this.getChunkFromBlockCoords(pos);
                return chunk.getLightSubtracted(pos, this.skylightSubtracted);
            }
        }
        else
        {
            return 15;
        }
    }

    /**
     * Returns the position at this x, z coordinate in the chunk with y set to the value from the height map.
     */
    public BlockPos getHeight(BlockPos pos)
    {
        int i;

        if (pos.getX() >= -30000000 && pos.getZ() >= -30000000 && pos.getX() < 30000000 && pos.getZ() < 30000000)
        {
            if (this.isChunkLoaded(pos.getX() >> 4, pos.getZ() >> 4, true))
            {
                i = this.getChunkFromChunkCoords(pos.getX() >> 4, pos.getZ() >> 4).getHeightValue(pos.getX() & 15, pos.getZ() & 15);
            }
            else
            {
                i = 0;
            }
        }
        else
        {
            i = this.getSeaLevel() + 1;
        }

        return new BlockPos(pos.getX(), i, pos.getZ());
    }

    /**
     * Gets the lowest height of the chunk where sunlight directly reaches
     */
    public int getChunksLowestHorizon(int x, int z)
    {
        if (x >= -30000000 && z >= -30000000 && x < 30000000 && z < 30000000)
        {
            if (!this.isChunkLoaded(x >> 4, z >> 4, true))
            {
                return 0;
            }
            else
            {
                Chunk chunk = this.getChunkFromChunkCoords(x >> 4, z >> 4);
                return chunk.getLowestHeight();
            }
        }
        else
        {
            return this.getSeaLevel() + 1;
        }
    }

    public int getLightFromNeighborsFor(EnumSkyBlock type, BlockPos pos)
    {
        if (this.provider.getHasNoSky() && type == EnumSkyBlock.SKY)
        {
            return 0;
        }
        else
        {
            if (pos.getY() < 0)
            {
                pos = new BlockPos(pos.getX(), 0, pos.getZ());
            }

            if (!this.isValid(pos))
            {
                return type.defaultLightValue;
            }
            else if (!this.isBlockLoaded(pos))
            {
                return type.defaultLightValue;
            }
            else if (this.getBlockState(pos).useNeighborBrightness())
            {
                int i1 = this.getLightFor(type, pos.up());
                int i = this.getLightFor(type, pos.east());
                int j = this.getLightFor(type, pos.west());
                int k = this.getLightFor(type, pos.south());
                int l = this.getLightFor(type, pos.north());

                if (i > i1)
                {
                    i1 = i;
                }

                if (j > i1)
                {
                    i1 = j;
                }

                if (k > i1)
                {
                    i1 = k;
                }

                if (l > i1)
                {
                    i1 = l;
                }

                return i1;
            }
            else
            {
                Chunk chunk = this.getChunkFromBlockCoords(pos);
                return chunk.getLightFor(type, pos);
            }
        }
    }

    public int getLightFor(EnumSkyBlock type, BlockPos pos)
    {
        if (pos.getY() < 0)
        {
            pos = new BlockPos(pos.getX(), 0, pos.getZ());
        }

        if (!this.isValid(pos))
        {
            return type.defaultLightValue;
        }
        else if (!this.isBlockLoaded(pos))
        {
            return type.defaultLightValue;
        }
        else
        {
            Chunk chunk = this.getChunkFromBlockCoords(pos);
            return chunk.getLightFor(type, pos);
        }
    }

    public void setLightFor(EnumSkyBlock type, BlockPos pos, int lightValue)
    {
        if (this.isValid(pos))
        {
            if (this.isBlockLoaded(pos))
            {
                Chunk chunk = this.getChunkFromBlockCoords(pos);
                chunk.setLightFor(type, pos, lightValue);
                this.notifyLightSet(pos);
            }
        }
    }

    public void notifyLightSet(BlockPos pos)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).notifyLightSet(pos);
        }
    }

    public int getCombinedLight(BlockPos pos, int lightValue)
    {
        int i = this.getLightFromNeighborsFor(EnumSkyBlock.SKY, pos);
        int j = this.getLightFromNeighborsFor(EnumSkyBlock.BLOCK, pos);

        if (j < lightValue)
        {
            j = lightValue;
        }

        return i << 20 | j << 4;
    }

    public float getLightBrightness(BlockPos pos)
    {
        return this.provider.getLightBrightnessTable()[this.getLightFromNeighbors(pos)];
    }

    public IBlockState getBlockState(BlockPos pos)
    {
        if (!this.isValid(pos))
        {
            return Blocks.air.getDefaultState();
        }
        else
        {
            Chunk chunk = this.getChunkFromBlockCoords(pos);
            return chunk.getBlockState(pos);
        }
    }

    /**
     * Checks whether its daytime by seeing if the light subtracted from the skylight is less than 4
     */
    public boolean isDaytime()
    {
        return this.skylightSubtracted < 4;
    }

    /**
     * ray traces all blocks, including non-collideable ones
     */
    public RayTraceResult rayTraceBlocks(Vec3d p_72933_1_, Vec3d p_72933_2_)
    {
        return this.rayTraceBlocks(p_72933_1_, p_72933_2_, false, false, false);
    }

    public RayTraceResult rayTraceBlocks(Vec3d start, Vec3d end, boolean stopOnLiquid)
    {
        return this.rayTraceBlocks(start, end, stopOnLiquid, false, false);
    }

    /**
     * Performs a raycast against all blocks in the world. Args : Vec1, Vec2, stopOnLiquid,
     * ignoreBlockWithoutBoundingBox, returnLastUncollidableBlock
     */
    public RayTraceResult rayTraceBlocks(Vec3d vec31, Vec3d vec32, boolean stopOnLiquid, boolean ignoreBlockWithoutBoundingBox, boolean returnLastUncollidableBlock)
    {
        if (!Double.isNaN(vec31.xCoord) && !Double.isNaN(vec31.yCoord) && !Double.isNaN(vec31.zCoord))
        {
            if (!Double.isNaN(vec32.xCoord) && !Double.isNaN(vec32.yCoord) && !Double.isNaN(vec32.zCoord))
            {
                int i = MathHelper.floor_double(vec32.xCoord);
                int j = MathHelper.floor_double(vec32.yCoord);
                int k = MathHelper.floor_double(vec32.zCoord);
                int l = MathHelper.floor_double(vec31.xCoord);
                int i1 = MathHelper.floor_double(vec31.yCoord);
                int j1 = MathHelper.floor_double(vec31.zCoord);
                BlockPos blockpos = new BlockPos(l, i1, j1);
                IBlockState iblockstate = this.getBlockState(blockpos);
                Block block = iblockstate.getBlock();

                if ((!ignoreBlockWithoutBoundingBox || iblockstate.getSelectedBoundingBox(this, blockpos) != Block.NULL_AABB) && block.canCollideCheck(iblockstate, stopOnLiquid))
                {
                    RayTraceResult raytraceresult = iblockstate.func_185910_a(this, blockpos, vec31, vec32);

                    if (raytraceresult != null)
                    {
                        return raytraceresult;
                    }
                }

                RayTraceResult raytraceresult2 = null;
                int k1 = 200;

                while (k1-- >= 0)
                {
                    if (Double.isNaN(vec31.xCoord) || Double.isNaN(vec31.yCoord) || Double.isNaN(vec31.zCoord))
                    {
                        return null;
                    }

                    if (l == i && i1 == j && j1 == k)
                    {
                        return returnLastUncollidableBlock ? raytraceresult2 : null;
                    }

                    boolean flag2 = true;
                    boolean flag = true;
                    boolean flag1 = true;
                    double d0 = 999.0D;
                    double d1 = 999.0D;
                    double d2 = 999.0D;

                    if (i > l)
                    {
                        d0 = (double)l + 1.0D;
                    }
                    else if (i < l)
                    {
                        d0 = (double)l + 0.0D;
                    }
                    else
                    {
                        flag2 = false;
                    }

                    if (j > i1)
                    {
                        d1 = (double)i1 + 1.0D;
                    }
                    else if (j < i1)
                    {
                        d1 = (double)i1 + 0.0D;
                    }
                    else
                    {
                        flag = false;
                    }

                    if (k > j1)
                    {
                        d2 = (double)j1 + 1.0D;
                    }
                    else if (k < j1)
                    {
                        d2 = (double)j1 + 0.0D;
                    }
                    else
                    {
                        flag1 = false;
                    }

                    double d3 = 999.0D;
                    double d4 = 999.0D;
                    double d5 = 999.0D;
                    double d6 = vec32.xCoord - vec31.xCoord;
                    double d7 = vec32.yCoord - vec31.yCoord;
                    double d8 = vec32.zCoord - vec31.zCoord;

                    if (flag2)
                    {
                        d3 = (d0 - vec31.xCoord) / d6;
                    }

                    if (flag)
                    {
                        d4 = (d1 - vec31.yCoord) / d7;
                    }

                    if (flag1)
                    {
                        d5 = (d2 - vec31.zCoord) / d8;
                    }

                    if (d3 == -0.0D)
                    {
                        d3 = -1.0E-4D;
                    }

                    if (d4 == -0.0D)
                    {
                        d4 = -1.0E-4D;
                    }

                    if (d5 == -0.0D)
                    {
                        d5 = -1.0E-4D;
                    }

                    EnumFacing enumfacing;

                    if (d3 < d4 && d3 < d5)
                    {
                        enumfacing = i > l ? EnumFacing.WEST : EnumFacing.EAST;
                        vec31 = new Vec3d(d0, vec31.yCoord + d7 * d3, vec31.zCoord + d8 * d3);
                    }
                    else if (d4 < d5)
                    {
                        enumfacing = j > i1 ? EnumFacing.DOWN : EnumFacing.UP;
                        vec31 = new Vec3d(vec31.xCoord + d6 * d4, d1, vec31.zCoord + d8 * d4);
                    }
                    else
                    {
                        enumfacing = k > j1 ? EnumFacing.NORTH : EnumFacing.SOUTH;
                        vec31 = new Vec3d(vec31.xCoord + d6 * d5, vec31.yCoord + d7 * d5, d2);
                    }

                    l = MathHelper.floor_double(vec31.xCoord) - (enumfacing == EnumFacing.EAST ? 1 : 0);
                    i1 = MathHelper.floor_double(vec31.yCoord) - (enumfacing == EnumFacing.UP ? 1 : 0);
                    j1 = MathHelper.floor_double(vec31.zCoord) - (enumfacing == EnumFacing.SOUTH ? 1 : 0);
                    blockpos = new BlockPos(l, i1, j1);
                    IBlockState iblockstate1 = this.getBlockState(blockpos);
                    Block block1 = iblockstate1.getBlock();

                    if (!ignoreBlockWithoutBoundingBox || iblockstate1.getMaterial() == Material.portal || iblockstate1.getSelectedBoundingBox(this, blockpos) != Block.NULL_AABB)
                    {
                        if (block1.canCollideCheck(iblockstate1, stopOnLiquid))
                        {
                            RayTraceResult raytraceresult1 = iblockstate1.func_185910_a(this, blockpos, vec31, vec32);

                            if (raytraceresult1 != null)
                            {
                                return raytraceresult1;
                            }
                        }
                        else
                        {
                            raytraceresult2 = new RayTraceResult(RayTraceResult.Type.MISS, vec31, enumfacing, blockpos);
                        }
                    }
                }

                return returnLastUncollidableBlock ? raytraceresult2 : null;
            }
            else
            {
                return null;
            }
        }
        else
        {
            return null;
        }
    }

    public void func_184133_a(EntityPlayer p_184133_1_, BlockPos p_184133_2_, SoundEvent p_184133_3_, SoundCategory p_184133_4_, float p_184133_5_, float p_184133_6_)
    {
        this.func_184148_a(p_184133_1_, (double)p_184133_2_.getX() + 0.5D, (double)p_184133_2_.getY() + 0.5D, (double)p_184133_2_.getZ() + 0.5D, p_184133_3_, p_184133_4_, p_184133_5_, p_184133_6_);
    }

    public void func_184148_a(EntityPlayer player, double x, double y, double z, SoundEvent soundIn, SoundCategory category, float volume, float pitch)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).func_184375_a(player, soundIn, category, x, y, z, volume, pitch);
        }
    }

    public void func_184134_a(double p_184134_1_, double p_184134_3_, double p_184134_5_, SoundEvent p_184134_7_, SoundCategory p_184134_8_, float p_184134_9_, float p_184134_10_, boolean p_184134_11_)
    {
    }

    public void func_184149_a(BlockPos p_184149_1_, SoundEvent p_184149_2_)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).func_184377_a(p_184149_2_, p_184149_1_);
        }
    }

    public void spawnParticle(EnumParticleTypes particleType, double xCoord, double yCoord, double zCoord, double xOffset, double yOffset, double zOffset, int... p_175688_14_)
    {
        this.spawnParticle(particleType.getParticleID(), particleType.getShouldIgnoreRange(), xCoord, yCoord, zCoord, xOffset, yOffset, zOffset, p_175688_14_);
    }

    public void spawnParticle(EnumParticleTypes particleType, boolean p_175682_2_, double xCoord, double yCoord, double zCoord, double xOffset, double yOffset, double zOffset, int... p_175682_15_)
    {
        this.spawnParticle(particleType.getParticleID(), particleType.getShouldIgnoreRange() | p_175682_2_, xCoord, yCoord, zCoord, xOffset, yOffset, zOffset, p_175682_15_);
    }

    private void spawnParticle(int particleID, boolean p_175720_2_, double xCood, double yCoord, double zCoord, double xOffset, double yOffset, double zOffset, int... p_175720_15_)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).spawnParticle(particleID, p_175720_2_, xCood, yCoord, zCoord, xOffset, yOffset, zOffset, p_175720_15_);
        }
    }

    /**
     * adds a lightning bolt to the list of lightning bolts in this world.
     */
    public boolean addWeatherEffect(Entity entityIn)
    {
        this.weatherEffects.add(entityIn);
        return true;
    }

    /**
     * Called when an entity is spawned in the world. This includes players.
     */
    public boolean spawnEntityInWorld(Entity entityIn)
    {
        int i = MathHelper.floor_double(entityIn.posX / 16.0D);
        int j = MathHelper.floor_double(entityIn.posZ / 16.0D);
        boolean flag = entityIn.forceSpawn;

        if (entityIn instanceof EntityPlayer)
        {
            flag = true;
        }

        if (!flag && !this.isChunkLoaded(i, j, false))
        {
            return false;
        }
        else
        {
            if (entityIn instanceof EntityPlayer)
            {
                EntityPlayer entityplayer = (EntityPlayer)entityIn;
                this.playerEntities.add(entityplayer);
                this.updateAllPlayersSleepingFlag();
            }

            this.getChunkFromChunkCoords(i, j).addEntity(entityIn);
            this.loadedEntityList.add(entityIn);
            this.onEntityAdded(entityIn);
            return true;
        }
    }

    protected void onEntityAdded(Entity entityIn)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).onEntityAdded(entityIn);
        }
    }

    protected void onEntityRemoved(Entity entityIn)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).onEntityRemoved(entityIn);
        }
    }

    /**
     * Schedule the entity for removal during the next tick. Marks the entity dead in anticipation.
     */
    public void removeEntity(Entity entityIn)
    {
        if (entityIn.isBeingRidden())
        {
            entityIn.removePassengers();
        }

        if (entityIn.isRiding())
        {
            entityIn.dismountRidingEntity();
        }

        entityIn.setDead();

        if (entityIn instanceof EntityPlayer)
        {
            this.playerEntities.remove(entityIn);
            this.updateAllPlayersSleepingFlag();
            this.onEntityRemoved(entityIn);
        }
    }

    /**
     * Do NOT use this method to remove normal entities- use normal removeEntity
     */
    public void removePlayerEntityDangerously(Entity entityIn)
    {
        entityIn.func_184174_b(false);
        entityIn.setDead();

        if (entityIn instanceof EntityPlayer)
        {
            this.playerEntities.remove(entityIn);
            this.updateAllPlayersSleepingFlag();
        }

        int i = entityIn.chunkCoordX;
        int j = entityIn.chunkCoordZ;

        if (entityIn.addedToChunk && this.isChunkLoaded(i, j, true))
        {
            this.getChunkFromChunkCoords(i, j).removeEntity(entityIn);
        }

        this.loadedEntityList.remove(entityIn);
        this.onEntityRemoved(entityIn);
    }

    public PathWorldListener func_184140_C()
    {
        return this.pathListener;
    }

    /**
     * Adds a IWorldAccess to the list of worldAccesses
     */
    public void addWorldAccess(IWorldEventListener worldAccess)
    {
        this.worldAccesses.add(worldAccess);
    }

    /**
     * Removes a worldAccess from the worldAccesses object
     */
    public void removeWorldAccess(IWorldEventListener worldAccess)
    {
        this.worldAccesses.remove(worldAccess);
    }

    public List<AxisAlignedBB> func_184144_a(Entity p_184144_1_, AxisAlignedBB p_184144_2_)
    {
        List<AxisAlignedBB> list = Lists.<AxisAlignedBB>newArrayList();
        int i = MathHelper.floor_double(p_184144_2_.minX) - 1;
        int j = MathHelper.ceiling_double_int(p_184144_2_.maxX) + 1;
        int k = MathHelper.floor_double(p_184144_2_.minY) - 1;
        int l = MathHelper.ceiling_double_int(p_184144_2_.maxY) + 1;
        int i1 = MathHelper.floor_double(p_184144_2_.minZ) - 1;
        int j1 = MathHelper.ceiling_double_int(p_184144_2_.maxZ) + 1;
        WorldBorder worldborder = this.getWorldBorder();
        boolean flag = p_184144_1_ != null && p_184144_1_.isOutsideBorder();
        boolean flag1 = p_184144_1_ != null && this.isInsideBorder(worldborder, p_184144_1_);
        IBlockState iblockstate = Blocks.stone.getDefaultState();
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        for (int k1 = i; k1 < j; ++k1)
        {
            for (int l1 = i1; l1 < j1; ++l1)
            {
                int i2 = (k1 != i && k1 != j - 1 ? 0 : 1) + (l1 != i1 && l1 != j1 - 1 ? 0 : 1);

                if (i2 != 2 && this.isBlockLoaded(blockpos$pooledmutableblockpos.set(k1, 64, l1)))
                {
                    for (int j2 = k; j2 < l; ++j2)
                    {
                        if (i2 <= 0 || j2 != k && j2 != l - 1)
                        {
                            blockpos$pooledmutableblockpos.set(k1, j2, l1);

                            if (p_184144_1_ != null)
                            {
                                if (flag && flag1)
                                {
                                    p_184144_1_.setOutsideBorder(false);
                                }
                                else if (!flag && !flag1)
                                {
                                    p_184144_1_.setOutsideBorder(true);
                                }
                            }

                            IBlockState iblockstate1 = iblockstate;

                            if (worldborder.contains(blockpos$pooledmutableblockpos) || !flag1)
                            {
                                iblockstate1 = this.getBlockState(blockpos$pooledmutableblockpos);
                            }

                            iblockstate1.func_185908_a(this, blockpos$pooledmutableblockpos, p_184144_2_, list, p_184144_1_);
                        }
                    }
                }
            }
        }

        blockpos$pooledmutableblockpos.release();

        if (p_184144_1_ != null)
        {
            List<Entity> list1 = this.getEntitiesWithinAABBExcludingEntity(p_184144_1_, p_184144_2_.func_186662_g(0.25D));

            for (int k2 = 0; k2 < list1.size(); ++k2)
            {
                Entity entity = (Entity)list1.get(k2);

                if (!p_184144_1_.isRidingSameEntity(entity))
                {
                    AxisAlignedBB axisalignedbb = entity.getCollisionBoundingBox();

                    if (axisalignedbb != null && axisalignedbb.intersectsWith(p_184144_2_))
                    {
                        list.add(axisalignedbb);
                    }

                    axisalignedbb = p_184144_1_.getCollisionBox(entity);

                    if (axisalignedbb != null && axisalignedbb.intersectsWith(p_184144_2_))
                    {
                        list.add(axisalignedbb);
                    }
                }
            }
        }

        return list;
    }

    public boolean isInsideBorder(WorldBorder worldBorderIn, Entity entityIn)
    {
        double d0 = worldBorderIn.minX();
        double d1 = worldBorderIn.minZ();
        double d2 = worldBorderIn.maxX();
        double d3 = worldBorderIn.maxZ();

        if (entityIn.isOutsideBorder())
        {
            ++d0;
            ++d1;
            --d2;
            --d3;
        }
        else
        {
            --d0;
            --d1;
            ++d2;
            ++d3;
        }

        return entityIn.posX > d0 && entityIn.posX < d2 && entityIn.posZ > d1 && entityIn.posZ < d3;
    }

    public List<AxisAlignedBB> getCollisionBoxes(AxisAlignedBB bb)
    {
        List<AxisAlignedBB> list = Lists.<AxisAlignedBB>newArrayList();
        int i = MathHelper.floor_double(bb.minX) - 1;
        int j = MathHelper.ceiling_double_int(bb.maxX) + 1;
        int k = MathHelper.floor_double(bb.minY) - 1;
        int l = MathHelper.ceiling_double_int(bb.maxY) + 1;
        int i1 = MathHelper.floor_double(bb.minZ) - 1;
        int j1 = MathHelper.ceiling_double_int(bb.maxZ) + 1;
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        for (int k1 = i; k1 < j; ++k1)
        {
            for (int l1 = i1; l1 < j1; ++l1)
            {
                int i2 = (k1 != i && k1 != j - 1 ? 0 : 1) + (l1 != i1 && l1 != j1 - 1 ? 0 : 1);

                if (i2 != 2 && this.isBlockLoaded(blockpos$pooledmutableblockpos.set(k1, 64, l1)))
                {
                    for (int j2 = k; j2 < l; ++j2)
                    {
                        if (i2 <= 0 || j2 != k && j2 != l - 1)
                        {
                            blockpos$pooledmutableblockpos.set(k1, j2, l1);
                            IBlockState iblockstate;

                            if (k1 >= -30000000 && k1 < 30000000 && l1 >= -30000000 && l1 < 30000000)
                            {
                                iblockstate = this.getBlockState(blockpos$pooledmutableblockpos);
                            }
                            else
                            {
                                iblockstate = Blocks.bedrock.getDefaultState();
                            }

                            iblockstate.func_185908_a(this, blockpos$pooledmutableblockpos, bb, list, (Entity)null);
                        }
                    }
                }
            }
        }

        blockpos$pooledmutableblockpos.release();
        return list;
    }

    public boolean func_184143_b(AxisAlignedBB p_184143_1_)
    {
        List<AxisAlignedBB> list = Lists.<AxisAlignedBB>newArrayList();
        int i = MathHelper.floor_double(p_184143_1_.minX) - 1;
        int j = MathHelper.ceiling_double_int(p_184143_1_.maxX) + 1;
        int k = MathHelper.floor_double(p_184143_1_.minY) - 1;
        int l = MathHelper.ceiling_double_int(p_184143_1_.maxY) + 1;
        int i1 = MathHelper.floor_double(p_184143_1_.minZ) - 1;
        int j1 = MathHelper.ceiling_double_int(p_184143_1_.maxZ) + 1;
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        try
        {
            for (int k1 = i; k1 < j; ++k1)
            {
                for (int l1 = i1; l1 < j1; ++l1)
                {
                    int i2 = (k1 != i && k1 != j - 1 ? 0 : 1) + (l1 != i1 && l1 != j1 - 1 ? 0 : 1);

                    if (i2 != 2 && this.isBlockLoaded(blockpos$pooledmutableblockpos.set(k1, 64, l1)))
                    {
                        for (int j2 = k; j2 < l; ++j2)
                        {
                            if (i2 <= 0 || j2 != k && j2 != l - 1)
                            {
                                blockpos$pooledmutableblockpos.set(k1, j2, l1);

                                if (k1 < -30000000 || k1 >= 30000000 || l1 < -30000000 || l1 >= 30000000)
                                {
                                    boolean flag1 = true;
                                    return flag1;
                                }

                                IBlockState iblockstate = this.getBlockState(blockpos$pooledmutableblockpos);
                                iblockstate.func_185908_a(this, blockpos$pooledmutableblockpos, p_184143_1_, list, (Entity)null);

                                if (!list.isEmpty())
                                {
                                    boolean flag = true;
                                    return flag;
                                }
                            }
                        }
                    }
                }
            }

            return false;
        }
        finally
        {
            blockpos$pooledmutableblockpos.release();
        }
    }

    /**
     * Returns the amount of skylight subtracted for the current time
     */
    public int calculateSkylightSubtracted(float p_72967_1_)
    {
        float f = this.getCelestialAngle(p_72967_1_);
        float f1 = 1.0F - (MathHelper.cos(f * ((float)Math.PI * 2F)) * 2.0F + 0.5F);
        f1 = MathHelper.clamp_float(f1, 0.0F, 1.0F);
        f1 = 1.0F - f1;
        f1 = (float)((double)f1 * (1.0D - (double)(this.getRainStrength(p_72967_1_) * 5.0F) / 16.0D));
        f1 = (float)((double)f1 * (1.0D - (double)(this.getThunderStrength(p_72967_1_) * 5.0F) / 16.0D));
        f1 = 1.0F - f1;
        return (int)(f1 * 11.0F);
    }

    /**
     * Returns the sun brightness - checks time of day, rain and thunder
     */
    public float getSunBrightness(float p_72971_1_)
    {
        float f = this.getCelestialAngle(p_72971_1_);
        float f1 = 1.0F - (MathHelper.cos(f * ((float)Math.PI * 2F)) * 2.0F + 0.2F);
        f1 = MathHelper.clamp_float(f1, 0.0F, 1.0F);
        f1 = 1.0F - f1;
        f1 = (float)((double)f1 * (1.0D - (double)(this.getRainStrength(p_72971_1_) * 5.0F) / 16.0D));
        f1 = (float)((double)f1 * (1.0D - (double)(this.getThunderStrength(p_72971_1_) * 5.0F) / 16.0D));
        return f1 * 0.8F + 0.2F;
    }

    /**
     * Calculates the color for the skybox
     */
    public Vec3d getSkyColor(Entity entityIn, float partialTicks)
    {
        float f = this.getCelestialAngle(partialTicks);
        float f1 = MathHelper.cos(f * ((float)Math.PI * 2F)) * 2.0F + 0.5F;
        f1 = MathHelper.clamp_float(f1, 0.0F, 1.0F);
        int i = MathHelper.floor_double(entityIn.posX);
        int j = MathHelper.floor_double(entityIn.posY);
        int k = MathHelper.floor_double(entityIn.posZ);
        BlockPos blockpos = new BlockPos(i, j, k);
        BiomeGenBase biomegenbase = this.getBiomeGenForCoords(blockpos);
        float f2 = biomegenbase.getFloatTemperature(blockpos);
        int l = biomegenbase.getSkyColorByTemp(f2);
        float f3 = (float)(l >> 16 & 255) / 255.0F;
        float f4 = (float)(l >> 8 & 255) / 255.0F;
        float f5 = (float)(l & 255) / 255.0F;
        f3 = f3 * f1;
        f4 = f4 * f1;
        f5 = f5 * f1;
        float f6 = this.getRainStrength(partialTicks);

        if (f6 > 0.0F)
        {
            float f7 = (f3 * 0.3F + f4 * 0.59F + f5 * 0.11F) * 0.6F;
            float f8 = 1.0F - f6 * 0.75F;
            f3 = f3 * f8 + f7 * (1.0F - f8);
            f4 = f4 * f8 + f7 * (1.0F - f8);
            f5 = f5 * f8 + f7 * (1.0F - f8);
        }

        float f10 = this.getThunderStrength(partialTicks);

        if (f10 > 0.0F)
        {
            float f11 = (f3 * 0.3F + f4 * 0.59F + f5 * 0.11F) * 0.2F;
            float f9 = 1.0F - f10 * 0.75F;
            f3 = f3 * f9 + f11 * (1.0F - f9);
            f4 = f4 * f9 + f11 * (1.0F - f9);
            f5 = f5 * f9 + f11 * (1.0F - f9);
        }

        if (this.lastLightningBolt > 0)
        {
            float f12 = (float)this.lastLightningBolt - partialTicks;

            if (f12 > 1.0F)
            {
                f12 = 1.0F;
            }

            f12 = f12 * 0.45F;
            f3 = f3 * (1.0F - f12) + 0.8F * f12;
            f4 = f4 * (1.0F - f12) + 0.8F * f12;
            f5 = f5 * (1.0F - f12) + 1.0F * f12;
        }

        return new Vec3d((double)f3, (double)f4, (double)f5);
    }

    /**
     * calls calculateCelestialAngle
     */
    public float getCelestialAngle(float partialTicks)
    {
        return this.provider.calculateCelestialAngle(this.worldInfo.getWorldTime(), partialTicks);
    }

    public int getMoonPhase()
    {
        return this.provider.getMoonPhase(this.worldInfo.getWorldTime());
    }

    /**
     * gets the current fullness of the moon expressed as a float between 1.0 and 0.0, in steps of .25
     */
    public float getCurrentMoonPhaseFactor()
    {
        return WorldProvider.moonPhaseFactors[this.provider.getMoonPhase(this.worldInfo.getWorldTime())];
    }

    /**
     * Return getCelestialAngle()*2*PI
     */
    public float getCelestialAngleRadians(float partialTicks)
    {
        float f = this.getCelestialAngle(partialTicks);
        return f * ((float)Math.PI * 2F);
    }

    public Vec3d getCloudColour(float partialTicks)
    {
        float f = this.getCelestialAngle(partialTicks);
        float f1 = MathHelper.cos(f * ((float)Math.PI * 2F)) * 2.0F + 0.5F;
        f1 = MathHelper.clamp_float(f1, 0.0F, 1.0F);
        float f2 = (float)(this.cloudColour >> 16 & 255L) / 255.0F;
        float f3 = (float)(this.cloudColour >> 8 & 255L) / 255.0F;
        float f4 = (float)(this.cloudColour & 255L) / 255.0F;
        float f5 = this.getRainStrength(partialTicks);

        if (f5 > 0.0F)
        {
            float f6 = (f2 * 0.3F + f3 * 0.59F + f4 * 0.11F) * 0.6F;
            float f7 = 1.0F - f5 * 0.95F;
            f2 = f2 * f7 + f6 * (1.0F - f7);
            f3 = f3 * f7 + f6 * (1.0F - f7);
            f4 = f4 * f7 + f6 * (1.0F - f7);
        }

        f2 = f2 * (f1 * 0.9F + 0.1F);
        f3 = f3 * (f1 * 0.9F + 0.1F);
        f4 = f4 * (f1 * 0.85F + 0.15F);
        float f9 = this.getThunderStrength(partialTicks);

        if (f9 > 0.0F)
        {
            float f10 = (f2 * 0.3F + f3 * 0.59F + f4 * 0.11F) * 0.2F;
            float f8 = 1.0F - f9 * 0.95F;
            f2 = f2 * f8 + f10 * (1.0F - f8);
            f3 = f3 * f8 + f10 * (1.0F - f8);
            f4 = f4 * f8 + f10 * (1.0F - f8);
        }

        return new Vec3d((double)f2, (double)f3, (double)f4);
    }

    /**
     * Returns vector(ish) with R/G/B for fog
     */
    public Vec3d getFogColor(float partialTicks)
    {
        float f = this.getCelestialAngle(partialTicks);
        return this.provider.getFogColor(f, partialTicks);
    }

    public BlockPos getPrecipitationHeight(BlockPos pos)
    {
        return this.getChunkFromBlockCoords(pos).getPrecipitationHeight(pos);
    }

    /**
     * Finds the highest block on the x and z coordinate that is solid or liquid, and returns its y coord.
     */
    public BlockPos getTopSolidOrLiquidBlock(BlockPos pos)
    {
        Chunk chunk = this.getChunkFromBlockCoords(pos);
        BlockPos blockpos;
        BlockPos blockpos1;

        for (blockpos = new BlockPos(pos.getX(), chunk.getTopFilledSegment() + 16, pos.getZ()); blockpos.getY() >= 0; blockpos = blockpos1)
        {
            blockpos1 = blockpos.down();
            Material material = chunk.getBlockState(blockpos1).getMaterial();

            if (material.blocksMovement() && material != Material.leaves)
            {
                break;
            }
        }

        return blockpos;
    }

    /**
     * How bright are stars in the sky
     */
    public float getStarBrightness(float partialTicks)
    {
        float f = this.getCelestialAngle(partialTicks);
        float f1 = 1.0F - (MathHelper.cos(f * ((float)Math.PI * 2F)) * 2.0F + 0.25F);
        f1 = MathHelper.clamp_float(f1, 0.0F, 1.0F);
        return f1 * f1 * 0.5F;
    }

    public boolean func_184145_b(BlockPos p_184145_1_, Block p_184145_2_)
    {
        return true;
    }

    public void scheduleUpdate(BlockPos pos, Block blockIn, int delay)
    {
    }

    public void updateBlockTick(BlockPos pos, Block blockIn, int delay, int priority)
    {
    }

    public void scheduleBlockUpdate(BlockPos pos, Block blockIn, int delay, int priority)
    {
    }

    /**
     * Updates (and cleans up) entities and tile entities
     */
    public void updateEntities()
    {
        this.theProfiler.startSection("entities");
        this.theProfiler.startSection("global");

        for (int i = 0; i < this.weatherEffects.size(); ++i)
        {
            Entity entity = (Entity)this.weatherEffects.get(i);

            try
            {
                ++entity.ticksExisted;
                entity.onUpdate();
            }
            catch (Throwable throwable2)
            {
                CrashReport crashreport = CrashReport.makeCrashReport(throwable2, "Ticking entity");
                CrashReportCategory crashreportcategory = crashreport.makeCategory("Entity being ticked");

                if (entity == null)
                {
                    crashreportcategory.addCrashSection("Entity", "~~NULL~~");
                }
                else
                {
                    entity.addEntityCrashInfo(crashreportcategory);
                }

                throw new ReportedException(crashreport);
            }

            if (entity.isDead)
            {
                this.weatherEffects.remove(i--);
            }
        }

        this.theProfiler.endStartSection("remove");
        this.loadedEntityList.removeAll(this.unloadedEntityList);

        for (int k = 0; k < this.unloadedEntityList.size(); ++k)
        {
            Entity entity1 = (Entity)this.unloadedEntityList.get(k);
            int j = entity1.chunkCoordX;
            int k1 = entity1.chunkCoordZ;

            if (entity1.addedToChunk && this.isChunkLoaded(j, k1, true))
            {
                this.getChunkFromChunkCoords(j, k1).removeEntity(entity1);
            }
        }

        for (int l = 0; l < this.unloadedEntityList.size(); ++l)
        {
            this.onEntityRemoved((Entity)this.unloadedEntityList.get(l));
        }

        this.unloadedEntityList.clear();
        this.tickPlayers();
        this.theProfiler.endStartSection("regular");

        for (int i1 = 0; i1 < this.loadedEntityList.size(); ++i1)
        {
            Entity entity2 = (Entity)this.loadedEntityList.get(i1);
            Entity entity3 = entity2.getRidingEntity();

            if (entity3 != null)
            {
                if (!entity3.isDead && entity3.isPassenger(entity2))
                {
                    continue;
                }

                entity2.dismountRidingEntity();
            }

            this.theProfiler.startSection("tick");

            if (!entity2.isDead && !(entity2 instanceof EntityPlayerMP))
            {
                try
                {
                    this.updateEntity(entity2);
                }
                catch (Throwable throwable1)
                {
                    CrashReport crashreport1 = CrashReport.makeCrashReport(throwable1, "Ticking entity");
                    CrashReportCategory crashreportcategory1 = crashreport1.makeCategory("Entity being ticked");
                    entity2.addEntityCrashInfo(crashreportcategory1);
                    throw new ReportedException(crashreport1);
                }
            }

            this.theProfiler.endSection();
            this.theProfiler.startSection("remove");

            if (entity2.isDead)
            {
                int l1 = entity2.chunkCoordX;
                int i2 = entity2.chunkCoordZ;

                if (entity2.addedToChunk && this.isChunkLoaded(l1, i2, true))
                {
                    this.getChunkFromChunkCoords(l1, i2).removeEntity(entity2);
                }

                this.loadedEntityList.remove(i1--);
                this.onEntityRemoved(entity2);
            }

            this.theProfiler.endSection();
        }

        this.theProfiler.endStartSection("blockEntities");
        this.processingLoadedTiles = true;
        Iterator<TileEntity> iterator = this.tickableTileEntities.iterator();

        while (iterator.hasNext())
        {
            TileEntity tileentity = (TileEntity)iterator.next();

            if (!tileentity.isInvalid() && tileentity.hasWorldObj())
            {
                BlockPos blockpos = tileentity.getPos();

                if (this.isBlockLoaded(blockpos) && this.worldBorder.contains(blockpos))
                {
                    try
                    {
                        this.theProfiler.startSection(tileentity.getClass().getSimpleName());
                        ((ITickable)tileentity).update();
                        this.theProfiler.endSection();
                    }
                    catch (Throwable throwable)
                    {
                        CrashReport crashreport2 = CrashReport.makeCrashReport(throwable, "Ticking block entity");
                        CrashReportCategory crashreportcategory2 = crashreport2.makeCategory("Block entity being ticked");
                        tileentity.addInfoToCrashReport(crashreportcategory2);
                        throw new ReportedException(crashreport2);
                    }
                }
            }

            if (tileentity.isInvalid())
            {
                iterator.remove();
                this.loadedTileEntityList.remove(tileentity);

                if (this.isBlockLoaded(tileentity.getPos()))
                {
                    this.getChunkFromBlockCoords(tileentity.getPos()).removeTileEntity(tileentity.getPos());
                }
            }
        }

        this.processingLoadedTiles = false;

        if (!this.tileEntitiesToBeRemoved.isEmpty())
        {
            this.tickableTileEntities.removeAll(this.tileEntitiesToBeRemoved);
            this.loadedTileEntityList.removeAll(this.tileEntitiesToBeRemoved);
            this.tileEntitiesToBeRemoved.clear();
        }

        this.theProfiler.endStartSection("pendingBlockEntities");

        if (!this.addedTileEntityList.isEmpty())
        {
            for (int j1 = 0; j1 < this.addedTileEntityList.size(); ++j1)
            {
                TileEntity tileentity1 = (TileEntity)this.addedTileEntityList.get(j1);

                if (!tileentity1.isInvalid())
                {
                    if (!this.loadedTileEntityList.contains(tileentity1))
                    {
                        this.addTileEntity(tileentity1);
                    }

                    if (this.isBlockLoaded(tileentity1.getPos()))
                    {
                        Chunk chunk = this.getChunkFromBlockCoords(tileentity1.getPos());
                        IBlockState iblockstate = chunk.getBlockState(tileentity1.getPos());
                        chunk.addTileEntity(tileentity1.getPos(), tileentity1);
                        this.notifyBlockUpdate(tileentity1.getPos(), iblockstate, iblockstate, 3);
                    }
                }
            }

            this.addedTileEntityList.clear();
        }

        this.theProfiler.endSection();
        this.theProfiler.endSection();
    }

    protected void tickPlayers()
    {
    }

    public boolean addTileEntity(TileEntity tile)
    {
        boolean flag = this.loadedTileEntityList.add(tile);

        if (flag && tile instanceof ITickable)
        {
            this.tickableTileEntities.add(tile);
        }

        return flag;
    }

    public void addTileEntities(Collection<TileEntity> tileEntityCollection)
    {
        if (this.processingLoadedTiles)
        {
            this.addedTileEntityList.addAll(tileEntityCollection);
        }
        else
        {
            for (TileEntity tileentity : tileEntityCollection)
            {
                this.addTileEntity(tileentity);
            }
        }
    }

    /**
     * Will update the entity in the world if the chunk the entity is in is currently loaded. Args: entity
     */
    public void updateEntity(Entity ent)
    {
        this.updateEntityWithOptionalForce(ent, true);
    }

    /**
     * Will update the entity in the world if the chunk the entity is in is currently loaded or its forced to update.
     * Args: entity, forceUpdate
     */
    public void updateEntityWithOptionalForce(Entity entityIn, boolean forceUpdate)
    {
        int i = MathHelper.floor_double(entityIn.posX);
        int j = MathHelper.floor_double(entityIn.posZ);
        int k = 32;

        if (!forceUpdate || this.isAreaLoaded(i - k, 0, j - k, i + k, 0, j + k, true))
        {
            entityIn.lastTickPosX = entityIn.posX;
            entityIn.lastTickPosY = entityIn.posY;
            entityIn.lastTickPosZ = entityIn.posZ;
            entityIn.prevRotationYaw = entityIn.rotationYaw;
            entityIn.prevRotationPitch = entityIn.rotationPitch;

            if (forceUpdate && entityIn.addedToChunk)
            {
                ++entityIn.ticksExisted;

                if (entityIn.isRiding())
                {
                    entityIn.updateRidden();
                }
                else
                {
                    entityIn.onUpdate();
                }
            }

            this.theProfiler.startSection("chunkCheck");

            if (Double.isNaN(entityIn.posX) || Double.isInfinite(entityIn.posX))
            {
                entityIn.posX = entityIn.lastTickPosX;
            }

            if (Double.isNaN(entityIn.posY) || Double.isInfinite(entityIn.posY))
            {
                entityIn.posY = entityIn.lastTickPosY;
            }

            if (Double.isNaN(entityIn.posZ) || Double.isInfinite(entityIn.posZ))
            {
                entityIn.posZ = entityIn.lastTickPosZ;
            }

            if (Double.isNaN((double)entityIn.rotationPitch) || Double.isInfinite((double)entityIn.rotationPitch))
            {
                entityIn.rotationPitch = entityIn.prevRotationPitch;
            }

            if (Double.isNaN((double)entityIn.rotationYaw) || Double.isInfinite((double)entityIn.rotationYaw))
            {
                entityIn.rotationYaw = entityIn.prevRotationYaw;
            }

            int l = MathHelper.floor_double(entityIn.posX / 16.0D);
            int i1 = MathHelper.floor_double(entityIn.posY / 16.0D);
            int j1 = MathHelper.floor_double(entityIn.posZ / 16.0D);

            if (!entityIn.addedToChunk || entityIn.chunkCoordX != l || entityIn.chunkCoordY != i1 || entityIn.chunkCoordZ != j1)
            {
                if (entityIn.addedToChunk && this.isChunkLoaded(entityIn.chunkCoordX, entityIn.chunkCoordZ, true))
                {
                    this.getChunkFromChunkCoords(entityIn.chunkCoordX, entityIn.chunkCoordZ).removeEntityAtIndex(entityIn, entityIn.chunkCoordY);
                }

                if (!entityIn.func_184189_br() && !this.isChunkLoaded(l, j1, true))
                {
                    entityIn.addedToChunk = false;
                }
                else
                {
                    this.getChunkFromChunkCoords(l, j1).addEntity(entityIn);
                }
            }

            this.theProfiler.endSection();

            if (forceUpdate && entityIn.addedToChunk)
            {
                for (Entity entity : entityIn.getPassengers())
                {
                    if (!entity.isDead && entity.getRidingEntity() == entityIn)
                    {
                        this.updateEntity(entity);
                    }
                    else
                    {
                        entity.dismountRidingEntity();
                    }
                }
            }
        }
    }

    /**
     * Returns true if there are no solid, live entities in the specified AxisAlignedBB
     */
    public boolean checkNoEntityCollision(AxisAlignedBB bb)
    {
        return this.checkNoEntityCollision(bb, (Entity)null);
    }

    /**
     * Returns true if there are no solid, live entities in the specified AxisAlignedBB, excluding the given entity
     */
    public boolean checkNoEntityCollision(AxisAlignedBB bb, Entity entityIn)
    {
        List<Entity> list = this.getEntitiesWithinAABBExcludingEntity((Entity)null, bb);

        for (int i = 0; i < list.size(); ++i)
        {
            Entity entity = (Entity)list.get(i);

            if (!entity.isDead && entity.preventEntitySpawning && entity != entityIn && (entityIn == null || entity.isRidingSameEntity(entityIn)))
            {
                return false;
            }
        }

        return true;
    }

    /**
     * Returns true if there are any blocks in the region constrained by an AxisAlignedBB
     */
    public boolean checkBlockCollision(AxisAlignedBB bb)
    {
        int i = MathHelper.floor_double(bb.minX);
        int j = MathHelper.ceiling_double_int(bb.maxX);
        int k = MathHelper.floor_double(bb.minY);
        int l = MathHelper.ceiling_double_int(bb.maxY);
        int i1 = MathHelper.floor_double(bb.minZ);
        int j1 = MathHelper.ceiling_double_int(bb.maxZ);
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        for (int k1 = i; k1 < j; ++k1)
        {
            for (int l1 = k; l1 < l; ++l1)
            {
                for (int i2 = i1; i2 < j1; ++i2)
                {
                    IBlockState iblockstate = this.getBlockState(blockpos$pooledmutableblockpos.set(k1, l1, i2));

                    if (iblockstate.getMaterial() != Material.air)
                    {
                        blockpos$pooledmutableblockpos.release();
                        return true;
                    }
                }
            }
        }

        blockpos$pooledmutableblockpos.release();
        return false;
    }

    /**
     * Returns if any of the blocks within the aabb are liquids. Args: aabb
     */
    public boolean isAnyLiquid(AxisAlignedBB bb)
    {
        int i = MathHelper.floor_double(bb.minX);
        int j = MathHelper.ceiling_double_int(bb.maxX);
        int k = MathHelper.floor_double(bb.minY);
        int l = MathHelper.ceiling_double_int(bb.maxY);
        int i1 = MathHelper.floor_double(bb.minZ);
        int j1 = MathHelper.ceiling_double_int(bb.maxZ);
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        for (int k1 = i; k1 < j; ++k1)
        {
            for (int l1 = k; l1 < l; ++l1)
            {
                for (int i2 = i1; i2 < j1; ++i2)
                {
                    IBlockState iblockstate = this.getBlockState(blockpos$pooledmutableblockpos.set(k1, l1, i2));

                    if (iblockstate.getMaterial().isLiquid())
                    {
                        blockpos$pooledmutableblockpos.release();
                        return true;
                    }
                }
            }
        }

        blockpos$pooledmutableblockpos.release();
        return false;
    }

    public boolean isFlammableWithin(AxisAlignedBB bb)
    {
        int i = MathHelper.floor_double(bb.minX);
        int j = MathHelper.ceiling_double_int(bb.maxX);
        int k = MathHelper.floor_double(bb.minY);
        int l = MathHelper.ceiling_double_int(bb.maxY);
        int i1 = MathHelper.floor_double(bb.minZ);
        int j1 = MathHelper.ceiling_double_int(bb.maxZ);

        if (this.isAreaLoaded(i, k, i1, j, l, j1, true))
        {
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

            for (int k1 = i; k1 < j; ++k1)
            {
                for (int l1 = k; l1 < l; ++l1)
                {
                    for (int i2 = i1; i2 < j1; ++i2)
                    {
                        Block block = this.getBlockState(blockpos$pooledmutableblockpos.set(k1, l1, i2)).getBlock();

                        if (block == Blocks.fire || block == Blocks.flowing_lava || block == Blocks.lava)
                        {
                            blockpos$pooledmutableblockpos.release();
                            return true;
                        }
                    }
                }
            }

            blockpos$pooledmutableblockpos.release();
        }

        return false;
    }

    /**
     * handles the acceleration of an object whilst in water. Not sure if it is used elsewhere.
     */
    public boolean handleMaterialAcceleration(AxisAlignedBB bb, Material materialIn, Entity entityIn)
    {
        int i = MathHelper.floor_double(bb.minX);
        int j = MathHelper.ceiling_double_int(bb.maxX);
        int k = MathHelper.floor_double(bb.minY);
        int l = MathHelper.ceiling_double_int(bb.maxY);
        int i1 = MathHelper.floor_double(bb.minZ);
        int j1 = MathHelper.ceiling_double_int(bb.maxZ);

        if (!this.isAreaLoaded(i, k, i1, j, l, j1, true))
        {
            return false;
        }
        else
        {
            boolean flag = false;
            Vec3d vec3d = Vec3d.ZERO;
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

            for (int k1 = i; k1 < j; ++k1)
            {
                for (int l1 = k; l1 < l; ++l1)
                {
                    for (int i2 = i1; i2 < j1; ++i2)
                    {
                        blockpos$pooledmutableblockpos.set(k1, l1, i2);
                        IBlockState iblockstate = this.getBlockState(blockpos$pooledmutableblockpos);
                        Block block = iblockstate.getBlock();

                        if (iblockstate.getMaterial() == materialIn)
                        {
                            double d0 = (double)((float)(l1 + 1) - BlockLiquid.getLiquidHeightPercent(((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue()));

                            if ((double)l >= d0)
                            {
                                flag = true;
                                vec3d = block.modifyAcceleration(this, blockpos$pooledmutableblockpos, entityIn, vec3d);
                            }
                        }
                    }
                }
            }

            blockpos$pooledmutableblockpos.release();

            if (vec3d.lengthVector() > 0.0D && entityIn.isPushedByWater())
            {
                vec3d = vec3d.normalize();
                double d1 = 0.014D;
                entityIn.motionX += vec3d.xCoord * d1;
                entityIn.motionY += vec3d.yCoord * d1;
                entityIn.motionZ += vec3d.zCoord * d1;
            }

            return flag;
        }
    }

    /**
     * Returns true if the given bounding box contains the given material
     */
    public boolean isMaterialInBB(AxisAlignedBB bb, Material materialIn)
    {
        int i = MathHelper.floor_double(bb.minX);
        int j = MathHelper.ceiling_double_int(bb.maxX);
        int k = MathHelper.floor_double(bb.minY);
        int l = MathHelper.ceiling_double_int(bb.maxY);
        int i1 = MathHelper.floor_double(bb.minZ);
        int j1 = MathHelper.ceiling_double_int(bb.maxZ);
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        for (int k1 = i; k1 < j; ++k1)
        {
            for (int l1 = k; l1 < l; ++l1)
            {
                for (int i2 = i1; i2 < j1; ++i2)
                {
                    if (this.getBlockState(blockpos$pooledmutableblockpos.set(k1, l1, i2)).getMaterial() == materialIn)
                    {
                        blockpos$pooledmutableblockpos.release();
                        return true;
                    }
                }
            }
        }

        blockpos$pooledmutableblockpos.release();
        return false;
    }

    /**
     * checks if the given AABB is in the material given. Used while swimming.
     */
    public boolean isAABBInMaterial(AxisAlignedBB bb, Material materialIn)
    {
        int i = MathHelper.floor_double(bb.minX);
        int j = MathHelper.ceiling_double_int(bb.maxX);
        int k = MathHelper.floor_double(bb.minY);
        int l = MathHelper.ceiling_double_int(bb.maxY);
        int i1 = MathHelper.floor_double(bb.minZ);
        int j1 = MathHelper.ceiling_double_int(bb.maxZ);
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        for (int k1 = i; k1 < j; ++k1)
        {
            for (int l1 = k; l1 < l; ++l1)
            {
                for (int i2 = i1; i2 < j1; ++i2)
                {
                    IBlockState iblockstate = this.getBlockState(blockpos$pooledmutableblockpos.set(k1, l1, i2));

                    if (iblockstate.getMaterial() == materialIn)
                    {
                        int j2 = ((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue();
                        double d0 = (double)(l1 + 1);

                        if (j2 < 8)
                        {
                            d0 = (double)(l1 + 1) - (double)j2 / 8.0D;
                        }

                        if (d0 >= bb.minY)
                        {
                            blockpos$pooledmutableblockpos.release();
                            return true;
                        }
                    }
                }
            }
        }

        blockpos$pooledmutableblockpos.release();
        return false;
    }

    /**
     * Creates an explosion. Args: entity, x, y, z, strength
     */
    public Explosion createExplosion(Entity entityIn, double x, double y, double z, float strength, boolean isSmoking)
    {
        return this.newExplosion(entityIn, x, y, z, strength, false, isSmoking);
    }

    /**
     * returns a new explosion. Does initiation (at time of writing Explosion is not finished)
     */
    public Explosion newExplosion(Entity entityIn, double x, double y, double z, float strength, boolean isFlaming, boolean isSmoking)
    {
        Explosion explosion = new Explosion(this, entityIn, x, y, z, strength, isFlaming, isSmoking);
        explosion.doExplosionA();
        explosion.doExplosionB(true);
        return explosion;
    }

    /**
     * Gets the percentage of real blocks within within a bounding box, along a specified vector.
     */
    public float getBlockDensity(Vec3d vec, AxisAlignedBB bb)
    {
        double d0 = 1.0D / ((bb.maxX - bb.minX) * 2.0D + 1.0D);
        double d1 = 1.0D / ((bb.maxY - bb.minY) * 2.0D + 1.0D);
        double d2 = 1.0D / ((bb.maxZ - bb.minZ) * 2.0D + 1.0D);
        double d3 = (1.0D - Math.floor(1.0D / d0) * d0) / 2.0D;
        double d4 = (1.0D - Math.floor(1.0D / d2) * d2) / 2.0D;

        if (d0 >= 0.0D && d1 >= 0.0D && d2 >= 0.0D)
        {
            int i = 0;
            int j = 0;

            for (float f = 0.0F; f <= 1.0F; f = (float)((double)f + d0))
            {
                for (float f1 = 0.0F; f1 <= 1.0F; f1 = (float)((double)f1 + d1))
                {
                    for (float f2 = 0.0F; f2 <= 1.0F; f2 = (float)((double)f2 + d2))
                    {
                        double d5 = bb.minX + (bb.maxX - bb.minX) * (double)f;
                        double d6 = bb.minY + (bb.maxY - bb.minY) * (double)f1;
                        double d7 = bb.minZ + (bb.maxZ - bb.minZ) * (double)f2;

                        if (this.rayTraceBlocks(new Vec3d(d5 + d3, d6, d7 + d4), vec) == null)
                        {
                            ++i;
                        }

                        ++j;
                    }
                }
            }

            return (float)i / (float)j;
        }
        else
        {
            return 0.0F;
        }
    }

    /**
     * Attempts to extinguish a fire
     */
    public boolean extinguishFire(EntityPlayer player, BlockPos pos, EnumFacing side)
    {
        pos = pos.offset(side);

        if (this.getBlockState(pos).getBlock() == Blocks.fire)
        {
            this.playAuxSFXAtEntity(player, 1009, pos, 0);
            this.setBlockToAir(pos);
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * This string is 'All: (number of loaded entities)' Viewable by press ing F3
     */
    public String getDebugLoadedEntities()
    {
        return "All: " + this.loadedEntityList.size();
    }

    /**
     * Returns the name of the current chunk provider, by calling chunkprovider.makeString()
     */
    public String getProviderName()
    {
        return this.chunkProvider.makeString();
    }

    public TileEntity getTileEntity(BlockPos pos)
    {
        if (!this.isValid(pos))
        {
            return null;
        }
        else
        {
            TileEntity tileentity = null;

            if (this.processingLoadedTiles)
            {
                for (int i = 0; i < this.addedTileEntityList.size(); ++i)
                {
                    TileEntity tileentity1 = (TileEntity)this.addedTileEntityList.get(i);

                    if (!tileentity1.isInvalid() && tileentity1.getPos().equals(pos))
                    {
                        tileentity = tileentity1;
                        break;
                    }
                }
            }

            if (tileentity == null)
            {
                tileentity = this.getChunkFromBlockCoords(pos).getTileEntity(pos, Chunk.EnumCreateEntityType.IMMEDIATE);
            }

            if (tileentity == null)
            {
                for (int j = 0; j < this.addedTileEntityList.size(); ++j)
                {
                    TileEntity tileentity2 = (TileEntity)this.addedTileEntityList.get(j);

                    if (!tileentity2.isInvalid() && tileentity2.getPos().equals(pos))
                    {
                        tileentity = tileentity2;
                        break;
                    }
                }
            }

            return tileentity;
        }
    }

    public void setTileEntity(BlockPos pos, TileEntity tileEntityIn)
    {
        if (tileEntityIn != null && !tileEntityIn.isInvalid())
        {
            if (this.processingLoadedTiles)
            {
                tileEntityIn.setPos(pos);
                Iterator<TileEntity> iterator = this.addedTileEntityList.iterator();

                while (iterator.hasNext())
                {
                    TileEntity tileentity = (TileEntity)iterator.next();

                    if (tileentity.getPos().equals(pos))
                    {
                        tileentity.invalidate();
                        iterator.remove();
                    }
                }

                this.addedTileEntityList.add(tileEntityIn);
            }
            else
            {
                this.addTileEntity(tileEntityIn);
                this.getChunkFromBlockCoords(pos).addTileEntity(pos, tileEntityIn);
            }
        }
    }

    public void removeTileEntity(BlockPos pos)
    {
        TileEntity tileentity = this.getTileEntity(pos);

        if (tileentity != null && this.processingLoadedTiles)
        {
            tileentity.invalidate();
            this.addedTileEntityList.remove(tileentity);
        }
        else
        {
            if (tileentity != null)
            {
                this.addedTileEntityList.remove(tileentity);
                this.loadedTileEntityList.remove(tileentity);
                this.tickableTileEntities.remove(tileentity);
            }

            this.getChunkFromBlockCoords(pos).removeTileEntity(pos);
        }
    }

    /**
     * Adds the specified TileEntity to the pending removal list.
     */
    public void markTileEntityForRemoval(TileEntity tileEntityIn)
    {
        this.tileEntitiesToBeRemoved.add(tileEntityIn);
    }

    public boolean isBlockFullCube(BlockPos pos)
    {
        AxisAlignedBB axisalignedbb = this.getBlockState(pos).getSelectedBoundingBox(this, pos);
        return axisalignedbb != Block.NULL_AABB && axisalignedbb.getAverageEdgeLength() >= 1.0D;
    }

    /**
     * Checks if a block's material is opaque, and that it takes up a full cube
     */
    public boolean isBlockNormalCube(BlockPos pos, boolean _default)
    {
        if (!this.isValid(pos))
        {
            return _default;
        }
        else
        {
            Chunk chunk = this.chunkProvider.getLoadedChunk(pos.getX() >> 4, pos.getZ() >> 4);

            if (chunk != null && !chunk.isEmpty())
            {
                IBlockState iblockstate = this.getBlockState(pos);
                return iblockstate.getMaterial().isOpaque() && iblockstate.isFullCube();
            }
            else
            {
                return _default;
            }
        }
    }

    /**
     * Called on construction of the World class to setup the initial skylight values
     */
    public void calculateInitialSkylight()
    {
        int i = this.calculateSkylightSubtracted(1.0F);

        if (i != this.skylightSubtracted)
        {
            this.skylightSubtracted = i;
        }
    }

    /**
     * first boolean for hostile mobs and second for peaceful mobs
     */
    public void setAllowedSpawnTypes(boolean hostile, boolean peaceful)
    {
        this.spawnHostileMobs = hostile;
        this.spawnPeacefulMobs = peaceful;
    }

    /**
     * Runs a single tick for the world
     */
    public void tick()
    {
        this.updateWeather();
    }

    /**
     * Called from World constructor to set rainingStrength and thunderingStrength
     */
    protected void calculateInitialWeather()
    {
        if (this.worldInfo.isRaining())
        {
            this.rainingStrength = 1.0F;

            if (this.worldInfo.isThundering())
            {
                this.thunderingStrength = 1.0F;
            }
        }
    }

    /**
     * Updates all weather states.
     */
    protected void updateWeather()
    {
        if (!this.provider.getHasNoSky())
        {
            if (!this.isRemote)
            {
                int i = this.worldInfo.getCleanWeatherTime();

                if (i > 0)
                {
                    --i;
                    this.worldInfo.setCleanWeatherTime(i);
                    this.worldInfo.setThunderTime(this.worldInfo.isThundering() ? 1 : 2);
                    this.worldInfo.setRainTime(this.worldInfo.isRaining() ? 1 : 2);
                }

                int j = this.worldInfo.getThunderTime();

                if (j <= 0)
                {
                    if (this.worldInfo.isThundering())
                    {
                        this.worldInfo.setThunderTime(this.rand.nextInt(12000) + 3600);
                    }
                    else
                    {
                        this.worldInfo.setThunderTime(this.rand.nextInt(168000) + 12000);
                    }
                }
                else
                {
                    --j;
                    this.worldInfo.setThunderTime(j);

                    if (j <= 0)
                    {
                        this.worldInfo.setThundering(!this.worldInfo.isThundering());
                    }
                }

                this.prevThunderingStrength = this.thunderingStrength;

                if (this.worldInfo.isThundering())
                {
                    this.thunderingStrength = (float)((double)this.thunderingStrength + 0.01D);
                }
                else
                {
                    this.thunderingStrength = (float)((double)this.thunderingStrength - 0.01D);
                }

                this.thunderingStrength = MathHelper.clamp_float(this.thunderingStrength, 0.0F, 1.0F);
                int k = this.worldInfo.getRainTime();

                if (k <= 0)
                {
                    if (this.worldInfo.isRaining())
                    {
                        this.worldInfo.setRainTime(this.rand.nextInt(12000) + 12000);
                    }
                    else
                    {
                        this.worldInfo.setRainTime(this.rand.nextInt(168000) + 12000);
                    }
                }
                else
                {
                    --k;
                    this.worldInfo.setRainTime(k);

                    if (k <= 0)
                    {
                        this.worldInfo.setRaining(!this.worldInfo.isRaining());
                    }
                }

                this.prevRainingStrength = this.rainingStrength;

                if (this.worldInfo.isRaining())
                {
                    this.rainingStrength = (float)((double)this.rainingStrength + 0.01D);
                }
                else
                {
                    this.rainingStrength = (float)((double)this.rainingStrength - 0.01D);
                }

                this.rainingStrength = MathHelper.clamp_float(this.rainingStrength, 0.0F, 1.0F);
            }
        }
    }

    protected void playMoodSoundAndCheckLight(int p_147467_1_, int p_147467_2_, Chunk chunkIn)
    {
        chunkIn.enqueueRelightChecks();
    }

    protected void updateBlocks()
    {
    }

    public void forceBlockUpdateTick(Block blockType, BlockPos pos, Random random)
    {
        this.scheduledUpdatesAreImmediate = true;
        blockType.updateTick(this, pos, this.getBlockState(pos), random);
        this.scheduledUpdatesAreImmediate = false;
    }

    public boolean canBlockFreezeWater(BlockPos pos)
    {
        return this.canBlockFreeze(pos, false);
    }

    public boolean canBlockFreezeNoWater(BlockPos pos)
    {
        return this.canBlockFreeze(pos, true);
    }

    /**
     * Checks to see if a given block is both water and cold enough to freeze.
     */
    public boolean canBlockFreeze(BlockPos pos, boolean noWaterAdj)
    {
        BiomeGenBase biomegenbase = this.getBiomeGenForCoords(pos);
        float f = biomegenbase.getFloatTemperature(pos);

        if (f > 0.15F)
        {
            return false;
        }
        else
        {
            if (pos.getY() >= 0 && pos.getY() < 256 && this.getLightFor(EnumSkyBlock.BLOCK, pos) < 10)
            {
                IBlockState iblockstate = this.getBlockState(pos);
                Block block = iblockstate.getBlock();

                if ((block == Blocks.water || block == Blocks.flowing_water) && ((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue() == 0)
                {
                    if (!noWaterAdj)
                    {
                        return true;
                    }

                    boolean flag = this.isWater(pos.west()) && this.isWater(pos.east()) && this.isWater(pos.north()) && this.isWater(pos.south());

                    if (!flag)
                    {
                        return true;
                    }
                }
            }

            return false;
        }
    }

    private boolean isWater(BlockPos pos)
    {
        return this.getBlockState(pos).getMaterial() == Material.water;
    }

    /**
     * Checks to see if a given block can accumulate snow from it snowing
     */
    public boolean canSnowAt(BlockPos pos, boolean checkLight)
    {
        BiomeGenBase biomegenbase = this.getBiomeGenForCoords(pos);
        float f = biomegenbase.getFloatTemperature(pos);

        if (f > 0.15F)
        {
            return false;
        }
        else if (!checkLight)
        {
            return true;
        }
        else
        {
            if (pos.getY() >= 0 && pos.getY() < 256 && this.getLightFor(EnumSkyBlock.BLOCK, pos) < 10)
            {
                IBlockState iblockstate = this.getBlockState(pos);

                if (iblockstate.getMaterial() == Material.air && Blocks.snow_layer.canPlaceBlockAt(this, pos))
                {
                    return true;
                }
            }

            return false;
        }
    }

    public boolean checkLight(BlockPos pos)
    {
        boolean flag = false;

        if (!this.provider.getHasNoSky())
        {
            flag |= this.checkLightFor(EnumSkyBlock.SKY, pos);
        }

        flag = flag | this.checkLightFor(EnumSkyBlock.BLOCK, pos);
        return flag;
    }

    /**
     * gets the light level at the supplied position
     */
    private int getRawLight(BlockPos pos, EnumSkyBlock lightType)
    {
        if (lightType == EnumSkyBlock.SKY && this.canSeeSky(pos))
        {
            return 15;
        }
        else
        {
            IBlockState iblockstate = this.getBlockState(pos);
            int i = lightType == EnumSkyBlock.SKY ? 0 : iblockstate.getlightValue();
            int j = iblockstate.getLightOpacity();

            if (j >= 15 && iblockstate.getlightValue() > 0)
            {
                j = 1;
            }

            if (j < 1)
            {
                j = 1;
            }

            if (j >= 15)
            {
                return 0;
            }
            else if (i >= 14)
            {
                return i;
            }
            else
            {
                BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

                for (EnumFacing enumfacing : EnumFacing.values())
                {
                    blockpos$pooledmutableblockpos.set(pos).func_185341_c(enumfacing);
                    int k = this.getLightFor(lightType, blockpos$pooledmutableblockpos) - j;

                    if (k > i)
                    {
                        i = k;
                    }

                    if (i >= 14)
                    {
                        return i;
                    }
                }

                blockpos$pooledmutableblockpos.release();
                return i;
            }
        }
    }

    public boolean checkLightFor(EnumSkyBlock lightType, BlockPos pos)
    {
        if (!this.isAreaLoaded(pos, 17, false))
        {
            return false;
        }
        else
        {
            int i = 0;
            int j = 0;
            this.theProfiler.startSection("getBrightness");
            int k = this.getLightFor(lightType, pos);
            int l = this.getRawLight(pos, lightType);
            int i1 = pos.getX();
            int j1 = pos.getY();
            int k1 = pos.getZ();

            if (l > k)
            {
                this.lightUpdateBlockList[j++] = 133152;
            }
            else if (l < k)
            {
                this.lightUpdateBlockList[j++] = 133152 | k << 18;

                while (i < j)
                {
                    int l1 = this.lightUpdateBlockList[i++];
                    int i2 = (l1 & 63) - 32 + i1;
                    int j2 = (l1 >> 6 & 63) - 32 + j1;
                    int k2 = (l1 >> 12 & 63) - 32 + k1;
                    int l2 = l1 >> 18 & 15;
                    BlockPos blockpos = new BlockPos(i2, j2, k2);
                    int i3 = this.getLightFor(lightType, blockpos);

                    if (i3 == l2)
                    {
                        this.setLightFor(lightType, blockpos, 0);

                        if (l2 > 0)
                        {
                            int j3 = MathHelper.abs_int(i2 - i1);
                            int k3 = MathHelper.abs_int(j2 - j1);
                            int l3 = MathHelper.abs_int(k2 - k1);

                            if (j3 + k3 + l3 < 17)
                            {
                                BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

                                for (EnumFacing enumfacing : EnumFacing.values())
                                {
                                    int i4 = i2 + enumfacing.getFrontOffsetX();
                                    int j4 = j2 + enumfacing.getFrontOffsetY();
                                    int k4 = k2 + enumfacing.getFrontOffsetZ();
                                    blockpos$pooledmutableblockpos.set(i4, j4, k4);
                                    int l4 = Math.max(1, this.getBlockState(blockpos$pooledmutableblockpos).getLightOpacity());
                                    i3 = this.getLightFor(lightType, blockpos$pooledmutableblockpos);

                                    if (i3 == l2 - l4 && j < this.lightUpdateBlockList.length)
                                    {
                                        this.lightUpdateBlockList[j++] = i4 - i1 + 32 | j4 - j1 + 32 << 6 | k4 - k1 + 32 << 12 | l2 - l4 << 18;
                                    }
                                }

                                blockpos$pooledmutableblockpos.release();
                            }
                        }
                    }
                }

                i = 0;
            }

            this.theProfiler.endSection();
            this.theProfiler.startSection("checkedPosition < toCheckCount");

            while (i < j)
            {
                int i5 = this.lightUpdateBlockList[i++];
                int j5 = (i5 & 63) - 32 + i1;
                int k5 = (i5 >> 6 & 63) - 32 + j1;
                int l5 = (i5 >> 12 & 63) - 32 + k1;
                BlockPos blockpos1 = new BlockPos(j5, k5, l5);
                int i6 = this.getLightFor(lightType, blockpos1);
                int j6 = this.getRawLight(blockpos1, lightType);

                if (j6 != i6)
                {
                    this.setLightFor(lightType, blockpos1, j6);

                    if (j6 > i6)
                    {
                        int k6 = Math.abs(j5 - i1);
                        int l6 = Math.abs(k5 - j1);
                        int i7 = Math.abs(l5 - k1);
                        boolean flag = j < this.lightUpdateBlockList.length - 6;

                        if (k6 + l6 + i7 < 17 && flag)
                        {
                            if (this.getLightFor(lightType, blockpos1.west()) < j6)
                            {
                                this.lightUpdateBlockList[j++] = j5 - 1 - i1 + 32 + (k5 - j1 + 32 << 6) + (l5 - k1 + 32 << 12);
                            }

                            if (this.getLightFor(lightType, blockpos1.east()) < j6)
                            {
                                this.lightUpdateBlockList[j++] = j5 + 1 - i1 + 32 + (k5 - j1 + 32 << 6) + (l5 - k1 + 32 << 12);
                            }

                            if (this.getLightFor(lightType, blockpos1.down()) < j6)
                            {
                                this.lightUpdateBlockList[j++] = j5 - i1 + 32 + (k5 - 1 - j1 + 32 << 6) + (l5 - k1 + 32 << 12);
                            }

                            if (this.getLightFor(lightType, blockpos1.up()) < j6)
                            {
                                this.lightUpdateBlockList[j++] = j5 - i1 + 32 + (k5 + 1 - j1 + 32 << 6) + (l5 - k1 + 32 << 12);
                            }

                            if (this.getLightFor(lightType, blockpos1.north()) < j6)
                            {
                                this.lightUpdateBlockList[j++] = j5 - i1 + 32 + (k5 - j1 + 32 << 6) + (l5 - 1 - k1 + 32 << 12);
                            }

                            if (this.getLightFor(lightType, blockpos1.south()) < j6)
                            {
                                this.lightUpdateBlockList[j++] = j5 - i1 + 32 + (k5 - j1 + 32 << 6) + (l5 + 1 - k1 + 32 << 12);
                            }
                        }
                    }
                }
            }

            this.theProfiler.endSection();
            return true;
        }
    }

    /**
     * Runs through the list of updates to run and ticks them
     */
    public boolean tickUpdates(boolean p_72955_1_)
    {
        return false;
    }

    public List<NextTickListEntry> getPendingBlockUpdates(Chunk chunkIn, boolean p_72920_2_)
    {
        return null;
    }

    public List<NextTickListEntry> func_175712_a(StructureBoundingBox structureBB, boolean p_175712_2_)
    {
        return null;
    }

    public List<Entity> getEntitiesWithinAABBExcludingEntity(Entity entityIn, AxisAlignedBB bb)
    {
        return this.getEntitiesInAABBexcluding(entityIn, bb, EntitySelectors.NOT_SPECTATING);
    }

    public List<Entity> getEntitiesInAABBexcluding(Entity entityIn, AxisAlignedBB boundingBox, Predicate <? super Entity > predicate)
    {
        List<Entity> list = Lists.<Entity>newArrayList();
        int i = MathHelper.floor_double((boundingBox.minX - 2.0D) / 16.0D);
        int j = MathHelper.floor_double((boundingBox.maxX + 2.0D) / 16.0D);
        int k = MathHelper.floor_double((boundingBox.minZ - 2.0D) / 16.0D);
        int l = MathHelper.floor_double((boundingBox.maxZ + 2.0D) / 16.0D);

        for (int i1 = i; i1 <= j; ++i1)
        {
            for (int j1 = k; j1 <= l; ++j1)
            {
                if (this.isChunkLoaded(i1, j1, true))
                {
                    this.getChunkFromChunkCoords(i1, j1).getEntitiesWithinAABBForEntity(entityIn, boundingBox, list, predicate);
                }
            }
        }

        return list;
    }

    public <T extends Entity> List<T> getEntities(Class <? extends T > entityType, Predicate <? super T > filter)
    {
        List<T> list = Lists.<T>newArrayList();

        for (Entity entity : this.loadedEntityList)
        {
            if (entityType.isAssignableFrom(entity.getClass()) && filter.apply((T)entity))
            {
                list.add((T)entity);
            }
        }

        return list;
    }

    public <T extends Entity> List<T> getPlayers(Class <? extends T > playerType, Predicate <? super T > filter)
    {
        List<T> list = Lists.<T>newArrayList();

        for (Entity entity : this.playerEntities)
        {
            if (playerType.isAssignableFrom(entity.getClass()) && filter.apply((T)entity))
            {
                list.add((T)entity);
            }
        }

        return list;
    }

    public <T extends Entity> List<T> getEntitiesWithinAABB(Class <? extends T > classEntity, AxisAlignedBB bb)
    {
        return this.<T>getEntitiesWithinAABB(classEntity, bb, EntitySelectors.NOT_SPECTATING);
    }

    public <T extends Entity> List<T> getEntitiesWithinAABB(Class <? extends T > clazz, AxisAlignedBB aabb, Predicate <? super T > filter)
    {
        int i = MathHelper.floor_double((aabb.minX - 2.0D) / 16.0D);
        int j = MathHelper.ceiling_double_int((aabb.maxX + 2.0D) / 16.0D);
        int k = MathHelper.floor_double((aabb.minZ - 2.0D) / 16.0D);
        int l = MathHelper.ceiling_double_int((aabb.maxZ + 2.0D) / 16.0D);
        List<T> list = Lists.<T>newArrayList();

        for (int i1 = i; i1 < j; ++i1)
        {
            for (int j1 = k; j1 < l; ++j1)
            {
                if (this.isChunkLoaded(i1, j1, true))
                {
                    this.getChunkFromChunkCoords(i1, j1).getEntitiesOfTypeWithinAAAB(clazz, aabb, list, filter);
                }
            }
        }

        return list;
    }

    public <T extends Entity> T findNearestEntityWithinAABB(Class <? extends T > entityType, AxisAlignedBB aabb, T closestTo)
    {
        List<T> list = this.<T>getEntitiesWithinAABB(entityType, aabb);
        T t = null;
        double d0 = Double.MAX_VALUE;

        for (int i = 0; i < list.size(); ++i)
        {
            T t1 = list.get(i);

            if (t1 != closestTo && EntitySelectors.NOT_SPECTATING.apply(t1))
            {
                double d1 = closestTo.getDistanceSqToEntity(t1);

                if (d1 <= d0)
                {
                    t = t1;
                    d0 = d1;
                }
            }
        }

        return t;
    }

    /**
     * Returns the Entity with the given ID, or null if it doesn't exist in this World.
     */
    public Entity getEntityByID(int id)
    {
        return (Entity)this.entitiesById.lookup(id);
    }

    public List<Entity> getLoadedEntityList()
    {
        return this.loadedEntityList;
    }

    public void markChunkDirty(BlockPos pos, TileEntity unusedTileEntity)
    {
        if (this.isBlockLoaded(pos))
        {
            this.getChunkFromBlockCoords(pos).setChunkModified();
        }
    }

    /**
     * Counts how many entities of an entity class exist in the world. Args: entityClass
     */
    public int countEntities(Class<?> entityType)
    {
        int i = 0;

        for (Entity entity : this.loadedEntityList)
        {
            if ((!(entity instanceof EntityLiving) || !((EntityLiving)entity).isNoDespawnRequired()) && entityType.isAssignableFrom(entity.getClass()))
            {
                ++i;
            }
        }

        return i;
    }

    public void loadEntities(Collection<Entity> entityCollection)
    {
        this.loadedEntityList.addAll(entityCollection);

        for (Entity entity : entityCollection)
        {
            this.onEntityAdded(entity);
        }
    }

    public void unloadEntities(Collection<Entity> entityCollection)
    {
        this.unloadedEntityList.addAll(entityCollection);
    }

    public boolean canBlockBePlaced(Block blockIn, BlockPos pos, boolean p_175716_3_, EnumFacing side, Entity entityIn, ItemStack itemStackIn)
    {
        IBlockState iblockstate = this.getBlockState(pos);
        AxisAlignedBB axisalignedbb = p_175716_3_ ? null : blockIn.getDefaultState().getSelectedBoundingBox(this, pos);
        return axisalignedbb != Block.NULL_AABB && !this.checkNoEntityCollision(axisalignedbb.offset(pos), entityIn) ? false : (iblockstate.getMaterial() == Material.circuits && blockIn == Blocks.anvil ? true : iblockstate.getMaterial().isReplaceable() && blockIn.canReplace(this, pos, side, itemStackIn));
    }

    public int getSeaLevel()
    {
        return this.seaLevel;
    }

    /**
     * Warning this value may not be respected in all cases as it is still hardcoded in many places.
     */
    public void setSeaLevel(int seaLevelIn)
    {
        this.seaLevel = seaLevelIn;
    }

    public int getStrongPower(BlockPos pos, EnumFacing direction)
    {
        return this.getBlockState(pos).getStrongPower(this, pos, direction);
    }

    public WorldType getWorldType()
    {
        return this.worldInfo.getTerrainType();
    }

    /**
     * Returns the single highest strong power out of all directions using getStrongPower(BlockPos, EnumFacing)
     */
    public int getStrongPower(BlockPos pos)
    {
        int i = 0;
        i = Math.max(i, this.getStrongPower(pos.down(), EnumFacing.DOWN));

        if (i >= 15)
        {
            return i;
        }
        else
        {
            i = Math.max(i, this.getStrongPower(pos.up(), EnumFacing.UP));

            if (i >= 15)
            {
                return i;
            }
            else
            {
                i = Math.max(i, this.getStrongPower(pos.north(), EnumFacing.NORTH));

                if (i >= 15)
                {
                    return i;
                }
                else
                {
                    i = Math.max(i, this.getStrongPower(pos.south(), EnumFacing.SOUTH));

                    if (i >= 15)
                    {
                        return i;
                    }
                    else
                    {
                        i = Math.max(i, this.getStrongPower(pos.west(), EnumFacing.WEST));

                        if (i >= 15)
                        {
                            return i;
                        }
                        else
                        {
                            i = Math.max(i, this.getStrongPower(pos.east(), EnumFacing.EAST));
                            return i >= 15 ? i : i;
                        }
                    }
                }
            }
        }
    }

    public boolean isSidePowered(BlockPos pos, EnumFacing side)
    {
        return this.getRedstonePower(pos, side) > 0;
    }

    public int getRedstonePower(BlockPos pos, EnumFacing facing)
    {
        IBlockState iblockstate = this.getBlockState(pos);
        return iblockstate.isNormalCube() ? this.getStrongPower(pos) : iblockstate.getWeakPower(this, pos, facing);
    }

    public boolean isBlockPowered(BlockPos pos)
    {
        return this.getRedstonePower(pos.down(), EnumFacing.DOWN) > 0 ? true : (this.getRedstonePower(pos.up(), EnumFacing.UP) > 0 ? true : (this.getRedstonePower(pos.north(), EnumFacing.NORTH) > 0 ? true : (this.getRedstonePower(pos.south(), EnumFacing.SOUTH) > 0 ? true : (this.getRedstonePower(pos.west(), EnumFacing.WEST) > 0 ? true : this.getRedstonePower(pos.east(), EnumFacing.EAST) > 0))));
    }

    /**
     * Checks if the specified block or its neighbors are powered by a neighboring block. Used by blocks like TNT and
     * Doors.
     */
    public int isBlockIndirectlyGettingPowered(BlockPos pos)
    {
        int i = 0;

        for (EnumFacing enumfacing : EnumFacing.values())
        {
            int j = this.getRedstonePower(pos.offset(enumfacing), enumfacing);

            if (j >= 15)
            {
                return 15;
            }

            if (j > i)
            {
                i = j;
            }
        }

        return i;
    }

    /**
     * Gets the closest player to the entity within the specified distance (if distance is less than 0 then ignored).
     * Args: entity, dist
     */
    public EntityPlayer getClosestPlayerToEntity(Entity entityIn, double distance)
    {
        return this.func_184137_a(entityIn.posX, entityIn.posY, entityIn.posZ, distance, false);
    }

    public EntityPlayer func_184136_b(Entity p_184136_1_, double p_184136_2_)
    {
        return this.func_184137_a(p_184136_1_.posX, p_184136_1_.posY, p_184136_1_.posZ, p_184136_2_, true);
    }

    public EntityPlayer func_184137_a(double p_184137_1_, double p_184137_3_, double p_184137_5_, double p_184137_7_, boolean p_184137_9_)
    {
        double d0 = -1.0D;
        EntityPlayer entityplayer = null;

        for (int i = 0; i < this.playerEntities.size(); ++i)
        {
            EntityPlayer entityplayer1 = (EntityPlayer)this.playerEntities.get(i);

            if ((EntitySelectors.field_188444_d.apply(entityplayer1) || !p_184137_9_) && (EntitySelectors.NOT_SPECTATING.apply(entityplayer1) || p_184137_9_))
            {
                double d1 = entityplayer1.getDistanceSq(p_184137_1_, p_184137_3_, p_184137_5_);

                if ((p_184137_7_ < 0.0D || d1 < p_184137_7_ * p_184137_7_) && (d0 == -1.0D || d1 < d0))
                {
                    d0 = d1;
                    entityplayer = entityplayer1;
                }
            }
        }

        return entityplayer;
    }

    public boolean isAnyPlayerWithinRangeAt(double x, double y, double z, double range)
    {
        for (int i = 0; i < this.playerEntities.size(); ++i)
        {
            EntityPlayer entityplayer = (EntityPlayer)this.playerEntities.get(i);

            if (EntitySelectors.NOT_SPECTATING.apply(entityplayer))
            {
                double d0 = entityplayer.getDistanceSq(x, y, z);

                if (range < 0.0D || d0 < range * range)
                {
                    return true;
                }
            }
        }

        return false;
    }

    public EntityPlayer func_184142_a(Entity p_184142_1_, double p_184142_2_, double p_184142_4_)
    {
        return this.func_184150_a(p_184142_1_.posX, p_184142_1_.posY, p_184142_1_.posZ, p_184142_2_, p_184142_4_, (Function<EntityPlayer, Double>)null, (Predicate<EntityPlayer>)null);
    }

    public EntityPlayer func_184139_a(BlockPos p_184139_1_, double p_184139_2_, double p_184139_4_)
    {
        return this.func_184150_a((double)((float)p_184139_1_.getX() + 0.5F), (double)((float)p_184139_1_.getY() + 0.5F), (double)((float)p_184139_1_.getZ() + 0.5F), p_184139_2_, p_184139_4_, (Function<EntityPlayer, Double>)null, (Predicate<EntityPlayer>)null);
    }

    public EntityPlayer func_184150_a(double p_184150_1_, double p_184150_3_, double p_184150_5_, double p_184150_7_, double p_184150_9_, Function<EntityPlayer, Double> p_184150_11_, Predicate<EntityPlayer> p_184150_12_)
    {
        double d0 = -1.0D;
        EntityPlayer entityplayer = null;

        for (int i = 0; i < this.playerEntities.size(); ++i)
        {
            EntityPlayer entityplayer1 = (EntityPlayer)this.playerEntities.get(i);

            if (!entityplayer1.capabilities.disableDamage && entityplayer1.isEntityAlive() && !entityplayer1.isSpectator() && (p_184150_12_ == null || p_184150_12_.apply(entityplayer1)))
            {
                double d1 = entityplayer1.getDistanceSq(p_184150_1_, entityplayer1.posY, p_184150_5_);
                double d2 = p_184150_7_;

                if (entityplayer1.isSneaking())
                {
                    d2 = p_184150_7_ * 0.800000011920929D;
                }

                if (entityplayer1.isInvisible())
                {
                    float f = entityplayer1.getArmorVisibility();

                    if (f < 0.1F)
                    {
                        f = 0.1F;
                    }

                    d2 *= (double)(0.7F * f);
                }

                if (p_184150_11_ != null)
                {
                    d2 *= ((Double)Objects.firstNonNull(p_184150_11_.apply(entityplayer1), Double.valueOf(1.0D))).doubleValue();
                }

                if ((p_184150_9_ < 0.0D || Math.abs(entityplayer1.posY - p_184150_3_) < p_184150_9_ * p_184150_9_) && (p_184150_7_ < 0.0D || d1 < d2 * d2) && (d0 == -1.0D || d1 < d0))
                {
                    d0 = d1;
                    entityplayer = entityplayer1;
                }
            }
        }

        return entityplayer;
    }

    /**
     * Find a player by name in this world.
     */
    public EntityPlayer getPlayerEntityByName(String name)
    {
        for (int i = 0; i < this.playerEntities.size(); ++i)
        {
            EntityPlayer entityplayer = (EntityPlayer)this.playerEntities.get(i);

            if (name.equals(entityplayer.getName()))
            {
                return entityplayer;
            }
        }

        return null;
    }

    public EntityPlayer getPlayerEntityByUUID(UUID uuid)
    {
        for (int i = 0; i < this.playerEntities.size(); ++i)
        {
            EntityPlayer entityplayer = (EntityPlayer)this.playerEntities.get(i);

            if (uuid.equals(entityplayer.getUniqueID()))
            {
                return entityplayer;
            }
        }

        return null;
    }

    /**
     * If on MP, sends a quitting packet.
     */
    public void sendQuittingDisconnectingPacket()
    {
    }

    /**
     * Checks whether the session lock file was modified by another process
     */
    public void checkSessionLock() throws MinecraftException
    {
        this.saveHandler.checkSessionLock();
    }

    public void setTotalWorldTime(long worldTime)
    {
        this.worldInfo.setWorldTotalTime(worldTime);
    }

    /**
     * gets the random world seed
     */
    public long getSeed()
    {
        return this.worldInfo.getSeed();
    }

    public long getTotalWorldTime()
    {
        return this.worldInfo.getWorldTotalTime();
    }

    public long getWorldTime()
    {
        return this.worldInfo.getWorldTime();
    }

    /**
     * Sets the world time.
     */
    public void setWorldTime(long time)
    {
        this.worldInfo.setWorldTime(time);
    }

    /**
     * Gets the spawn point in the world
     */
    public BlockPos getSpawnPoint()
    {
        BlockPos blockpos = new BlockPos(this.worldInfo.getSpawnX(), this.worldInfo.getSpawnY(), this.worldInfo.getSpawnZ());

        if (!this.getWorldBorder().contains(blockpos))
        {
            blockpos = this.getHeight(new BlockPos(this.getWorldBorder().getCenterX(), 0.0D, this.getWorldBorder().getCenterZ()));
        }

        return blockpos;
    }

    public void setSpawnPoint(BlockPos pos)
    {
        this.worldInfo.setSpawn(pos);
    }

    /**
     * spwans an entity and loads surrounding chunks
     */
    public void joinEntityInSurroundings(Entity entityIn)
    {
        int i = MathHelper.floor_double(entityIn.posX / 16.0D);
        int j = MathHelper.floor_double(entityIn.posZ / 16.0D);
        int k = 2;

        for (int l = -k; l <= k; ++l)
        {
            for (int i1 = -k; i1 <= k; ++i1)
            {
                this.getChunkFromChunkCoords(i + l, j + i1);
            }
        }

        if (!this.loadedEntityList.contains(entityIn))
        {
            this.loadedEntityList.add(entityIn);
        }
    }

    public boolean isBlockModifiable(EntityPlayer player, BlockPos pos)
    {
        return true;
    }

    /**
     * sends a Packet 38 (Entity Status) to all tracked players of that entity
     */
    public void setEntityState(Entity entityIn, byte state)
    {
    }

    /**
     * gets the world's chunk provider
     */
    public IChunkProvider getChunkProvider()
    {
        return this.chunkProvider;
    }

    public void addBlockEvent(BlockPos pos, Block blockIn, int eventID, int eventParam)
    {
        blockIn.onBlockEventReceived(this, pos, this.getBlockState(pos), eventID, eventParam);
    }

    /**
     * Returns this world's current save handler
     */
    public ISaveHandler getSaveHandler()
    {
        return this.saveHandler;
    }

    /**
     * Returns the world's WorldInfo object
     */
    public WorldInfo getWorldInfo()
    {
        return this.worldInfo;
    }

    /**
     * Gets the GameRules instance.
     */
    public GameRules getGameRules()
    {
        return this.worldInfo.getGameRulesInstance();
    }

    /**
     * Updates the flag that indicates whether or not all players in the world are sleeping.
     */
    public void updateAllPlayersSleepingFlag()
    {
    }

    public float getThunderStrength(float delta)
    {
        return (this.prevThunderingStrength + (this.thunderingStrength - this.prevThunderingStrength) * delta) * this.getRainStrength(delta);
    }

    /**
     * Sets the strength of the thunder.
     */
    public void setThunderStrength(float strength)
    {
        this.prevThunderingStrength = strength;
        this.thunderingStrength = strength;
    }

    /**
     * Returns rain strength.
     */
    public float getRainStrength(float delta)
    {
        return this.prevRainingStrength + (this.rainingStrength - this.prevRainingStrength) * delta;
    }

    /**
     * Sets the strength of the rain.
     */
    public void setRainStrength(float strength)
    {
        this.prevRainingStrength = strength;
        this.rainingStrength = strength;
    }

    /**
     * Returns true if the current thunder strength (weighted with the rain strength) is greater than 0.9
     */
    public boolean isThundering()
    {
        return (double)this.getThunderStrength(1.0F) > 0.9D;
    }

    /**
     * Returns true if the current rain strength is greater than 0.2
     */
    public boolean isRaining()
    {
        return (double)this.getRainStrength(1.0F) > 0.2D;
    }

    /**
     * Check if precipitation is currently happening at a position
     */
    public boolean isRainingAt(BlockPos strikePosition)
    {
        if (!this.isRaining())
        {
            return false;
        }
        else if (!this.canSeeSky(strikePosition))
        {
            return false;
        }
        else if (this.getPrecipitationHeight(strikePosition).getY() > strikePosition.getY())
        {
            return false;
        }
        else
        {
            BiomeGenBase biomegenbase = this.getBiomeGenForCoords(strikePosition);
            return biomegenbase.getEnableSnow() ? false : (this.canSnowAt(strikePosition, false) ? false : biomegenbase.canRain());
        }
    }

    public boolean isBlockinHighHumidity(BlockPos pos)
    {
        BiomeGenBase biomegenbase = this.getBiomeGenForCoords(pos);
        return biomegenbase.isHighHumidity();
    }

    public MapStorage getMapStorage()
    {
        return this.mapStorage;
    }

    /**
     * Assigns the given String id to the given MapDataBase using the MapStorage, removing any existing ones of the same
     * id.
     */
    public void setItemData(String dataID, WorldSavedData worldSavedDataIn)
    {
        this.mapStorage.setData(dataID, worldSavedDataIn);
    }

    /**
     * Loads an existing MapDataBase corresponding to the given String id from disk using the MapStorage, instantiating
     * the given Class, or returns null if none such file exists. args: Class to instantiate, String dataid
     */
    public WorldSavedData loadItemData(Class <? extends WorldSavedData > clazz, String dataID)
    {
        return this.mapStorage.loadData(clazz, dataID);
    }

    /**
     * Returns an unique new data id from the MapStorage for the given prefix and saves the idCounts map to the
     * 'idcounts' file.
     */
    public int getUniqueDataId(String key)
    {
        return this.mapStorage.getUniqueDataId(key);
    }

    public void playBroadcastSound(int p_175669_1_, BlockPos pos, int p_175669_3_)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            ((IWorldEventListener)this.worldAccesses.get(i)).broadcastSound(p_175669_1_, pos, p_175669_3_);
        }
    }

    public void playAuxSFX(int p_175718_1_, BlockPos pos, int p_175718_3_)
    {
        this.playAuxSFXAtEntity((EntityPlayer)null, p_175718_1_, pos, p_175718_3_);
    }

    public void playAuxSFXAtEntity(EntityPlayer player, int sfxType, BlockPos pos, int p_180498_4_)
    {
        try
        {
            for (int i = 0; i < this.worldAccesses.size(); ++i)
            {
                ((IWorldEventListener)this.worldAccesses.get(i)).playAuxSFX(player, sfxType, pos, p_180498_4_);
            }
        }
        catch (Throwable throwable)
        {
            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Playing level event");
            CrashReportCategory crashreportcategory = crashreport.makeCategory("Level event being played");
            crashreportcategory.addCrashSection("Block coordinates", CrashReportCategory.getCoordinateInfo(pos));
            crashreportcategory.addCrashSection("Event source", player);
            crashreportcategory.addCrashSection("Event type", Integer.valueOf(sfxType));
            crashreportcategory.addCrashSection("Event data", Integer.valueOf(p_180498_4_));
            throw new ReportedException(crashreport);
        }
    }

    /**
     * Returns maximum world height.
     */
    public int getHeight()
    {
        return 256;
    }

    /**
     * Returns current world height.
     */
    public int getActualHeight()
    {
        return this.provider.getHasNoSky() ? 128 : 256;
    }

    /**
     * puts the World Random seed to a specific state dependant on the inputs
     */
    public Random setRandomSeed(int p_72843_1_, int p_72843_2_, int p_72843_3_)
    {
        long i = (long)p_72843_1_ * 341873128712L + (long)p_72843_2_ * 132897987541L + this.getWorldInfo().getSeed() + (long)p_72843_3_;
        this.rand.setSeed(i);
        return this.rand;
    }

    /**
     * set by !chunk.getAreLevelsEmpty
     */
    public boolean extendedLevelsInChunkCache()
    {
        return false;
    }

    /**
     * Returns horizon height for use in rendering the sky.
     */
    public double getHorizon()
    {
        return this.worldInfo.getTerrainType() == WorldType.FLAT ? 0.0D : 63.0D;
    }

    /**
     * Adds some basic stats of the world to the given crash report.
     */
    public CrashReportCategory addWorldInfoToCrashReport(CrashReport report)
    {
        CrashReportCategory crashreportcategory = report.makeCategoryDepth("Affected level", 1);
        crashreportcategory.addCrashSection("Level name", this.worldInfo == null ? "????" : this.worldInfo.getWorldName());
        crashreportcategory.addCrashSectionCallable("All players", new Callable<String>()
        {
            public String call()
            {
                return World.this.playerEntities.size() + " total; " + World.this.playerEntities.toString();
            }
        });
        crashreportcategory.addCrashSectionCallable("Chunk stats", new Callable<String>()
        {
            public String call()
            {
                return World.this.chunkProvider.makeString();
            }
        });

        try
        {
            this.worldInfo.addToCrashReport(crashreportcategory);
        }
        catch (Throwable throwable)
        {
            crashreportcategory.addCrashSectionThrowable("Level Data Unobtainable", throwable);
        }

        return crashreportcategory;
    }

    public void sendBlockBreakProgress(int breakerId, BlockPos pos, int progress)
    {
        for (int i = 0; i < this.worldAccesses.size(); ++i)
        {
            IWorldEventListener iworldeventlistener = (IWorldEventListener)this.worldAccesses.get(i);
            iworldeventlistener.sendBlockBreakProgress(breakerId, pos, progress);
        }
    }

    /**
     * returns a calendar object containing the current date
     */
    public Calendar getCurrentDate()
    {
        if (this.getTotalWorldTime() % 600L == 0L)
        {
            this.theCalendar.setTimeInMillis(MinecraftServer.getCurrentTimeMillis());
        }

        return this.theCalendar;
    }

    public void makeFireworks(double x, double y, double z, double motionX, double motionY, double motionZ, NBTTagCompound compund)
    {
    }

    public Scoreboard getScoreboard()
    {
        return this.worldScoreboard;
    }

    public void updateComparatorOutputLevel(BlockPos pos, Block blockIn)
    {
        for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
        {
            BlockPos blockpos = pos.offset(enumfacing);

            if (this.isBlockLoaded(blockpos))
            {
                IBlockState iblockstate = this.getBlockState(blockpos);

                if (Blocks.unpowered_comparator.func_185547_C(iblockstate))
                {
                    iblockstate.getBlock().onNeighborBlockChange(this, blockpos, iblockstate, blockIn);
                }
                else if (iblockstate.isNormalCube())
                {
                    blockpos = blockpos.offset(enumfacing);
                    iblockstate = this.getBlockState(blockpos);

                    if (Blocks.unpowered_comparator.func_185547_C(iblockstate))
                    {
                        iblockstate.getBlock().onNeighborBlockChange(this, blockpos, iblockstate, blockIn);
                    }
                }
            }
        }
    }

    public DifficultyInstance getDifficultyForLocation(BlockPos pos)
    {
        long i = 0L;
        float f = 0.0F;

        if (this.isBlockLoaded(pos))
        {
            f = this.getCurrentMoonPhaseFactor();
            i = this.getChunkFromBlockCoords(pos).getInhabitedTime();
        }

        return new DifficultyInstance(this.getDifficulty(), this.getWorldTime(), i, f);
    }

    public EnumDifficulty getDifficulty()
    {
        return this.getWorldInfo().getDifficulty();
    }

    public int getSkylightSubtracted()
    {
        return this.skylightSubtracted;
    }

    public void setSkylightSubtracted(int newSkylightSubtracted)
    {
        this.skylightSubtracted = newSkylightSubtracted;
    }

    public int getLastLightningBolt()
    {
        return this.lastLightningBolt;
    }

    public void setLastLightningBolt(int lastLightningBoltIn)
    {
        this.lastLightningBolt = lastLightningBoltIn;
    }

    public VillageCollection getVillageCollection()
    {
        return this.villageCollectionObj;
    }

    public WorldBorder getWorldBorder()
    {
        return this.worldBorder;
    }

    /**
     * Returns true if the chunk is located near the spawn point
     */
    public boolean isSpawnChunk(int x, int z)
    {
        BlockPos blockpos = this.getSpawnPoint();
        int i = x * 16 + 8 - blockpos.getX();
        int j = z * 16 + 8 - blockpos.getZ();
        int k = 128;
        return i >= -k && i <= k && j >= -k && j <= k;
    }

    public void sendPacketToServer(Packet<?> packetIn)
    {
        throw new UnsupportedOperationException("Can\'t send packets to server unless you\'re on the client.");
    }

    public LootTableManager getLootTableManager()
    {
        return this.lootTable;
    }
}
