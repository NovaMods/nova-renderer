package com.continuum.nova.chunks;

import com.continuum.nova.NovaNative;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.chunk.Chunk;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.Map;

import static com.continuum.nova.NovaConstants.CHUNK_HEIGHT;
import static com.continuum.nova.NovaConstants.CHUNK_WIDTH;

/**
 * Splits chunks up into meshes with one mesh for each shader
 *
 * @author ddubois
 * @since 27-Jul-17
 */
public class ChunkBuilder {
    private static final Logger LOG = LogManager.getLogger(ChunkBuilder.class);
    private final World world;

    private Map<String, IGeometryFilter> filters;

    public ChunkBuilder(Map<String, IGeometryFilter> filters, World world) {
        this.filters = filters;
        this.world = world;
    }

    public void createMeshesForChunk(ChunkUpdateListener.BlockUpdateRange range) {
        LOG.debug("Updating chunk {}", range);
        NovaNative.mc_chunk updateChunk = new NovaNative.mc_chunk();
        long startTime = System.currentTimeMillis();

        Chunk mcChunk = world.getChunkFromBlockCoords(new BlockPos(range.min.x, range.min.y, range.min.z));

        for(int x = range.min.x; x <= range.max.x; x++) {
            for(int y = range.min.y; y < range.max.y; y++) {
                for(int z = range.min.z; z <= range.max.z; z++) {
                    int chunkX = x - range.min.x;
                    int chunkY = y - range.min.y;
                    int chunkZ = z - range.min.z;
                    int idx = chunkX + chunkY * CHUNK_WIDTH + chunkZ * CHUNK_WIDTH * CHUNK_HEIGHT;

                    NovaNative.mc_block curBlock = updateChunk.blocks[idx];
                    copyBlockStateIntoMcBlock(mcChunk.getBlockState(x, y, z), curBlock);
                }
            }
        }

        updateChunk.x = range.min.x;
        updateChunk.z = range.min.z;

        int chunkHashCode = (int) updateChunk.x;
        chunkHashCode = (int) (31 * chunkHashCode + updateChunk.z);

        updateChunk.chunk_id = chunkHashCode;

        long timeAfterBuildingStruct = System.currentTimeMillis();
        // Using JNA: 550 ms / chunk
        NovaNative.INSTANCE.add_chunk(updateChunk);
        long timeAfterSendingToNative = System.currentTimeMillis();

        long deltaTime = System.currentTimeMillis() - startTime;
        timeSpentInBlockRenderUpdate.addAndGet(deltaTime);
        numChunksUpdated.incrementAndGet();

        if(numChunksUpdated.get() % 10 == 0) {
            LOG.debug("It's taken an average of {}ms to update {} chunks",
                    (float) timeSpentInBlockRenderUpdate.get() / numChunksUpdated.get(), numChunksUpdated);
            LOG.debug("Detailed stats:\nTime to build chunk: {}ms\nTime to process chunk in native code: {}ms",
                    timeAfterBuildingStruct - startTime, timeAfterSendingToNative - timeAfterBuildingStruct);
        }
    }

    public void setWorld(World world) {
        this.world = world;
    }
}
