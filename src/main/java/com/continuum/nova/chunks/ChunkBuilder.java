package com.continuum.nova.chunks;

import com.continuum.nova.NovaNative;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.BlockFluidRenderer;
import net.minecraft.client.renderer.BlockRendererDispatcher;
import net.minecraft.client.renderer.block.model.BakedQuad;
import net.minecraft.client.renderer.block.model.IBakedModel;
import net.minecraft.client.renderer.color.BlockColors;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Splits chunks up into meshes with one mesh for each shader
 *
 * @author ddubois
 * @since 27-Jul-17
 */
public class ChunkBuilder {
    private static final Logger LOG = LogManager.getLogger(ChunkBuilder.class);
    private static final int VERTEX_COLOR_OFFSET = 3;
    private static final int LIGHTMAP_COORD_OFFSET = 6;
    private World world;

    private final Map<String, IGeometryFilter> filters;

    private final AtomicLong timeSpentInBlockRenderUpdate = new AtomicLong(0);
    private final AtomicInteger numChunksUpdated = new AtomicInteger(0);

    private final BlockColors blockColors;

    private BlockRendererDispatcher blockRendererDispatcher;

    public ChunkBuilder(Map<String, IGeometryFilter> filters, World world, BlockColors blockColors) {
        this.filters = filters;
        this.world = world;
        this.blockColors = blockColors;
    }

    public void createMeshesForChunk(ChunkUpdateListener.BlockUpdateRange range) {
        blockRendererDispatcher =  Minecraft.getMinecraft().getBlockRenderDispatcher();
        Map<String, List<BlockPos>> blocksForFilter = new HashMap<>();
        long startTime = System.currentTimeMillis();

        for(int x = range.min.x; x <= range.max.x; x++) {
            for(int y = range.min.y; y < range.max.y; y++) {
                for(int z = range.min.z; z <= range.max.z; z++) {
                    filterBlockAtPos(blocksForFilter, new BlockPos(x, y, z));
                }
            }
        }

        final int chunkHashCode = 31 * range.min.x + range.min.z;

        for(String filterName : blocksForFilter.keySet()) {
            Optional<NovaNative.mc_chunk_render_object> renderObj = makeMeshForBlocks(blocksForFilter.get(filterName), world, new BlockPos(range.min.x, range.min.y, range.min.z));
            renderObj.ifPresent(obj -> {
                obj.id = chunkHashCode;
                obj.x = range.min.x;
                obj.y = range.min.y;
                obj.z = range.min.z;

                NovaNative.INSTANCE.add_chunk_geometry_for_filter(filterName, obj);
            });
        }

        long timeAfterBuildingStruct = System.currentTimeMillis();
        // Using JNA in the old way: 550 ms / chunk

        long deltaTime = System.currentTimeMillis() - startTime;
        timeSpentInBlockRenderUpdate.addAndGet(deltaTime);
        numChunksUpdated.incrementAndGet();

        if(numChunksUpdated.get() % 10 == 0) {
            LOG.debug("It's taken an average of {}ms to update {} chunks",
                    (float) timeSpentInBlockRenderUpdate.get() / numChunksUpdated.get(), numChunksUpdated);
            LOG.debug("Detailed stats:\nTime to build chunk: {}ms",
                    timeAfterBuildingStruct - startTime);
        }
    }

    /**
     * Adds the block at the given position to the blocksForFilter map under each filter that matches the block
     *
     * @param blocksForFilter The map of blocks to potentially add the given block to
     * @param pos The position of the block to add
     */
    private void filterBlockAtPos(Map<String, List<BlockPos>> blocksForFilter, BlockPos pos) {
        IBlockState blockState = world.getBlockState(pos);
        if(blockState.getRenderType().equals(EnumBlockRenderType.INVISIBLE)) {
            return;
        }

        for(Map.Entry<String, IGeometryFilter> entry : filters.entrySet()) {
            if(entry.getValue().matches(blockState)) {
                if(!blocksForFilter.containsKey(entry.getKey())) {
                    blocksForFilter.put(entry.getKey(), new ArrayList<>());
                }
                blocksForFilter.get(entry.getKey()).add(pos);
            }
        }
    }

    private Optional<NovaNative.mc_chunk_render_object> makeMeshForBlocks(List<BlockPos> positions, World world, BlockPos chunkPos) {
        List<Integer> vertexData = new ArrayList<>();
        IndexList indices = new IndexList();
        NovaNative.mc_chunk_render_object chunk_render_object = new NovaNative.mc_chunk_render_object();
        CapturingVertexBuffer capturingVertexBuffer = new CapturingVertexBuffer(chunkPos);
        BlockFluidRenderer fluidRenderer = blockRendererDispatcher.getFluidRenderer();

        int blockIndexCounter = 0;
        for(BlockPos blockPos : positions) {
            IBlockState blockState = world.getBlockState(blockPos);

            if(blockState.getRenderType() == EnumBlockRenderType.MODEL) {
                IBakedModel blockModel = blockRendererDispatcher.getModelForState(blockState);
                int colorMultiplier = blockColors.colorMultiplier(blockState, null, null, 0);

                int faceIndexCounter = 0;
                for(EnumFacing facing : EnumFacing.values()) {
                    List<BakedQuad> quads = blockModel.getQuads(blockState, facing, 0);
                    boolean shouldSideBeRendered = blockState.shouldSideBeRendered(world, blockPos, facing);
                    boolean hasQuads = !quads.isEmpty();
                    if(shouldSideBeRendered && hasQuads) {
                        int lmCoords = blockState.getPackedLightmapCoords(world, blockPos.offset(facing));
                        
                        for(BakedQuad quad : quads) {
                            int[] quadVertexData = addPosition(quad, blockPos.subtract(chunkPos));
                            setVertexColor(quadVertexData, colorMultiplier);
                            setLightmapCoord(quadVertexData, lmCoords);

                            for(int data : quadVertexData) {
                                vertexData.add(data);
                            }

                            indices.addIndicesForFace(faceIndexCounter, blockIndexCounter);
                            faceIndexCounter += 4;
                        }
                    }
                }

                // FUCK YOU NULL
                //quads.addAll(blockModel.getQuads(blockState, null, 0));

                //for(BakedQuad quad : quads) {

               // }

                blockIndexCounter += faceIndexCounter;

            } else if(blockState.getRenderType() == EnumBlockRenderType.LIQUID) {
                // Why do liquids have to be different? :(
                fluidRenderer.renderFluid(world, blockState, blockPos, capturingVertexBuffer);
            }
        }

        vertexData.addAll(capturingVertexBuffer.getData());

        int offset = indices.size();
        for(int i = 0; i < capturingVertexBuffer.getVertexCount() / 4; i++) {
            indices.addIndicesForFace(offset, 0);

            offset += 4;
        }

        if(vertexData.isEmpty()) {
            return Optional.empty();
        }

        chunk_render_object.setVertex_data(vertexData);
        chunk_render_object.setIndices(indices);
        chunk_render_object.format = NovaNative.NovaVertexFormat.POS_UV_LIGHTMAPUV_NORMAL_TANGENT.ordinal();

        return Optional.of(chunk_render_object);
    }

    private void setLightmapCoord(int[] quadVertexData, int lmCoords) {
        for(int i = 0; i < 4; i++) {
            quadVertexData[i * 7 + LIGHTMAP_COORD_OFFSET] = lmCoords;
        }
    }

    private void setVertexColor(int[] vertexData, int vertexColor) {
        for(int i = 0; i < 4; i++) {
            vertexData[i * 7 + VERTEX_COLOR_OFFSET] = vertexColor;
        }
    }

    private int[] addPosition(BakedQuad quad, BlockPos blockPos) {
        int[] data = Arrays.copyOf(quad.getVertexData(), quad.getVertexData().length);

        for(int vertex = 0; vertex < 28; vertex += 7) {
            addPosToVertex(blockPos, data, vertex);
        }

        return data;
    }

    private void addPosToVertex(BlockPos blockPos, int[] data, int vertex) {
        float x = Float.intBitsToFloat(data[vertex + 0]);
        float y = Float.intBitsToFloat(data[vertex + 1]);
        float z = Float.intBitsToFloat(data[vertex + 2]);

        x += blockPos.getX();
        y += blockPos.getY();
        z += blockPos.getZ();

        data[vertex + 0] = Float.floatToIntBits(x);
        data[vertex + 1] = Float.floatToIntBits(y);
        data[vertex + 2] = Float.floatToIntBits(z);
    }

    public void setWorld(World world) {
        this.world = world;
    }

    /**
     * Returns the IBlockState of the block next to the block at the provided position in the given direction
     *
     * @param pos The position to get the block next to
     * @param world The world that all these blocks are in
     * @param direction The direction to look in
     * @return The IBlockState of the block in the provided direction
     */
    private static IBlockState blockNextTo(BlockPos pos, World world, EnumFacing direction) {
        BlockPos lookupPos = pos.add(direction.getDirectionVec());
        return world.getBlockState(lookupPos);
    }
}
