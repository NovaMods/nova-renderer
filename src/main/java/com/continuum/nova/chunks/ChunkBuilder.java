package com.continuum.nova.chunks;

import com.continuum.nova.NovaNative;
import com.continuum.nova.utils.DefaultHashMap;
import net.minecraft.block.Block;
import net.minecraft.block.BlockRailBase;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.BlockModelShapes;
import net.minecraft.client.renderer.block.model.BakedQuad;
import net.minecraft.client.renderer.block.model.IBakedModel;
import net.minecraft.client.renderer.block.model.ModelManager;
import net.minecraft.client.renderer.block.statemap.BlockStateMapper;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.Tuple;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.chunk.Chunk;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

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
    private final BlockModelShapes modelManager;
    private World world;

    private final Map<String, IGeometryFilter> filters;

    private final AtomicLong timeSpentInBlockRenderUpdate = new AtomicLong(0);
    private final AtomicInteger numChunksUpdated = new AtomicInteger(0);

    public ChunkBuilder(Map<String, IGeometryFilter> filters, World world) {
        this.filters = filters;
        this.world = world;
    }

    public void createMeshesForChunk(ChunkUpdateListener.BlockUpdateRange range) {
        LOG.debug("Updating chunk {}", range);
        Map<String, List<Tuple<BlockPos, IBlockState>>> blocksForFilter = new DefaultHashMap<>(ArrayList::new);
        NovaNative.mc_chunk updateChunk = new NovaNative.mc_chunk();
        long startTime = System.currentTimeMillis();

        Chunk mcChunk = world.getChunkFromBlockCoords(new BlockPos(range.min.x, range.min.y, range.min.z));

        for(int x = range.min.x; x <= range.max.x; x++) {
            for(int y = range.min.y; y < range.max.y; y++) {
                for(int z = range.min.z; z <= range.max.z; z++) {
                    IBlockState blockState = mcChunk.getBlockState(x, y, z);
                    for(Map.Entry<String, IGeometryFilter> entry : filters.entrySet()) {
                        if(entry.getValue().matches(blockState)) {
                            blocksForFilter.get(entry.getKey()).add(new Tuple<>(new BlockPos(x, y, z), blockState));
                        }
                    }
                }
            }
        }

        for(String filterName : blocksForFilter.keySet()) {
            int[] vertexData = makeVertexDataForBlocks(blocksForFilter.get(filterName));
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

    private int[] makeVertexDataForBlocks(List<Tuple<BlockPos, IBlockState>> blockStates) {
        for(Tuple<BlockPos, IBlockState> posBlockState : blockStates) {
            IBlockState blockState = posBlockState.getSecond();
            IBakedModel blockModel = modelManager.getModelForState(blockState);

            List<BakedQuad> quads = new ArrayList<>();
            for(EnumFacing facing : EnumFacing.values()) {
                quads.addAll(blockModel.getQuads(blockState, facing, 0));
            }

            LOG.trace("Retrieved {} faces for BlocKState {}", quads.size(), blockState);
        }


    }

    public void setWorld(World world) {
        this.world = world;
    }
}
