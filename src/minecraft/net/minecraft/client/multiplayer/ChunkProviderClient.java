package net.minecraft.client.multiplayer;

import com.google.common.base.Objects;
import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.util.LongHashMap;
import net.minecraft.world.ChunkCoordIntPair;
import net.minecraft.world.World;
import net.minecraft.world.chunk.Chunk;
import net.minecraft.world.chunk.EmptyChunk;
import net.minecraft.world.chunk.IChunkProvider;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ChunkProviderClient implements IChunkProvider
{
    private static final Logger logger = LogManager.getLogger();

    /**
     * The completely empty chunk used by ChunkProviderClient when chunkMapping doesn't contain the requested
     * coordinates.
     */
    private final Chunk blankChunk;
    private final LongHashMap<Chunk> chunkMapping = new LongHashMap();
    private final List<Chunk> chunkListing = Lists.<Chunk>newArrayList();

    /** Reference to the World object. */
    private final World worldObj;

    public ChunkProviderClient(World worldIn)
    {
        this.blankChunk = new EmptyChunk(worldIn, 0, 0);
        this.worldObj = worldIn;
    }

    /**
     * Unload chunk from ChunkProviderClient's hashmap. Called in response to a Packet50PreChunk with its mode field set
     * to false
     */
    public void unloadChunk(int x, int z)
    {
        Chunk chunk = this.func_186025_d(x, z);

        if (!chunk.isEmpty())
        {
            chunk.onChunkUnload();
        }

        this.chunkMapping.remove(ChunkCoordIntPair.chunkXZ2Int(x, z));
        this.chunkListing.remove(chunk);
    }

    public Chunk getLoadedChunk(int x, int z)
    {
        return (Chunk)this.chunkMapping.getValueByKey(ChunkCoordIntPair.chunkXZ2Int(x, z));
    }

    /**
     * loads or generates the chunk at the chunk location specified
     */
    public Chunk loadChunk(int chunkX, int chunkZ)
    {
        Chunk chunk = new Chunk(this.worldObj, chunkX, chunkZ);
        this.chunkMapping.add(ChunkCoordIntPair.chunkXZ2Int(chunkX, chunkZ), chunk);
        this.chunkListing.add(chunk);
        chunk.setChunkLoaded(true);
        return chunk;
    }

    public Chunk func_186025_d(int x, int z)
    {
        return (Chunk)Objects.firstNonNull(this.getLoadedChunk(x, z), this.blankChunk);
    }

    /**
     * Unloads chunks that are marked to be unloaded. This is not guaranteed to unload every such chunk.
     */
    public boolean unloadQueuedChunks()
    {
        long i = System.currentTimeMillis();

        for (Chunk chunk : this.chunkListing)
        {
            chunk.func_150804_b(System.currentTimeMillis() - i > 5L);
        }

        if (System.currentTimeMillis() - i > 100L)
        {
            logger.info("Warning: Clientside chunk ticking took {} ms", new Object[] {Long.valueOf(System.currentTimeMillis() - i)});
        }

        return false;
    }

    /**
     * Converts the instance data to a readable string.
     */
    public String makeString()
    {
        return "MultiplayerChunkCache: " + this.chunkMapping.getNumHashElements() + ", " + this.chunkListing.size();
    }
}
